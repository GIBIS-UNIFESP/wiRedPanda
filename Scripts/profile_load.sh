#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Profiles wiredpanda opening a .panda file, using either callgrind or perf.
# Usage: Scripts/profile_load.sh <callgrind|perf|callgrind-attach|perf-attach> <file.panda> [seconds] [offscreen] [settle_seconds]
#
#   seconds        - callgrind/perf: how long to let the app run before SIGTERM (default 20)
#                     *-attach: how long to record once attached (default 10)
#   offscreen      - pass any value to run with QT_QPA_PLATFORM=offscreen, isolating
#                     simulation/logic cost from paint cost (default: real display)
#   settle_seconds - *-attach only: how long to let the app load+settle, unprofiled,
#                     before attaching (default 10; measured empirically per fixture —
#                     e.g. ~8s for a 20k-element circuit with simulation paused via
#                     File > Play/Pause, since a running simulation keeps the process
#                     busy and would otherwise mask when loading actually finished)
#
# callgrind/perf profile from process launch (load included). callgrind-attach/
# perf-attach instead run the app unprofiled until it settles, then start
# recording — isolating steady-state/rendering cost from one-time load cost:
#   - perf-attach: perf can attach to an already-running process directly.
#   - callgrind-attach: valgrind can't attach to an unmodified running process, so
#     it's launched with --instr-atstart=no (near-native speed until toggled) and
#     callgrind_control -i on/off starts/stops instrumentation around the window.
#
# Builds against build-relwithdebinfo/wiredpanda (cmake --preset relwithdebinfo).
# Output is written under Scripts/profiling-out/, not to stdout.

set -euo pipefail

USAGE="usage: $0 <callgrind|perf|callgrind-attach|perf-attach> <file.panda> [seconds] [offscreen] [settle_seconds]"
TOOL="${1:?$USAGE}"
PANDA_FILE="${2:?$USAGE}"
SECONDS_TO_RUN="${3:-}"
OFFSCREEN="${4:-}"
SETTLE_SECONDS="${5:-10}"

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
        RUN_SECONDS="${SECONDS_TO_RUN:-20}"
        OUT_FILE="$OUT_DIR/callgrind.out.$STAMP"
        echo "Recording callgrind to $OUT_FILE (running ${RUN_SECONDS}s)..."
        timeout --signal=TERM "$RUN_SECONDS" "${ENV_ARGS[@]}" \
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
        RUN_SECONDS="${SECONDS_TO_RUN:-20}"
        OUT_FILE="$OUT_DIR/perf.data.$STAMP"
        echo "Recording perf to $OUT_FILE (running ${RUN_SECONDS}s)..."
        timeout --signal=TERM "$RUN_SECONDS" "${ENV_ARGS[@]}" \
            env -u DEBUGINFOD_URLS perf record -F 99 --call-graph dwarf -o "$OUT_FILE" -- \
            "$BINARY" "$PANDA_FILE" || true
        echo "Done. Report with: env -u DEBUGINFOD_URLS perf report --stdio -g -i $OUT_FILE > report.txt"
        ;;
    perf-attach)
        # Launches unprofiled, lets it load+settle, then attaches perf to the
        # already-running process — isolating steady-state/rendering cost
        # from one-time load cost (unlike plain 'perf', which profiles from
        # process launch and so includes loading).
        RUN_SECONDS="${SECONDS_TO_RUN:-10}"
        OUT_FILE="$OUT_DIR/perf.data.$STAMP"
        echo "Launching app, settling for ${SETTLE_SECONDS}s before attaching..."
        "${ENV_ARGS[@]}" "$BINARY" "$PANDA_FILE" &
        APP_PID=$!
        sleep "$SETTLE_SECONDS"
        if ! kill -0 "$APP_PID" 2>/dev/null; then
            echo "App exited during the settle window" >&2
            exit 1
        fi
        echo "Attaching perf to PID $APP_PID for ${RUN_SECONDS}s..."
        timeout --signal=TERM "$RUN_SECONDS" \
            env -u DEBUGINFOD_URLS perf record -F 99 --call-graph dwarf -p "$APP_PID" -o "$OUT_FILE" || true
        kill "$APP_PID" 2>/dev/null || true
        wait "$APP_PID" 2>/dev/null || true
        echo "Done. Report with: env -u DEBUGINFOD_URLS perf report --stdio -g -i $OUT_FILE > report.txt"
        ;;
    callgrind-attach)
        # valgrind can't attach to an unmodified running process, so instead:
        # launch with --instr-atstart=no (runs at near-native speed until
        # toggled), let it load+settle, then callgrind_control -i on/off
        # brackets the recording window around steady-state activity only.
        RUN_SECONDS="${SECONDS_TO_RUN:-10}"
        OUT_FILE="$OUT_DIR/callgrind.out.$STAMP"
        echo "Launching app under callgrind (instrumentation off), settling for ${SETTLE_SECONDS}s..."
        "${ENV_ARGS[@]}" valgrind --tool=callgrind --instr-atstart=no --callgrind-out-file="$OUT_FILE" \
            "$BINARY" "$PANDA_FILE" &
        VALGRIND_PID=$!
        sleep "$SETTLE_SECONDS"
        if ! kill -0 "$VALGRIND_PID" 2>/dev/null; then
            echo "App exited during the settle window" >&2
            exit 1
        fi
        echo "Turning on instrumentation for ${RUN_SECONDS}s..."
        callgrind_control -i on "$VALGRIND_PID"
        sleep "$RUN_SECONDS"
        callgrind_control -i off "$VALGRIND_PID"
        kill "$VALGRIND_PID" 2>/dev/null || true
        wait "$VALGRIND_PID" 2>/dev/null || true
        echo "Done. Annotate with: callgrind_annotate --auto=yes $OUT_FILE | head -80"
        ;;
    *)
        echo "Unknown tool '$TOOL' - expected 'callgrind', 'perf', 'callgrind-attach', or 'perf-attach'" >&2
        exit 1
        ;;
esac
