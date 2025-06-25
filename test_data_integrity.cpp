#include "src/data/DataManager.h"
#include "src/strategies/MovingAverageCrossover.h"
#include "src/strategies/VWAPReversion.h" 
#include "src/strategies/PairsTrading.h"
#include "src/core/Backtester.h"
#include <iostream>
#include <cassert>
#include <iomanip>

void test_data_loading() {
    std::cout << "=== Testing Data Loading ===" << std::endl;
    
    DataManager dm;
    dm.setMaxRowsToLoad(1000); // Small sample
    
    // Test each dataset
    std::vector<std::string> datasets = {"data/stocks_april", "data/2024_only", "data/2024_2025"};
    
    for (const auto& dataset : datasets) {
        std::cout << "\nTesting dataset: " << dataset << std::endl;
        
        if (!dm.loadData(dataset)) {
            std::cerr << "FAILED to load " << dataset << std::endl;
            continue;
        }
        
        auto symbols = dm.getAllSymbols();
        std::cout << "Symbols loaded: ";
        for (const auto& sym : symbols) {
            std::cout << sym << " ";
        }
        std::cout << std::endl;
        
        // Test first few bars
        int bars_checked = 0;
        while (!dm.isDataFinished() && bars_checked < 10) {
            auto snapshot = dm.getNextBars();
            if (!snapshot.empty()) {
                auto time_t_val = std::chrono::system_clock::to_time_t(dm.getCurrentTime());
                std::cout << "Bar " << bars_checked << " @ " << std::put_time(std::gmtime(&time_t_val), "%Y-%m-%d %H:%M:%S") << ": ";
                for (const auto& [symbol, bar] : snapshot) {
                    std::cout << symbol << "={O:" << bar.Open << ",H:" << bar.High << ",L:" << bar.Low << ",C:" << bar.Close << ",V:" << bar.Volume << "} ";
                    
                    // Validate OHLC logic
                    if (bar.High < bar.Low) {
                        std::cerr << "ERROR: High < Low for " << symbol << std::endl;
                    }
                    if (bar.High < bar.Open || bar.High < bar.Close || bar.Low > bar.Open || bar.Low > bar.Close) {
                        std::cerr << "ERROR: OHLC validation failed for " << symbol << std::endl;
                    }
                    if (bar.Open <= 0 || bar.High <= 0 || bar.Low <= 0 || bar.Close <= 0) {
                        std::cerr << "ERROR: Non-positive prices for " << symbol << std::endl;
                    }
                }
                std::cout << std::endl;
                bars_checked++;
            }
        }
    }
}

void test_strategy_basic_logic() {
    std::cout << "\n=== Testing Strategy Basic Logic ===" << std::endl;
    
    // Test MovingAverageCrossover with controlled data
    try {
        MovingAverageCrossover mac(5, 20, 100.0);
        std::cout << "MovingAverageCrossover instantiated successfully" << std::endl;
        
        // Test invalid parameters
        try {
            MovingAverageCrossover bad_mac(20, 5, 100.0); // short > long
            std::cerr << "ERROR: Should have thrown for invalid parameters" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Good: Caught expected exception: " << e.what() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to create MovingAverageCrossover: " << e.what() << std::endl;
    }
    
    // Test VWAPReversion
    try {
        VWAPReversion vwap(2.0, 100.0);
        std::cout << "VWAPReversion instantiated successfully" << std::endl;
        
        try {
            VWAPReversion bad_vwap(-1.0, 100.0); // negative threshold
            std::cerr << "ERROR: Should have thrown for negative threshold" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Good: Caught expected exception: " << e.what() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to create VWAPReversion: " << e.what() << std::endl;
    }
    
    // Test PairsTrading
    try {
        PairsTrading pairs("MSFT", "NVDA", 60, 2.0, 0.5, 10000.0);
        std::cout << "PairsTrading instantiated successfully" << std::endl;
        
        try {
            PairsTrading bad_pairs("MSFT", "MSFT", 60, 2.0, 0.5, 10000.0); // same symbol
            std::cerr << "ERROR: Should have thrown for same symbols" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Good: Caught expected exception: " << e.what() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to create PairsTrading: " << e.what() << std::endl;
    }
}

void test_timestamp_parsing() {
    std::cout << "\n=== Testing Timestamp Parsing ===" << std::endl;
    
    // Test various timestamp formats that might be in our data
    std::vector<std::pair<std::string, std::string>> test_cases = {
        {"2024-04-01", "09:30:00"},
        {"2025-01-01", "16:00:00"},
        {"2024-12-31", "23:59:59"}
    };
    
    for (const auto& [date, time] : test_cases) {
        try {
            auto timestamp = PriceBar::stringToTimestamp(date, time);
            auto time_t_val = std::chrono::system_clock::to_time_t(timestamp);
            std::cout << "Parsed " << date << " " << time << " -> " 
                      << std::put_time(std::gmtime(&time_t_val), "%Y-%m-%d %H:%M:%S UTC") << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "ERROR parsing " << date << " " << time << ": " << e.what() << std::endl;
        }
    }
}

void test_single_strategy_run() {
    std::cout << "\n=== Testing Single Strategy Run ===" << std::endl;
    
    try {
        // Test with stocks_april (smaller dataset)
        auto strategy = std::make_unique<MovingAverageCrossover>(5, 20, 100.0);
        Backtester backtester("data/stocks_april", std::move(strategy), 100000.0);
        
        const Portfolio* result = backtester.run_and_get_portfolio();
        if (result) {
            auto summary = result->get_results_summary();
            std::cout << "Strategy run completed:" << std::endl;
            std::cout << "  Final Equity: $" << summary.final_equity << std::endl;
            std::cout << "  Total Return: " << summary.total_return_pct << "%" << std::endl;
            std::cout << "  Max Drawdown: " << summary.max_drawdown_pct << "%" << std::endl;
            std::cout << "  Fills: " << summary.num_fills << std::endl;
            std::cout << "  Realized PnL: $" << summary.realized_pnl << std::endl;
            std::cout << "  Commission: $" << summary.total_commission << std::endl;
            
            // Sanity checks
            if (summary.final_equity < 50000 || summary.final_equity > 200000) {
                std::cerr << "WARNING: Final equity seems unreasonable" << std::endl;
            }
            if (summary.max_drawdown_pct > 50) {
                std::cerr << "WARNING: Excessive drawdown" << std::endl;
            }
            if (summary.num_fills == 0) {
                std::cerr << "WARNING: No fills executed" << std::endl;
            }
        } else {
            std::cerr << "ERROR: Strategy run returned null portfolio" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR during strategy run: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "Financial Trading System - Data Integrity & Strategy Tests" << std::endl;
    std::cout << "=========================================================" << std::endl;
    
    try {
        test_timestamp_parsing();
        test_data_loading();
        test_strategy_basic_logic();
        test_single_strategy_run();
        
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Tests completed. Check output above for any ERRORs or WARNINGs." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 