#pragma once
#include "Strategy.h"
#include <deque>
#include <algorithm>
#include <cmath>
#include <numeric>

class AdaptiveMeanReversion : public Strategy {
private:
    // Core parameters
    size_t lookback_period_;
    double base_threshold_;
    double max_position_value_;
    
    // Enhanced state tracking
    std::deque<double> price_history_;
    std::deque<double> volume_history_;
    std::deque<double> returns_;
    std::deque<double> volatility_estimates_;
    
    // Regime detection
    std::deque<double> regime_indicators_;
    size_t regime_window_;
    double trend_threshold_;
    
    // Kalman filter for dynamic mean estimation
    double kalman_gain_;
    double process_noise_;
    double measurement_noise_;
    double mean_estimate_;
    double error_covariance_;
    
    // Risk management
    double max_leverage_;
    double volatility_target_;
    size_t warmup_period_;
    
    // Market microstructure
    double bid_ask_spread_estimate_;
    double execution_cost_buffer_;
    
    void updateKalmanFilter(double new_price) {
        // Prediction step
        double predicted_error = error_covariance_ + process_noise_;
        
        // Update step
        kalman_gain_ = predicted_error / (predicted_error + measurement_noise_);
        mean_estimate_ = mean_estimate_ + kalman_gain_ * (new_price - mean_estimate_);
        error_covariance_ = (1.0 - kalman_gain_) * predicted_error;
    }
    
    double calculateGARCHVolatility() const {
        if (returns_.size() < 20) return 0.02; // Default 2%
        
        // Simple GARCH(1,1) approximation
        double alpha = 0.1, beta = 0.85, omega = 0.000001;
        double long_run_var = 0.0004; // 2% annual vol squared
        
        double garch_var = long_run_var;
        for (size_t i = 1; i < returns_.size(); ++i) {
            double prev_return = returns_[i-1];
            garch_var = omega + alpha * prev_return * prev_return + beta * garch_var;
        }
        
        return std::sqrt(garch_var);
    }
    
    double calculateHurstExponent() const {
        if (price_history_.size() < 50) return 0.5; // Random walk default
        
        // Simplified R/S analysis
        size_t n = std::min(static_cast<size_t>(50), price_history_.size());
        std::vector<double> log_prices;
        
        for (size_t i = price_history_.size() - n; i < price_history_.size(); ++i) {
            log_prices.push_back(std::log(price_history_[i]));
        }
        
        double mean_log_return = 0.0;
        for (size_t i = 1; i < log_prices.size(); ++i) {
            mean_log_return += log_prices[i] - log_prices[i-1];
        }
        mean_log_return /= (log_prices.size() - 1);
        
        // Calculate cumulative deviations
        std::vector<double> cumulative_deviations;
        double cumsum = 0.0;
        for (size_t i = 1; i < log_prices.size(); ++i) {
            cumsum += (log_prices[i] - log_prices[i-1]) - mean_log_return;
            cumulative_deviations.push_back(cumsum);
        }
        
        if (cumulative_deviations.empty()) return 0.5;
        
        // Calculate range
        auto minmax = std::minmax_element(cumulative_deviations.begin(), cumulative_deviations.end());
        double range = *minmax.second - *minmax.first;
        
        // Calculate standard deviation
        double variance = 0.0;
        for (size_t i = 1; i < log_prices.size(); ++i) {
            double ret = log_prices[i] - log_prices[i-1];
            variance += (ret - mean_log_return) * (ret - mean_log_return);
        }
        double std_dev = std::sqrt(variance / (log_prices.size() - 2));
        
        if (std_dev < 1e-8) return 0.5;
        
        // Hurst exponent approximation
        double rs_ratio = range / std_dev;
        double hurst = std::log(rs_ratio) / std::log(static_cast<double>(n));
        
        return std::max(0.1, std::min(0.9, hurst)); // Clamp to reasonable range
    }
    
    double calculateRegimeIndicator() const {
        if (returns_.size() < 20) return 0.0;
        
        // Trend vs mean reversion indicator
        size_t window = std::min(static_cast<size_t>(20), returns_.size());
        
        // Calculate autocorrelation at lag 1
        double mean_return = 0.0;
        for (size_t i = returns_.size() - window; i < returns_.size(); ++i) {
            mean_return += returns_[i];
        }
        mean_return /= window;
        
        double numerator = 0.0, denominator = 0.0;
        for (size_t i = returns_.size() - window + 1; i < returns_.size(); ++i) {
            double ret_t = returns_[i] - mean_return;
            double ret_t_minus_1 = returns_[i-1] - mean_return;
            numerator += ret_t * ret_t_minus_1;
            denominator += ret_t * ret_t;
        }
        
        if (denominator < 1e-10) return 0.0;
        return numerator / denominator; // Autocorrelation coefficient
    }
    
    double getAdaptiveThreshold(double volatility, double regime_indicator) const {
        // Dynamic threshold based on volatility and regime
        double vol_adjustment = volatility / volatility_target_;
        double regime_adjustment = 1.0 + std::abs(regime_indicator) * 0.5;
        
        return base_threshold_ * vol_adjustment * regime_adjustment;
    }
    
    double getOptimalPositionSize(double signal_strength, double volatility, double price) const {
        // Kelly criterion with volatility targeting
        double vol_target_multiplier = volatility_target_ / std::max(volatility, 0.001);
        
        // Signal strength adjustment (stronger signals get larger positions)
        double signal_multiplier = std::min(2.0, signal_strength);
        
        // Base position from volatility targeting
        double base_position = (max_position_value_ / price) * vol_target_multiplier * signal_multiplier;
        
        // Apply leverage constraint
        return std::min(base_position, max_leverage_ * (max_position_value_ / price));
    }

public:
    AdaptiveMeanReversion(size_t lookback = 50, double threshold = 2.0, double max_pos_value = 3000.0)
        : lookback_period_(lookback), base_threshold_(threshold), max_position_value_(max_pos_value),
          regime_window_(30), trend_threshold_(0.1),
          kalman_gain_(0.1), process_noise_(0.0001), measurement_noise_(0.01),
          mean_estimate_(0.0), error_covariance_(1.0),
          max_leverage_(2.0), volatility_target_(0.02),
          warmup_period_(std::max(lookback, static_cast<size_t>(100))),
          bid_ask_spread_estimate_(0.001), execution_cost_buffer_(0.0005) {
        
        // Validate parameters
        lookback_period_ = std::max(static_cast<size_t>(20), std::min(lookback_period_, static_cast<size_t>(200)));
        base_threshold_ = std::max(1.0, std::min(threshold, 5.0));
        max_position_value_ = std::max(1000.0, std::min(max_pos_value, 10000.0));
    }
    
    void handle_market_event(const MarketEvent& event, EventQueue& queue) override {
        for (const auto& [symbol, bar] : event.data) {
            on_data(bar, symbol, queue);
        }
    }
    
    void on_data(const PriceBar& bar, const std::string& symbol, EventQueue& queue) {
        double mid_price = (bar.High + bar.Low) / 2.0;
        double volume = static_cast<double>(bar.Volume);
        
        // Update price and volume history
        price_history_.push_back(mid_price);
        volume_history_.push_back(volume);
        
        // Maintain window sizes
        if (price_history_.size() > lookback_period_ * 2) {
            price_history_.pop_front();
        }
        if (volume_history_.size() > lookback_period_) {
            volume_history_.pop_front();
        }
        
        // Calculate returns
        if (price_history_.size() >= 2) {
            double prev_price = price_history_[price_history_.size() - 2];
            double return_pct = (mid_price - prev_price) / prev_price;
            returns_.push_back(return_pct);
            
            if (returns_.size() > lookback_period_) {
                returns_.pop_front();
            }
        }
        
        // Update Kalman filter for dynamic mean
        if (price_history_.size() >= 10) {
            updateKalmanFilter(mid_price);
        }
        
        // Wait for sufficient warmup
        if (price_history_.size() < warmup_period_) {
            return;
        }
        
        // Calculate current metrics
        double volatility = calculateGARCHVolatility();
        double hurst = calculateHurstExponent();
        double regime_indicator = calculateRegimeIndicator();
        
        // Skip if in trending regime (Hurst > 0.6 or strong positive autocorrelation)
        if (hurst > 0.6 || regime_indicator > 0.3) {
            return;
        }
        
        // Calculate mean reversion signal
        double current_deviation = (mid_price - mean_estimate_) / mean_estimate_;
        double adaptive_threshold = getAdaptiveThreshold(volatility, regime_indicator);
        
        // Volume filter - ensure sufficient liquidity
        double avg_volume = std::accumulate(volume_history_.begin(), volume_history_.end(), 0.0) / volume_history_.size();
        if (volume < avg_volume * 0.3) { // Skip low volume periods
            return;
        }
        
        // Signal generation with adaptive thresholds
        double signal_strength = std::abs(current_deviation) / adaptive_threshold;
        
        if (signal_strength > 1.0) { // Signal threshold exceeded
            // Account for execution costs in signal
            double effective_threshold = adaptive_threshold + execution_cost_buffer_;
            
            if (current_deviation < -effective_threshold) {
                // Price below mean - buy signal
                double position_size = getOptimalPositionSize(signal_strength, volatility, mid_price);
                auto order_event = std::make_shared<OrderEvent>(
                    bar.timestamp,
                    symbol,
                    OrderType::MARKET,
                    OrderDirection::BUY,
                    position_size
                );
                queue.push(order_event);
                
            } else if (current_deviation > effective_threshold) {
                // Price above mean - sell signal  
                double position_size = getOptimalPositionSize(signal_strength, volatility, mid_price);
                auto order_event = std::make_shared<OrderEvent>(
                    bar.timestamp,
                    symbol,
                    OrderType::MARKET,
                    OrderDirection::SELL,
                    position_size
                );
                queue.push(order_event);
            }
        }
        
        // Update regime indicators
        regime_indicators_.push_back(regime_indicator);
        if (regime_indicators_.size() > regime_window_) {
            regime_indicators_.pop_front();
        }
    }
    
    std::string get_name() const {
        return "AdaptiveMeanRev_" + std::to_string(lookback_period_) + "_" + 
               std::to_string(static_cast<int>(base_threshold_ * 10));
    }
}; 