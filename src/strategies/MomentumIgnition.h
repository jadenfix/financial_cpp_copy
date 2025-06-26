#pragma once

#include "Strategy.h"
#include "core/Event.h"
#include "core/EventQueue.h"
#include "core/Utils.h"
#include "core/Portfolio.h"
#include "data/PriceBar.h"

#include <boost/circular_buffer.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>

#ifndef LOG_TRACE
#define LOG_TRACE(fmt, ...) do{} while(0)
#endif

class MomentumIgnition : public Strategy {
private:
    //――――――――――――――――――――――――――――――――――
    // 1) All your “magic numbers” here
    static constexpr double EPS                    = 1e-12;
    static constexpr size_t MIN_PRICE_WINDOW       = 1;
    static constexpr size_t MIN_VOLUME_WINDOW      = 1;
    static constexpr size_t MIN_RET_DELTA_WINDOW   = 1;
    static constexpr double MIN_VOL_MULTIPLIER     = 1e-6;
    static constexpr double MIN_TARGET_POSITION    = 1e-6;

    //――――――――――――――――――――――――――――――――――
    // 2) User parameters (validated & clamped)
    const size_t price_breakout_window_;
    const size_t volume_avg_window_;
    const double volume_multiplier_;
    const size_t return_delta_window_;
    const double target_position_size_;
    const size_t warmup_bars_;   // = max(window)+1

    //――――――――――――――――――――――――――――――――――
    // 3) Per-symbol state with fixed-size ring buffer
    struct SymbolState {
        boost::circular_buffer<PriceBar> hist;
        SignalDirection current_signal = SignalDirection::FLAT;

        SymbolState(size_t max_hist) : hist(max_hist) {}
    };
    std::map<std::string, SymbolState> states_;

    //――――――――――――――――――――――――――――――――――
    // 4) Helpers for each condition

    // 4a) price breakout
    static void calcHighLow(const boost::circular_buffer<PriceBar>& H,
                            size_t window,
                            double& outHigh,
                            double& outLow)
    {
        auto start = H.end() - 1 - window;
        auto finish = H.end() - 1; // exclude current bar
        auto hi = std::max_element(start, finish,
                                   [](auto &a, auto &b){ return a.High < b.High; });
        auto lo = std::min_element(start, finish,
                                   [](auto &a, auto &b){ return a.Low  < b.Low;  });
        outHigh = hi->High;
        outLow  = lo->Low;
    }

    // 4b) average volume
    static double calcAvgVolume(const boost::circular_buffer<PriceBar>& H,
                                size_t window)
    {
        auto start = H.end() - 1 - window;
        auto finish = H.end() - 1;
        double sum = std::accumulate(start, finish, 0.0,
            [](double acc, auto &bar){ return acc + double(bar.Volume); });
        return sum / window;
    }

    // 4c) sum of return deltas
    static double calcReturnDelta(const boost::circular_buffer<PriceBar>& H,
                                  size_t window)
    {
        double sum = 0.0;
        auto it_end = H.end() - 1;
        auto it_begin = it_end - window;
        for (auto it = it_begin; it != it_end; ++it) {
            double prev = std::prev(it)->Close;
            if (prev > EPS) {
                sum += (it->Close / prev) - 1.0;
            }
        }
        return sum;
    }

public:
    MomentumIgnition(size_t price_window   = 5,
                     size_t vol_window     = 10,
                     double vol_mult       = 2.0,
                     size_t ret_window     = 3,
                     double target_pos_sz  = 3.0)
      : price_breakout_window_( std::clamp(price_window, MIN_PRICE_WINDOW, price_window) )
      , volume_avg_window_(    std::clamp(vol_window,   MIN_VOLUME_WINDOW,  vol_window)   )
      , volume_multiplier_(    std::max(vol_mult,       MIN_VOL_MULTIPLIER)             )
      , return_delta_window_(  std::clamp(ret_window,   MIN_RET_DELTA_WINDOW, ret_window)   )
      , target_position_size_( std::max(target_pos_sz,  MIN_TARGET_POSITION)            )
      , warmup_bars_(
          1 + std::max({price_breakout_window_,
                        volume_avg_window_,
                        return_delta_window_})
        )
    {
        if (price_window==0 || vol_window==0 || vol_mult<=0
         || ret_window==0 || target_pos_sz<=0)
        {
            throw std::invalid_argument("MomentumIgnition: invalid constructor args");
        }
    }

    void handle_market_event(const MarketEvent& ev,
                             EventQueue& queue) override
    {
        if (!portfolio_) return;

        for (auto const& [symbol, bar] : ev.data) {
            // get-or-create
            auto it = states_.find(symbol);
            if (it == states_.end()) {
                it = states_.emplace(
                    symbol,
                    SymbolState(warmup_bars_)
                ).first;
            }
            auto &st = it->second;

            // 5) rolling history
            st.hist.push_back(bar);
            if (st.hist.size() < warmup_bars_) continue;  // warm-up

            // 6) compute conditions
            double high, low;
            calcHighLow(  st.hist, price_breakout_window_, high, low);
            bool breakout_up   = (bar.Close > high);
            bool breakout_down = (bar.Close < low);

            double avg_vol = calcAvgVolume(st.hist, volume_avg_window_);
            bool   vol_surge= (avg_vol> EPS && bar.Volume > volume_multiplier_ * avg_vol);

            double ret_delta = calcReturnDelta(st.hist, return_delta_window_);
            bool   pos_delta = (ret_delta>0);
            bool   neg_delta = (ret_delta<0);

            // 7) decide signal
            SignalDirection want = SignalDirection::FLAT;
            if (breakout_up && vol_surge && pos_delta)   want = SignalDirection::LONG;
            if (breakout_down && vol_surge && neg_delta) want = SignalDirection::SHORT;

            // 8) emit orders if changed
            if (want != st.current_signal) {
                LOG_TRACE("MI: {} up={} down={} vsz={} rd={} -> {}",
                          symbol,
                          breakout_up, breakout_down,
                          volume_multiplier_, ret_delta,
                          (want==SignalDirection::LONG?"LONG":
                           want==SignalDirection::SHORT?"SHORT":"FLAT"));

                double target_qty = (want==SignalDirection::LONG
                                      ? +target_position_size_
                                      : want==SignalDirection::SHORT
                                        ? -target_position_size_
                                        : 0.0);

                double cur_qty = portfolio_->get_position_quantity(symbol);
                double delta   = target_qty - cur_qty;

                if (std::abs(delta) > EPS) {
                    auto dir = (delta>0
                                ? OrderDirection::BUY
                                : OrderDirection::SELL);
                    send_event(std::make_shared<OrderEvent>(
                                   ev.timestamp,
                                   symbol,
                                   OrderType::MARKET,
                                   dir,
                                   std::abs(delta)),
                               queue);
                }
                st.current_signal = want;
            }
        }
    }

    void handle_fill_event(const FillEvent&, EventQueue&) override {
        // no state updates on fills
    }
};