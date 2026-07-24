#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Analyzes every core file found in the given core-dump directory (written by the
# kernel per the core_pattern configured in CI) against the given test binary,
# producing a symbolicated backtrace for each. Called only from CI's `if: failure()`
# steps -- see .github/workflows/build.yml and sanitizers.yml. Checking for the
# physical presence of a core file, rather than parsing CTest's own log for which
# test crashed, sidesteps a real attribution gap: every CTest test invokes the same
# test_wiredpanda binary with a different QTest class argument, so a core file's
# executable name alone can't be matched back to a specific ctest test name if more
# than one test crashes in the same run.

set -uo pipefail # not -e: one missing/unreadable core file must not abort the rest

TEST_BINARY="${1:?usage: $0 <test-binary> <core-dump-dir> <output-dir>}"
CORE_DIR="${2:?usage: $0 <test-binary> <core-dump-dir> <output-dir>}"
OUT_DIR="${3:?usage: $0 <test-binary> <core-dump-dir> <output-dir>}"

if [ ! -d "$CORE_DIR" ] || [ -z "$(ls -A "$CORE_DIR" 2> /dev/null)" ]; then
    echo "No core files in $CORE_DIR."
    exit 0
fi

if ! command -v gdb &> /dev/null; then
    echo "gdb not found -- cannot analyze core files in $CORE_DIR." >&2
    exit 0
fi

mkdir -p "$OUT_DIR"

for CORE_FILE in "$CORE_DIR"/*; do
    [ -f "$CORE_FILE" ] || continue
    NAME=$(basename "$CORE_FILE")
    echo "=== Analyzing core file: $NAME ==="
    gdb --batch -ex "thread apply all bt full" -ex quit "$TEST_BINARY" "$CORE_FILE" \
        > "$OUT_DIR/$NAME.txt" 2>&1
    echo "Backtrace written to $OUT_DIR/$NAME.txt"
done
