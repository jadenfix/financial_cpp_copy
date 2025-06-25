#include "core/Backtester.h"
#include "strategies/Strategy.h"
// --- Include ALL strategy headers ---
#include "strategies/MovingAverageCrossover.h"
#include "strategies/VWAPReversion.h"
#include "strategies/OpeningRangeBreakout.h"
#include "strategies/MomentumIgnition.h"
#include "strategies/PairsTrading.h"
#include "strategies/LeadLagStrategy.h"

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <map>
#include <iomanip>
#include <functional> // For std::function
#include <filesystem> // For checking data dir existence

// --- StrategyResult struct defined in Portfolio.h ---
#include "core/Portfolio.h" // Make sure this is included

// --- Global Data Cache ---
std::map<std::string, std::unique_ptr<DataManager>> cached_data_managers;

// --- Helper Function to Build Data Path ---
std::string build_data_path(const std::string& base_dir, const std::string& subdir_name) {
    std::filesystem::path path = std::filesystem::path(base_dir) / subdir_name;
    return path.string();
}

// --- Helper Function to Get Cached DataManager ---
DataManager* get_cached_data_manager(const std::string& data_path) {
    auto it = cached_data_managers.find(data_path);
    if (it != cached_data_managers.end()) {
        std::cout << "Using cached data for: " << data_path << std::endl;
        return it->second.get();
    }
    
    // Load data for the first time
    std::cout << "Loading and caching data for: " << data_path << std::endl;
    auto data_manager = std::make_unique<DataManager>();
    if (!data_manager->loadData(data_path)) {
        std::cerr << "Failed to load data from: " << data_path << std::endl;
        return nullptr;
    }
    
    DataManager* ptr = data_manager.get();
    cached_data_managers[data_path] = std::move(data_manager);
    return ptr;
}

int main(int argc, char* argv[]) {
    // --- UPDATED TITLE ---
    std::cout << "--- HFT Backtesting System - COMPREHENSIVE Multi-Strategy & Multi-Dataset Testing ---" << std::endl;
    std::cout << "--- Running ALL 8 Algorithmic Trading Strategy Types with Multiple Variants ---" << std::endl;

    // --- Configuration ---
    std::string data_base_dir = "data";
    double initial_cash = 100000.0;

    // --- Define Datasets to Test ---
    std::vector<std::string> datasets_to_test = {
        "stocks_april",
        "2024_only",
        "2024_2025"
    };

    // --- Map to Store All Results ---
    // Key: Combined name like "StrategyName_on_DataSetName"
    std::map<std::string, StrategyResult> all_results;

    // --- OUTER LOOP: Iterate Through Datasets ---
    for (const std::string& target_dataset_subdir : datasets_to_test) {

        std::cout << "\n\n ///////////////////////////////////////////////////////////" << std::endl;
        std::cout << " ///// Starting Tests for Dataset: " << target_dataset_subdir << " /////" << std::endl;
        std::cout << " ///////////////////////////////////////////////////////////" << std::endl;

        // --- Build Final Data Path & Check Existence ---
        std::string data_path = build_data_path(data_base_dir, target_dataset_subdir);
        std::cout << "Using data path: " << data_path << std::endl;
        if (!std::filesystem::exists(data_path) || !std::filesystem::is_directory(data_path)) {
            std::cerr << "ERROR: Data directory '" << data_path << "' not found. Skipping dataset." << std::endl;
            continue; // Skip to the next dataset
        }

        // --- Get or Load Cached Data ---
        DataManager* cached_data = get_cached_data_manager(data_path);
        if (!cached_data) {
            std::cerr << "ERROR: Failed to load data for '" << data_path << "'. Skipping dataset." << std::endl;
            continue;
        }

        // --- Define Symbol Names BASED ON CURRENT DATASET ---
        // Reset symbols for each dataset iteration
        std::string msft_sym = "", nvda_sym = "", goog_sym = "";
        std::string btc_sym = "", eth_sym = "", sol_sym = "", ada_sym = "";

        if (target_dataset_subdir == "stocks_april") {
            std::cout << "Loading symbols for dataset: stocks_april" << std::endl;
            msft_sym = "quant_seconds_data_MSFT";
            nvda_sym = "quant_seconds_data_NVDA";
            goog_sym = "quant_seconds_data_google";
        } else if (target_dataset_subdir == "2024_only") {
            std::cout << "Loading symbols for dataset: 2024_only" << std::endl;
            btc_sym = "btc_2024_data";
            eth_sym = "eth_2024_data";
            sol_sym = "sol_2024_data";
            ada_sym = "ada_2024_data";
        } else if (target_dataset_subdir == "2024_2025") {
            std::cout << "Loading symbols for dataset: 2024_2025" << std::endl;
            btc_sym = "2024_to_april_2025_btc_data";
            eth_sym = "2024_to_april_2025_eth_data";
            sol_sym = "2024_to_april_2025_solana_data"; // Match exact filename stem
            ada_sym = "2024_to_april_2025_ada_data";
        } else {
            std::cerr << "Warning: Unknown dataset subdirectory '" << target_dataset_subdir << "' encountered in loop logic." << std::endl;
            continue; // Skip dataset
        }

        // --- Define All Available Strategy Configurations FOR THIS DATASET ITERATION ---
        // Need to redefine inside loop because lambda captures [&] use current symbol values
        struct StrategyConfig {
            std::string name;
            std::function<std::unique_ptr<Strategy>()> factory;
            std::vector<std::string> required_datasets; // Datasets this config applies to
        };
        std::vector<StrategyConfig> available_strategies_this_iteration;

        // === 1. MOVING AVERAGE CROSSOVER STRATEGIES (Multiple variants) ===
        available_strategies_this_iteration.push_back({"MACrossover_5_20", [](){ return std::make_unique<MovingAverageCrossover>(5, 20, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});
        available_strategies_this_iteration.push_back({"MACrossover_10_50", [](){ return std::make_unique<MovingAverageCrossover>(10, 50, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});
        available_strategies_this_iteration.push_back({"MACrossover_20_100", [](){ return std::make_unique<MovingAverageCrossover>(20, 100, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});
        available_strategies_this_iteration.push_back({"MACrossover_3_15", [](){ return std::make_unique<MovingAverageCrossover>(3, 15, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});

        // === 2. VWAP REVERSION STRATEGIES (Multiple thresholds) ===
        available_strategies_this_iteration.push_back({"VWAP_1.5", [](){ return std::make_unique<VWAPReversion>(1.5, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});
        available_strategies_this_iteration.push_back({"VWAP_2.0", [](){ return std::make_unique<VWAPReversion>(2.0, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});
        available_strategies_this_iteration.push_back({"VWAP_2.5", [](){ return std::make_unique<VWAPReversion>(2.5, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});
        available_strategies_this_iteration.push_back({"VWAP_3.0", [](){ return std::make_unique<VWAPReversion>(3.0, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});

        // === 3. OPENING RANGE BREAKOUT STRATEGIES (Multiple timeframes) ===
        available_strategies_this_iteration.push_back({"ORB_15", [](){ return std::make_unique<OpeningRangeBreakout>(15, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});
        available_strategies_this_iteration.push_back({"ORB_30", [](){ return std::make_unique<OpeningRangeBreakout>(30, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});
        available_strategies_this_iteration.push_back({"ORB_60", [](){ return std::make_unique<OpeningRangeBreakout>(60, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});
        available_strategies_this_iteration.push_back({"ORB_120", [](){ return std::make_unique<OpeningRangeBreakout>(120, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});

        // === 4. MOMENTUM IGNITION STRATEGIES (Multiple configurations) ===
        available_strategies_this_iteration.push_back({"Momentum_5_10_2_3", [](){ return std::make_unique<MomentumIgnition>(5, 10, 2.0, 3, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});
        available_strategies_this_iteration.push_back({"Momentum_3_8_1.5_2", [](){ return std::make_unique<MomentumIgnition>(3, 8, 1.5, 2, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});
        available_strategies_this_iteration.push_back({"Momentum_10_20_2.5_5", [](){ return std::make_unique<MomentumIgnition>(10, 20, 2.5, 5, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});
        available_strategies_this_iteration.push_back({"Momentum_7_15_3.0_4", [](){ return std::make_unique<MomentumIgnition>(7, 15, 3.0, 4, 100.0); }, {"stocks_april", "2024_only", "2024_2025"}});

        // === 5. PAIRS TRADING STRATEGIES (All possible pairs with multiple configurations) ===
        
        // Multiple parameter sets for pairs trading
        struct PairsConfig {
            size_t lookback;
            double entry_z;
            double exit_z;
            double trade_value;
            std::string suffix;
        };
        std::vector<PairsConfig> pairs_configs = {
            {60, 2.0, 0.5, 10000.0, "Standard"},
            {30, 1.5, 0.3, 10000.0, "Sensitive"},
            {120, 2.5, 0.8, 10000.0, "Conservative"},
            {40, 1.8, 0.4, 10000.0, "Balanced"}
        };

        // Stock pairs (for stocks_april dataset)
        for (const auto& pc : pairs_configs) {
            available_strategies_this_iteration.push_back({
                "Pairs_MSFT_NVDA_" + pc.suffix,
                [&, pc](){ return std::make_unique<PairsTrading>(msft_sym, nvda_sym, pc.lookback, pc.entry_z, pc.exit_z, pc.trade_value); },
                {"stocks_april"}
            });
            available_strategies_this_iteration.push_back({
                "Pairs_NVDA_GOOG_" + pc.suffix,
                [&, pc](){ return std::make_unique<PairsTrading>(nvda_sym, goog_sym, pc.lookback, pc.entry_z, pc.exit_z, pc.trade_value); },
                {"stocks_april"}
            });
            available_strategies_this_iteration.push_back({
                "Pairs_MSFT_GOOG_" + pc.suffix,
                [&, pc](){ return std::make_unique<PairsTrading>(msft_sym, goog_sym, pc.lookback, pc.entry_z, pc.exit_z, pc.trade_value); },
                {"stocks_april"}
            });
        }

        // Crypto pairs (for 2024_only and 2024_2025 datasets)
        for (const auto& pc : pairs_configs) {
            available_strategies_this_iteration.push_back({
                "Pairs_BTC_ETH_" + pc.suffix,
                [&, pc](){ return std::make_unique<PairsTrading>(btc_sym, eth_sym, pc.lookback, pc.entry_z, pc.exit_z, pc.trade_value); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "Pairs_ETH_SOL_" + pc.suffix,
                [&, pc](){ return std::make_unique<PairsTrading>(eth_sym, sol_sym, pc.lookback, pc.entry_z, pc.exit_z, pc.trade_value); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "Pairs_BTC_SOL_" + pc.suffix,
                [&, pc](){ return std::make_unique<PairsTrading>(btc_sym, sol_sym, pc.lookback, pc.entry_z, pc.exit_z, pc.trade_value); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "Pairs_ETH_ADA_" + pc.suffix,
                [&, pc](){ return std::make_unique<PairsTrading>(eth_sym, ada_sym, pc.lookback, pc.entry_z, pc.exit_z, pc.trade_value); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "Pairs_SOL_ADA_" + pc.suffix,
                [&, pc](){ return std::make_unique<PairsTrading>(sol_sym, ada_sym, pc.lookback, pc.entry_z, pc.exit_z, pc.trade_value); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "Pairs_BTC_ADA_" + pc.suffix,
                [&, pc](){ return std::make_unique<PairsTrading>(btc_sym, ada_sym, pc.lookback, pc.entry_z, pc.exit_z, pc.trade_value); },
                {"2024_only", "2024_2025"}
            });
        }

        // === 6. LEAD-LAG STRATEGIES (Multiple configurations) ===
        
        // Multiple parameter sets for lead-lag
        struct LeadLagConfig {
            size_t window;
            size_t lag;
            double correlation_threshold;
            double return_threshold;
            double trade_size;
            std::string suffix;
        };
        std::vector<LeadLagConfig> leadlag_configs = {
            {30, 1, 0.5, 0.0002, 100.0, "Fast"},
            {60, 2, 0.6, 0.0003, 100.0, "Medium"},
            {120, 3, 0.7, 0.0005, 100.0, "Slow"},
            {20, 1, 0.4, 0.0001, 100.0, "Aggressive"}
        };

        // Stock lead-lag pairs (for stocks_april dataset)
        for (const auto& llc : leadlag_configs) {
            available_strategies_this_iteration.push_back({
                "LeadLag_MSFT->NVDA_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(msft_sym, nvda_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"stocks_april"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_NVDA->MSFT_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(nvda_sym, msft_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"stocks_april"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_GOOG->MSFT_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(goog_sym, msft_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"stocks_april"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_MSFT->GOOG_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(msft_sym, goog_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"stocks_april"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_NVDA->GOOG_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(nvda_sym, goog_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"stocks_april"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_GOOG->NVDA_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(goog_sym, nvda_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"stocks_april"}
            });
        }

        // Crypto lead-lag pairs (for 2024_only and 2024_2025 datasets)
        for (const auto& llc : leadlag_configs) {
            available_strategies_this_iteration.push_back({
                "LeadLag_BTC->ETH_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(btc_sym, eth_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_ETH->BTC_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(eth_sym, btc_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_ETH->SOL_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(eth_sym, sol_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_SOL->ETH_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(sol_sym, eth_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_BTC->SOL_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(btc_sym, sol_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_SOL->BTC_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(sol_sym, btc_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_ETH->ADA_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(eth_sym, ada_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_ADA->ETH_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(ada_sym, eth_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_BTC->ADA_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(btc_sym, ada_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_ADA->BTC_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(ada_sym, btc_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_SOL->ADA_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(sol_sym, ada_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"2024_only", "2024_2025"}
            });
            available_strategies_this_iteration.push_back({
                "LeadLag_ADA->SOL_" + llc.suffix,
                [&, llc](){ return std::make_unique<LeadLagStrategy>(ada_sym, sol_sym, llc.window, llc.lag, llc.correlation_threshold, llc.return_threshold, llc.trade_size); },
                {"2024_only", "2024_2025"}
            });
        }

        // --- Filter strategies applicable to the CURRENT dataset ---
        std::vector<StrategyConfig> strategies_to_run_this_dataset;
        for (const auto& config : available_strategies_this_iteration) {
             bool applicable = false;
             for (const auto& ds_name : config.required_datasets) {
                 if (ds_name == target_dataset_subdir) {
                     applicable = true;
                     break;
                 }
             }
             if (applicable) {
                 strategies_to_run_this_dataset.push_back(config);
             }
        }

        if (strategies_to_run_this_dataset.empty()) {
            std::cout << "No applicable strategies found for dataset '" << target_dataset_subdir << "'. Skipping dataset." << std::endl;
            continue;
        }
         std::cout << "Preparing to run " << strategies_to_run_this_dataset.size() << " strategies for dataset '" << target_dataset_subdir << "'." << std::endl;
         
         // Print strategy breakdown for this dataset
         std::map<std::string, int> strategy_type_count;
         for (const auto& config : strategies_to_run_this_dataset) {
             std::string type = config.name.substr(0, config.name.find('_'));
             strategy_type_count[type]++;
         }
         std::cout << "Strategy Type Breakdown:" << std::endl;
         for (const auto& pair : strategy_type_count) {
             std::cout << "  " << pair.first << ": " << pair.second << " variants" << std::endl;
         }
         std::cout << std::endl;

        // --- INNER LOOP: Iterate Through Applicable Strategies for this Dataset ---
        for (const auto& config : strategies_to_run_this_dataset) {
            std::cout << "\n\n===== Running Strategy: " << config.name << " on Dataset: " << target_dataset_subdir << " =====" << std::endl;

            std::unique_ptr<Strategy> strategy = nullptr;
            try {
                 strategy = config.factory(); // Create strategy using the factory
            } catch (const std::exception& e) {
                std::cerr << "Error creating strategy '" << config.name << "': " << e.what() << ". Skipping." << std::endl;
                continue; // Skip to next strategy
            }
            if (!strategy) { continue; } // Should not happen with factory, but safety check

            // Create a new Backtester for each specific run - but use cached data
            Backtester backtester(*cached_data, std::move(strategy), initial_cash);
            Portfolio const* result_portfolio = nullptr;

            try {
                result_portfolio = backtester.run_and_get_portfolio();
            } catch (const std::exception& e) {
                std::cerr << "FATAL ERROR during backtest for '" << config.name << "' on '" << target_dataset_subdir << "': " << e.what() << std::endl;
                continue; // Skip to next strategy
            } catch (...) {
                std::cerr << "FATAL UNKNOWN ERROR during backtest for '" << config.name << "' on '" << target_dataset_subdir << "'." << std::endl;
                continue; // Skip to next strategy
            }

            // Store results using a combined key
            if (result_portfolio) {
                std::string result_key = config.name + "_on_" + target_dataset_subdir;
                all_results[result_key] = result_portfolio->get_results_summary();
            } else {
                 std::cerr << "Warning: Backtest ran but portfolio pointer was null for " << config.name << " on " << target_dataset_subdir << "." << std::endl;
            }
            std::cout << "===== Finished Strategy: " << config.name << " on " << target_dataset_subdir << " =====" << std::endl;

        } // End INNER strategy loop

    } // End OUTER dataset loop

    // --- Print Combined Comparison Table ---
    if (!all_results.empty()) {
        std::cout << "\n\n===== COMBINED Strategy Comparison Results =====" << std::endl;
        std::cout << std::left << std::setw(50) << "Strategy (on Dataset)" // Wider column for combined name
                  << std::right << std::setw(15) << "Return (%)"
                  << std::right << std::setw(15) << "Max DD (%)"
                  << std::right << std::setw(15) << "Realized PnL"
                  << std::right << std::setw(15) << "Commission"
                  << std::right << std::setw(10) << "Fills"
                  << std::right << std::setw(18) << "Final Equity"
                  << std::endl;
        std::cout << std::string(138, '-') << std::endl; // Adjust separator width

        for (const auto& pair : all_results) {
            const std::string& name = pair.first; // Combined name
            const StrategyResult& res = pair.second;
            std::cout << std::left << std::setw(50) << name // Wider column
                      << std::fixed << std::setprecision(2)
                      << std::right << std::setw(15) << res.total_return_pct
                      << std::right << std::setw(15) << res.max_drawdown_pct
                      << std::right << std::setw(15) << res.realized_pnl
                      << std::right << std::setw(15) << res.total_commission
                      << std::right << std::setw(10) << res.num_fills
                      << std::right << std::setw(18) << res.final_equity
                      << std::endl;
        }
        std::cout << std::string(138, '=') << std::endl; // Adjust end separator
    } else {
         std::cout << "\nNo strategy results to display." << std::endl;
    }

    std::cout << "\n--- Comprehensive Run Invocation Complete ---" << std::endl;
    return 0;
}