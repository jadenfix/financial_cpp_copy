#pragma once

#include "Strategy.h"
#include "core/Event.h"
#include "core/EventQueue.h"
#include "core/Utils.h"
#include "core/Portfolio.h"

#include <boost/circular_buffer.hpp>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

// no-op logger—swap out for your real logging later
#ifndef LOG_TRACE
#define LOG_TRACE(fmt, ...) do{} while(0)
#endif

class LeadLagStrategy : public Strategy {
private:
    //――――――――――――――――――――――――――――――――――――――
    // 1) “Magic-numbers” in one place
    static constexpr double EPS                    = 1e-12;
    static constexpr size_t MIN_CORR_WINDOW        = 2;
    static constexpr size_t MIN_LAG                = 1;
    static constexpr double MIN_CORR_THRESH        = 0.0;
    static constexpr double MAX_CORR_THRESH        = 1.0;
    static constexpr double MIN_RET_THRESH         = 0.0;
    static constexpr double MIN_TARGET_SIZE        = 1e-9;

    //――――――――――――――――――――――――――――――――――――――
    // 2) User parameters (validated & clamped)
    const std::string leading_symbol_;
    const std::string lagging_symbol_;
    const size_t      correlation_window_;
    const size_t      lag_period_;
    const double      correlation_threshold_;
    const double      leader_return_threshold_;
    const double      target_position_size_;

    //――――――――――――――――――――――――――――――――――――――
    // 3) State: rolling return history + O(1) corr-buffers
    boost::circular_buffer<std::pair<double,double>> ret_hist_;
    boost::circular_buffer<double>                   corr_x_, corr_y_;

    // running sums for Pearson correlation
    double sum_x_   = 0.0;
    double sum_y_   = 0.0;
    double sum_xy_  = 0.0;
    double sum_x2_  = 0.0;
    double sum_y2_  = 0.0;

    // last closes
    double last_leader_close_  = NAN;
    double last_lagging_close_ = NAN;
    bool   has_last_leader_    = false;
    bool   has_last_lagging_   = false;

    // current signal
    SignalDirection current_signal_ = SignalDirection::FLAT;

    //――――――――――――――――――――――――――――――――――――――
    // 4) Rolling‐correlation updater
    void updateRollingCorrelation(double x, double y) {
        // remove oldest if full
        if (corr_x_.full()) {
            double ox = corr_x_.front();
            double oy = corr_y_.front();
            corr_x_.pop_front();
            corr_y_.pop_front();
            sum_x_  -= ox;
            sum_y_  -= oy;
            sum_xy_ -= ox*oy;
            sum_x2_ -= ox*ox;
            sum_y2_ -= oy*oy;
        }

        // add new
        corr_x_.push_back(x);
        corr_y_.push_back(y);
        sum_x_  += x;
        sum_y_  += y;
        sum_xy_ += x*y;
        sum_x2_ += x*x;
        sum_y2_ += y*y;
    }

    double computeCorrelation() const {
        const size_t N = corr_x_.size();
        if (N < 2) return 0.0;

        // Pearson: (N·Σxy – Σx·Σy) / sqrt((N·Σx²–Σx²)(N·Σy²–Σy²))
        double num = double(N)*sum_xy_ - sum_x_*sum_y_;
        double den_x = double(N)*sum_x2_ - sum_x_*sum_x_;
        double den_y = double(N)*sum_y2_ - sum_y_*sum_y_;
        if (den_x <= 0 || den_y <= 0) return 0.0;
        return num / std::sqrt(den_x * den_y);
    }

public:
    LeadLagStrategy(std::string leader,
                    std::string lagger,
                    size_t corr_window = 30,
                    size_t lag          = 1,
                    double corr_thresh  = 0.6,
                    double lead_ret_thr = 0.0005,
                    double tgt_size     = 100.0)
      : leading_symbol_(std::move(leader))
      , lagging_symbol_(std::move(lagger))
      , correlation_window_(std::clamp(corr_window, MIN_CORR_WINDOW, corr_window))
      , lag_period_(std::clamp(lag,       MIN_LAG,       corr_window-1))
      , correlation_threshold_(std::clamp(corr_thresh, MIN_CORR_THRESH, MAX_CORR_THRESH))
      , leader_return_threshold_(std::max(lead_ret_thr, MIN_RET_THRESH))
      , target_position_size_(std::max(tgt_size, MIN_TARGET_SIZE))
      , ret_hist_(corr_window + lag)
      , corr_x_(corr_window)
      , corr_y_(corr_window)
    {
        if (leading_symbol_ == lagging_symbol_)
            throw std::invalid_argument("Leader and lagger must differ");
    }

    void handle_market_event(const MarketEvent& ev, EventQueue& queue) override {
        if (!portfolio_) return;

        // grab bars
        auto itL = ev.data.find(leading_symbol_);
        auto itG = ev.data.find(lagging_symbol_);
        if (itL==ev.data.end() || itG==ev.data.end()) return;

        const auto& lb = itL->second;
        const auto& lg = itG->second;
        const double   lc = lb.Close;
        const double   gc = lg.Close;

        // compute leader return
        double leader_ret = 0.0;
        if (has_last_leader_ && last_leader_close_>EPS)
            leader_ret = (lc/last_leader_close_) - 1.0;
        last_leader_close_ = lc;
        has_last_leader_   = true;

        // compute lagger return
        double lagger_ret = 0.0;
        if (has_last_lagging_ && last_lagging_close_>EPS)
            lagger_ret = (gc/last_lagging_close_) - 1.0;
        last_lagging_close_ = gc;
        has_last_lagging_   = true;

        // only push once both returns valid
        if (has_last_leader_ && has_last_lagging_) {
            ret_hist_.push_back({leader_ret, lagger_ret});
        }

        // need full history
        if (ret_hist_.size() < correlation_window_ + lag_period_) return;

        // extract lagged leader + current lagger
        size_t idx = ret_hist_.size() - 1 - lag_period_;
        double lead_lagged = ret_hist_[idx].first;
        double lag_now     = ret_hist_.back().second;

        // update O(1) rolling correlation
        updateRollingCorrelation(lead_lagged, lag_now);
        double corr = computeCorrelation();

        // signal logic
        SignalDirection desired = SignalDirection::FLAT;
        if (corr >= correlation_threshold_) {
            if (lead_lagged >  leader_return_threshold_) desired = SignalDirection::LONG;
            if (lead_lagged < -leader_return_threshold_) desired = SignalDirection::SHORT;
        }
        else if (corr <= -correlation_threshold_) {
            // invert for negative correlation
            if (lead_lagged >  leader_return_threshold_) desired = SignalDirection::SHORT;
            if (lead_lagged < -leader_return_threshold_) desired = SignalDirection::LONG;
        }

        // emit order if changed
        if (desired != current_signal_) {
            LOG_TRACE("LeadLag: corr={:.4f} leadLagRet={:.5f} signal={}",
                      corr, lead_lagged,
                      (desired==SignalDirection::LONG?"LONG":
                       desired==SignalDirection::SHORT?"SHORT":"FLAT"));

            double target_qty = (desired==SignalDirection::LONG ? +target_position_size_
                                   : desired==SignalDirection::SHORT? -target_position_size_
                                   : 0.0);

            double current_qty = portfolio_->get_position_quantity(lagging_symbol_);
            double delta       = target_qty - current_qty;
            if (std::abs(delta) > EPS) {
                auto dir = delta>0 ? OrderDirection::BUY : OrderDirection::SELL;
                send_event(std::make_shared<OrderEvent>(
                               ev.timestamp,
                               lagging_symbol_,
                               OrderType::MARKET,
                               dir,
                               std::abs(delta)),
                           queue);
            }
            current_signal_ = desired;
        }
    }

    void handle_fill_event(const FillEvent&, EventQueue&) override {
        // nothing to do
    }
};