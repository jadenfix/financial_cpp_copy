#pragma once

#include "Strategy.h"
#include "core/Event.h"
#include "core/EventQueue.h"
#include "core/Utils.h"
#include "core/Portfolio.h"
#include "data/PriceBar.h"

#include <boost/circular_buffer.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>

#ifndef LOG_TRACE
#define LOG_TRACE(fmt, ...) do{} while(0)
#endif

class OpeningRangeBreakout : public Strategy {
private:
    //――――――――――――――――――――――――――――――――――
    // 1) Magic‐numbers centralized
    static constexpr double EPS                   = 1e-9;
    static constexpr size_t MIN_RANGE_MINUTES     = 1;
    static constexpr double MIN_POSITION_SIZE     = 1e-6;
    static constexpr size_t MIN_VOL_WINDOW        = 1;
    static constexpr double MIN_VOL_MULTIPLIER    = 1e-6;
    static constexpr double DEFAULT_PROFIT_ATR_M  = 2.0;
    static constexpr double DEFAULT_STOP_ATR_M    = 1.0;

    //――――――――――――――――――――――――――――――――――
    // 2) User parameters (validated & clamped)
    const size_t opening_range_minutes_;
    const double target_position_size_;
    const size_t volume_avg_window_;
    const double volume_multiplier_;
    const double profit_target_atr_mult_;
    const double stop_loss_atr_mult_;

    // Derived
    const size_t warmup_bars_;

    //――――――――――――――――――――――――――――――――――
    // 3) Per‐symbol state
    struct SymbolState {
        // Opening range
        std::chrono::system_clock::time_point start_time;
        double range_high = -std::numeric_limits<double>::infinity();
        double range_low  = std::numeric_limits<double>::infinity();
        bool   range_established = false;

        // Volume for filter
        boost::circular_buffer<double> volume_hist;

        // Position management
        bool   position_open    = false;
        double entry_price      = 0.0;
        double trailing_stop    = NAN;
        double profit_target    = NAN;
    };

    std::map<std::string, SymbolState> states_;
    std::mutex                         state_mutex_;

    //――――――――――――――――――――――――――――――――――
    // 4) Helpers

    // Extract just the “time since start” in minutes
    static int minutesSince(const std::chrono::system_clock::time_point& then,
                            const std::chrono::system_clock::time_point& now)
    {
        using namespace std::chrono;
        return duration_cast<minutes>(now - then).count();
    }

    // Reset state at new session
    static void resetState(SymbolState& s,
                           const std::chrono::system_clock::time_point& now)
    {
        s.start_time        = now;
        s.range_high        = -std::numeric_limits<double>::infinity();
        s.range_low         =  std::numeric_limits<double>::infinity();
        s.range_established = false;
        s.volume_hist.clear();
        s.position_open     = false;
        s.entry_price       = 0;
        s.trailing_stop     = NAN;
        s.profit_target     = NAN;
    }

    // Rolling average volume
    static double avgVolume(const boost::circular_buffer<double>& buf) {
        if (buf.empty()) return 0.0;
        double sum = std::accumulate(buf.begin(), buf.end(), 0.0);
        return sum / double(buf.size());
    }

    // True Range for bar
    static double trueRange(const PriceBar& bar, double prev_close) {
        if (prev_close <= 0) return bar.High - bar.Low;
        double a = bar.High - bar.Low;
        double b = std::abs(bar.High - prev_close);
        double c = std::abs(bar.Low  - prev_close);
        return std::max({a,b,c});
    }

public:
    OpeningRangeBreakout(int range_minutes = 30,
                        double target_size   = 3.0,
                        int vol_window       = 10,
                        double vol_mult      = 2.0,
                        double profit_atr_m  = DEFAULT_PROFIT_ATR_M,
                        double stop_atr_m    = DEFAULT_STOP_ATR_M)
      : opening_range_minutes_( std::clamp(range_minutes, int(MIN_RANGE_MINUTES), range_minutes) )
      , target_position_size_( std::max(target_size, MIN_POSITION_SIZE) )
      , volume_avg_window_( std::clamp(vol_window, int(MIN_VOL_WINDOW), vol_window) )
      , volume_multiplier_( std::max(vol_mult, MIN_VOL_MULTIPLIER) )
      , profit_target_atr_mult_( std::max(profit_atr_m, 0.0) )
      , stop_loss_atr_mult_( std::max(stop_atr_m,   0.0) )
      , warmup_bars_( opening_range_minutes_ + 1 )
    {
        if (opening_range_minutes_ <= 0
         || target_position_size_ <= 0
         || volume_avg_window_   <= 0)
        {
            throw std::invalid_argument("Invalid OpeningRangeBreakout parameters");
        }
    }

    void handle_market_event(const MarketEvent& ev,
                             EventQueue& queue) override
    {
        std::lock_guard<std::mutex> lk(state_mutex_);
        if (!portfolio_) return;

        for (auto const& [symbol, bar] : ev.data) {
            auto &st = states_.try_emplace(
                symbol,
                SymbolState{ev.timestamp,
                            -std::numeric_limits<double>::infinity(),
                             std::numeric_limits<double>::infinity(),
                             false,
                             boost::circular_buffer<double>(volume_avg_window_),
                             false, 0, NAN, NAN}
            ).first->second;

            // 1) New session detection: if clock went backwards or >24h, reset
            if ((ev.timestamp < st.start_time) ||
                (minutesSince(st.start_time, ev.timestamp) > 24*60))
            {
                resetState(st, ev.timestamp);
            }

            // 2) Warm-up volume history
            st.volume_hist.push_back(double(bar.Volume));

            // 3) Build opening range
            int mins = minutesSince(st.start_time, ev.timestamp);
            if (!st.range_established && mins <= int(opening_range_minutes_)) {
                st.range_high = std::max(st.range_high, bar.High);
                st.range_low  = std::min(st.range_low,  bar.Low);
                continue;
            }
            if (!st.range_established) {
                st.range_established = true;
                std::cout << "ORB ESTABLISHED: " << symbol << " @ "
                          << formatTimestampUTC(ev.timestamp)
                          << " H=" << st.range_high
                          << " L=" << st.range_low << "\n";
            }

            // 4) On each new bar AFTER range is set:
            double prev_close = st.entry_price>0 ? st.entry_price : bar.Open;
            double tr = trueRange(bar, prev_close);
            double avg_vol = avgVolume(st.volume_hist);
            bool  volume_ok = (avg_vol>EPS && bar.Volume > volume_multiplier_*avg_vol);

            // 5) Entry logic
            if (!st.position_open && volume_ok) {
                SignalDirection want = SignalDirection::FLAT;
                if (bar.Close > st.range_high) want = SignalDirection::LONG;
                if (bar.Close < st.range_low)  want = SignalDirection::SHORT;

                if (want != SignalDirection::FLAT) {
                    // compute ATR for sizing/stop
                    double atr = calculate_atr({bar}, 14);  // or your Utils::calcATR
                    st.profit_target = bar.Close + want==SignalDirection::LONG
                        ? profit_target_atr_mult_*atr
                        : -profit_target_atr_mult_*atr;
                    st.trailing_stop = bar.Close - want==SignalDirection::LONG
                        ? stop_loss_atr_mult_*atr
                        : -stop_loss_atr_mult_*atr;

                    double qty = (want==SignalDirection::LONG? +target_position_size_ : -target_position_size_);
                    double cur = portfolio_->get_position_quantity(symbol);
                    double delta = qty - cur;
                    if (std::abs(delta)>EPS) {
                        send_event(std::make_shared<OrderEvent>(
                                       ev.timestamp, symbol,
                                       OrderType::MARKET,
                                       delta>0?OrderDirection::BUY:OrderDirection::SELL,
                                       std::abs(delta)),
                                   queue);
                        st.position_open = true;
                        st.entry_price   = bar.Close;
                        st.current_signal = want;
                        LOG_TRACE("ORB ENTRY: {} {} at {} qty={}", symbol,
                                  (want==SignalDirection::LONG?"LONG":"SHORT"),
                                  bar.Close, qty);
                    }
                }
            }

            // 6) Exit logic: profit target, trailing stop, or EOD
            if (st.position_open) {
                double pos = portfolio_->get_position_quantity(symbol);
                bool exit = false;
                if (pos>0) {
                    if (bar.High  >= st.profit_target) exit = true;
                    if (bar.Low   <= st.trailing_stop) exit = true;
                } else if (pos<0) {
                    if (bar.Low   <= st.profit_target) exit = true;
                    if (bar.High  >= st.trailing_stop) exit = true;
                }
                // EOD exit: >1h before midnight
                int mins_since = minutesSince(st.start_time, ev.timestamp);
                if (mins_since > (24*60 - 60)) exit = true;

                if (exit) {
                    send_event(std::make_shared<OrderEvent>(
                                   ev.timestamp, symbol,
                                   OrderType::MARKET,
                                   pos>0?OrderDirection::SELL:OrderDirection::BUY,
                                   std::abs(pos)),
                               queue);
                    st.position_open = false;
                    st.current_signal = SignalDirection::FLAT;
                    LOG_TRACE("ORB EXIT: {} pos={} at {}", symbol, pos, bar.Close);
                }
            }
        }
    }

    void handle_fill_event(const FillEvent&, EventQueue&) override {
        // no-op
    }

    std::string get_name() const override {
        return "RobustOpeningRangeBreakout";
    }
};