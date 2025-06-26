#pragma once
#include "Strategy.h"

// #include <boost/circular_buffer.hpp>  // Using our own circular_buffer from Utils.h
#include <algorithm>
#include <cmath>
#include <numeric>
#include <mutex>
#include <vector>
#include <iterator>
#include <limits>

// simple no-op logger; replace with spdlog/your favorite later
#ifndef LOG_TRACE
#define LOG_TRACE(fmt, ...) do{} while(0)
#endif

class AdaptiveMeanReversion : public Strategy {
private:
    //––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
    // Constants: all your “magic numbers” in one place
    static constexpr size_t MIN_LOOKBACK   = 20;
    static constexpr size_t MAX_LOOKBACK   = 200;
    static constexpr double MIN_THRESHOLD  = 1.0;
    static constexpr double MAX_THRESHOLD  = 5.0;
    static constexpr double EPS            = 1e-8;
    
    // Core params
    size_t lookback_period_;
    double base_threshold_;
    double max_position_value_;

    // State buffers
    circular_buffer<double> price_history_;
    circular_buffer<double> volume_history_;
    circular_buffer<double> returns_;
    circular_buffer<double> regime_indicators_;

    // Regime detection
    size_t regime_window_    = 30;
    double trend_threshold_  = 0.1;

    // Kalman filter
    double kalman_gain_      = 0.1;
    double process_noise_    = 1e-4;
    double measurement_noise_= 1e-2;
    double mean_estimate_    = 0.0;
    double error_covariance_ = 1.0;

    // Risk / execution
    double max_leverage_         = 2.0;
    double volatility_target_    = 0.02;
    size_t warmup_period_        = 100;
    size_t bar_count_            = 0;
    double bid_ask_spread_estimate_ = 0.001;
    double execution_cost_buffer_   = 0.0005;

    // For thread-safety if you ever parallelize symbols
    mutable std::mutex state_mutex_;

    //––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
    // Static helper: simple GARCH(1,1)
    template<typename It>
    static double simpleGarch(It begin, It end,
                              double omega, double alpha, double beta,
                              double long_run_var)
    {
        if (std::distance(begin,end) < 1) return long_run_var;
        double garch_var = long_run_var;
        double prev_sq   = (*begin)*(*begin);
        for (auto it = std::next(begin); it!=end; ++it) {
            garch_var = omega + alpha*prev_sq + beta*garch_var;
            prev_sq   = (*it)*(*it);
        }
        return std::sqrt(garch_var);
    }

    //––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
    // Static helper: Hurst exponent via R/S
    template<typename It>
    static double hurstRS(It begin, It end) {
        size_t n = std::distance(begin,end);
        if (n<2) return 0.5;
        std::vector<double> logp; logp.reserve(n);
        for (auto it=begin; it!=end; ++it)
            logp.push_back(std::log(*it));

        // mean log-return
        double mlr=0;
        for (size_t i=1;i<logp.size();++i)
            mlr += (logp[i]-logp[i-1]);
        mlr /= (logp.size()-1);

        // cumulative deviations
        std::vector<double> cum; cum.reserve(n-1);
        double sum=0;
        for (size_t i=1;i<logp.size();++i) {
            sum += (logp[i]-logp[i-1]) - mlr;
            cum.push_back(sum);
        }
        if (cum.empty()) return 0.5;
        auto [mn,mx] = std::minmax_element(cum.begin(),cum.end());
        double R = *mx - *mn;

        // std dev
        double var=0;
        for (size_t i=1;i<logp.size();++i) {
            double d=(logp[i]-logp[i-1]) - mlr;
            var += d*d;
        }
        double S = std::sqrt(var/(logp.size()-2));
        if (S<EPS) return 0.5;

        double rs = R/S;
        double H  = std::log(rs)/std::log(double(n));
        return std::clamp(H, 0.1, 0.9);
    }

    //––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
    // Static helper: lag-1 autocorrelation
    template<typename It>
    static double autocorr1(It begin, It end) {
        ptrdiff_t N = std::distance(begin,end);
        if (N<2) return 0.0;
        double mean = std::accumulate(begin,end,0.0)/N;
        double num=0, den=0;
        for (auto it=begin; std::next(it)!=end; ++it) {
            double x = *it  - mean;
            double y = *std::next(it) - mean;
            num += x*y;
            den += y*y;
        }
        return (den<EPS? 0.0 : num/den);
    }

    //––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
    // Standard Kalman update
    void updateKalmanFilter(double price) {
        double pred_err = error_covariance_ + process_noise_;
        kalman_gain_ = pred_err/(pred_err+measurement_noise_);
        mean_estimate_ += kalman_gain_*(price-mean_estimate_);
        error_covariance_ = (1-kalman_gain_)*pred_err;
    }

    // Dynamic threshold & position sizing
    double getAdaptiveThreshold(double vol, double regime) const {
        double v = vol/std::max(volatility_target_,EPS);
        double r = 1.0 + std::abs(regime)*0.5;
        return base_threshold_ * v * r;
    }
    double getOptimalPositionSize(double signal, double vol, double price) const {
        double vm = volatility_target_/std::max(vol,EPS);
        double sm = std::min(2.0,signal);
        double base = (max_position_value_/price)*vm*sm;
        double cap  = max_leverage_*(max_position_value_/price);
        return std::min(base,cap);
    }

public:
    AdaptiveMeanReversion(size_t lookback=50,
                          double threshold=2.0,
                          double max_pos=3000.0)
      : lookback_period_( std::clamp(lookback,MIN_LOOKBACK,MAX_LOOKBACK) )
      , base_threshold_( std::clamp(threshold,MIN_THRESHOLD,MAX_THRESHOLD) )
      , max_position_value_( std::max(1000.0, max_pos) )
      , price_history_(lookback_period_*2)
      , volume_history_(lookback_period_)
      , returns_(lookback_period_)
      , regime_indicators_(regime_window_)
    {
        warmup_period_ = std::max(lookback_period_, size_t(100));
        // no further resize needed—circular_buffer is fixed
    }

    // Entry point
    void handle_market_event(const MarketEvent& ev, EventQueue& q) override {
        std::lock_guard<std::mutex> lk(state_mutex_);
        for (auto const& [sym,bar] : ev.data)
            on_data(bar, sym, q);
    }

    // Main per-bar logic
    void on_data(const PriceBar& bar,
                 const std::string& sym,
                 EventQueue& q)
    {
        // single early warm-up exit
        if (++bar_count_ < warmup_period_) return;

        double mid = (bar.High + bar.Low)*0.5;
        double vol = double(bar.Volume);

        price_history_.push_back(mid);
        volume_history_.push_back(vol);

        // compute 1-period return
        if (price_history_.size()>1) {
            auto it = price_history_.end();
            double prev = *std::prev(it,2);
            returns_.push_back((mid - prev)/std::max(prev,EPS));
        }

        // once we have lookback worth of bars, signal
        if (price_history_.size() >= lookback_period_) {
            updateKalmanFilter(mid);

            double vol_est = simpleGarch(returns_.begin(), returns_.end(),
                                         /*ω*/1e-6, /*α*/0.1, /*β*/0.85,
                                         /*long-run var*/0.0004);
            double H  = hurstRS(price_history_.begin(), price_history_.end());
            double R  = autocorr1(returns_.begin(), returns_.end());

            if (H>0.6 || R>0.3) {
                LOG_TRACE("skipTrending: H={} R={}", H, R);
                return;
            }

            double dev = (mid - mean_estimate_)/std::max(mean_estimate_,EPS);
            double thr = getAdaptiveThreshold(vol_est, R);

            // liquidity filter
            double avg_vol = std::accumulate(volume_history_.begin(),
                                             volume_history_.end(),0.0)
                             / volume_history_.size();
            if (vol < avg_vol*0.3) {
                LOG_TRACE("skipLowVol: v={} < {}", vol, avg_vol*0.3);
                return;
            }

            double signal = std::abs(dev)/thr;
            if (signal>1.0) {
                double eff = thr + execution_cost_buffer_;
                if (dev < -eff) {
                    double sz = getOptimalPositionSize(signal, vol_est, mid);
                    q.push(std::make_shared<OrderEvent>(
                              bar.timestamp, sym,
                              OrderType::MARKET,
                              OrderDirection::BUY,
                              sz));
                }
                else if (dev > eff) {
                    double sz = getOptimalPositionSize(signal, vol_est, mid);
                    q.push(std::make_shared<OrderEvent>(
                              bar.timestamp, sym,
                              OrderType::MARKET,
                              OrderDirection::SELL,
                              sz));
                }
            }
        }
    }

    std::string get_name() const override {
        return "AdaptiveMeanRev_" + std::to_string(lookback_period_) + "_" +
               std::to_string(int(base_threshold_*10));
    }
};