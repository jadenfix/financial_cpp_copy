#pragma once

#include "../core/Event.h"
#include "../core/EventQueue.h"
#include "../core/Portfolio.h" // Include Portfolio header
#include <string>
#include <vector>
#include <map>
#include <memory> // For std::shared_ptr
#include <cmath> // For std::floor

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

    // --- Helper for Strategies ---
    void set_portfolio(Portfolio* portfolio) { portfolio_ = portfolio; }

    // --- NEW: Helper for Position Sizing ---
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