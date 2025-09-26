#!/bin/bash

# Comprehensive Verilog test script for wiRedPanda exported circuits
# Tests all exported Verilog files using available EDA tools

set -e  # Exit on any error

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_RESULTS_DIR="$SCRIPT_DIR/test_results"
LOG_FILE="$TEST_RESULTS_DIR/test.log"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Create test results directory
mkdir -p "$TEST_RESULTS_DIR"

# Initialize log file
echo "Verilog Test Suite - $(date)" > "$LOG_FILE"
echo "===========================================" >> "$LOG_FILE"

log() {
    local level="$1"
    local message="$2"
    local timestamp=$(date '+%H:%M:%S')
    echo -e "[$timestamp] $level: $message" | tee -a "$LOG_FILE"
}

check_tool() {
    local tool="$1"
    if command -v "$tool" &> /dev/null; then
        log "INFO" "${GREEN}$tool is available${NC}"
        return 0
    else
        log "WARN" "${YELLOW}$tool is not available${NC}"
        return 1
    fi
}

test_with_iverilog() {
    local verilog_file="$1"
    local file_stem=$(basename "$verilog_file" .v)
    local testbench_file="$SCRIPT_DIR/${file_stem}_tb.v"
    local output_file="$TEST_RESULTS_DIR/${file_stem}_iverilog.out"

    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    if [[ -f "$testbench_file" ]]; then
        log "INFO" "Testing $verilog_file with testbench $testbench_file"

        # Compile with testbench
        if iverilog -o "$output_file" "$testbench_file" "$verilog_file" 2>>"$LOG_FILE"; then
            # Run simulation
            if vvp "$output_file" >> "$LOG_FILE" 2>&1; then
                log "INFO" "${GREEN}✅ $file_stem: iverilog simulation PASSED${NC}"
                PASSED_TESTS=$((PASSED_TESTS + 1))
                return 0
            else
                log "ERROR" "${RED}❌ $file_stem: iverilog simulation FAILED${NC}"
                FAILED_TESTS=$((FAILED_TESTS + 1))
                return 1
            fi
        else
            log "ERROR" "${RED}❌ $file_stem: iverilog compilation FAILED${NC}"
            FAILED_TESTS=$((FAILED_TESTS + 1))
            return 1
        fi
    else
        log "INFO" "Syntax checking $verilog_file (no testbench found)"

        # Just syntax check
        if iverilog -t null "$verilog_file" 2>>"$LOG_FILE"; then
            log "INFO" "${GREEN}✅ $file_stem: iverilog syntax check PASSED${NC}"
            PASSED_TESTS=$((PASSED_TESTS + 1))
            return 0
        else
            log "ERROR" "${RED}❌ $file_stem: iverilog syntax check FAILED${NC}"
            FAILED_TESTS=$((FAILED_TESTS + 1))
            return 1
        fi
    fi
}

test_with_verilator() {
    local verilog_file="$1"
    local file_stem=$(basename "$verilog_file" .v)

    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    log "INFO" "Verilator lint checking $verilog_file"

    if verilator --lint-only "$verilog_file" 2>>"$LOG_FILE"; then
        log "INFO" "${GREEN}✅ $file_stem: verilator lint PASSED${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        log "ERROR" "${RED}❌ $file_stem: verilator lint FAILED${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
}

test_with_yosys() {
    local verilog_file="$1"
    local file_stem=$(basename "$verilog_file" .v)

    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    log "INFO" "Yosys synthesis checking $verilog_file"

    # Create temporary Yosys script
    local yosys_script="$TEST_RESULTS_DIR/${file_stem}_yosys.ys"
    cat > "$yosys_script" << EOF
read_verilog $verilog_file
hierarchy -check
proc
opt
check
EOF

    if yosys -s "$yosys_script" >> "$LOG_FILE" 2>&1; then
        log "INFO" "${GREEN}✅ $file_stem: yosys synthesis PASSED${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        log "ERROR" "${RED}❌ $file_stem: yosys synthesis FAILED${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
}

run_tests() {
    log "INFO" "${BLUE}Starting comprehensive Verilog test suite${NC}"
    echo "===========================================" >> "$LOG_FILE"

    # Check available tools
    local tools_available=0

    if check_tool "iverilog"; then
        IVERILOG_AVAILABLE=1
        tools_available=1
    else
        IVERILOG_AVAILABLE=0
    fi

    if check_tool "verilator"; then
        VERILATOR_AVAILABLE=1
        tools_available=1
    else
        VERILATOR_AVAILABLE=0
    fi

    if check_tool "yosys"; then
        YOSYS_AVAILABLE=1
        tools_available=1
    else
        YOSYS_AVAILABLE=0
    fi

    if [[ $tools_available -eq 0 ]]; then
        log "ERROR" "${RED}No EDA tools available! Cannot run tests.${NC}"
        exit 1
    fi

    # Test all Verilog files
    for verilog_file in "$SCRIPT_DIR"/*.v; do
        # Skip testbench files
        if [[ "$(basename "$verilog_file")" =~ _tb\.v$ ]]; then
            continue
        fi

        # Skip non-exported files (check if it's in our list)
        local file_name=$(basename "$verilog_file")
        if [[ ! " counter.v decoder.v dflipflop-masterslave.v dflipflop.v dflipflop2.v display-3bits-counter.v display-3bits.v display-4bits-counter.v display-4bits.v dlatch.v ic.v input.v jkflipflop.v notes.v register.v sequential.v serialize.v tflipflop.v " =~ " $file_name " ]]; then
            log "INFO" "Skipping $file_name (not in exported list)"
            continue
        fi

        if [[ ! -f "$verilog_file" ]]; then
            log "ERROR" "${RED}❌ $verilog_file: FILE NOT FOUND${NC}"
            continue
        fi

        local file_stem=$(basename "$verilog_file" .v)
        log "INFO" "${BLUE}Testing $file_name${NC}"
        echo "----------------------------------------" >> "$LOG_FILE"

        # Test with available tools
        if [[ $IVERILOG_AVAILABLE -eq 1 ]]; then
            test_with_iverilog "$verilog_file"
        fi

        if [[ $VERILATOR_AVAILABLE -eq 1 ]]; then
            test_with_verilator "$verilog_file"
        fi

        if [[ $YOSYS_AVAILABLE -eq 1 ]]; then
            test_with_yosys "$verilog_file"
        fi
    done
}

generate_report() {
    log "INFO" "${BLUE}===========================================${NC}"
    log "INFO" "${BLUE}FINAL TEST REPORT${NC}"
    log "INFO" "${BLUE}===========================================${NC}"

    log "INFO" "Total tests run: $TOTAL_TESTS"
    log "INFO" "${GREEN}Passed: $PASSED_TESTS${NC}"
    log "INFO" "${RED}Failed: $FAILED_TESTS${NC}"

    if [[ $TOTAL_TESTS -gt 0 ]]; then
        local success_rate=$((PASSED_TESTS * 100 / TOTAL_TESTS))
        log "INFO" "Success rate: ${success_rate}%"
    else
        log "INFO" "Success rate: 0%"
    fi

    # Write summary to report file
    local report_file="$TEST_RESULTS_DIR/test_summary.txt"
    cat > "$report_file" << EOF
Verilog Test Summary - $(date)
==========================================

Total tests: $TOTAL_TESTS
Passed: $PASSED_TESTS
Failed: $FAILED_TESTS
Success rate: $((TOTAL_TESTS > 0 ? PASSED_TESTS * 100 / TOTAL_TESTS : 0))%

See $LOG_FILE for detailed results.
EOF

    log "INFO" "Detailed report written to: $report_file"
    log "INFO" "Full log available at: $LOG_FILE"

    # Return success if all tests passed
    return $((FAILED_TESTS > 0 ? 1 : 0))
}

main() {
    cd "$SCRIPT_DIR"

    run_tests
    generate_report

    if [[ $FAILED_TESTS -eq 0 ]]; then
        echo -e "\n${GREEN}🎉 All tests passed!${NC}"
        exit 0
    else
        echo -e "\n${YELLOW}⚠️  Some tests failed. Check the report for details.${NC}"
        exit 1
    fi
}

# Run main function
main "$@"