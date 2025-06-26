#pragma once

#include "Strategy.h"
#include "core/Event.h"
#include "core/EventQueue.h"
#include "core/Utils.h"
#include "core/Portfolio.h"
#include "data/PriceBar.h"

#include <string>
#include <unordered_map>
#include <mutex>

class BuyAndHoldStrategy : public Strategy {
private:
    // Track which symbols we've already bought
    std::unordered_map<std::string, bool> positions_taken_;
    
    // Position size (percentage of portfolio)
    const double position_percent_;
    
    std::mutex mtx_;

public:
    explicit BuyAndHoldStrategy(double position_percent = 0.95) 
        : position_percent_(position_percent) {}

    void handle_market_event(const MarketEvent& event, EventQueue& queue) override {
        std::lock_guard<std::mutex> lock(mtx_);
        
        if (!portfolio_ || event.data.empty()) return;

        for (const auto& [symbol, bar] : event.data) {
            // If we haven't taken a position in this symbol yet, buy and hold
            if (positions_taken_.find(symbol) == positions_taken_.end()) {
                // Calculate position size based on available cash
                double cash = 100000.0; // Use initial cash amount since Portfolio doesn't have get_cash()
                double position_value = cash * position_percent_;
                double shares = position_value / bar.Close;
                
                if (shares > 0.01 && cash > position_value) { // Minimum viable position
                    auto order = std::make_shared<OrderEvent>(
                        bar.timestamp,
                        symbol,
                        OrderType::MARKET,
                        OrderDirection::BUY,
                        shares
                    );
                    
                    send_event(order, queue);
                    positions_taken_[symbol] = true;
                }
            }
        }
    }

    void handle_fill_event(const FillEvent& event, EventQueue& queue) override {
        // Buy and hold - no action needed on fills
    }

    std::string get_name() const override {
        return "BuyAndHold";
    }
}; 