#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Finds tests CTest reported as crashed in the given build directory's LastTest.log
# and re-runs each under lldb, retrying up to a bounded number of times to improve
# the odds of reproducing a rare, timing-dependent crash. macOS has no viable native
# core-dump mechanism on GitHub-hosted runners: System Integrity Protection blocks
# core generation even with the required code-signing entitlement in place, and SIP
# can't be disabled without booting into Recovery Mode -- categorically impossible
# on a remotely-provisioned, GitHub-hosted runner. A single retry has weak odds of
# reproducing a genuinely rare race, hence the bounded loop rather than one attempt.
# Called only from CI's `if: failure()` steps -- see .github/workflows/build.yml.

set -uo pipefail # not -e: a test that doesn't reproduce must not abort the others

BUILD_DIR="${1:?usage: $0 <build-dir> <test-binary> <output-dir> [max-attempts]}"
TEST_BINARY="${2:?usage: $0 <build-dir> <test-binary> <output-dir> [max-attempts]}"
OUT_DIR="${3:?usage: $0 <build-dir> <test-binary> <output-dir> [max-attempts]}"
MAX_ATTEMPTS="${4:-20}"

LOG_FILE="$BUILD_DIR/Testing/Temporary/LastTest.log"
mkdir -p "$OUT_DIR"

CRASHED_TESTS=""
if [ -f "$LOG_FILE" ]; then
    # "***Exception: SegFault" is real, but it's ctest's live CONSOLE summary
    # line -- LastTest.log's per-test detail blocks never contain that string
    # at all (confirmed: grepping a real crashed run's LastTest.log for it
    # finds nothing). What the log file DOES contain is QTestLib's own crash
    # handler line ("Received signal 11 (SIGSEGV)...") inside the block for
    # whichever "<N>/<Total> Test: <TestName>" header precedes it.
    CRASHED_TESTS=$(awk '
        /^[0-9]+\/[0-9]+ Test: / { name = $3 }
        /^Received signal [0-9]+ \(SIG(SEGV|ABRT|BUS|ILL|FPE)\)/ { print name }
    ' "$LOG_FILE" | sort -u)
fi

if [ -z "$CRASHED_TESTS" ]; then
    echo "No crashed (SegFault) tests found in $LOG_FILE."
elif ! command -v lldb &> /dev/null; then
    echo "lldb not found -- cannot retry crashed tests: $CRASHED_TESTS" >&2
else
    for TEST_NAME in $CRASHED_TESTS; do
        echo "=== Crashed test: $TEST_NAME -- retrying up to $MAX_ATTEMPTS times under lldb ==="
        REPRODUCED=0
        for ATTEMPT in $(seq 1 "$MAX_ATTEMPTS"); do
            OUT_FILE="$OUT_DIR/$TEST_NAME.attempt$ATTEMPT.txt"
            lldb -b -o run -o "bt all" -o quit -- "$TEST_BINARY" "$TEST_NAME" > "$OUT_FILE" 2>&1
            # SIGSTOP/SIGINT can appear as a benign "stop reason = signal" during normal
            # process launch under a debugger -- match the specific crash-indicating
            # signals only, not any stop, to avoid a false "reproduced" on a non-crash.
            # A hardware memory/instruction fault stops lldb via its Mach exception
            # name (e.g. "stop reason = EXC_BAD_ACCESS (code=1, address=0x...)"), NOT
            # "signal SIGSEGV" -- confirmed empirically (20/20 real, guaranteed-crashing
            # attempts went undetected under the signal-name-only pattern). abort() still
            # comes through as a genuine POSIX signal, so both forms are matched here.
            if grep -qE 'stop reason = (signal SIG(SEGV|ABRT|BUS|ILL|FPE)|EXC_(BAD_ACCESS|BAD_INSTRUCTION|ARITHMETIC|CRASH))' "$OUT_FILE"; then
                echo "Reproduced on attempt $ATTEMPT -- see $OUT_FILE"
                REPRODUCED=1
                break
            fi
            rm -f "$OUT_FILE" # didn't crash this attempt -- don't keep a useless clean-run log
        done
        if [ "$REPRODUCED" -eq 0 ]; then
            echo "Did not reproduce within $MAX_ATTEMPTS attempts for $TEST_NAME." \
                | tee "$OUT_DIR/$TEST_NAME.not-reproduced.txt"
        fi
    done
fi

# Free, zero-cost bonus: macOS's own ReportCrash service might have produced a report
# despite the headless CI session and SIP -- not confirmed to work reliably, so not the
# primary mechanism above, but costs nothing to also check for.
DIAG_DIR="$HOME/Library/Logs/DiagnosticReports"
BINARY_NAME=$(basename "$TEST_BINARY")
if [ -d "$DIAG_DIR" ]; then
    find "$DIAG_DIR" -maxdepth 1 -iname "*${BINARY_NAME}*" \( -name "*.ips" -o -name "*.crash" \) \
        -exec cp {} "$OUT_DIR/" \; 2> /dev/null
fi
