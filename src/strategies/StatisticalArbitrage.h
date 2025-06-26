#pragma once

#include "Strategy.h"
#include "core/Event.h"
#include "core/EventQueue.h"
#include "core/Utils.h"      // for calculate_kelly_position_size, calculate_volatility_adjusted_shares, formatTimestampUTC
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

class StatisticalArbitrage : public Strategy {
private:
    //――――――――――――――――――――――――――――――――――
    // 1) Magic constants
    static constexpr double EPS             = 1e-12;
    static constexpr size_t MIN_LOOKBACK    = 5;
    static constexpr double MIN_Z_ENTRY     = 0.1;
    static constexpr double MIN_Z_EXIT      = 0.0;
    static constexpr double MIN_RISK        = 0.0;

    //――――――――――――――――――――――――――――――――――
    // 2) User parameters (validated & clamped)
    const std::string primary_symbol_;
    const std::string hedge_symbol_;
    const size_t      lookback_window_;        // N
    const double      entry_zscore_threshold_; // to enter
    const double      exit_zscore_threshold_;  // to exit
    const double      max_position_risk_;      // fraction of equity

    //――――――――――――――――――――――――――――――――――
    // 3) Rolling buffers & running sums for O(1) stats
    boost::circular_buffer<double> buf_primary_;      // price A
    boost::circular_buffer<double> buf_hedge_;        // price B
    boost::circular_buffer<double> buf_spread_;       // spread = A - β*B

    // Running sums for regression: y=A, x=B
    double sum_x_   = 0.0;  // Σ B
    double sum_y_   = 0.0;  // Σ A
    double sum_xy_  = 0.0;  // Σ A·B
    double sum_x2_  = 0.0;  // Σ B²

    // Running sums for spread z-score
    double sum_s_   = 0.0;  // Σ spread
    double sum_s2_  = 0.0;  // Σ spread²

    enum class PairSignal { FLAT, LONG_SPREAD, SHORT_SPREAD };
    PairSignal          current_signal_ = PairSignal::FLAT;

    mutable std::mutex  mtx_;  // guard all state

public:
    //――――――――――――――――――――――――――――――――――
    StatisticalArbitrage(std::string a,
                        std::string b,
                        int lookback=100,
                        double z_entry=2.0,
                        double z_exit=0.5,
                        double max_risk=0.04)
      : primary_symbol_(std::move(a))
      , hedge_symbol_(std::move(b))
      , lookback_window_(std::clamp<size_t>(lookback, MIN_LOOKBACK, lookback))
      , entry_zscore_threshold_(std::max(z_entry, MIN_Z_ENTRY))
      , exit_zscore_threshold_(std::max(z_exit,  MIN_Z_EXIT))
      , max_position_risk_(std::max(max_risk, MIN_RISK))
      , buf_primary_(lookback_window_)
      , buf_hedge_(lookback_window_)
      , buf_spread_(lookback_window_)
    {
        if (primary_symbol_.empty() || hedge_symbol_.empty()
         || primary_symbol_==hedge_symbol_) {
            throw std::invalid_argument("Symbols must be distinct and non-empty");
        }
        if (entry_zscore_threshold_ <= exit_zscore_threshold_) {
            throw std::invalid_argument("entry_z must be > exit_z");
        }
    }

    //――――――――――――――――――――――――――――――――――
    void handle_market_event(const MarketEvent& ev, EventQueue& queue) override {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!portfolio_) return;

        // 1) fetch bars
        auto ita = ev.data.find(primary_symbol_);
        auto itb = ev.data.find(hedge_symbol_);
        if (ita==ev.data.end()||itb==ev.data.end()) return;
        double pa = ita->second.Close;
        double pb = itb->second.Close;
        if (pa<=EPS || pb<=EPS) return;

        // 2) update rolling OLS sums for β = cov(A,B)/var(B)
        if (buf_primary_.full()) {
            double oldA = buf_primary_.front();
            double oldB = buf_hedge_.front();
            sum_y_   -= oldA;
            sum_x_   -= oldB;
            sum_xy_  -= oldA*oldB;
            sum_x2_  -= oldB*oldB;
        }
        buf_primary_.push_back(pa);
        buf_hedge_.push_back(pb);
        sum_y_  += pa;
        sum_x_  += pb;
        sum_xy_ += pa*pb;
        sum_x2_ += pb*pb;

        // 3) need warm-up
        size_t N = buf_primary_.size();
        if (N < lookback_window_) return;

        // 4) compute hedge β
        double denom = (N*sum_x2_ - sum_x_*sum_x_);
        double beta  = (std::abs(denom)>EPS)
                       ? (N*sum_xy_ - sum_x_*sum_y_) / denom
                       : 1.0;
        beta = std::abs(beta);

        // 5) compute spread and update rolling stats
        double spread = pa - beta*pb;
        if (buf_spread_.full()) {
            double oldS = buf_spread_.front();
            sum_s_  -= oldS;
            sum_s2_ -= oldS*oldS;
        }
        buf_spread_.push_back(spread);
        sum_s_  += spread;
        sum_s2_ += spread*spread;

        // 6) compute mean & σ of spread
        double mean = sum_s_ / double(N);
        double var  = (sum_s2_ - (sum_s_*sum_s_)/double(N)) / double(N-1);
        if (var <= 0) return;
        double sigma = std::sqrt(var);

        // 7) z-score
        double z = (spread - mean) / sigma;

        // 8) decide new signal
        PairSignal desired = current_signal_;
        if (current_signal_==PairSignal::FLAT) {
            if (z >  entry_zscore_threshold_) desired = PairSignal::SHORT_SPREAD;
            if (z < -entry_zscore_threshold_) desired = PairSignal::LONG_SPREAD;
        } else if (current_signal_==PairSignal::SHORT_SPREAD) {
            if (z < exit_zscore_threshold_) desired = PairSignal::FLAT;
        } else {
            if (z > -exit_zscore_threshold_) desired = PairSignal::FLAT;
        }

        // 9) if changed, size & send orders
        if (desired != current_signal_) {
            LOG_TRACE(
              "STAT_ARB: {}|{}  spread={:.4f} mean={:.4f} σ={:.4f} z={:.4f} -> {}",
              primary_symbol_, hedge_symbol_,
              spread, mean, sigma, z,
              (desired==PairSignal::LONG_SPREAD?  "LONG_SPREAD":
               desired==PairSignal::SHORT_SPREAD? "SHORT_SPREAD": "FLAT")
            );

            // Kelly‐based $risk
            double kelly = calculate_kelly_position_size(
                              /*win_rate*/0.5, /*avg_win*/0.01, /*avg_loss*/-0.01,
                              max_position_risk_);
            double eq = portfolio_->get_total_equity();
            double dollar_risk = eq * kelly;

            // size by spread σ in price terms
            double unit = std::max(sigma, EPS);
            double shar = dollar_risk / unit;    // total spread units

            // allocate A and B legs
            double qa=0, qb=0;
            if (desired==PairSignal::LONG_SPREAD) {   // long A, short B
                qa = +shar;  qb = -shar*beta;
            } else if (desired==PairSignal::SHORT_SPREAD) {
                qa = -shar;  qb = +shar*beta;
            }

            // convert to shares
            double targetA = qa;
            double targetB = qb;
            double curA = portfolio_->get_position_quantity(primary_symbol_);
            double curB = portfolio_->get_position_quantity(hedge_symbol_);
            double dA = targetA - curA;
            double dB = targetB - curB;

            if (std::abs(dA)>EPS) {
                send_event(std::make_shared<OrderEvent>(
                  ev.timestamp, primary_symbol_,
                  OrderType::MARKET,
                  dA>0?OrderDirection::BUY:OrderDirection::SELL,
                  std::abs(dA)
                ), queue);
            }
            if (std::abs(dB)>EPS) {
                send_event(std::make_shared<OrderEvent>(
                  ev.timestamp, hedge_symbol_,
                  OrderType::MARKET,
                  dB>0?OrderDirection::BUY:OrderDirection::SELL,
                  std::abs(dB)
                ), queue);
            }

            current_signal_ = desired;
        }
    }

    void handle_fill_event(const FillEvent&, EventQueue&) override {
        // no-op
    }

    std::string get_name() const override {
        return "CitadelStatArb_" + primary_symbol_ + "_" + hedge_symbol_;
    }
};