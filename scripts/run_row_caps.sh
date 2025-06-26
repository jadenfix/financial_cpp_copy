#!/usr/bin/env bash
# scripts/run_row_caps.sh
# ------------------------------------------------------------
# Run the HFT backtesting executable with multiple --max-rows
# limits (100k, 500k, 1M, and full) and extract the combined
# strategy comparison tables into a single markdown file.
#
# Usage:
#   bash scripts/run_row_caps.sh
# ------------------------------------------------------------
set -euo pipefail

# Build (if not already built)
echo "[INFO] Building release binary..."
cmake -B build -DCMAKE_BUILD_TYPE=Release 1>/dev/null
cmake --build build -j$(sysctl -n hw.ncpu) 1>/dev/null

# Locate executable (assumes add_executable financial_cpp in CMakeLists)
EXECUTABLE="$(find build -type f -perm -111 -name "trading_system*" | head -n 1)"
if [[ -z "$EXECUTABLE" ]]; then
  echo "[ERROR] Unable to locate built executable in ./build."
  exit 1
fi

echo "[INFO] Using executable: $EXECUTABLE"

# Output markdown file
OUTPUT_MD="results_row_caps.md"
echo "# Combined Strategy Comparison Tables by Row Cap" > "$OUTPUT_MD"

declare -a CAPS=("100000" "500000" "1000000" "full")

for CAP in "${CAPS[@]}"; do
  if [[ "$CAP" == "full" ]]; then
    RUN_CMD=("$EXECUTABLE")
    LABEL="Full Dataset (Unlimited Rows)"
  else
    RUN_CMD=("$EXECUTABLE" "--max-rows=${CAP}")
    LABEL="${CAP} Rows per CSV"
  fi

  echo "[INFO] Running: ${RUN_CMD[*]}"   
  TMP_OUT="$(mktemp)"

  # Capture output silently (prevent flooding terminal). Still stored in TMP_OUT.
  "${RUN_CMD[@]}" > "$TMP_OUT" 2>&1

  # Extract section starting at combined results
  TABLE_CONTENT=$(awk '/===== COMBINED Strategy Comparison Results =====/{flag=1; next} flag && /^=+$/ {exit} flag {print}' "$TMP_OUT")

  printf "\n## %s\n\n" "$LABEL" >> "$OUTPUT_MD"
  # Write fenced code block explicitly to avoid backtick command substitution
  {
    printf '```text\n'         # opening fence
    printf '%s\n' "$TABLE_CONTENT"
    printf '```\n\n'          # closing fence & extra newline
  } >> "$OUTPUT_MD"

  rm "$TMP_OUT"
done

echo "[INFO] Results saved to $OUTPUT_MD" 