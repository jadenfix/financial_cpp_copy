#pragma once

#include "Strategy.h"
#include "core/Event.h"
#include "core/EventQueue.h"
#include "core/Utils.h"       // for formatTimestampUTC, etc.
#include "core/Portfolio.h"
#include "data/PriceBar.h"

// #include <boost/circular_buffer.hpp>  // Using our own circular_buffer from Utils.h
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <mutex>
#include <numeric>
#include <stdexcept>
#include <string>

#ifndef LOG_TRACE
#define LOG_TRACE(fmt, ...) do{} while(0)
#endif

class MovingAverageCrossover : public Strategy {
private:
    //――――――――――――――――――――――――――――――
    // 1) All “magic numbers” centralized
    static constexpr double EPS                 = 1e-8;
    static constexpr size_t MIN_PERIOD          = 1;
    static constexpr size_t MAX_PERIOD          = 100000;
    static constexpr double MIN_BASE_POS        = 1e-6;
    static constexpr double MAX_BASE_POS        = 1e6;
    static constexpr double DEFAULT_ATR         = 0.01;
    static constexpr double DEFAULT_VOL         = 0.02;

    //――――――――――――――――――――――――――――――
    // 2) Strategy parameters (validated & clamped)
    const size_t fast_period_;
    const size_t slow_period_;
    const double base_position_size_;
    const double max_position_value_;

    // lookback windows (derived)
    const size_t atr_period_;
    const size_t volatility_lookback_;
    const size_t regime_lookback_;
    const size_t warmup_bars_;

    //――――――――――――――――――――――――――――――
    // 3) Per‐symbol rolling state
    struct SymbolState {
        circular_buffer<double> prices;
        circular_buffer<double> volumes;
        circular_buffer<double> true_ranges;
        circular_buffer<double> returns;       // for volatility
        SignalDirection current_signal = SignalDirection::FLAT;

        SymbolState(size_t p, size_t v, size_t tr, size_t r) noexcept
          : prices(p), volumes(v), true_ranges(tr), returns(r) {}
    };
    std::map<std::string, SymbolState> states_;
    mutable std::mutex state_mutex_;

    //――――――――――――――――――――――――――――――
    // 4) Static helpers

    // 4a) Simple moving average over last N of buf
    static double calcSMA(const circular_buffer<double>& buf, size_t period) {
        if (buf.size() < period) return 0.0;
        auto start_it = buf.last(period);
        auto end_it = buf.end();
        return std::accumulate(start_it, end_it, 0.0) / double(period);
    }

    // 4b) Average True Range
    static double calcATR(const circular_buffer<double>& tr, size_t period) {
        if (tr.size() < period) return DEFAULT_ATR;
        auto start_it = tr.last(period);
        auto end_it = tr.end();
        return std::accumulate(start_it, end_it, 0.0) / double(period);
    }

    // 4c) Rolling volatility (sample std dev of returns)
    static double calcVol(const circular_buffer<double>& rets) {
        size_t n = rets.size();
        if (n < 2) return DEFAULT_VOL;
        double mean = std::accumulate(rets.begin(), rets.end(), 0.0) / double(n);
        double var = 0.0;
        for (double r : rets) var += (r - mean)*(r - mean);
        return std::sqrt(var / double(n-1));
    }

    // 4d) Trend strength via normalized slope of a linear regression
    static double calcTrend(const circular_buffer<double>& buf, size_t lookback) {
        size_t n = std::min(lookback, buf.size());
        if (n < 2) return 0.0;
        double sum_x=0, sum_y=0, sum_xy=0, sum_x2=0;
        auto it = buf.last(n);
        for (size_t i = 0; i < n; ++i, ++it) {
            double x = double(i), y = *it;
            sum_x  += x;
            sum_y  += y;
            sum_xy += x*y;
            sum_x2 += x*x;
        }
        double num = n*sum_xy - sum_x*sum_y;
        double den = n*sum_x2 - sum_x*sum_x;
        if (std::abs(den) < EPS) return 0.0;
        double slope = num/den;
        double avg_y = sum_y/double(n);
        return (avg_y > EPS ? std::abs(slope)/avg_y : 0.0);
    }

    // 4e) True range for bar
    static double calcTrueRange(const PriceBar& bar, double prev_close) {
        if (prev_close <= 0) return bar.High - bar.Low;
        double a = bar.High - bar.Low;
        double b = std::abs(bar.High - prev_close);
        double c = std::abs(bar.Low  - prev_close);
        return std::max({a,b,c});
    }

    //――――――――――――――――――――――――――――――
    // 5) Adaptive position sizing
    double getAdaptiveSize(double price, const SymbolState& st) const {
        double atr      = calcATR(st.true_ranges, atr_period_);
        double vol      = calcVol(st.returns);
        double trend    = calcTrend(st.prices, regime_lookback_);

        // volatility shrinkage
        double v_adj = std::clamp(1.0 - vol*5.0, 0.1, 2.0);
        // trend boost
        double t_adj = 1.0 + trend*0.5;

        double size = base_position_size_ * v_adj * t_adj;
        if (atr > EPS) {
            // risk 1% of price per ATR
            size = std::min(size, (price*0.01)/atr);
        }
        // cap by max position value
        return std::min(size, max_position_value_/price);
    }

public:
    MovingAverageCrossover(size_t fast_period,
                           size_t slow_period,
                           double base_position_size,
                           double max_position_value = 5000.0,
                           size_t atr_period         = 14,
                           size_t vol_lookback       = 20,
                           size_t regime_lookback    = 50)
      : fast_period_( std::clamp(fast_period, MIN_PERIOD, MAX_PERIOD) )
      , slow_period_( std::max(std::clamp(slow_period, MIN_PERIOD, MAX_PERIOD),
                               fast_period_+1) )
      , base_position_size_( std::clamp(base_position_size, MIN_BASE_POS, MAX_BASE_POS) )
      , max_position_value_( max_position_value )
      , atr_period_( atr_period )
      , volatility_lookback_( vol_lookback )
      , regime_lookback_( regime_lookback )
      , warmup_bars_( std::max({slow_period_,
                                atr_period_+1,
                                volatility_lookback_+1,
                                regime_lookback_}) )
    {
        if (fast_period_ >= slow_period_
         || base_position_size_ <= 0
         || max_position_value_ <= 0)
        {
            throw std::invalid_argument("Invalid MovingAverageCrossover parameters");
        }
    }

    void handle_market_event(const MarketEvent& ev, EventQueue& queue) override {
        std::lock_guard<std::mutex> lk(state_mutex_);
        if (!portfolio_) return;

        for (auto const& [symbol, bar] : ev.data) {
            // 6) init per‐symbol state on first use
            auto it = states_.find(symbol);
            if (it == states_.end()) {
                it = states_.emplace(
                    symbol,
                    SymbolState(
                      slow_period_,
                      volatility_lookback_+1,
                      atr_period_+1,
                      volatility_lookback_+1
                    )
                ).first;
            }
            auto &st = it->second;

            // 7) update rolling buffers
            st.prices.push_back(bar.Close);
            st.volumes.push_back(double(bar.Volume));

            // true range needs prev-close
            double prev_close = (st.prices.size()>1
                                 ? *(st.prices.end()-2)
                                 : 0.0);
            st.true_ranges.push_back(calcTrueRange(bar, prev_close));

            // simple return for volatility
            if (prev_close > EPS) {
                st.returns.push_back((bar.Close/prev_close)-1.0);
            }

            // 8) warm-up check
            if (st.prices.size() < warmup_bars_) continue;

            // 9) compute SMAs
            double fast_sma = calcSMA(st.prices, fast_period_);
            double slow_sma = calcSMA(st.prices, slow_period_);

            // 10) decide signal
            SignalDirection want = SignalDirection::FLAT;
            if (fast_sma > slow_sma + EPS)       want = SignalDirection::LONG;
            else if (fast_sma + EPS < slow_sma)  want = SignalDirection::SHORT;

            // 11) on signal change, size & send order
            if (want != st.current_signal) {
                LOG_TRACE("MAC: {} fast={} slow={} -> {}",
                          symbol, fast_sma, slow_sma,
                          (want==SignalDirection::LONG?"LONG":
                           want==SignalDirection::SHORT?"SHORT":"FLAT"));

                double target_qty = 0.0;
                if (want == SignalDirection::LONG)
                    target_qty = getAdaptiveSize(bar.Close, st);
                else if (want == SignalDirection::SHORT)
                    target_qty = -getAdaptiveSize(bar.Close, st);

                double cur_qty = portfolio_->get_position_quantity(symbol);
                double delta   = target_qty - cur_qty;
                if (std::abs(delta) > EPS) {
                    auto dir = delta>0
                               ? OrderDirection::BUY
                               : OrderDirection::SELL;
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
        // No strategy-specific fill logic
    }

    std::string get_name() const override {
        return "RobustMACrossover_" +
               std::to_string(fast_period_) + "_" +
               std::to_string(slow_period_);
    }
};