#!/bin/bash
echo "=================================="
echo "🧪 wiRedPanda MCP - Full Test Suite"
echo "=================================="
echo ""

TOTAL_PASS=0
TOTAL_FAIL=0

run_test() {
    echo "Running: $1"
    echo "---"
    if python3 "$1" > /tmp/test_output.txt 2>&1; then
        # Count passed tests from output
        PASS=$(grep -o "✅ Test PASSED" /tmp/test_output.txt | wc -l)
        TOTAL_PASS=$((TOTAL_PASS + PASS))
        echo "✅ PASSED ($PASS tests)"
    else
        FAIL=$(grep -o "❌ Test FAILED" /tmp/test_output.txt | wc -l)
        TOTAL_FAIL=$((TOTAL_FAIL + FAIL))
        echo "❌ FAILED ($FAIL tests)"
    fi
    echo ""
}

echo "📊 Test Results Summary"
echo ""
echo "Phase 1: Safety Fixes"
run_test "test/mcp/run_phase1_validation.py"

echo "Phase 2: Architectural Improvements"
run_test "test/mcp/run_phase2_validation.py"

echo "Phase 3: Robustness Enhancements"
run_test "test/mcp/run_phase3_validation.py"

echo "Phase 4: Comprehensive Regression"
run_test "test/mcp/test_phase4_comprehensive_regression.py"

echo "Phase 5.1: Edge Case Testing"
run_test "test/mcp/test_phase5_edge_cases.py"

echo "Phase 5.2: Combined Operations"
run_test "test/mcp/test_phase5_2_combined_operations.py"

echo "Phase 5.3: Stress & Performance"
run_test "test/mcp/test_phase5_3_stress_performance.py"

echo "Phase 6: Undo/Redo with Connected Elements"
run_test "test/mcp/test_phase6_connected_elements.py"

echo "=================================="
echo "📈 Final Summary"
echo "=================================="
echo "Total Tests Passed: $TOTAL_PASS"
echo "Total Tests Failed: $TOTAL_FAIL"
echo ""
if [ $TOTAL_FAIL -eq 0 ]; then
    echo "✅ ALL TESTS PASSED"
else
    echo "❌ SOME TESTS FAILED"
fi
echo "=================================="
