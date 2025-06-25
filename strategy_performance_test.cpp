#include "src/data/DataManager.h"
#include "src/strategies/MovingAverageCrossover.h"
#include "src/core/Backtester.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== Strategy Performance Validation Test ===" << std::endl;
    
    // Test 1: Symbol Name Extraction
    std::cout << "\n1. Testing Symbol Name Extraction:" << std::endl;
    DataManager dm;
    dm.setMaxRowsToLoad(100);
    
    if (dm.loadData("data/stocks_april")) {
        auto symbols = dm.getAllSymbols();
        std::cout << "Extracted symbols: ";
        for (const auto& sym : symbols) {
            std::cout << sym << " ";
        }
        std::cout << std::endl;
        
        // Should see: MSFT NVDA GOOG (not long filenames)
        bool correct_symbols = true;
        for (const auto& sym : symbols) {
            if (sym.length() > 10) {  // Long symbol names indicate problem
                correct_symbols = false;
                break;
            }
        }
        std::cout << "Symbol extraction: " << (correct_symbols ? "✓ PASS" : "✗ FAIL") << std::endl;
    }
    
    // Test 2: Position Sizing
    std::cout << "\n2. Testing Position Sizing (Small vs Large):" << std::endl;
    
    // Create two strategies with different position sizes
    auto small_strategy = std::make_unique<MovingAverageCrossover>(5, 20, 5.0);  // 5 shares
    auto large_strategy = std::make_unique<MovingAverageCrossover>(5, 20, 50.0); // 50 shares
    
    std::cout << "Created strategies with position sizes 5 and 50 shares" << std::endl;
    
    // Test 3: Quick Single Strategy Backtest
    std::cout << "\n3. Running Quick Single Strategy Test:" << std::endl;
    
    try {
        // Run a small backtest with limited data
        DataManager cached_data;
        if (cached_data.loadData("data/stocks_april")) {
            cached_data.setMaxRowsToLoad(200); // Very small test
            
            auto test_strategy = std::make_unique<MovingAverageCrossover>(3, 10, 5.0);
            Backtester backtester(cached_data, std::move(test_strategy), 10000.0, 500.0);
            
            Portfolio* result = backtester.run_and_get_portfolio();
            if (result) {
                auto summary = result->get_results_summary();
                std::cout << std::fixed << std::setprecision(2);
                std::cout << "  Final Equity: $" << summary.final_equity << std::endl;
                std::cout << "  Return: " << summary.total_return_pct << "%" << std::endl;
                std::cout << "  Fills: " << summary.num_fills << std::endl;
                std::cout << "  Max Drawdown: " << summary.max_drawdown_pct << "%" << std::endl;
                
                // Reasonable result check
                bool reasonable = (summary.total_return_pct > -50.0 && summary.total_return_pct < 50.0 && 
                                  summary.final_equity > 5000 && summary.final_equity < 20000);
                std::cout << "Result reasonableness: " << (reasonable ? "✓ PASS" : "✗ FAIL") << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cout << "Exception during test: " << e.what() << std::endl;
    }
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    return 0;
} 