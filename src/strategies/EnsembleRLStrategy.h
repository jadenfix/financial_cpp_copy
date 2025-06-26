#pragma once

#include "Strategy.h"
#include "core/Event.h"
#include "core/EventQueue.h"
#include "core/Utils.h"
#include "core/Portfolio.h"
#include "data/PriceBar.h"

#include <algorithm>
#include <cmath>
#include <mutex>
#include <random>
#include <vector>
#include <memory>
#include <numeric>

#ifndef LOG_TRACE
#define LOG_TRACE(fmt, ...) do{} while(0)
#endif

// Each action is "follow" one of three providers or "blend" them
enum Action : int { ACT_ML=0, ACT_MR=1, ACT_OB=2, ACT_BLEND=3, N_ACTION=4 };

// Signal direction enum (using existing enum from Event.h)
using SignalDirection = ::SignalDirection;

// Simple signal provider interface
class SignalProvider {
public:
    virtual ~SignalProvider() = default;
    virtual int signal(const PriceBar& bar) = 0;
    virtual double probability() const { return 0.5; }
};

// ML Signal Provider (simplified XGBoost-like)
class MLSignalProvider : public SignalProvider {
private:
    circular_buffer<double> prices_{20};
    circular_buffer<double> volumes_{20};
    
public:
    int signal(const PriceBar& bar) override {
        prices_.push_back(bar.Close);
        volumes_.push_back(static_cast<double>(bar.Volume));
        
        if (prices_.size() < 10) return 0;
        
        // Simple ML-like features: momentum + volume trend
        double price_momentum = (prices_.back() - prices_[prices_.size()-10]) / prices_[prices_.size()-10];
        double vol_trend = volumes_.size() >= 5 ? 
            (volumes_.back() - volumes_[volumes_.size()-5]) / volumes_[volumes_.size()-5] : 0;
        
        // Simple decision tree logic
        if (price_momentum > 0.02 && vol_trend > 0.1) return 1;  // LONG
        if (price_momentum < -0.02 && vol_trend > 0.1) return -1; // SHORT
        return 0; // FLAT
    }
    
    double probability() const override {
        return prices_.size() >= 10 ? 0.6 : 0.5;
    }
};

// Mean Reversion Signal Provider
class MRSignalProvider : public SignalProvider {
private:
    circular_buffer<double> prices_{50};
    double kalman_mean_ = 0.0;
    double kalman_var_ = 1.0;
    const double Q_ = 0.001; // process noise
    const double R_ = 0.01;  // measurement noise
    
public:
    int signal(const PriceBar& bar) override {
        prices_.push_back(bar.Close);
        
        if (prices_.size() < 20) return 0;
        
        // Simple Kalman filter for mean
        double prediction = kalman_mean_;
        double pred_var = kalman_var_ + Q_;
        
        // Update with measurement
        double innovation = bar.Close - prediction;
        double S = pred_var + R_;
        double K = pred_var / S;
        
        kalman_mean_ = prediction + K * innovation;
        kalman_var_ = (1 - K) * pred_var;
        
        // Mean reversion signal
        double deviation = (bar.Close - kalman_mean_) / kalman_mean_;
        
        if (deviation > 0.03) return -1; // Price too high, expect reversion (SHORT)
        if (deviation < -0.03) return 1; // Price too low, expect reversion (LONG)
        return 0;
    }
};

// Breakout Signal Provider  
class OBSignalProvider : public SignalProvider {
private:
    circular_buffer<PriceBar> bars_{20};
    
public:
    int signal(const PriceBar& bar) override {
        bars_.push_back(bar);
        
        if (bars_.size() < 15) return 0;
        
        // Calculate range high/low over lookback
        double range_high = -std::numeric_limits<double>::infinity();
        double range_low = std::numeric_limits<double>::infinity();
        
        for (size_t i = 0; i < std::min(size_t(10), bars_.size()-1); ++i) {
            const auto& b = bars_[bars_.size()-2-i]; // exclude current bar
            range_high = std::max(range_high, b.High);
            range_low = std::min(range_low, b.Low);
        }
        
        // Breakout signals
        if (bar.Close > range_high * 1.01) return 1;  // LONG breakout
        if (bar.Close < range_low * 0.99) return -1;  // SHORT breakdown
        return 0;
    }
};

class EnsembleRLStrategy : public Strategy {
private:
    //―――― Hyper-parameters ―――――――――――――――――――――――
    static constexpr double EPS       = 1e-8;
    static constexpr double ALPHA     = 0.05;    // learning rate for Q-update
    static constexpr double GAMMA     = 0.9;     // discount
    static constexpr double EPSILON   = 0.1;     // ε-greedy exploration
    static constexpr size_t STATE_DIM = 7;       // feature vector size
    static constexpr double MAX_POS   = 5.0;     // maximum position size

    //―――― Sub-strategies ―――――――――――――――――――――――
    std::unique_ptr<SignalProvider> ml_, mr_, ob_;

    //―――― Q-function parameters: θ[action][state_dim]
    std::vector<std::vector<double>> theta_;

    //―――― For exploration & RNG ―――――――――――――――
    std::mt19937                            rng_{std::random_device{}()};
    std::uniform_real_distribution<double>  uni_{0.0,1.0};

    //―――― Bookkeeping ―――――――――――――――――――――――
    circular_buffer<PriceBar>               history_{50};
    circular_buffer<double>                 returns_{20};
    double                                  current_qty_ = 0.0;
    std::vector<double>                     prev_state_;
    int                                     prev_action_ = -1;
    double                                  prev_price_ = 0.0;
    std::mutex                              mtx_;

    //―――― Compute Hurst exponent (simplified) ―――――――――――――――
    double computeHurst() {
        if (returns_.size() < 10) return 0.5;
        
        // Simplified R/S analysis
        std::vector<double> rets;
        for (size_t i = 0; i < returns_.size(); ++i) {
            rets.push_back(returns_[i]);
        }
        
        double mean = std::accumulate(rets.begin(), rets.end(), 0.0) / rets.size();
        double cumsum = 0.0;
        double range = 0.0;
        double min_cum = 0.0, max_cum = 0.0;
        
        for (double r : rets) {
            cumsum += (r - mean);
            min_cum = std::min(min_cum, cumsum);
            max_cum = std::max(max_cum, cumsum);
        }
        range = max_cum - min_cum;
        
        double variance = 0.0;
        for (double r : rets) {
            variance += (r - mean) * (r - mean);
        }
        double stddev = std::sqrt(variance / rets.size());
        
        double rs = (stddev > EPS) ? range / stddev : 1.0;
        return std::log(rs) / std::log(2.0) / std::log(static_cast<double>(rets.size()));
    }

    //―――― Compute autocorrelation lag-1 ―――――――――――――――――
    double computeAutocorr1() {
        if (returns_.size() < 5) return 0.0;
        
        std::vector<double> rets;
        for (size_t i = 0; i < returns_.size(); ++i) {
            rets.push_back(returns_[i]);
        }
        
        double mean = std::accumulate(rets.begin(), rets.end(), 0.0) / rets.size();
        
        double num = 0.0, den = 0.0;
        for (size_t i = 1; i < rets.size(); ++i) {
            num += (rets[i] - mean) * (rets[i-1] - mean);
        }
        for (double r : rets) {
            den += (r - mean) * (r - mean);
        }
        
        return (den > EPS) ? num / den : 0.0;
    }

    //―――― Build a STATE_DIM-long feature vector ―――――――――――――――
    void buildState(const PriceBar& bar,
                    std::vector<double>& s_out,
                    int& regime_ctx,
                    int& sig_ml,
                    int& sig_mr,
                    int& sig_ob)
    {
        // 1) Regime features
        double hurst = computeHurst();
        double ac1   = computeAutocorr1();

        regime_ctx = (hurst > 0.6 || ac1 > 0.3) ? 2 :  // trending
                     (hurst < 0.4 || ac1 < -0.3) ? 0 : // mean-reverting
                     1; // neutral

        // 2) Sub-signals
        sig_ml = ml_->signal(bar);
        sig_mr = mr_->signal(bar);
        sig_ob = ob_->signal(bar);

        // 3) ML probability
        double p_ml = ml_->probability();

        // 4) Recent realized volatility
        double vol = 0.02; // default
        if (returns_.size() >= 5) {
            double mean_ret = 0.0;
            for (size_t i = returns_.size()-5; i < returns_.size(); ++i) {
                mean_ret += returns_[i];
            }
            mean_ret /= 5.0;
            
            double var = 0.0;
            for (size_t i = returns_.size()-5; i < returns_.size(); ++i) {
                var += (returns_[i] - mean_ret) * (returns_[i] - mean_ret);
            }
            vol = std::sqrt(var / 4.0);
        }

        // 5) Mean-reversion deviation (simplified)
        double dev = 0.0;
        if (history_.size() >= 20) {
            double sma = 0.0;
            for (size_t i = history_.size()-20; i < history_.size(); ++i) {
                sma += history_[i].Close;
            }
            sma /= 20.0;
            dev = (bar.Close - sma) / sma;
        }

        s_out = { static_cast<double>(regime_ctx),
                  static_cast<double>(sig_ml), 
                  static_cast<double>(sig_mr), 
                  static_cast<double>(sig_ob),
                  p_ml,
                  vol,
                  dev };
    }

    //―――― ε-greedy action selection ―――――――――――――――――――――――
    int selectAction(const std::vector<double>& s, int ctx) {
        if (uni_(rng_) < EPSILON) {
            return std::uniform_int_distribution<int>(0, N_ACTION-1)(rng_);
        }
        // pick argmax_a θ[a] · s
        int best = 0;
        double bestv = -1e9;
        for (int a = 0; a < N_ACTION; ++a) {
            double q = 0;
            for (size_t i = 0; i < STATE_DIM; ++i) {
                q += theta_[a][i] * s[i];
            }
            if (q > bestv) { 
                bestv = q; 
                best = a; 
            }
        }
        return best;
    }

    //―――― One‐step Q-learning update ―――――――――――――――――――――
    void qUpdate(const std::vector<double>& s,
                 int a,
                 double r,
                 const std::vector<double>& s2)
    {
        // compute Q(s2,a') max
        double q2max = -1e9;
        for (int ap = 0; ap < N_ACTION; ++ap) {
            double q2 = 0;
            for (size_t i = 0; i < STATE_DIM; ++i) {
                q2 += theta_[ap][i] * s2[i];
            }
            q2max = std::max(q2max, q2);
        }
        
        // TD error
        double qsa = 0;
        for (size_t i = 0; i < STATE_DIM; ++i) {
            qsa += theta_[a][i] * s[i];
        }
        double td = r + GAMMA * q2max - qsa;

        // gradient step
        for (size_t i = 0; i < STATE_DIM; ++i) {
            theta_[a][i] += ALPHA * td * s[i];
        }
    }

public:
    EnsembleRLStrategy() {
        // Instantiate sub-strategies
        ml_ = std::make_unique<MLSignalProvider>();
        mr_ = std::make_unique<MRSignalProvider>();
        ob_ = std::make_unique<OBSignalProvider>();
        
        // Initialize Q-function parameters
        theta_.assign(N_ACTION, std::vector<double>(STATE_DIM, 0.0));
        prev_state_.resize(STATE_DIM, 0.0);
    }

    void handle_market_event(const MarketEvent& ev, EventQueue& queue) override {
        std::lock_guard<std::mutex> lk(mtx_);
        if (!portfolio_ || ev.data.empty()) return;

        auto const& kv = *ev.data.begin();
        const PriceBar& bar = kv.second;

        history_.push_back(bar);
        
        // Calculate returns
        if (history_.size() >= 2) {
            double ret = (bar.Close - history_[history_.size()-2].Close) / history_[history_.size()-2].Close;
            returns_.push_back(ret);
        }
        
        if (history_.size() < 10) return; // Need enough history

        std::vector<double> s;
        int ctx, sig_ml, sig_mr, sig_ob;
        buildState(bar, s, ctx, sig_ml, sig_mr, sig_ob);

        // Q-learning update from previous step
        if (prev_action_ >= 0 && prev_price_ > 0) {
            double reward = current_qty_ * (bar.Close - prev_price_); // P&L
            qUpdate(prev_state_, prev_action_, reward, s);
        }

        // Select and execute action
        int act = selectAction(s, ctx);
        int sig = 0;
        switch(act) {
            case ACT_ML:    sig = sig_ml; break;
            case ACT_MR:    sig = sig_mr; break;
            case ACT_OB:    sig = sig_ob; break;
            case ACT_BLEND: {
                int total = sig_ml + sig_mr + sig_ob;
                sig = (total > 0) ? 1 : (total < 0) ? -1 : 0;
                break;
            }
        }

        double tgt = sig * MAX_POS;
        double delta = tgt - current_qty_;
        
        if (std::abs(delta) > EPS) {
            auto order = std::make_shared<OrderEvent>(
                bar.timestamp, 
                kv.first, 
                OrderType::MARKET,
                delta > 0 ? OrderDirection::BUY : OrderDirection::SELL,
                std::abs(delta)
            );
            send_event(order, queue);
        }

        // Store state for next update
        prev_state_ = s;
        prev_action_ = act;
        prev_price_ = bar.Close;

        LOG_TRACE("RL+: ctx={} act={} sig={} qty={:.2f}", ctx, act, sig, tgt);
    }

    void handle_fill_event(const FillEvent& event, EventQueue& queue) override {
        std::lock_guard<std::mutex> lk(mtx_);
        if (event.direction == OrderDirection::BUY) {
            current_qty_ += event.quantity;
        } else {
            current_qty_ -= event.quantity;
        }
    }

    std::string get_name() const override {
        return "EnsembleRL";
    }
}; 