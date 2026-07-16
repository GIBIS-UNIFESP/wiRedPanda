#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Profiles wiredpanda opening a .panda file, using either callgrind or perf.
# Usage: Scripts/profile_load.sh <callgrind|perf> <file.panda> [seconds] [offscreen]
#
#   seconds   - how long to let the app run before it's sent SIGTERM (default 20)
#   offscreen - pass any value to run with QT_QPA_PLATFORM=offscreen, isolating
#               simulation/logic cost from paint cost (default: real display)
#
# Builds against build-relwithdebinfo/wiredpanda (cmake --preset relwithdebinfo).
# Output is written under Scripts/profiling-out/, not to stdout.

set -euo pipefail

TOOL="${1:?usage: $0 <callgrind|perf> <file.panda> [seconds] [offscreen]}"
PANDA_FILE="${2:?usage: $0 <callgrind|perf> <file.panda> [seconds] [offscreen]}"
SECONDS_TO_RUN="${3:-20}"
OFFSCREEN="${4:-}"

BINARY="build-relwithdebinfo/wiredpanda"
OUT_DIR="Scripts/profiling-out"

if [[ ! -x "$BINARY" ]]; then
    echo "Missing $BINARY - build it first: cmake --build --preset relwithdebinfo" >&2
    exit 1
fi

if [[ ! -f "$PANDA_FILE" ]]; then
    echo "No such file: $PANDA_FILE" >&2
    exit 1
fi

mkdir -p "$OUT_DIR"

ENV_ARGS=()
if [[ -n "$OFFSCREEN" ]]; then
    ENV_ARGS+=(env QT_QPA_PLATFORM=offscreen)
fi

STAMP="$(basename "$PANDA_FILE" .panda)_$$"

case "$TOOL" in
    callgrind)
        OUT_FILE="$OUT_DIR/callgrind.out.$STAMP"
        echo "Recording callgrind to $OUT_FILE (running ${SECONDS_TO_RUN}s)..."
        timeout --signal=TERM "$SECONDS_TO_RUN" "${ENV_ARGS[@]}" \
            valgrind --tool=callgrind --callgrind-out-file="$OUT_FILE" \
            "$BINARY" "$PANDA_FILE" || true
        echo "Done. Annotate with: callgrind_annotate --auto=yes $OUT_FILE | head -80"
        ;;
    perf)
        # -F 99 caps the sample rate (the standard rate for callgraph
        # profiling): perf's default (~2000Hz) produces so many DWARF stack
        # dumps over a multi-second run that unwinding them all at report
        # time can take minutes. DEBUGINFOD_URLS is unset because this host
        # has it pointed at debuginfod.ubuntu.com, and perf report stalls
        # trying to reach it for every system library without local symbols.
        OUT_FILE="$OUT_DIR/perf.data.$STAMP"
        echo "Recording perf to $OUT_FILE (running ${SECONDS_TO_RUN}s)..."
        timeout --signal=TERM "$SECONDS_TO_RUN" "${ENV_ARGS[@]}" \
            env -u DEBUGINFOD_URLS perf record -F 99 --call-graph dwarf -o "$OUT_FILE" -- \
            "$BINARY" "$PANDA_FILE" || true
        echo "Done. Report with: env -u DEBUGINFOD_URLS perf report --stdio -g -i $OUT_FILE > report.txt"
        ;;
    *)
        echo "Unknown tool '$TOOL' - expected 'callgrind' or 'perf'" >&2
        exit 1
        ;;
esac
