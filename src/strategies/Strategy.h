#pragma once

#include "../core/Event.h"
#include "../core/EventQueue.h"
#include "../core/Portfolio.h" // Include Portfolio header
#include <string>
#include <vector>
#include <map>
#include <memory> // For std::shared_ptr
#include <cmath> // For std::floor
#include <deque>
#include <algorithm>
#include <numeric>

class Strategy {
protected:
    Portfolio* portfolio_ = nullptr; // Pointer to the portfolio (non-owning)

public:
    // --- Constructor and Virtual Destructor ---
    Strategy() = default;
    virtual ~Strategy() = default;

    // --- Interface Methods ---
    virtual void handle_market_event(const MarketEvent& event, EventQueue& queue) = 0;
    virtual void handle_fill_event(const FillEvent& event, EventQueue& queue) {}
    virtual std::string get_name() const { return "Strategy"; }

    // --- Helper for Strategies ---
    void set_portfolio(Portfolio* portfolio) { portfolio_ = portfolio; }

    // --- ADVANCED QUANTITATIVE HELPERS ---
    
    // Calculate realized volatility using Garman-Klass estimator (more accurate than close-to-close)
    double calculate_garman_klass_volatility(const std::deque<PriceBar>& bars, int window = 20) const {
        if (bars.size() < static_cast<size_t>(window)) return 0.05; // Default 5% daily vol
        
        double sum_gk = 0.0;
        int count = 0;
        
        for (int i = bars.size() - window; i < static_cast<int>(bars.size()); ++i) {
            const auto& bar = bars[i];
            if (bar.High > 0 && bar.Low > 0 && bar.Open > 0 && bar.Close > 0) {
                double ln_hl = std::log(bar.High / bar.Low);
                double ln_co = std::log(bar.Close / bar.Open);
                sum_gk += 0.5 * ln_hl * ln_hl - (2 * std::log(2) - 1) * ln_co * ln_co;
                count++;
            }
        }
        
        if (count == 0) return 0.05;
        return std::sqrt(sum_gk / count * 252); // Annualized volatility
    }
    
    // Kelly criterion for optimal position sizing
    double calculate_kelly_position_size(double win_rate, double avg_win, double avg_loss, 
                                       double max_risk_per_trade = 0.02) const {
        if (avg_loss >= 0 || win_rate <= 0 || win_rate >= 1) return 0.01; // Conservative fallback
        
        double b = avg_win / std::abs(avg_loss); // Win/loss ratio
        double p = win_rate;
        double kelly_fraction = (b * p - (1 - p)) / b;
        
        // Cap at max_risk_per_trade for safety
        return std::min(std::max(kelly_fraction, 0.005), max_risk_per_trade);
    }
    
    // Volatility-adjusted position sizing
    double calculate_volatility_adjusted_shares(double target_dollar_risk, double current_price, 
                                              double volatility, double confidence = 2.0) const {
        if (current_price <= 0 || volatility <= 0) return 0.0;
        
        // Risk per share = price * volatility * confidence_interval
        double risk_per_share = current_price * volatility * confidence / std::sqrt(252);
        
        if (risk_per_share <= 0) return 0.0;
        return std::floor(target_dollar_risk / risk_per_share);
    }
    
    // Market regime detection (trending vs mean-reverting)
    struct MarketRegime {
        enum Type { TRENDING, MEAN_REVERTING, UNCERTAIN };
        Type regime;
        double strength; // 0-1, confidence in regime
        double trend_direction; // -1 to 1
    };
    
    MarketRegime detect_market_regime(const std::deque<double>& prices, int lookback = 50) const {
        MarketRegime regime;
        if (prices.size() < static_cast<size_t>(lookback)) {
            regime.regime = MarketRegime::UNCERTAIN;
            regime.strength = 0.0;
            regime.trend_direction = 0.0;
            return regime;
        }
        
        // Calculate Hurst exponent for trend vs mean reversion
        std::vector<double> returns;
        for (size_t i = prices.size() - lookback + 1; i < prices.size(); ++i) {
            returns.push_back(std::log(prices[i] / prices[i-1]));
        }
        
        double hurst = calculate_hurst_exponent(returns);
        
        // Hurst > 0.5 = trending, Hurst < 0.5 = mean reverting
        if (hurst > 0.55) {
            regime.regime = MarketRegime::TRENDING;
            regime.strength = (hurst - 0.55) / 0.45; // Scale 0.55-1.0 to 0-1
        } else if (hurst < 0.45) {
            regime.regime = MarketRegime::MEAN_REVERTING;
            regime.strength = (0.45 - hurst) / 0.45; // Scale 0-0.45 to 1-0
        } else {
            regime.regime = MarketRegime::UNCERTAIN;
            regime.strength = 0.0;
        }
        
        // Calculate trend direction
        double first_price = prices[prices.size() - lookback];
        double last_price = prices.back();
        regime.trend_direction = (last_price - first_price) / first_price;
        
        return regime;
    }
    
private:
    // Simplified Hurst exponent calculation using R/S method
    double calculate_hurst_exponent(const std::vector<double>& returns) const {
        if (returns.size() < 10) return 0.5; // Random walk default
        
        int n = returns.size();
        double mean_return = std::accumulate(returns.begin(), returns.end(), 0.0) / n;
        
        // Calculate cumulative deviations
        std::vector<double> cumulative_dev(n);
        cumulative_dev[0] = returns[0] - mean_return;
        for (int i = 1; i < n; ++i) {
            cumulative_dev[i] = cumulative_dev[i-1] + (returns[i] - mean_return);
        }
        
        // Calculate range
        double max_dev = *std::max_element(cumulative_dev.begin(), cumulative_dev.end());
        double min_dev = *std::min_element(cumulative_dev.begin(), cumulative_dev.end());
        double range = max_dev - min_dev;
        
        // Calculate standard deviation
        double variance = 0.0;
        for (double ret : returns) {
            variance += std::pow(ret - mean_return, 2);
        }
        double std_dev = std::sqrt(variance / (n - 1));
        
        if (std_dev <= 0) return 0.5;
        
        // R/S ratio
        double rs_ratio = range / std_dev;
        
        // Hurst = log(R/S) / log(n)
        if (rs_ratio <= 0) return 0.5;
        return std::log(rs_ratio) / std::log(n);
    }

public:
    // --- Original helpers ---
    double calculate_shares_for_dollar_amount(double target_dollar_amount, double current_price) const {
        if (current_price <= 0) return 0.0;
        return std::floor(target_dollar_amount / current_price);
    }
    
    double calculate_target_position_size(double allocation_percentage, double current_price) const {
        if (!portfolio_ || current_price <= 0) return 0.0;
        double available_capital = portfolio_->get_total_equity();
        double target_dollar_amount = available_capital * (allocation_percentage / 100.0);
        return calculate_shares_for_dollar_amount(target_dollar_amount, current_price);
    }

    // Helper to push events onto the queue
    void send_event(EventPtr event, EventQueue& queue) {
        queue.push(std::move(event));
    }
};