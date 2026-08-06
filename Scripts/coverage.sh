#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

set -euo pipefail

if ! command -v lcov &> /dev/null; then
    echo "lcov is not installed. Install it with: sudo apt-get install lcov" >&2
    exit 1
fi

echo "Building with coverage enabled..."
cmake --preset coverage
cmake --build --preset coverage

# Reset counters left over from any previous local run -- otherwise a file that isn't
# recompiled this time (its .gcno/.gcda are still fresh) keeps accumulating hit counts
# across separate script invocations instead of reflecting just this run. Doesn't affect
# whether a line shows as covered at all, but does inflate the per-line hit counts genhtml
# displays.
lcov --zerocounters --directory build-coverage

echo "Running tests with coverage collection..."
ctest --preset coverage

echo "Generating HTML coverage report..."

cd build-coverage
lcov --capture --directory . --output-file coverage.info --ignore-errors source,inconsistent
lcov --remove coverage.info \
    '/usr/*' \
    '*/Tests/*' \
    '*/build-coverage/*' \
    '*/Qt*/' \
    '*MCP/*' \
    --output-file coverage_filtered.info --ignore-errors inconsistent,unused

lcov --summary coverage_filtered.info

genhtml coverage_filtered.info --output-directory coverage_html --dark-mode

echo "Coverage report generated."
echo "Open coverage_html/index.html in your browser to view the report"
echo "Coverage data location: build-coverage/coverage.info"
echo "HTML report location: build-coverage/coverage_html/"
