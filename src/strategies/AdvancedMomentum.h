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

class AdvancedMomentum : public Strategy {
private:
    // --- Strategy Parameters ---
    int lookback_period_;
    double momentum_threshold_;
    double volatility_filter_;
    double max_position_risk_;
    
    // --- Per-Symbol State ---
    struct SymbolState {
        std::deque<PriceBar> price_history;
        std::deque<double> returns;
        std::deque<double> prices; // For regime detection
        
        // Performance tracking for Kelly sizing
        std::vector<double> trade_returns;
        double win_rate = 0.5;
        double avg_win = 0.01;
        double avg_loss = -0.01;
        
        // Current state
        SignalDirection current_signal = SignalDirection::FLAT;
        double entry_price = 0.0;
        std::chrono::system_clock::time_point last_trade_time;
        
        // Risk management
        double current_volatility = 0.05;
        MarketRegime current_regime;
    };
    
    std::map<std::string, SymbolState> symbol_states_;
    
    // --- Helper Functions ---
    double calculate_momentum_score(const SymbolState& state) const {
        if (state.returns.size() < static_cast<size_t>(lookback_period_)) {
            return 0.0;
        }
        
        // Calculate risk-adjusted momentum using Sharpe-like ratio
        double mean_return = std::accumulate(state.returns.end() - lookback_period_, 
                                           state.returns.end(), 0.0) / lookback_period_;
        
        double variance = 0.0;
        for (auto it = state.returns.end() - lookback_period_; it != state.returns.end(); ++it) {
            variance += std::pow(*it - mean_return, 2);
        }
        double std_dev = std::sqrt(variance / (lookback_period_ - 1));
        
        if (std_dev <= 0) return 0.0;
        
        // Risk-adjusted momentum score
        return mean_return / std_dev;
    }
    
    void update_performance_stats(SymbolState& state, double trade_return) {
        state.trade_returns.push_back(trade_return);
        
        // Keep only recent trades for Kelly calculation
        if (state.trade_returns.size() > 50) {
            state.trade_returns.erase(state.trade_returns.begin());
        }
        
        if (state.trade_returns.size() >= 10) {
            int wins = 0;
            double sum_wins = 0.0, sum_losses = 0.0;
            int losses = 0;
            
            for (double ret : state.trade_returns) {
                if (ret > 0) {
                    wins++;
                    sum_wins += ret;
                } else if (ret < 0) {
                    losses++;
                    sum_losses += ret;
                }
            }
            
            if (wins + losses > 0) {
                state.win_rate = static_cast<double>(wins) / (wins + losses);
                state.avg_win = wins > 0 ? sum_wins / wins : 0.01;
                state.avg_loss = losses > 0 ? sum_losses / losses : -0.01;
            }
        }
    }
    
public:
    AdvancedMomentum(int lookback = 20, double momentum_thresh = 1.5, 
                    double vol_filter = 0.8, double max_risk = 0.03)
        : lookback_period_(lookback), momentum_threshold_(momentum_thresh),
          volatility_filter_(vol_filter), max_position_risk_(max_risk) {}

    void handle_market_event(const MarketEvent& event, EventQueue& queue) override {
        if (!portfolio_) return;
        
        for (const auto& pair : event.data) {
            const std::string& symbol = pair.first;
            const PriceBar& bar = pair.second;
            
            SymbolState& state = symbol_states_[symbol];
            
            // Update price history
            state.price_history.push_back(bar);
            state.prices.push_back(bar.Close);
            
            if (state.price_history.size() > 100) { // Keep reasonable history
                state.price_history.pop_front();
                state.prices.pop_front();
            }
            
            // Calculate returns
            if (state.price_history.size() >= 2) {
                double prev_close = state.price_history[state.price_history.size()-2].Close;
                if (prev_close > 0) {
                    double return_val = std::log(bar.Close / prev_close);
                    state.returns.push_back(return_val);
                    
                    if (state.returns.size() > 100) {
                        state.returns.pop_front();
                    }
                }
            }
            
            // Need sufficient data for analysis
            if (state.price_history.size() < static_cast<size_t>(lookback_period_ + 10)) {
                continue;
            }
            
            // --- Advanced Analytics ---
            
            // 1. Calculate current volatility using Garman-Klass
            state.current_volatility = calculate_garman_klass_volatility(state.price_history, 20);
            
            // 2. Detect market regime
            state.current_regime = detect_market_regime(state.prices, 50);
            
            // 3. Calculate momentum score
            double momentum_score = calculate_momentum_score(state);
            
            // --- Signal Generation (Regime-Aware) ---
            SignalDirection desired_signal = SignalDirection::FLAT;
            
            // Only trade momentum in trending regimes, or very strong signals in mean-reverting
            if (state.current_regime.regime == MarketRegime::TRENDING && 
                state.current_regime.strength > 0.3) {
                
                if (momentum_score > momentum_threshold_ && 
                    state.current_volatility < volatility_filter_) {
                    desired_signal = SignalDirection::LONG;
                } else if (momentum_score < -momentum_threshold_ && 
                          state.current_volatility < volatility_filter_) {
                    desired_signal = SignalDirection::SHORT;
                }
            } else if (state.current_regime.regime == MarketRegime::MEAN_REVERTING &&
                      state.current_regime.strength > 0.5) {
                // Counter-momentum in strong mean-reverting regimes
                if (momentum_score > momentum_threshold_ * 2.0) {
                    desired_signal = SignalDirection::SHORT; // Fade the momentum
                } else if (momentum_score < -momentum_threshold_ * 2.0) {
                    desired_signal = SignalDirection::LONG;
                }
            }
            
            // --- Position Sizing & Execution ---
            if (desired_signal != state.current_signal) {
                
                // Close existing position if any
                double current_position = portfolio_->get_position_quantity(symbol);
                if (std::abs(current_position) > 0.01 && state.entry_price > 0) {
                    double trade_return = 0.0;
                    if (current_position > 0) {
                        trade_return = std::log(bar.Close / state.entry_price);
                    } else {
                        trade_return = std::log(state.entry_price / bar.Close);
                    }
                    update_performance_stats(state, trade_return);
                }
                
                // Calculate optimal position size using Kelly + volatility adjustment
                double kelly_fraction = calculate_kelly_position_size(
                    state.win_rate, state.avg_win, state.avg_loss, max_position_risk_);
                
                double portfolio_value = portfolio_->get_total_equity();
                double target_dollar_risk = portfolio_value * kelly_fraction;
                
                double target_shares = calculate_volatility_adjusted_shares(
                    target_dollar_risk, bar.Close, state.current_volatility, 2.0);
                
                // Apply regime-based sizing adjustment
                if (state.current_regime.regime == MarketRegime::TRENDING) {
                    target_shares *= 1.2; // Increase size in trending markets
                } else if (state.current_regime.regime == MarketRegime::MEAN_REVERTING) {
                    target_shares *= 0.6; // Reduce size in mean-reverting markets
                }
                
                // Determine target quantity
                double target_quantity = 0.0;
                if (desired_signal == SignalDirection::LONG) {
                    target_quantity = std::min(target_shares, 20.0); // Cap at 20 shares
                } else if (desired_signal == SignalDirection::SHORT) {
                    target_quantity = -std::min(target_shares, 20.0);
                }
                
                double order_quantity_needed = target_quantity - current_position;
                
                if (std::abs(order_quantity_needed) > 0.1) {
                    OrderDirection direction = (order_quantity_needed > 0) ? 
                        OrderDirection::BUY : OrderDirection::SELL;
                    double quantity_to_order = std::abs(order_quantity_needed);
                    
                    std::cout << "ADVANCED_MOMENTUM: " << symbol << " @ " 
                              << formatTimestampUTC(event.timestamp)
                              << " Score=" << momentum_score
                              << " Vol=" << state.current_volatility
                              << " Regime=" << (state.current_regime.regime == MarketRegime::TRENDING ? "TREND" : 
                                               state.current_regime.regime == MarketRegime::MEAN_REVERTING ? "MEAN_REV" : "UNCERTAIN")
                              << " Strength=" << state.current_regime.strength
                              << " Kelly=" << kelly_fraction
                              << " Shares=" << quantity_to_order
                              << " " << (direction == OrderDirection::BUY ? "BUY" : "SELL") << std::endl;
                    
                    send_event(std::make_shared<OrderEvent>(event.timestamp, symbol, 
                        OrderType::MARKET, direction, quantity_to_order), queue);
                    
                    state.current_signal = desired_signal;
                    state.entry_price = bar.Close;
                    state.last_trade_time = event.timestamp;
                }
            }
        }
    }
    
    void handle_fill_event(const FillEvent& event, EventQueue& queue) override {
        // Update any strategy-specific state on fills if needed
    }
}; 