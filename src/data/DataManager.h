#pragma once

#include <string>
#include <vector>
#include <unordered_map> // Still uses unordered_map internally
#include <chrono>
#include <optional>
#include <filesystem>
#include <functional>
#include <memory> // Include for EventPtr potentially later? Or just use Event.h?
#include <limits>
#include <map>
#include <sstream>
#include <iomanip>
#include <ctime>

#include "data/PriceBar.h" // Correct path
#include "core/Event.h"    // Include for DataSnapshot definition and Event types

// Removed the duplicate 'using DataSnapshot = ...;' line

class DataManager {
public:
    DataManager() : max_rows_to_load_(std::numeric_limits<size_t>::max()), 
                   streaming_mode_(false), warmup_buffer_size_(200) {}
    bool loadData(const std::string& dataPath);
    std::optional<std::reference_wrapper<const std::vector<PriceBar>>> getAssetData(const std::string& symbol) const;
    std::vector<std::string> getAllSymbols() const;

    // Changed: This now returns the snapshot directly for the Backtester to wrap in an event
    DataSnapshot getNextBars();

    std::chrono::system_clock::time_point getCurrentTime() const;
    bool isDataFinished() const;

    // NEW: Setter to limit maximum rows to load (for testing)
    void setMaxRowsToLoad(size_t max_rows) { max_rows_to_load_ = max_rows; }
    size_t getMaxRowsToLoad() const { return max_rows_to_load_; }

    // Enable streaming mode with state preservation
    void enableStreamingMode(size_t warmup_buffer = 200) {
        streaming_mode_ = true;
        warmup_buffer_size_ = warmup_buffer;
    }
    
    // Load data with continuity preservation
    bool loadDataWithContinuity(const std::string& data_dir, size_t chunk_start = 0, size_t chunk_size = 0);
    
    // Get warmup data for strategy initialization
    std::vector<PriceBar> getWarmupData(const std::string& symbol, size_t lookback) const;

private:
    // Internal storage remains unordered_map for performance
    std::unordered_map<std::string, std::vector<PriceBar>> historicalData_;
    std::unordered_map<std::string, size_t> currentIndices_;
    std::chrono::system_clock::time_point currentTime_ = std::chrono::system_clock::time_point::min();
    std::vector<std::string> symbols_;
    bool dataLoaded_ = false;
    size_t max_rows_to_load_;
    
    // State preservation for streaming
    std::map<std::string, size_t> last_processed_index_;
    std::map<std::string, PriceBar> last_bar_per_symbol_;
    bool streaming_mode_;
    size_t warmup_buffer_size_;

    // --- Private Helper Methods ---
    std::string extractSymbolFromFilename(const std::string& filename) const;
    bool parseCsvFile(const std::string& filename);
    
    // Streaming support methods
    bool parseCsvFileWithContinuity(const std::string& file_path, size_t chunk_start, size_t chunk_size);
    
    template<typename RowType>
    PriceBar parseRowToBar(const RowType& row, const std::string& symbol) {
        std::vector<std::string> cells;
        for (const auto& cell : row) {
            std::string cell_view;
            cell.read_value(cell_view);
            cells.push_back(cell_view);
        }
        
        if (cells.size() < 6) {
            throw std::runtime_error("Insufficient columns in CSV row");
        }
        
        // Parse timestamp
        std::tm tm = {};
        std::string timestamp_str = cells[0];
        std::istringstream ss(timestamp_str);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        
        if (ss.fail()) {
            throw std::runtime_error("Failed to parse timestamp: " + timestamp_str);
        }
        
        std::time_t timestamp = std::mktime(&tm);
        
        // Parse OHLCV values with validation
        double open = std::stod(cells[1]);
        double high = std::stod(cells[2]);
        double low = std::stod(cells[3]);
        double close = std::stod(cells[4]);
        double volume = std::stod(cells[5]);
        
        // Basic data validation
        if (high < low || open <= 0 || close <= 0 || volume < 0) {
            throw std::runtime_error("Invalid OHLCV data");
        }
        
        auto chrono_timestamp = std::chrono::system_clock::from_time_t(timestamp);
        return PriceBar{chrono_timestamp, open, high, low, close, static_cast<long long>(volume)};
    }
    
    void initializeSimulationState();
};