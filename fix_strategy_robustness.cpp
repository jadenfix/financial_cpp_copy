#include "src/strategies/MovingAverageCrossover.h"
#include "src/strategies/VWAPReversion.h"
#include "src/strategies/OpeningRangeBreakout.h"
#include "src/strategies/MomentumIgnition.h"
#include "src/core/Backtester.h"
#include <iostream>

void demo_improved_strategies() {
    std::cout << "=== Strategy Robustness Fixes Demo ===" << std::endl;
    
    std::cout << "\n1. Issues Found:" << std::endl;
    std::cout << "   - Over-trading: 33 trades per bar" << std::endl;
    std::cout << "   - Negative cash positions" << std::endl;
    std::cout << "   - High commission costs" << std::endl;
    std::cout << "   - MA crossovers too sensitive" << std::endl;
    
    std::cout << "\n2. Recommended Fixes:" << std::endl;
    std::cout << "   - Add minimum time between trades (cooldown)" << std::endl;
    std::cout << "   - Implement minimum price movement threshold" << std::endl;
    std::cout << "   - Add cash availability checks" << std::endl;
    std::cout << "   - Use longer MA periods for less noise" << std::endl;
    std::cout << "   - Reduce position sizes further" << std::endl;
    
    std::cout << "\n3. Better Strategy Parameters:" << std::endl;
    std::cout << "   - MA Cross: (10,30), (20,50) instead of (3,10), (5,20)" << std::endl;
    std::cout << "   - Position size: 2-3 shares max for $400+ stocks" << std::endl;
    std::cout << "   - VWAP threshold: 3.0+ instead of 1.5" << std::endl;
    std::cout << "   - ORB period: 60+ minutes instead of 15" << std::endl;
    
    std::cout << "\n4. Risk Management Improvements:" << std::endl;
    std::cout << "   - Maximum position value: 5% of portfolio" << std::endl;
    std::cout << "   - Stop-loss at 2% per trade" << std::endl;
    std::cout << "   - Maximum trades per day limit" << std::endl;
    std::cout << "   - Cash buffer: 20% minimum" << std::endl;
    
    DataManager dm;
    if (dm.loadData("data/stocks_april")) {
        dm.setMaxRowsToLoad(50);  // Very small test
        
        // Test improved parameters
        auto conservative_ma = std::make_unique<MovingAverageCrossover>(20, 50, 2.0);  // Longer periods, smaller size
        
        Backtester backtester(dm, std::move(conservative_ma), 10000.0, 2000.0);  // Higher cash buffer
        
        std::cout << "\n5. Running Conservative MA Strategy Test..." << std::endl;
        Portfolio* result = backtester.run_and_get_portfolio();
        
        if (result) {
            auto summary = result->get_results_summary();
            std::cout << "Conservative Results:" << std::endl;
            std::cout << "  Return: " << summary.total_return_pct << "%" << std::endl;
            std::cout << "  Fills: " << summary.num_fills << std::endl;
            std::cout << "  Commission: $" << summary.total_commission << std::endl;
            std::cout << "  Final Equity: $" << summary.final_equity << std::endl;
        }
    }
    
    std::cout << "\n=== Next Steps ===" << std::endl;
    std::cout << "1. Update main.cpp with conservative parameters" << std::endl;
    std::cout << "2. Add cooldown periods to strategy base class" << std::endl;
    std::cout << "3. Implement better risk management" << std::endl;
    std::cout << "4. Test with realistic position sizing" << std::endl;
    std::cout << "5. Add strategy performance validation" << std::endl;
}

int main() {
    demo_improved_strategies();
    return 0;
} 