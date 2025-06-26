// --- Include necessary headers at the top ---
#include "DataManager.h"
#include "PriceBar.h"
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <cctype>
#include "csv2/reader.hpp"

namespace fs = std::filesystem;

// --- Enhanced extractSymbolFromFilename ---
std::string DataManager::extractSymbolFromFilename(const std::string& filename) const {
    fs::path filePath(filename);
    if (!filePath.has_stem()) {
        return "";
    }
    
    std::string stem = filePath.stem().string();
    
    // Extract clean symbol from various filename patterns
    // Pattern 1: quant_seconds_data_SYMBOL.csv -> SYMBOL
    if (stem.find("quant_seconds_data_") == 0) {
        std::string symbol = stem.substr(19); // Skip "quant_seconds_data_"
        // Handle special cases
        if (symbol == "google") return "GOOG";
        return symbol;
    }
    
    // Pattern 2: SYMBOL_2024_data.csv -> SYMBOL  
    size_t data_pos = stem.find("_2024_data");
    if (data_pos != std::string::npos) {
        return stem.substr(0, data_pos);
    }
    
    // Pattern 3: 2024_to_april_2025_SYMBOL_data.csv -> SYMBOL
    if (stem.find("2024_to_april_2025_") == 0) {
        std::string remainder = stem.substr(19); // Skip "2024_to_april_2025_"
        size_t data_pos = remainder.find("_data");
        if (data_pos != std::string::npos) {
            return remainder.substr(0, data_pos);
        }
        return remainder;
    }
    
    // Default: return the stem as-is
    return stem;
}


// --- REVISED parseCsvFile ---
bool DataManager::parseCsvFile(const std::string& filename) {
    fs::path filePath(filename);
    std::string symbol = extractSymbolFromFilename(filename);
    if (symbol.empty()) {
        std::cerr << "Could not extract symbol from filename: " << filename << std::endl;
        return false;
    }
    csv2::Reader<csv2::delimiter<','>,
                 csv2::quote_character<'"'>,
                 csv2::first_row_is_header<true>,
                 csv2::trim_policy::trim_whitespace> csv;

    if (!csv.mmap(filePath.string())) {
        std::cerr << "      Error: Failed to memory map file: " << filePath << std::endl;
        return false;
    }

    std::vector<PriceBar> barsForSymbol;
    size_t rowNumber = 1; // After header

    const int OPEN_IDX = 0, HIGH_IDX = 1, LOW_IDX = 2, CLOSE_IDX = 3,
              VOLUME_IDX = 4, DATE_IDX = 5, TIME_IDX = 6;
    const size_t EXPECTED_COLUMNS = 7;

    // Iterate through each row provided by the reader
    for (const auto& row : csv) {
        rowNumber++;
        std::vector<std::string> cells; // Temporary vector to hold cell values for this row
        cells.reserve(EXPECTED_COLUMNS); // Optional: pre-allocate space

        try {
            // Iterate through cells *within* the current row
            for (const auto& cell : row) {
                std::string cellValue;
                cell.read_value(cellValue); // Read value from the cell object
                cells.push_back(cellValue);
            }

            // Now check if we got the expected number of columns
            if (cells.size() != EXPECTED_COLUMNS) {
                // Only warn if not an empty line (common at end of files)
                if (cells.size() > 0) {
                    std::cerr << "      Warning: Skipping row " << rowNumber << " in " << filePath.filename().string()
                              << ". Expected " << EXPECTED_COLUMNS << " columns, found " << cells.size() << "." << std::endl;
                }
                continue; // Skip this row
            }

            // Access data from the temporary 'cells' vector using indices
            const std::string& openStr   = cells[OPEN_IDX];
            const std::string& highStr   = cells[HIGH_IDX];
            const std::string& lowStr    = cells[LOW_IDX];
            const std::string& closeStr  = cells[CLOSE_IDX];
            const std::string& volumeStr = cells[VOLUME_IDX];
            const std::string& dateStr   = cells[DATE_IDX];
            const std::string& timeStr   = cells[TIME_IDX];

            // --- Convert strings ---
            auto timestamp = PriceBar::stringToTimestamp(dateStr, timeStr); // Uses M/D/YY H:MM:SS
            double open = std::stod(openStr);
            double high = std::stod(highStr);
            double low = std::stod(lowStr);
            double close = std::stod(closeStr);
            long long volume = std::stoll(volumeStr);

            // --- Basic Data Validation ---
            bool valid = true;
            std::string validationError;
            if (open <= 0 || high <= 0 || low <= 0 || close <= 0 || volume < 0) {
                valid = false;
                validationError = "Non-positive price or negative volume.";
            } else if (high < low) {
                valid = false;
                validationError = "High (" + highStr + ") < Low (" + lowStr + ").";
            } else if (high < open || high < close || low > open || low > close) {
                 valid = false;
                 validationError = "O/C outside H/L range.";
            }

            if (!valid) {
                 std::cerr << "      Warning: Skipping row " << rowNumber << " in " << filePath.filename().string()
                           << ". Validation failed: " << validationError << " (O=" << openStr << ", H=" << highStr
                           << ", L=" << lowStr << ", C=" << closeStr << ", V=" << volumeStr << ")" << std::endl;
                 continue;
            }

            barsForSymbol.emplace_back(PriceBar{timestamp, open, high, low, close, volume});

            // NEW: Stop loading after reaching max_rows_to_load_
            if (barsForSymbol.size() >= max_rows_to_load_) {
                // Optional: print once per file
                std::cout << "      Reached row limit (" << max_rows_to_load_ << ") for " << symbol << ". Truncating data." << std::endl;
                break;
            }

        // Catch exceptions from string conversions or timestamp parsing
        } catch (const std::exception& e) {
             std::cerr << "      Warning: Skipping row " << rowNumber << " in " << filePath.filename().string()
                       << ". Exception during processing: " << e.what() << std::endl;
        }
    } // End row loop

    if (!barsForSymbol.empty()) {
        std::sort(barsForSymbol.begin(), barsForSymbol.end(),
                  [](const PriceBar& a, const PriceBar& b) {
                      return a.timestamp < b.timestamp;
                  });
        historicalData_[symbol] = std::move(barsForSymbol);
        symbols_.push_back(symbol);
        std::cout << "      Successfully parsed and stored " << historicalData_[symbol].size() << " valid bars for " << symbol << "." << std::endl;
        return true;
    } else {
         std::cerr << "      Warning: No valid price bars stored from file: " << filePath.filename().string() << std::endl;
         return true;
    }
}

// --- IMPORTANT: Make sure the rest of the DataManager methods ---
// --- (initializeSimulationState, loadData, getAssetData, getAllSymbols, ---
// ---  getNextBars, getCurrentTime, isDataFinished) ---
// --- are present and correct from the previous successful compilation attempt ---
// --- (specifically ensuring they use || for logical OR etc.) ---

// ... (Paste the rest of the DataManager methods here from the previous answer) ...

void DataManager::initializeSimulationState() {
    if (historicalData_.empty() || symbols_.empty()) {
        std::cerr << "Warning: No historical data loaded/symbols found. Cannot initialize simulation state." << std::endl;
        currentTime_ = std::chrono::system_clock::time_point::min();
        dataLoaded_ = false;
        return;
    }
    currentTime_ = std::chrono::system_clock::time_point::max();
    bool foundAnyData = false;
    for (const auto& symbol : symbols_) {
        if (historicalData_.count(symbol) && !historicalData_.at(symbol).empty()) {
            currentTime_ = std::min(currentTime_, historicalData_.at(symbol).front().timestamp);
            foundAnyData = true;
        }
    }
    if (!foundAnyData) {
        std::cerr << "Warning: Data files processed, but no valid bars found. Cannot initialize simulation time." << std::endl;
        currentTime_ = std::chrono::system_clock::time_point::min();
        dataLoaded_ = false;
        symbols_.clear();
        historicalData_.clear();
        return;
    }
    currentIndices_.clear();
    for (const auto& symbol : symbols_) {
        if (historicalData_.count(symbol)) {
             currentIndices_[symbol] = 0;
        }
    }
    std::sort(symbols_.begin(), symbols_.end());
    dataLoaded_ = true;
}

bool DataManager::loadData(const std::string& dataPath) {
    fs::path dirPath(dataPath);
    dataLoaded_ = false;
    historicalData_.clear();
    symbols_.clear();
    currentIndices_.clear();
    currentTime_ = std::chrono::system_clock::time_point::min();
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        std::cerr << "Error: Data path does not exist or is not a directory: " << dataPath << std::endl;
        return false;
    }
    std::cout << "Loading data from: " << dataPath << std::endl;
    bool anyFileParsedSuccessfullyWithData = false;
    try {
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            const auto& path = entry.path();
            if (entry.is_regular_file()) {
                std::string ext = path.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(),
                              [](unsigned char c){ return std::tolower(c); });
                if (ext == ".csv") {
                    std::string symbol = extractSymbolFromFilename(path.string());
                    if (!symbol.empty()) {
                        std::cout << "  Parsing file: " << path.filename().string() << " for symbol: " << symbol << std::endl;
                        if (parseCsvFile(path.string())) {
                            if (historicalData_.count(symbol) && !historicalData_.at(symbol).empty()) {
                                anyFileParsedSuccessfullyWithData = true;
                            }
                        } else {
                            std::cerr << "  Critical error parsing file: " << path.filename().string() << ". Skipping." << std::endl;
                             if (historicalData_.count(symbol)) {
                                 historicalData_.erase(symbol);
                                 symbols_.erase(std::remove(symbols_.begin(), symbols_.end(), symbol), symbols_.end());
                             }
                        }
                    } else {
                        std::cerr << "  Warning: Could not extract symbol from filename: " << path.filename().string() << ". Skipping." << std::endl;
                    }
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error while iterating directory " << dataPath << ": " << e.what() << std::endl;
        return false;
    }
    if (anyFileParsedSuccessfullyWithData) {
        initializeSimulationState();
        if (dataLoaded_) {
             std::cout << "Data loading complete. Initial simulation time: ";
             if (currentTime_ != std::chrono::system_clock::time_point::min()) {
                 auto time_t_currentTime = std::chrono::system_clock::to_time_t(currentTime_);
                 std::cout << std::put_time(std::gmtime(&time_t_currentTime), "%Y-%m-%d %H:%M:%S UTC") << std::endl;
             } else {
                 std::cout << "N/A (No valid bars found)" << std::endl;
             }
        } else {
             std::cerr << "Error: Files parsed, but state initialization failed (likely no valid bars found across all files)." << std::endl;
        }
    } else {
        std::cerr << "Error: No valid CSV data files found or loaded successfully in " << dataPath << std::endl;
        dataLoaded_ = false;
    }
    return dataLoaded_;
}

std::optional<std::reference_wrapper<const std::vector<PriceBar>>> DataManager::getAssetData(const std::string& symbol) const {
    auto it = historicalData_.find(symbol);
    if (it != historicalData_.end()) {
        return std::cref(it->second);
    }
    return std::nullopt;
}

std::vector<std::string> DataManager::getAllSymbols() const {
    return symbols_;
}

DataSnapshot DataManager::getNextBars() {
    if (!dataLoaded_ || isDataFinished()) {
        return {};
    }
    auto nextTimestamp = std::chrono::system_clock::time_point::max();
    bool foundNextTimestamp = false;
    for (const auto& symbol : symbols_) {
        auto it_idx = currentIndices_.find(symbol);
        auto it_data = historicalData_.find(symbol);
        if (it_idx != currentIndices_.end() && it_data != historicalData_.end()) {
            const size_t currentIndex = it_idx->second;
            const auto& bars = it_data->second;
            if (currentIndex < bars.size()) {
                nextTimestamp = std::min(nextTimestamp, bars[currentIndex].timestamp);
                foundNextTimestamp = true;
            }
        }
    }
    if (!foundNextTimestamp) {
        currentTime_ = std::chrono::system_clock::time_point::max();
        return {};
    }
    currentTime_ = nextTimestamp;
    DataSnapshot snapshot;
    for (const auto& symbol : symbols_) {
        auto it_idx = currentIndices_.find(symbol);
        auto it_data = historicalData_.find(symbol);
        if (it_idx != currentIndices_.end() && it_data != historicalData_.end()) {
            size_t& currentIndex = it_idx->second;
            const auto& bars = it_data->second;
            if (currentIndex < bars.size() && bars[currentIndex].timestamp == currentTime_) {
                snapshot[symbol] = bars[currentIndex];
                currentIndex++;
            }
        }
    }
    return snapshot;
}

std::chrono::system_clock::time_point DataManager::getCurrentTime() const {
    return currentTime_;
}

bool DataManager::isDataFinished() const {
    if (!dataLoaded_) return true;
    if (symbols_.empty()) return true;
    return std::all_of(symbols_.begin(), symbols_.end(), [this](const std::string& symbol) {
        auto it_idx = currentIndices_.find(symbol);
        auto it_data = historicalData_.find(symbol);
        if (it_idx == currentIndices_.end() || it_data == historicalData_.end()) {
            return true;
        }
        return it_idx->second >= it_data->second.size();
    });
}

bool DataManager::loadDataWithContinuity(const std::string& data_dir, size_t chunk_start, size_t chunk_size) {
    if (!streaming_mode_) {
        return loadData(data_dir); // Fall back to regular loading
    }
    
    std::filesystem::path dir_path(data_dir);
    if (!std::filesystem::exists(dir_path) || !std::filesystem::is_directory(dir_path)) {
        std::cerr << "Error: Directory '" << data_dir << "' does not exist or is not a directory." << std::endl;
        return false;
    }

    bool any_loaded = false;
    std::cout << "[STREAMING] Loading data chunk [" << chunk_start << ", " << (chunk_start + chunk_size) << "] from: " << data_dir << std::endl;

    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".csv") {
            std::string file_path = entry.path().string();
            std::cout << "[STREAMING] Processing file: " << file_path << std::endl;
            
            if (parseCsvFileWithContinuity(file_path, chunk_start, chunk_size)) {
                any_loaded = true;
            }
        }
    }

    if (any_loaded) {
        dataLoaded_ = true;
        std::cout << "[STREAMING] Data chunk loaded successfully. Symbols available: ";
        for (const auto& symbol : symbols_) {
            std::cout << symbol << " ";
        }
        std::cout << std::endl;
    }

    return any_loaded;
}

std::vector<PriceBar> DataManager::getWarmupData(const std::string& symbol, size_t lookback) const {
    std::vector<PriceBar> warmup_data;
    
    auto it = historicalData_.find(symbol);
    if (it == historicalData_.end() || lookback == 0) {
        return warmup_data;
    }
    
    const auto& symbol_data = it->second;
    size_t start_idx = 0;
    
    if (symbol_data.size() > lookback) {
        start_idx = symbol_data.size() - lookback;
    }
    
    // Return the last 'lookback' bars as warmup data
    warmup_data.reserve(lookback);
    for (size_t i = start_idx; i < symbol_data.size(); ++i) {
        warmup_data.push_back(symbol_data[i]);
    }
    
    return warmup_data;
}

bool DataManager::parseCsvFileWithContinuity(const std::string& file_path, size_t chunk_start, size_t chunk_size) {
    csv2::Reader<csv2::delimiter<','>> csv;
    if (!csv.mmap(file_path)) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return false;
    }

    std::string symbol = extractSymbolFromFilename(file_path);
    if (symbol.empty()) {
        std::cerr << "Could not extract symbol from filename: " << file_path << std::endl;
        return false;
    }

    // Prepare for streaming
    std::vector<PriceBar> chunk_data;
    size_t row_count = 0;
    size_t data_rows_processed = 0;
    
    // Add warmup buffer if this isn't the first chunk
    bool need_warmup = (chunk_start > 0) && last_bar_per_symbol_.count(symbol);
    
    for (const auto& row : csv) {
        if (row_count == 0) {
            // Skip header
            ++row_count;
            continue;
        }
        
        // Skip rows before our chunk start (but preserve some for warmup)
        if (data_rows_processed < chunk_start) {
            if (need_warmup && (chunk_start - data_rows_processed) <= warmup_buffer_size_) {
                // Include in warmup buffer
                try {
                    PriceBar bar = parseRowToBar(row, symbol);
                    chunk_data.push_back(bar);
                } catch (...) {
                    // Skip malformed rows
                }
            }
            ++data_rows_processed;
            continue;
        }
        
        // Process chunk data
        if (chunk_size > 0 && (data_rows_processed - chunk_start) >= chunk_size) {
            break; // Reached chunk limit
        }
        
        try {
            PriceBar bar = parseRowToBar(row, symbol);
            chunk_data.push_back(bar);
            last_bar_per_symbol_[symbol] = bar; // Keep track of last bar
            
        } catch (const std::exception& e) {
            std::cerr << "Error parsing row in " << file_path << ": " << e.what() << std::endl;
            continue;
        }
        
        ++data_rows_processed;
        
        // Apply max_rows_to_load_ limit if set
        if (max_rows_to_load_ != std::numeric_limits<size_t>::max() && 
            data_rows_processed >= max_rows_to_load_) {
            std::cout << "[LIMIT] Reached max_rows_to_load (" << max_rows_to_load_ 
                     << ") for symbol: " << symbol << std::endl;
            break;
        }
    }
    
    if (!chunk_data.empty()) {
        // For streaming mode, replace or append data
        if (chunk_start == 0) {
            historicalData_[symbol] = std::move(chunk_data); // Fresh start
        } else {
            // Append to existing data (removing warmup overlap)
            auto& existing_data = historicalData_[symbol];
            size_t warmup_overlap = need_warmup ? std::min(warmup_buffer_size_, chunk_data.size()) : 0;
            
            existing_data.insert(existing_data.end(), 
                               chunk_data.begin() + warmup_overlap, 
                               chunk_data.end());
        }
        
        // Update symbols list if new
        if (std::find(symbols_.begin(), symbols_.end(), symbol) == symbols_.end()) {
            symbols_.push_back(symbol);
        }
        
        last_processed_index_[symbol] = data_rows_processed;
        
        std::cout << "[STREAMING] Loaded " << chunk_data.size() << " bars for symbol: " << symbol 
                 << " (total: " << historicalData_[symbol].size() << ")" << std::endl;
        return true;
    }
    
    return false;
}

