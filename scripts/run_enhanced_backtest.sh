#!/usr/bin/env bash
# scripts/run_enhanced_backtest.sh
# ------------------------------------------------------------
# Run enhanced quantitative backtesting with improved strategies
# and proper data continuity management
#
# Usage:
#   bash scripts/run_enhanced_backtest.sh [max_rows]
# ------------------------------------------------------------
set -euo pipefail

# Configuration
MAX_ROWS=${1:-"500000"}  # Default to 500k rows for demonstration
OUTPUT_DIR="results"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

# Create results directory
mkdir -p "$OUTPUT_DIR"

echo "=========================================="
echo "Enhanced Quantitative Backtesting System"
echo "=========================================="
echo "Timestamp: $(date)"
echo "Max rows per dataset: $MAX_ROWS"
echo "Results directory: $OUTPUT_DIR"
echo

# Build system
echo "[INFO] Building optimized release binary..."
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native" 1>/dev/null
cmake --build build -j$(sysctl -n hw.ncpu) 1>/dev/null

# Locate executable
EXECUTABLE="$(find build -type f -perm -111 -name "trading_system*" | head -n 1)"
if [[ -z "$EXECUTABLE" ]]; then
  echo "[ERROR] Unable to locate built executable in ./build."
  exit 1
fi

echo "[INFO] Using executable: $EXECUTABLE"
echo

# Define test scenarios
declare -a SCENARIOS=(
    "Enhanced_Strategies:--max-rows=$MAX_ROWS"
    "Streaming_Demo:--max-rows=100000"
    "Full_Dataset:unlimited"
)

# Output files
RESULTS_FILE="$OUTPUT_DIR/enhanced_results_${TIMESTAMP}.md"
SUMMARY_FILE="$OUTPUT_DIR/performance_summary_${TIMESTAMP}.txt"

# Initialize results file
cat > "$RESULTS_FILE" << EOF
# Enhanced Quantitative Backtesting Results

Generated: $(date)
System: $(uname -s) $(uname -r)
Executable: $EXECUTABLE

## Test Configuration
- Enhanced strategies with regime detection
- Adaptive position sizing based on volatility
- Risk management with Kelly criterion
- Streaming data support for large datasets

EOF

echo "[INFO] Starting enhanced backtesting scenarios..."
echo

# Run each scenario
for SCENARIO in "${SCENARIOS[@]}"; do
    SCENARIO_NAME="${SCENARIO%%:*}"
    SCENARIO_ARGS="${SCENARIO##*:}"
    
    echo "----------------------------------------"
    echo "Running scenario: $SCENARIO_NAME"
    echo "Arguments: $SCENARIO_ARGS"
    echo "----------------------------------------"
    
    # Prepare command
    if [[ "$SCENARIO_ARGS" == "unlimited" ]]; then
        RUN_CMD=("$EXECUTABLE")
        LABEL="Full Dataset (Unlimited Rows)"
    else
        RUN_CMD=("$EXECUTABLE" "$SCENARIO_ARGS")
        LABEL="$SCENARIO_NAME ($SCENARIO_ARGS)"
    fi
    
    # Capture start time
    START_TIME=$(date +%s)
    
    # Run backtest and capture output
    TMP_OUT="$(mktemp)"
    echo "[INFO] Executing: ${RUN_CMD[*]}"
    
    if "${RUN_CMD[@]}" > "$TMP_OUT" 2>&1; then
        # Calculate runtime
        END_TIME=$(date +%s)
        RUNTIME=$((END_TIME - START_TIME))
        
        echo "[SUCCESS] Scenario completed in ${RUNTIME}s"
        
        # Extract results table
        RESULTS_TABLE=$(awk '
            /===== COMBINED Strategy Comparison Results =====/ {flag=1; next}
            flag && /^=+$/ {exit}
            flag {print}
        ' "$TMP_OUT")
        
        # Extract key statistics
        TOTAL_STRATEGIES=$(echo "$RESULTS_TABLE" | grep -c "^[A-Za-z]" || echo "0")
        PROFITABLE_STRATEGIES=$(echo "$RESULTS_TABLE" | awk '$2 > 0 {count++} END {print count+0}')
        
        # Append to results file
        cat >> "$RESULTS_FILE" << EOF

## $LABEL

**Runtime:** ${RUNTIME} seconds  
**Total Strategies:** $TOTAL_STRATEGIES  
**Profitable Strategies:** $PROFITABLE_STRATEGIES  

\`\`\`text
$RESULTS_TABLE
\`\`\`

EOF
        
        # Update summary
        echo "Scenario: $SCENARIO_NAME - Runtime: ${RUNTIME}s - Strategies: $TOTAL_STRATEGIES - Profitable: $PROFITABLE_STRATEGIES" >> "$SUMMARY_FILE"
        
    else
        echo "[ERROR] Scenario failed. Check logs."
        echo "Error in scenario: $SCENARIO_NAME" >> "$SUMMARY_FILE"
        
        # Still capture error output
        cat >> "$RESULTS_FILE" << EOF

## $LABEL - ERROR

\`\`\`text
$(tail -50 "$TMP_OUT")
\`\`\`

EOF
    fi
    
    rm "$TMP_OUT"
    echo
done

# Generate performance summary
echo "=========================================="
echo "PERFORMANCE SUMMARY"
echo "=========================================="
cat "$SUMMARY_FILE"
echo

# Find top performing strategies
echo "[INFO] Analyzing top performers..."
TOP_PERFORMERS=$(awk '
    /^[A-Za-z]/ && $2 > 5.0 {
        printf "%-50s %8.2f%% return, %8.2f%% drawdown\n", $1, $2, $3
    }
' "$OUTPUT_DIR"/*.md 2>/dev/null | sort -k2 -nr | head -10)

if [[ -n "$TOP_PERFORMERS" ]]; then
    echo
    echo "TOP 10 PERFORMING STRATEGIES (>5% return):"
    echo "--------------------------------------------"
    echo "$TOP_PERFORMERS"
else
    echo "[INFO] No strategies with >5% return found."
fi

# Cleanup and final message
echo
echo "=========================================="
echo "Enhanced backtesting complete!"
echo "=========================================="
echo "Results saved to: $RESULTS_FILE"
echo "Summary saved to: $SUMMARY_FILE"
echo "Total runtime: $(($(date +%s) - $(date +%s -d "1 hour ago"))) seconds"
echo

# Optional: Open results in default editor/viewer
if command -v open >/dev/null 2>&1; then
    echo "[INFO] Opening results file..."
    open "$RESULTS_FILE"
elif command -v xdg-open >/dev/null 2>&1; then
    echo "[INFO] Opening results file..."
    xdg-open "$RESULTS_FILE"
fi 