#!/bin/bash
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

set -e

# Fail fast if lcov is not installed
if ! command -v lcov &> /dev/null; then
    echo "❌ lcov is not installed. Install it with: sudo apt-get install lcov"
    exit 1
fi

echo "🔍 Building with coverage enabled..."
cmake --preset coverage
cmake --build --preset coverage

echo "🧪 Running tests with coverage collection..."
ctest --preset coverage

echo "📄 Generating HTML coverage report..."

# Generate lcov info file directly with filtering
cd build
lcov --capture --directory . --output-file coverage.info --ignore-errors source,inconsistent
lcov --remove coverage.info \
    '/usr/*' \
    '*/Tests/*' \
    '*/build/*' \
    '*/Qt*/' \
    --output-file coverage_filtered.info --ignore-errors inconsistent,unused

# Generate HTML report with dark theme
genhtml coverage_filtered.info --output-directory coverage_html --dark-mode

echo "✅ Coverage report generated!"
echo "📂 Open coverage_html/index.html in your browser to view the report"
echo "📍 Coverage data location: build/coverage.info"
echo "🌐 HTML report location: build/coverage_html/"
