#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Wraps every ctest test invocation on macOS under lldb, wired in via CMake's
# CROSSCOMPILING_EMULATOR target property (see CMakeLists.txt) so ctest
# prepends this ahead of every test_wiredpanda invocation with no per-test
# changes needed. This replaces retrying a crashed test under lldb after the
# fact: that approach confirmed unreliable for a genuinely rare, timing-
# dependent race (TestElementEditor's SIGSEGV never reproduced across 20
# retry attempts, on two separate real occurrences in CI). Running every test
# live under the debugger from the start catches a crash on its actual first
# occurrence instead of hoping a retry recreates the same timing.
#
# Only active when CI=true (set automatically by GitHub Actions) -- a local
# developer build gets zero overhead, running test_wiredpanda directly.

set -uo pipefail

if [ "${CI:-}" != "true" ] || ! command -v lldb &> /dev/null; then
    exec "$@"
fi

TEST_NAME="${2:-unknown}"
OUT_DIR="${CI_CRASH_DIAGNOSTICS_DIR:-crash-diagnostics}"

TMP_LOG=$(mktemp)
lldb -b -o run -o "bt all" -o quit -- "$@" > "$TMP_LOG" 2>&1
# ctest captures this exactly as it would the real binary's own stdout/stderr,
# so normal PASS/FAIL/QDEBUG reporting and LastTest.log are unaffected.
cat "$TMP_LOG"

# A hardware memory/instruction fault stops lldb via its Mach exception name
# (e.g. "stop reason = EXC_BAD_ACCESS (code=1, address=0x...)"), not "signal
# SIGSEGV" -- confirmed empirically. abort() still comes through as a real
# POSIX signal, so both forms are matched here.
if grep -qE 'stop reason = (signal SIG(SEGV|ABRT|BUS|ILL|FPE)|EXC_(BAD_ACCESS|BAD_INSTRUCTION|ARITHMETIC|CRASH))' "$TMP_LOG"; then
    mkdir -p "$OUT_DIR"
    cp "$TMP_LOG" "$OUT_DIR/$TEST_NAME.$$.txt"
    rm -f "$TMP_LOG"
    exit 139
fi

FAILED=0
grep -q '^FAIL!' "$TMP_LOG" && FAILED=1
rm -f "$TMP_LOG"
exit "$FAILED"
