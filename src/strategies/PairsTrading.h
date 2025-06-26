#pragma once

#include "Strategy.h"
#include "core/Event.h"
#include "core/EventQueue.h"
#include "core/Utils.h"    // for formatTimestampUTC
#include "core/Portfolio.h"
#include "data/PriceBar.h"

#include <boost/circular_buffer.hpp>
#include <algorithm>
#include <cmath>
#include <mutex>
#include <numeric>
#include <stdexcept>
#include <string>

#ifndef LOG_TRACE
#define LOG_TRACE(fmt, ...) do{} while(0)
#endif

class PairsTrading : public Strategy {
private:
    //――――――――――――――――――――――――――――――――――
    // 1) Centralize all “magic” constants
    static constexpr double EPS                     = 1e-12;
    static constexpr size_t MIN_LOOKBACK            = 2;
    static constexpr double MIN_Z_ENTRY             = 0.0;
    static constexpr double MIN_Z_EXIT              = 0.0;
    static constexpr double MIN_TRADE_VALUE         = 1e-6;

    //――――――――――――――――――――――――――――――――――
    // 2) Constructor arguments (validated and clamped)
    const std::string symbol_a_;
    const std::string symbol_b_;
    const size_t      lookback_window_;       // N
    const double      entry_zscore_threshold_; // Z-entry
    const double      exit_zscore_threshold_;  // Z-exit
    const double      trade_value_;            // $ per leg

    //――――――――――――――――――――――――――――――――――
    // 3) Runtime state
    boost::circular_buffer<double> ratio_hist_; // size = lookback_window_
    double sum_ratio_    = 0.0;                 // Σ ratio
    double sum_ratio_sq_ = 0.0;                 // Σ ratio²

    enum class PairSignal { FLAT, LONG_A_SHORT_B, SHORT_A_LONG_B };
    PairSignal      current_signal_ = PairSignal::FLAT;

    mutable std::mutex mtx_;  // guard all state

public:
    //――――――――――――――――――――――――――――――――――
    PairsTrading(std::string a,
                 std::string b,
                 size_t lookback = 30,
                 double z_entry  = 2.0,
                 double z_exit   = 0.5,
                 double trade_val= 10000.0)
      : symbol_a_(std::move(a))
      , symbol_b_(std::move(b))
      , lookback_window_(std::clamp(lookback, MIN_LOOKBACK, lookback))
      , entry_zscore_threshold_(std::max(z_entry, MIN_Z_ENTRY))
      , exit_zscore_threshold_(std::max(z_exit,  MIN_Z_EXIT))
      , trade_value_(std::max(trade_val, MIN_TRADE_VALUE))
      , ratio_hist_(lookback_window_)
    {
        if (symbol_a_.empty() || symbol_b_.empty() || symbol_a_ == symbol_b_) {
            throw std::invalid_argument("PairsTrading: symbols must be non-empty and distinct");
        }
        if (entry_zscore_threshold_ <= exit_zscore_threshold_) {
            throw std::invalid_argument("PairsTrading: entry_z must be > exit_z");
        }
    }

    //――――――――――――――――――――――――――――――――――
    void handle_market_event(const MarketEvent& ev, EventQueue& queue) override {
        std::lock_guard lock(mtx_);
        if (!portfolio_) return;

        // Fetch both bars
        auto ita = ev.data.find(symbol_a_);
        auto itb = ev.data.find(symbol_b_);
        if (ita == ev.data.end() || itb == ev.data.end()) return;

        const PriceBar& ba = ita->second;
        const PriceBar& bb = itb->second;
        double pa = ba.Close, pb = bb.Close;
        if (pa <= EPS || pb <= EPS) return;  // guard zero

        // 1) Incremental ratio stats
        double ratio = pa / pb;
        if (ratio_hist_.full()) {
            double old = ratio_hist_.front();
            sum_ratio_    -= old;
            sum_ratio_sq_ -= old*old;
        }
        ratio_hist_.push_back(ratio);
        sum_ratio_    += ratio;
        sum_ratio_sq_ += ratio*ratio;

        // 2) Warm-up
        if (ratio_hist_.size() < lookback_window_) return;

        // 3) Compute mean & stddev in O(1)
        size_t N = ratio_hist_.size();
        double mean = sum_ratio_ / double(N);
        double var  = (sum_ratio_sq_ - (sum_ratio_*sum_ratio_)/double(N)) / double(N-1);
        if (var <= 0) return;
        double stddev = std::sqrt(var);

        // 4) Z-score
        double z = (ratio - mean) / stddev;

        // 5) Determine desired new signal
        PairSignal desired = current_signal_;
        if (current_signal_ == PairSignal::FLAT) {
            if (z >  entry_zscore_threshold_) desired = PairSignal::SHORT_A_LONG_B;
            if (z < -entry_zscore_threshold_) desired = PairSignal::LONG_A_SHORT_B;
        } else if (current_signal_ == PairSignal::SHORT_A_LONG_B) {
            if (z < exit_zscore_threshold_) desired = PairSignal::FLAT;
        } else { /* LONG_A_SHORT_B */
            if (z > -exit_zscore_threshold_) desired = PairSignal::FLAT;
        }

        // 6) If signal changed, emit balanced orders
        if (desired != current_signal_) {
            LOG_TRACE(
                "PAIRS: {} / {}  ratio={:.4f} mean={:.4f} std={:.4f} z={:.4f} -> {}",
                symbol_a_, symbol_b_,
                ratio, mean, stddev, z,
                (desired==PairSignal::LONG_A_SHORT_B? "LONG_A_SHORT_B":
                 desired==PairSignal::SHORT_A_LONG_B? "SHORT_A_LONG_B": "FLAT")
            );

            // Compute target quantities
            double qa = trade_value_ / pa;
            double qb = trade_value_ / pb;
            double target_a = 0.0, target_b = 0.0;

            switch (desired) {
                case PairSignal::LONG_A_SHORT_B:
                    target_a = +qa;
                    target_b = -qb;
                    break;
                case PairSignal::SHORT_A_LONG_B:
                    target_a = -qa;
                    target_b = +qb;
                    break;
                case PairSignal::FLAT:
                    // both zero
                    break;
            }

            // Current positions
            double cur_a = portfolio_->get_position_quantity(symbol_a_);
            double cur_b = portfolio_->get_position_quantity(symbol_b_);
            double da = target_a - cur_a;
            double db = target_b - cur_b;

            // Send leg A
            if (std::abs(da) > EPS) {
                send_event(std::make_shared<OrderEvent>(
                    ev.timestamp,
                    symbol_a_,
                    OrderType::MARKET,
                    da>0 ? OrderDirection::BUY : OrderDirection::SELL,
                    std::abs(da)
                ), queue);
            }

            // Send leg B
            if (std::abs(db) > EPS) {
                send_event(std::make_shared<OrderEvent>(
                    ev.timestamp,
                    symbol_b_,
                    OrderType::MARKET,
                    db>0 ? OrderDirection::BUY : OrderDirection::SELL,
                    std::abs(db)
                ), queue);
            }

            current_signal_ = desired;
        }
    }

    // No special fill logic
    void handle_fill_event(const FillEvent&, EventQueue&) override {}

    std::string get_name() const override {
        return "CitadelPairsTrading_" + symbol_a_ + "_" + symbol_b_;
    }
};