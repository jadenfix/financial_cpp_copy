#pragma once

#include "Event.h"
#include "EventQueue.h"
#include "Utils.h" // Include for formatting
#include <string>
#include <map>
#include <memory>
#include <iostream> // For cout/cerr
#include <algorithm> // For std::max, std::abs

class ExecutionHandler {
private:
    EventQueue& event_queue_;
    std::map<std::string, double> last_known_prices_; // Cache last prices for each symbol

public:
    explicit ExecutionHandler(EventQueue& queue) : event_queue_(queue) {}

    void handle_order_event(const OrderEvent& order_event, const MarketEvent& next_market_event) {
        if (order_event.order_type == OrderType::MARKET) {
            auto symbol_iter = next_market_event.data.find(order_event.symbol);
            double fill_price = 0.0;
            bool can_fill = false;
            
            if (symbol_iter != next_market_event.data.end()) {
                // Current market data available - use it
                const PriceBar& next_bar = symbol_iter->second;
                fill_price = next_bar.Open;
                last_known_prices_[order_event.symbol] = fill_price; // Update cache
                can_fill = true;
            } else {
                // No current data - try to use last known price
                auto last_price_iter = last_known_prices_.find(order_event.symbol);
                if (last_price_iter != last_known_prices_.end()) {
                    fill_price = last_price_iter->second;
                    can_fill = true;
                    std::cout << "SIM EXEC: Using last known price " << fill_price 
                              << " for " << order_event.symbol << " (no current data)" << std::endl;
                }
            }

            if (can_fill) {
                double commission = calculate_commission(order_event.quantity, fill_price);

                std::cout << "SIM EXEC: Order for " << order_event.quantity << " " << order_event.symbol
                          << (order_event.direction == OrderDirection::BUY ? " BUY" : " SELL")
                          << " filled at " << fill_price << std::endl;

                EventPtr fill_ev = std::make_shared<FillEvent>(
                    next_market_event.timestamp,
                    order_event.symbol,
                    order_event.direction,
                    order_event.quantity,
                    fill_price,
                    commission
                );
                event_queue_.push(std::move(fill_ev));
            } else {
                std::cerr << "SIM EXEC WARN: No market data or last known price for " << order_event.symbol
                          << " at " << formatTimestampUTC(next_market_event.timestamp) 
                          << " to fill order." << std::endl;
            }
        } else {
             std::cerr << "SIM EXEC WARN: Limit orders not implemented." << std::endl;
        }
    }

    // Method to update price cache from market events (call from Backtester)
    void update_price_cache(const MarketEvent& market_event) {
        for (const auto& pair : market_event.data) {
            last_known_prices_[pair.first] = pair.second.Close;
        }
    }

private:
    // Example commission calculation
    double calculate_commission(double quantity, double price) {
        // Suppress warning if price is not used in this simple model
        (void)price; // Explicitly mark price as unused for now
        // Example: $0.005 per share, minimum $1.00
        double commission = std::abs(quantity) * 0.005;
        return std::max(1.0, commission);
    }
};