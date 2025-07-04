#pragma once

#include "Strategy.h"
#include "core/Event.h"
#include "core/EventQueue.h"
#include "core/Utils.h"
#include "core/Portfolio.h"
#include <string>
#include <map>
#include <vector>
#include <deque>
#include <cmath> // For std::sqrt, std::pow, std::abs
#include <numeric> // For std::accumulate
#include <iostream>

class VWAPReversion : public Strategy {
private:
    // --- Parameters ---
    double deviation_multiplier_ = 2.0; // 'k' standard deviations for entry
    double target_position_size_ = 3.0; // Reduced from 100 to 3 shares

    // --- State per Symbol ---
    struct SymbolState {
        double cumulative_price_volume = 0.0;
        double cumulative_volume = 0.0;
        double current_vwap = 0.0;
        std::deque<double> price_vwap_diffs; // Store recent price-VWAP differences
        int rolling_stddev_window = 50; // Window for std dev calculation
    };
    std::map<std::string, SymbolState> symbol_state_;
    std::map<std::string, SignalDirection> current_signal_state_; // Track long/short/flat

public:
    VWAPReversion(double deviation_multiplier = 2.0, double target_pos_size = 3.0) // Changed default from 100 to 3
        : deviation_multiplier_(deviation_multiplier), target_position_size_(target_pos_size)
    {
         if (deviation_multiplier_ <= 0 || target_pos_size <= 0) {
             throw std::invalid_argument("Invalid parameters for VWAPReversion");
         }
    }

    void handle_market_event(const MarketEvent& event, EventQueue& queue) override {
        if (!portfolio_) return; // Need portfolio access

        for (const auto& pair : event.data) {
            const std::string& symbol = pair.first;
            const PriceBar& bar = pair.second;

            // Use a typical price for VWAP calculation (e.g., average of HLC)
            double typical_price = (bar.High + bar.Low + bar.Close) / 3.0;
            double volume = static_cast<double>(bar.Volume); // Need double for calculations

             // Skip bars with no volume as they don't contribute to VWAP meaningfully
             if (volume < 1e-9) continue;

            // --- Update VWAP State ---
            SymbolState& state = symbol_state_[symbol]; // Get/create state for symbol
            state.cumulative_price_volume += typical_price * volume;
            state.cumulative_volume += volume;

            if (state.cumulative_volume > 1e-9) {
                state.current_vwap = state.cumulative_price_volume / state.cumulative_volume;
            } else {
                state.current_vwap = typical_price; // Use current price if volume is zero initially
            }

             // --- Conservative Standard Deviation Calculation ---
             double price_vwap_diff = typical_price - state.current_vwap;
             state.price_vwap_diffs.push_back(price_vwap_diff);
             if (state.price_vwap_diffs.size() > static_cast<size_t>(state.rolling_stddev_window)) {
                 state.price_vwap_diffs.pop_front();
             }
             
             double standard_deviation = typical_price * 0.02; // Default to 2% of price as conservative fallback
             if (state.price_vwap_diffs.size() >= 30) { // Need more samples for stability
                 double mean_diff = std::accumulate(state.price_vwap_diffs.begin(), state.price_vwap_diffs.end(), 0.0) / state.price_vwap_diffs.size();
                 double sq_sum = 0.0;
                 for (double diff : state.price_vwap_diffs) {
                     sq_sum += std::pow(diff - mean_diff, 2);
                 }
                 double calculated_stddev = std::sqrt(sq_sum / (state.price_vwap_diffs.size() - 1));
                 // Use the larger of calculated std dev or 1% of price to prevent tight bands
                 standard_deviation = std::max(calculated_stddev, typical_price * 0.01);
             }


            // --- Signal Generation ---
            SignalDirection desired_signal = SignalDirection::FLAT;
            double upper_band = state.current_vwap + deviation_multiplier_ * standard_deviation;
            double lower_band = state.current_vwap - deviation_multiplier_ * standard_deviation;

            // Use Close price to check for band crossing
            if (bar.Close > upper_band) {
                desired_signal = SignalDirection::SHORT; // Price high, expect reversion down
            } else if (bar.Close < lower_band) {
                desired_signal = SignalDirection::LONG;  // Price low, expect reversion up
            }

            // --- Generate Orders based on Target ---
            if (desired_signal != current_signal_state_[symbol]) {
                 std::cout << "VWAP REVERSION: " << symbol << " @ " << formatTimestampUTC(event.timestamp)
                           << " Close=" << bar.Close << " VWAP=" << state.current_vwap
                           << " LowBand=" << lower_band << " UpBand=" << upper_band
                           << " Signal=" << (desired_signal == SignalDirection::LONG ? "LONG" : desired_signal == SignalDirection::SHORT ? "SHORT" : "FLAT")
                           << " StdDev=" << standard_deviation
                           << std::endl;

                double target_quantity = 0.0;
                if (desired_signal == SignalDirection::LONG) target_quantity = target_position_size_;
                else if (desired_signal == SignalDirection::SHORT) target_quantity = -target_position_size_;

                double current_quantity = portfolio_->get_position_quantity(symbol);
                double order_quantity_needed = target_quantity - current_quantity;

                if (std::abs(order_quantity_needed) > 1e-9) {
                    OrderDirection direction = (order_quantity_needed > 0) ? OrderDirection::BUY : OrderDirection::SELL;
                    double quantity_to_order = std::abs(order_quantity_needed);

                    std::cout << " -> Target: " << target_quantity << ", Current: " << current_quantity
                              << ", Order Qty: " << quantity_to_order << " " << (direction==OrderDirection::BUY?"BUY":"SELL") << std::endl;

                    send_event(std::make_shared<OrderEvent>(event.timestamp, symbol, OrderType::MARKET, direction, quantity_to_order), queue);
                } else {
                    std::cout << " -> Target: " << target_quantity << ", Current: " << current_quantity << ". No order needed." << std::endl;
                }
                current_signal_state_[symbol] = desired_signal;
            }
        } // end loop over symbols
    } // end handle_market_event
};