#pragma once

#include "Strategy.h"
#include "core/Event.h"
#include "core/EventQueue.h"
#include "core/Utils.h"
#include "core/Portfolio.h"
#include "data/PriceBar.h"
#include <string>
#include <map>
#include <deque>
#include <vector>
#include <cmath>
#include <numeric>
#include <iostream>

class StatisticalArbitrage : public Strategy {
private:
    // --- Strategy Parameters ---
    std::string primary_symbol_;
    std::string hedge_symbol_;
    int lookback_window_;
    double entry_z_score_;
    double exit_z_score_;
    double max_position_risk_;
    
    // --- Strategy State ---
    struct ArbState {
        std::deque<double> primary_prices;
        std::deque<double> hedge_prices;
        std::deque<double> spread_values;
        std::deque<PriceBar> primary_bars;
        std::deque<PriceBar> hedge_bars;
        
        // Statistical measures
        double spread_mean = 0.0;
        double spread_std = 1.0;
        double hedge_ratio = 1.0;
        double half_life = 20.0; // Mean reversion half-life
        
        // Position tracking
        SignalDirection current_signal = SignalDirection::FLAT;
        double entry_spread = 0.0;
        std::chrono::system_clock::time_point last_trade_time;
        
        // Performance tracking
        std::vector<double> trade_returns;
        double win_rate = 0.5;
        double avg_win = 0.01;
        double avg_loss = -0.01;
    } state_;
    
    // --- Advanced Statistical Functions ---
    
    // Calculate optimal hedge ratio using linear regression
    double calculate_hedge_ratio(const std::deque<double>& y, const std::deque<double>& x) const {
        if (y.size() != x.size() || y.size() < 20) return 1.0;
        
        double sum_x = std::accumulate(x.begin(), x.end(), 0.0);
        double sum_y = std::accumulate(y.begin(), y.end(), 0.0);
        double sum_xy = 0.0, sum_xx = 0.0;
        
        for (size_t i = 0; i < x.size(); ++i) {
            sum_xy += x[i] * y[i];
            sum_xx += x[i] * x[i];
        }
        
        double n = static_cast<double>(x.size());
        double beta = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
        
        return std::abs(beta); // Ensure positive hedge ratio
    }
    
    // Calculate Ornstein-Uhlenbeck half-life for mean reversion speed
    double calculate_half_life(const std::deque<double>& spread) const {
        if (spread.size() < 30) return 20.0; // Default
        
        std::vector<double> y, x;
        for (size_t i = 1; i < spread.size(); ++i) {
            y.push_back(spread[i] - spread[i-1]); // First difference
            x.push_back(spread[i-1]); // Lagged level
        }
        
        if (x.empty()) return 20.0;
        
        // Simple linear regression: Δy = α + β*y(t-1)
        double sum_x = std::accumulate(x.begin(), x.end(), 0.0);
        double sum_y = std::accumulate(y.begin(), y.end(), 0.0);
        double sum_xy = 0.0, sum_xx = 0.0;
        
        for (size_t i = 0; i < x.size(); ++i) {
            sum_xy += x[i] * y[i];
            sum_xx += x[i] * x[i];
        }
        
        double n = static_cast<double>(x.size());
        double beta = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
        
        if (beta >= 0) return 20.0; // Not mean reverting
        
        // Half-life = -ln(2) / ln(1 + β)
        double half_life = -std::log(2.0) / std::log(1.0 + beta);
        return std::max(1.0, std::min(half_life, 100.0)); // Clamp to reasonable range
    }
    
    // Enhanced Z-score with exponential weighting
    double calculate_ewma_z_score(const std::deque<double>& values, double alpha = 0.05) const {
        if (values.size() < 10) return 0.0;
        
        double ewma = values[0];
        double ewma_sq = values[0] * values[0];
        
        for (size_t i = 1; i < values.size(); ++i) {
            ewma = alpha * values[i] + (1 - alpha) * ewma;
            ewma_sq = alpha * values[i] * values[i] + (1 - alpha) * ewma_sq;
        }
        
        double variance = ewma_sq - ewma * ewma;
        if (variance <= 0) return 0.0;
        
        double std_dev = std::sqrt(variance);
        return (values.back() - ewma) / std_dev;
    }
    
public:
    StatisticalArbitrage(std::string primary_sym, std::string hedge_sym,
                        int lookback = 100, double entry_z = 2.0, double exit_z = 0.5,
                        double max_risk = 0.04)
        : primary_symbol_(std::move(primary_sym)), hedge_symbol_(std::move(hedge_sym)),
          lookback_window_(lookback), entry_z_score_(entry_z), exit_z_score_(exit_z),
          max_position_risk_(max_risk) {}

    void handle_market_event(const MarketEvent& event, EventQueue& queue) override {
        if (!portfolio_) return;
        
        // Find data for both symbols
        auto primary_it = event.data.find(primary_symbol_);
        auto hedge_it = event.data.find(hedge_symbol_);
        
        if (primary_it == event.data.end() || hedge_it == event.data.end()) {
            return; // Need both symbols
        }
        
        const PriceBar& primary_bar = primary_it->second;
        const PriceBar& hedge_bar = hedge_it->second;
        
        // Update price history
        state_.primary_prices.push_back(primary_bar.Close);
        state_.hedge_prices.push_back(hedge_bar.Close);
        state_.primary_bars.push_back(primary_bar);
        state_.hedge_bars.push_back(hedge_bar);
        
        // Maintain window size
        if (state_.primary_prices.size() > static_cast<size_t>(lookback_window_)) {
            state_.primary_prices.pop_front();
            state_.hedge_prices.pop_front();
            state_.primary_bars.pop_front();
            state_.hedge_bars.pop_front();
        }
        
        // Need sufficient data for analysis
        if (state_.primary_prices.size() < static_cast<size_t>(lookback_window_ / 2)) {
            return;
        }
        
        // --- Statistical Analysis ---
        
        // 1. Update hedge ratio using rolling regression
        state_.hedge_ratio = calculate_hedge_ratio(state_.primary_prices, state_.hedge_prices);
        
        // 2. Calculate spread
        double current_spread = primary_bar.Close - state_.hedge_ratio * hedge_bar.Close;
        state_.spread_values.push_back(current_spread);
        
        if (state_.spread_values.size() > static_cast<size_t>(lookback_window_)) {
            state_.spread_values.pop_front();
        }
        
        // 3. Update statistical measures
        if (state_.spread_values.size() >= 20) {
            // Use EWMA for more responsive mean/std calculation
            double z_score = calculate_ewma_z_score(state_.spread_values, 0.03);
            
            // Update mean reversion speed
            state_.half_life = calculate_half_life(state_.spread_values);
            
            // --- Signal Generation ---
            SignalDirection desired_signal = SignalDirection::FLAT;
            
            // Entry conditions
            if (state_.current_signal == SignalDirection::FLAT) {
                if (z_score > entry_z_score_ && state_.half_life < 50.0) {
                    desired_signal = SignalDirection::SHORT; // Short spread (short primary, long hedge)
                } else if (z_score < -entry_z_score_ && state_.half_life < 50.0) {
                    desired_signal = SignalDirection::LONG; // Long spread (long primary, short hedge)
                }
            } else {
                // Exit conditions
                bool should_exit = false;
                
                if (state_.current_signal == SignalDirection::LONG && z_score > -exit_z_score_) {
                    should_exit = true;
                } else if (state_.current_signal == SignalDirection::SHORT && z_score < exit_z_score_) {
                    should_exit = true;
                }
                
                // Time-based exit if position held too long
                auto time_in_position = std::chrono::duration_cast<std::chrono::minutes>(
                    event.timestamp - state_.last_trade_time).count();
                if (time_in_position > state_.half_life * 5) { // 5x half-life timeout
                    should_exit = true;
                }
                
                if (should_exit) {
                    desired_signal = SignalDirection::FLAT;
                }
            }
            
            // --- Position Management ---
            if (desired_signal != state_.current_signal) {
                
                // Calculate Kelly-based position sizing
                double kelly_fraction = calculate_kelly_position_size(
                    state_.win_rate, state_.avg_win, state_.avg_loss, max_position_risk_);
                
                double portfolio_value = portfolio_->get_total_equity();
                double target_dollar_risk = portfolio_value * kelly_fraction;
                
                // Calculate volatility for both assets
                double primary_vol = calculate_garman_klass_volatility(state_.primary_bars, 20);
                double hedge_vol = calculate_garman_klass_volatility(state_.hedge_bars, 20);
                
                // Position sizing based on spread volatility
                double spread_vol = std::sqrt(primary_vol * primary_vol + 
                                            state_.hedge_ratio * state_.hedge_ratio * hedge_vol * hedge_vol);
                
                double primary_shares = calculate_volatility_adjusted_shares(
                    target_dollar_risk, primary_bar.Close, spread_vol, 1.5);
                
                double hedge_shares = primary_shares * state_.hedge_ratio;
                
                // Cap position sizes
                primary_shares = std::min(primary_shares, 15.0);
                hedge_shares = std::min(hedge_shares, 15.0);
                
                // Execute trades
                double current_primary = portfolio_->get_position_quantity(primary_symbol_);
                double current_hedge = portfolio_->get_position_quantity(hedge_symbol_);
                
                double target_primary = 0.0, target_hedge = 0.0;
                
                if (desired_signal == SignalDirection::LONG) {
                    target_primary = primary_shares;
                    target_hedge = -hedge_shares;
                } else if (desired_signal == SignalDirection::SHORT) {
                    target_primary = -primary_shares;
                    target_hedge = hedge_shares;
                }
                
                // Primary leg order
                double primary_order_qty = target_primary - current_primary;
                if (std::abs(primary_order_qty) > 0.1) {
                    OrderDirection primary_dir = (primary_order_qty > 0) ? 
                        OrderDirection::BUY : OrderDirection::SELL;
                    
                    std::cout << "STAT_ARB: " << primary_symbol_ << "/" << hedge_symbol_ 
                              << " @ " << formatTimestampUTC(event.timestamp)
                              << " Spread=" << current_spread
                              << " Z=" << z_score
                              << " HedgeRatio=" << state_.hedge_ratio
                              << " HalfLife=" << state_.half_life
                              << " Signal=" << (desired_signal == SignalDirection::LONG ? "LONG_SPREAD" :
                                               desired_signal == SignalDirection::SHORT ? "SHORT_SPREAD" : "FLAT")
                              << std::endl;
                    
                    send_event(std::make_shared<OrderEvent>(event.timestamp, primary_symbol_,
                        OrderType::MARKET, primary_dir, std::abs(primary_order_qty)), queue);
                }
                
                // Hedge leg order
                double hedge_order_qty = target_hedge - current_hedge;
                if (std::abs(hedge_order_qty) > 0.1) {
                    OrderDirection hedge_dir = (hedge_order_qty > 0) ? 
                        OrderDirection::BUY : OrderDirection::SELL;
                    
                    send_event(std::make_shared<OrderEvent>(event.timestamp, hedge_symbol_,
                        OrderType::MARKET, hedge_dir, std::abs(hedge_order_qty)), queue);
                }
                
                state_.current_signal = desired_signal;
                state_.entry_spread = current_spread;
                state_.last_trade_time = event.timestamp;
            }
        }
    }
    
    void handle_fill_event(const FillEvent& event, EventQueue& queue) override {
        // Track performance for Kelly optimization
        if (event.symbol == primary_symbol_ && state_.current_signal == SignalDirection::FLAT) {
            // Position was closed, calculate return
            // This is simplified - in practice you'd track the full round-trip
        }
    }
}; 