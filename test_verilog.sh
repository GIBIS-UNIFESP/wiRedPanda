#!/bin/bash

cd test/verilog

# Initialize results arrays
declare -A icarus_results
declare -A verilator_results
declare -A yosys_results

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

echo "Testing Verilog files with 3 EDA tools..."
echo "========================================="

# Test with Icarus Verilog
echo "Testing with Icarus Verilog..."
for file in "${files[@]}"; do
    echo -n "  $file: "
    if iverilog -o "${file%.v}_test.out" "$file" >/dev/null 2>&1; then
        icarus_results[$file]="✅ PASS"
        echo "PASS"
    else
        icarus_results[$file]="❌ FAIL"
        echo "FAIL"
    fi
    rm -f "${file%.v}_test.out" 2>/dev/null
done
echo

# Test with Verilator
echo "Testing with Verilator..."
for file in "${files[@]}"; do
    echo -n "  $file: "
    if verilator --lint-only "$file" >/dev/null 2>&1; then
        verilator_results[$file]="✅ PASS"
        echo "PASS"
    else
        verilator_results[$file]="❌ FAIL"
        echo "FAIL"
    fi
done
echo

# Test with Yosys
echo "Testing with Yosys..."
for file in "${files[@]}"; do
    echo -n "  $file: "
    module_name=$(grep "^module " "$file" | head -1 | sed 's/module \([a-zA-Z0-9_-]*\).*/\1/')
    if echo "read_verilog $file; synth -top $module_name" | yosys -q >/dev/null 2>&1; then
        yosys_results[$file]="✅ PASS"
        echo "PASS"
    else
        yosys_results[$file]="❌ FAIL"
        echo "FAIL"
    fi
done
echo

# Generate results table
echo "========================================="
echo "VALIDATION RESULTS TABLE"
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
echo "SUMMARY"
echo "========================================="
icarus_pass=$(echo "${icarus_results[@]}" | grep -o "PASS" | wc -l)
verilator_pass=$(echo "${verilator_results[@]}" | grep -o "PASS" | wc -l)
yosys_pass=$(echo "${yosys_results[@]}" | grep -o "PASS" | wc -l)
total=${#files[@]}

echo "Icarus Verilog: $icarus_pass/$total files passed"
echo "Verilator: $verilator_pass/$total files passed"
echo "Yosys: $yosys_pass/$total files passed"