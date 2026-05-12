#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Launch one harness in single-process mode (no -fork) so ASan/UBSan stack
# traces print to stderr inline.  Each iteration runs until the first crash,
# saves the reproducer to findings/, and exits.  Wrap in a loop to chain
# multiple crashes in one sweep — each restart picks up the previous run's
# crashing input from the corpus.
#
# Usage: run-debug.sh [harness] [iterations] [max-time-per-iter] [build-dir]
#   harness           one of: deserialize | ic_blob | structured | waveform |
#                     clipboard | old_format | ic_registry | codegen |
#                     round_trip | ic_file | copy_panda | undo
#                     defaults to old_format
#   iterations        how many restart-on-crash cycles  (default 5)
#   max-time-per-iter seconds before giving up on a clean run  (default 1800)
#   build-dir         defaults to build-fuzzer
#
# Output: build-dir/fuzz_<harness>-debug.log (tee'd live, persists across runs)
# Stack traces appear inline in the log after each ==pid==ERROR: line.
set -euo pipefail

HARNESS=${1:-old_format}
ITERATIONS=${2:-5}
MAX_TIME=${3:-1800}
BUILD_DIR=${4:-build-fuzzer}

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT=$(cd "${SCRIPT_DIR}/../.." && pwd)

CORPUS="${ROOT}/${BUILD_DIR}/seed-corpus"
IC_CORPUS="${ROOT}/${BUILD_DIR}/seed-corpus-ic"
FINDINGS="${ROOT}/${BUILD_DIR}/findings"
DICT="${ROOT}/Tests/Fuzz/panda.dict"
SUPPRESSIONS="${ROOT}/Tests/lsan_suppressions.txt"
LOG="${ROOT}/${BUILD_DIR}/fuzz_${HARNESS}-debug.log"
BINARY="${ROOT}/${BUILD_DIR}/fuzz_${HARNESS}"

if [[ ! -x "${BINARY}" ]]; then
    echo "Harness not found: ${BINARY}" >&2
    echo "Build it: cmake --build --preset fuzzer" >&2
    exit 1
fi

if [[ ! -d "${CORPUS}" ]]; then
    echo "Seed corpus missing at ${CORPUS}" >&2
    echo "Bootstrap: ${SCRIPT_DIR}/build-seed-corpus.sh ${CORPUS}" >&2
    exit 1
fi

# fuzz_ic_blob runs against the inner IC seed corpus, not the top-level one.
case "${HARNESS}" in
ic_blob) USE_CORPUS="${IC_CORPUS}" ;;
*)       USE_CORPUS="${CORPUS}" ;;
esac

mkdir -p "${FINDINGS}"

export ASAN_OPTIONS=symbolize=1:abort_on_error=0:halt_on_error=1
export UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1
export LSAN_OPTIONS=suppressions="${SUPPRESSIONS}"

DICT_FLAG=()
if [[ -f "${DICT}" ]]; then
    DICT_FLAG=(-dict="${DICT}")
fi

echo "Harness:    ${BINARY}"
echo "Corpus:     ${USE_CORPUS}"
echo "Findings:   ${FINDINGS}"
echo "Log:        ${LOG}"
echo "Iterations: ${ITERATIONS}  (each up to ${MAX_TIME}s, exits early on crash)"
echo

for i in $(seq 1 "${ITERATIONS}"); do
    echo "=== iteration ${i}/${ITERATIONS} ===" | tee -a "${LOG}"
    # `|| true` keeps the loop alive when libFuzzer exits non-zero after a crash.
    "${BINARY}" \
        -use_value_profile=1 \
        -rss_limit_mb=2048 \
        -timeout=30 \
        -artifact_prefix="${FINDINGS}/" \
        "${DICT_FLAG[@]}" \
        -max_total_time="${MAX_TIME}" \
        "${USE_CORPUS}" \
        2>&1 | tee -a "${LOG}" || true
    echo | tee -a "${LOG}"
done

echo "Done. Stack traces in ${LOG} after each '==pid==ERROR:' line."
echo "Reproducers in ${FINDINGS}/."
