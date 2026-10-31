#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Counts the instructions of one phase of Tests/Perf/TestPerfProbe.cpp under callgrind.
# Usage: Scripts/profile_probe.sh <phase|all|baseline|extents> [file.panda] [tree]
#
#   phase      load        open a file: read it into a workspace and build its simulation
#              loadframes  open a file in a view already on screen, then 3 frames at 1:1 and 3 zoomed to fit
#              idle        one simulation tick with nothing changed
#              addwire     add one wire as completing one does, then the tick that rebuilds the simulation
#              undo        undo that wire, then the tick
#              move        move one element 64 units as a drag does, then the tick
#              delete      delete one element as the Delete key does, then the tick
#              frames      3 frames at 1:1, then 3 zoomed to fit
#              pan         2 frames at 1:1, then 8 more, each after scrolling 48 pixels
#              run         the circuit simulated for 8 ticks of 501 ms, its time stepped by the probe so a 1 Hz clock
#                          flips at each, a frame painted at 1:1 after every tick
#              runfit      the same, zoomed to fit
#              extents     (not under valgrind) print how many wires fall in each length range
#              all         every phase above but extents, at once
#              baseline    run and runfit on clocked_2000, combinational_1000 and ic_heavy, at once: the rendering
#                          baseline every plan records at its end (.claude/RENDERING_BASELINE.md); file is ignored
#   file       the circuit; default is the PROBE_FILE environment variable, else Tests/Fixtures/Perf/clocked_2000.panda
#   tree       a checkout holding build-relwithdebinfo/test_wiredpanda (default: this one), such as a worktree of an
#              earlier commit that has the probe, to measure the same phase before a change
#
# Each phase runs in its own valgrind process and only the phase's function is collected (--toggle-collect),
# so the load and settling before it are not counted and numbers compare between builds. Frames are painted
# through the workspace's own view off screen, with wire antialiasing held on. Qt's hashes are seeded with 0, so a
# table is walked in the same order in every run. A count does not depend on the machine's load, so phases run at
# once, one core each, each with its own settings directory.
#
# The test binary is run directly: the probe is skipped unless WIREDPANDA_PERF_PROBE is set, which only this script
# sets, and CTest presets of schema version 3 ignore a filter, so a preset cannot pick one phase.
#
# Builds against build-relwithdebinfo (cmake --build --preset relwithdebinfo), whose symbols name the phases.
# Callgrind output is written under Scripts/profiling-out/; the table goes to stdout.

set -euo pipefail

USAGE="usage: $0 <load|loadframes|idle|addwire|undo|move|delete|frames|pan|run|runfit|extents|all|baseline> [file.panda] [tree]"
PHASE="${1:?$USAGE}"
FILE="${2:-${PROBE_FILE:-}}"
TREE="${3:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
BINARY="$TREE/build-relwithdebinfo/test_wiredpanda"

if [[ ! -x "$BINARY" ]]; then
    echo "Missing $BINARY - build it first: cmake --build --preset relwithdebinfo" >&2
    exit 1
fi

# The circuit's absolute path, which the probe reads relative to nothing
absolute() {
    if [[ ! -f "$1" ]]; then
        echo "No such file: $1" >&2
        exit 1
    fi
    echo "$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
}

if [[ -n "$FILE" ]]; then
    FILE="$(absolute "$FILE")"
fi

OUT_DIR="$TREE/Scripts/profiling-out"
mkdir -p "$OUT_DIR"
SCRATCH="$(mktemp -d)"
trap 'rm -rf "$SCRATCH"' EXIT

# Off screen, with hashes seeded alike, and no Qt Test watchdog: a phase under valgrind can run for minutes, and the
# watchdog's stack dump can hang the process.
export QT_QPA_PLATFORM=offscreen
export QT_HASH_SEED=0
export WIREDPANDA_PERF_PROBE=1
export QTEST_FUNCTION_TIMEOUT=7200000
export QTEST_DISABLE_STACK_DUMP=1

# phase -> "test function:function whose instructions are collected"
declare -A PHASES=(
    [load]="load:phaseLoad"
    [loadframes]="loadThenFrames:phaseLoadThenFrames"
    [idle]="idleTick:phaseIdleTick"
    [addwire]="addWire:phaseAddWire"
    [undo]="addWireThenUndo:phaseUndo"
    [move]="moveElement:phaseMove"
    [delete]="deleteElement:phaseDelete"
    [frames]="frames:phaseFrames"
    [pan]="pan:phasePan"
    [run]="run:phaseRun"
    [runfit]="runFitted:phaseRunFitted"
)
ORDER=(load loadframes idle addwire undo move delete frames pan run runfit)

# Counts one phase, on the circuit at $2 (the probe's default when empty), and prints its row
run_phase() {
    local name="$1"
    local file="$2"
    local spec="${PHASES[$name]}"
    local test_function="${spec%%:*}"
    local collected="${spec##*:}"
    local circuit
    circuit="$(basename "${file:-clocked_2000.panda}" .panda)"
    local out_file="$OUT_DIR/probe_${name}_${circuit}_$$.out"
    local config
    config="$(mktemp -d -p "$SCRATCH")"
    local log
    # "::name(" matches the function and its compiler clones, never a longer name that starts the same way:
    # phaseLoad does not match phaseLoadThenFrames.
    log="$(cd "$TREE" && PROBE_FILE="$file" XDG_CONFIG_HOME="$config" valgrind --tool=callgrind \
        --callgrind-out-file="$out_file" --toggle-collect="*::${collected}(*" \
        "$BINARY" TestPerfProbe "$test_function" 2>&1 || true)"
    if ! grep -q "^PASS   : TestPerfProbe::${test_function}()" <<<"$log"; then
        echo "phase $name on $circuit failed:" >&2
        tail -20 <<<"$log" >&2
        return 1
    fi
    local ir
    ir="$(sed -n 's/.*Collected *: *\([0-9]*\).*/\1/p' <<<"$log" | tail -1)"
    if [[ -z "$ir" || "$ir" == 0 ]]; then
        echo "phase $name on $circuit collected no instructions: no function matched *::${collected}(*" >&2
        return 1
    fi
    printf '%-12s %-20s %17s Ir   %s\n' "$name" "$circuit" "$(printf "%'d" "$ir")" "$(basename "$out_file")"
}

# Counts each "phase|file" given at once, one valgrind process each, and prints their rows in the order given
run_at_once() {
    local -a pids=()
    local i=0
    for job in "$@"; do
        run_phase "${job%%|*}" "${job#*|}" >"$SCRATCH/row_$i" 2>&1 &
        pids+=("$!")
        i=$((i + 1))
    done
    local failed=0
    for pid in "${pids[@]}"; do
        wait "$pid" || failed=1
    done
    for ((j = 0; j < i; j++)); do
        cat "$SCRATCH/row_$j"
    done
    return "$failed"
}

case "$PHASE" in
    extents)
        (cd "$TREE" && PROBE_FILE="$FILE" "$BINARY" TestPerfProbe wireExtents 2>&1) | sed -n '/WIRE EXTENTS/,/^PASS/p' | sed '$d'
        ;;
    all)
        jobs=()
        for name in "${ORDER[@]}"; do
            jobs+=("$name|$FILE")
        done
        run_at_once "${jobs[@]}"
        ;;
    baseline)
        fixtures="$TREE/Tests/Fixtures/Perf"
        jobs=()
        for circuit in clocked_2000 combinational_1000 ic_heavy; do
            for name in run runfit; do
                jobs+=("$name|$(absolute "$fixtures/$circuit.panda")")
            done
        done
        run_at_once "${jobs[@]}"
        ;;
    *)
        if [[ -z "${PHASES[$PHASE]:-}" ]]; then
            echo "$USAGE" >&2
            exit 1
        fi
        run_phase "$PHASE" "$FILE"
        ;;
esac
