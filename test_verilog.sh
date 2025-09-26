#!/bin/bash

cd test/verilog

# Initialize results arrays
declare -A icarus_results
declare -A verilator_results
declare -A yosys_results

# Store detailed error messages for failed tests
declare -A icarus_errors
declare -A verilator_errors
declare -A yosys_errors

files=(
    "counter.v"
    "decoder.v"
    "dflipflop-masterslave.v"
    "dflipflop.v"
    "dflipflop2.v"
    "display-3bits-counter.v"
    "display-3bits.v"
    "display-4bits-counter.v"
    "display-4bits.v"
    "dlatch.v"
    "ic.v"
    "input.v"
    "jkflipflop.v"
    "notes.v"
    "register.v"
    "sequential.v"
    "serialize.v"
    "tflipflop.v"
)

echo "Testing Verilog files with 3 EDA tools (STRICT MODE)..."
echo "============================================================="
echo "🔍 STRICT MODE ENABLED:"
echo "  - Icarus Verilog: -Wall (all warnings)"
echo "  - Verilator: --lint-only -Wall -Wpedantic"
echo "  - Yosys: -v 3 (verbose warnings)"
echo "============================================================="

# Test with Icarus Verilog (STRICT MODE)
echo "Testing with Icarus Verilog (STRICT)..."
for file in "${files[@]}"; do
    echo -n "  $file: "
    error_output=$(iverilog -Wall -o "${file%.v}_test.out" "$file" 2>&1)
    if [ $? -eq 0 ]; then
        icarus_results[$file]="✅ PASS"
        echo "PASS"
    else
        icarus_results[$file]="❌ FAIL"
        icarus_errors[$file]="$error_output"
        echo "FAIL"
    fi
    rm -f "${file%.v}_test.out" 2>/dev/null
done
echo

# Test with Verilator (STRICT MODE)
echo "Testing with Verilator (STRICT)..."
for file in "${files[@]}"; do
    echo -n "  $file: "
    error_output=$(verilator --lint-only -Wall -Wpedantic "$file" 2>&1)
    if [ $? -eq 0 ]; then
        verilator_results[$file]="✅ PASS"
        echo "PASS"
    else
        verilator_results[$file]="❌ FAIL"
        verilator_errors[$file]="$error_output"
        echo "FAIL"
    fi
done
echo

# Test with Yosys (STRICT MODE)
echo "Testing with Yosys (STRICT)..."
for file in "${files[@]}"; do
    echo -n "  $file: "
    module_name=$(grep "^module " "$file" | head -1 | sed 's/module \([a-zA-Z0-9_-]*\).*/\1/')
    error_output=$(echo "read_verilog $file; synth -top $module_name" | yosys -v 3 2>&1)
    if [ $? -eq 0 ]; then
        yosys_results[$file]="✅ PASS"
        echo "PASS"
    else
        yosys_results[$file]="❌ FAIL"
        yosys_errors[$file]="$error_output"
        echo "FAIL"
    fi
done
echo

# Generate results table
echo "========================================="
echo "VALIDATION RESULTS TABLE (STRICT MODE)"
echo "========================================="
printf "%-30s %-12s %-12s %-12s\n" "File" "Icarus" "Verilator" "Yosys"
echo "----------------------------------------------------------------"
for file in "${files[@]}"; do
    printf "%-30s %-12s %-12s %-12s\n" \
        "$file" \
        "${icarus_results[$file]}" \
        "${verilator_results[$file]}" \
        "${yosys_results[$file]}"
done

# Summary statistics
echo
echo "========================================="
echo "SUMMARY (STRICT MODE)"
echo "========================================="
icarus_pass=$(echo "${icarus_results[@]}" | grep -o "PASS" | wc -l)
verilator_pass=$(echo "${verilator_results[@]}" | grep -o "PASS" | wc -l)
yosys_pass=$(echo "${yosys_results[@]}" | grep -o "PASS" | wc -l)
total=${#files[@]}

echo "Icarus Verilog (STRICT): $icarus_pass/$total files passed"
echo "Verilator (STRICT): $verilator_pass/$total files passed"
echo "Yosys (STRICT): $yosys_pass/$total files passed"

# Show detailed error analysis for failed tests
if [ $icarus_pass -lt $total ] || [ $verilator_pass -lt $total ] || [ $yosys_pass -lt $total ]; then
    echo
    echo "========================================="
    echo "DETAILED ERROR ANALYSIS"
    echo "========================================="

    # Show Icarus Verilog errors
    if [ $icarus_pass -lt $total ]; then
        echo "🔴 ICARUS VERILOG FAILURES:"
        for file in "${files[@]}"; do
            if [[ "${icarus_results[$file]}" == *"FAIL"* ]]; then
                echo "  ❌ $file:"
                echo "${icarus_errors[$file]}" | sed 's/^/     /'
                echo
            fi
        done
    fi

    # Show Verilator errors
    if [ $verilator_pass -lt $total ]; then
        echo "🔴 VERILATOR FAILURES:"
        for file in "${files[@]}"; do
            if [[ "${verilator_results[$file]}" == *"FAIL"* ]]; then
                echo "  ❌ $file:"
                echo "${verilator_errors[$file]}" | sed 's/^/     /'
                echo
            fi
        done
    fi

    # Show Yosys errors
    if [ $yosys_pass -lt $total ]; then
        echo "🔴 YOSYS FAILURES:"
        for file in "${files[@]}"; do
            if [[ "${yosys_results[$file]}" == *"FAIL"* ]]; then
                echo "  ❌ $file:"
                echo "${yosys_errors[$file]}" | grep -E "(ERROR|Warning|Error)" | sed 's/^/     /'
                echo
            fi
        done
    fi
else
    echo
    echo "🎉 ALL TESTS PASSED IN STRICT MODE!"
fi

echo "========================================="
echo "NOTE: STRICT MODE ANALYSIS"
echo "========================================="
echo "This test runs all EDA tools in their strictest validation modes:"
echo "• More warnings and errors will be detected than in normal mode"
echo "• UNOPTFLAT warnings in Verilator may indicate optimization issues"
echo "• Pedantic mode catches IEEE standard compliance violations"
echo "• Results provide comprehensive code quality assessment"