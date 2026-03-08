#!/bin/bash
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

set -e

echo "🔍 Building with coverage enabled..."
cmake --preset coverage
cmake --build --preset coverage

echo "🧪 Running tests with coverage collection..."
ctest --preset coverage

echo "📄 Generating HTML coverage report..."
# Install lcov if not available
if ! command -v lcov &> /dev/null; then
    echo "Installing lcov..."
    sudo apt-get update && sudo apt-get install -y lcov
fi

# Generate lcov info file directly with filtering
cd build
lcov --capture --directory . --output-file coverage.info --ignore-errors source
lcov --remove coverage.info \
    '/usr/*' \
    '*/Tests/*' \
    '*/build/*' \
    '*/Qt*/include/Qt*' \
    '*/Qt/*' \
    --output-file coverage_filtered.info

# Generate HTML report with dark theme
genhtml coverage_filtered.info --output-directory coverage_html --css-file ../Scripts/lcov-dark-theme.css

echo "✅ Coverage report generated!"
echo "📂 Open coverage_html/index.html in your browser to view the report"
echo "📍 Coverage data location: build/coverage.info"
echo "🌐 HTML report location: build/coverage_html/"
