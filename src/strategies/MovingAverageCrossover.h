#pragma once

#include "Strategy.h"
#include "core/Event.h"
#include "core/EventQueue.h"
#include "core/Utils.h"
#include "core/Portfolio.h" // Make sure Portfolio is included
#include <deque>
#include <vector>
#include <string>
#include <numeric>
#include <iostream>
#include <map>
#include <cmath> // For std::abs
#include <algorithm>

class MovingAverageCrossover : public Strategy {
private:
    // --- Parameters ---
    size_t fast_period_;
    size_t slow_period_;
    double base_position_size_;
    
    // Enhanced state tracking
    std::deque<double> fast_prices_;
    std::deque<double> slow_prices_;
    std::deque<double> volume_window_;
    std::deque<double> volatility_window_;
    
    // Risk management
    double max_position_value_;
    double volatility_lookback_;
    double min_volume_threshold_;
    
    // Adaptive sizing
    double atr_period_;
    std::deque<double> true_ranges_;
    
    // Regime detection
    double trend_strength_threshold_;
    size_t regime_lookback_;
    
    double calculateMA(const std::deque<double>& prices) const {
        if (prices.empty()) return 0.0;
        double sum = 0.0;
        for (double price : prices) {
            sum += price;
        }
        return sum / prices.size();
    }
    
    double calculateATR() const {
        if (true_ranges_.size() < atr_period_) return 0.01; // Default small ATR
        double sum = 0.0;
        for (size_t i = true_ranges_.size() - atr_period_; i < true_ranges_.size(); ++i) {
            sum += true_ranges_[i];
        }
        return sum / atr_period_;
    }
    
    double calculateVolatility() const {
        if (volatility_window_.size() < 20) return 0.02; // Default 2%
        
        double mean = 0.0;
        for (double ret : volatility_window_) {
            mean += ret;
        }
        mean /= volatility_window_.size();
        
        double variance = 0.0;
        for (double ret : volatility_window_) {
            variance += (ret - mean) * (ret - mean);
        }
        return std::sqrt(variance / (volatility_window_.size() - 1));
    }
    
    double calculateTrendStrength() const {
        if (slow_prices_.size() < regime_lookback_) return 0.0;
        
        // Linear regression slope over regime window
        size_t n = std::min(regime_lookback_, slow_prices_.size());
        double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
        
        for (size_t i = 0; i < n; ++i) {
            double x = static_cast<double>(i);
            double y = slow_prices_[slow_prices_.size() - n + i];
            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
        }
        
        double slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
        return std::abs(slope) / (sum_y / n); // Normalized slope
    }
    
    double getAdaptivePositionSize(double current_price, double atr) const {
        // Kelly-inspired position sizing with volatility adjustment
        double vol = calculateVolatility();
        double trend_strength = calculateTrendStrength();
        
        // Base size adjusted for volatility and trend strength
        double vol_adjustment = std::max(0.1, 1.0 - (vol * 10.0)); // Reduce size in high vol
        double trend_adjustment = 1.0 + (trend_strength * 0.5); // Increase size in strong trends
        
        double adjusted_size = base_position_size_ * vol_adjustment * trend_adjustment;
        
        // ATR-based position sizing (risk parity)
        if (atr > 0.001) {
            double atr_based_size = (current_price * 0.01) / atr; // 1% risk per ATR
            adjusted_size = std::min(adjusted_size, atr_based_size);
        }
        
        // Cap at maximum position value
        double max_shares = max_position_value_ / current_price;
        return std::min(adjusted_size, max_shares);
    }

    // --- State per Symbol ---
    struct SymbolState {
        std::deque<double> price_history; // Stores historical prices for MA calculation
    };
    std::map<std::string, SymbolState> symbol_state_;
    std::map<std::string, SignalDirection> current_signal_state_; // Track current long/short/flat state per symbol

public:
    MovingAverageCrossover(size_t fast_period, size_t slow_period, double base_position_size)
        : fast_period_(fast_period), slow_period_(slow_period), base_position_size_(base_position_size),
          max_position_value_(5000.0), // $5k max position
          volatility_lookback_(20),
          min_volume_threshold_(1000.0),
          atr_period_(14),
          trend_strength_threshold_(0.001),
          regime_lookback_(50) {
        
        // Ensure reasonable parameters
        if (fast_period_ >= slow_period_) {
            slow_period_ = fast_period_ + 10;
        }
        
        // Conservative position sizing
        base_position_size_ = std::max(1.0, std::min(base_position_size_, 10.0));
    }

    void handle_market_event(const MarketEvent& event, EventQueue& queue) override {
        if (!portfolio_) { // Safety check
            std::cerr << "ERROR: Portfolio not set for strategy!" << std::endl;
            return;
        }

        for (const auto& pair : event.data) {
            const std::string& symbol = pair.first;
            const PriceBar& bar = pair.second;
            double price = bar.Close;

            // Initialize state if symbol is new
            if (symbol_state_.find(symbol) == symbol_state_.end()) {
                symbol_state_[symbol]; // Creates SymbolState with empty deque
                current_signal_state_[symbol] = SignalDirection::FLAT;
            }

            // Update history and SMAs
            std::deque<double>& history = symbol_state_[symbol].price_history;
            history.push_back(price);
            if (history.size() > slow_period_) { history.pop_front(); }

            double short_sma = 0.0;
            double long_sma = 0.0;

            if (history.size() >= fast_period_) {
                double short_sum = std::accumulate(history.end() - fast_period_, history.end(), 0.0);
                short_sma = short_sum / fast_period_;
            } else continue; // Need short window data

            if (history.size() >= slow_period_) {
                double long_sum = std::accumulate(history.begin(), history.end(), 0.0);
                long_sma = long_sum / slow_period_;

                // Determine desired signal based on SMA crossover
                SignalDirection desired_signal = SignalDirection::FLAT;
                constexpr double tolerance = 1e-9;
                if (short_sma > long_sma + tolerance) {
                    desired_signal = SignalDirection::LONG;
                } else if (short_sma < long_sma - tolerance) {
                    desired_signal = SignalDirection::SHORT;
                }

                // Generate orders based on signal change
                if (desired_signal != current_signal_state_[symbol]) {
                    std::cout << "CROSSOVER: " << symbol << " @ " << formatTimestampUTC(event.timestamp)
                              << " ShortSMA=" << short_sma << " LongSMA=" << long_sma
                              << " Signal=" << (desired_signal == SignalDirection::LONG ? "LONG" : desired_signal == SignalDirection::SHORT ? "SHORT" : "FLAT")
                              << std::endl;

                    double target_quantity = 0.0;
                    if (desired_signal == SignalDirection::LONG) {
                        target_quantity = getAdaptivePositionSize(price, calculateATR());
                    } else if (desired_signal == SignalDirection::SHORT) {
                        target_quantity = -getAdaptivePositionSize(price, calculateATR());
                    } // else target is 0.0 for FLAT

                    // Get current position from portfolio
                    double current_quantity = portfolio_->get_position_quantity(symbol);
                    double order_quantity_needed = target_quantity - current_quantity;

                    // Only send order if quantity needed is significant
                    if (std::abs(order_quantity_needed) > 1e-9) {
                        OrderDirection direction = (order_quantity_needed > 0) ? OrderDirection::BUY : OrderDirection::SELL;
                        double quantity_to_order = std::abs(order_quantity_needed);

                         std::cout << " -> Target: " << target_quantity << ", Current: " << current_quantity
                                   << ", Order Qty: " << quantity_to_order << " " << (direction==OrderDirection::BUY?"BUY":"SELL") << std::endl;

                        send_event(std::make_shared<OrderEvent>(event.timestamp, symbol, OrderType::MARKET, direction, quantity_to_order), queue);
                    } else {
                         std::cout << " -> Target: " << target_quantity << ", Current: " << current_quantity << ". No order needed." << std::endl;
                    }
                    current_signal_state_[symbol] = desired_signal; // Update state *after* decision
                }
            } // end if history.size >= long_window
        } // end loop over symbols in market event
    } // end handle_market_event

    std::string get_name() const {
        return "EnhancedMACrossover_" + std::to_string(fast_period_) + "_" + std::to_string(slow_period_);
    }
};