#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Launch one or more fuzz harnesses in unattended mode: -fork workers, ignore
# crash/oom/timeout findings (logged to disk, not fatal), survive terminal
# close.  Prints the PID(s) so you can `kill` them later.
#
# Usage: run-unattended.sh [build-dir] [workers] [harness]
#   build-dir  defaults to build-fuzzer
#   workers    defaults to 1
#   harness    one of: deserialize | ic_blob | structured | waveform | clipboard | old_format | ic_registry | codegen | round_trip | ic_file | copy_panda | undo | all
#              defaults to all  (runs all harnesses in parallel)
#
# Stop with:   kill <pid>   (or: pkill -f fuzz_)
# Watch with:  tail -f <build-dir>/fuzz_<harness>.log
set -euo pipefail

BUILD_DIR=${1:-build-fuzzer}
WORKERS=${2:-1}
HARNESS=${3:-all}

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT=$(cd "${SCRIPT_DIR}/../.." && pwd)

CORPUS="${ROOT}/${BUILD_DIR}/seed-corpus"
IC_CORPUS="${ROOT}/${BUILD_DIR}/seed-corpus-ic"
FINDINGS="${ROOT}/${BUILD_DIR}/findings"
DICT="${ROOT}/Tests/Fuzz/panda.dict"
SUPPRESSIONS="${ROOT}/Tests/lsan_suppressions.txt"

if [[ ! -d "${CORPUS}" ]]; then
    echo "Seed corpus missing at ${CORPUS}" >&2
    echo "Bootstrap it: ${SCRIPT_DIR}/build-seed-corpus.sh ${CORPUS}" >&2
    exit 1
fi

# IC corpus: the inner .panda files that live inside IC blobs.
# These are the raw sub-circuit files, perfect seeds for fuzz_ic_blob.
if [[ ! -d "${IC_CORPUS}" ]]; then
    mkdir -p "${IC_CORPUS}"
    find "${ROOT}/Tests/Integration/IC" -name "*.panda" -exec \
        sh -c 'cp "$1" "${2}/$(printf "%s" "$1" | sha1sum | cut -c1-12)-$(basename "$1")"' \
        _ {} "${IC_CORPUS}" \;
    echo "Built IC seed corpus: $(ls "${IC_CORPUS}" | wc -l) files"
fi

mkdir -p "${FINDINGS}"

export ASAN_OPTIONS=abort_on_error=1:halt_on_error=1
export UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1
export LSAN_OPTIONS=suppressions="${SUPPRESSIONS}"

DICT_FLAG=()
if [[ -f "${DICT}" ]]; then
    DICT_FLAG=(-dict="${DICT}")
fi

# Common flags shared by all harnesses
COMMON_FLAGS=(
    -fork="${WORKERS}"
    -ignore_crashes=1
    -ignore_ooms=1
    -ignore_timeouts=1
    -use_value_profile=1
    -rss_limit_mb=2048
    -timeout=30
    -artifact_prefix="${FINDINGS}/"
    "${DICT_FLAG[@]}"
)

launch() {
    local binary="$1"
    local log="$2"
    local corpus="$3"

    if [[ ! -x "${ROOT}/${BUILD_DIR}/${binary}" ]]; then
        echo "WARNING: ${binary} not found — skipping" >&2
        return
    fi

    nohup "${ROOT}/${BUILD_DIR}/${binary}" \
        "${COMMON_FLAGS[@]}" \
        "${corpus}" \
        > "${log}" 2>&1 &
    local pid=$!
    disown
    echo "${binary} started (pid ${pid}, ${WORKERS} workers)"
    echo "  log:  ${log}"
}

case "${HARNESS}" in
deserialize)
    launch fuzz_deserialize "${ROOT}/${BUILD_DIR}/fuzz_deserialize.log" "${CORPUS}"
    ;;
ic_blob)
    launch fuzz_ic_blob "${ROOT}/${BUILD_DIR}/fuzz_ic_blob.log" "${IC_CORPUS}"
    ;;
structured)
    launch fuzz_structured "${ROOT}/${BUILD_DIR}/fuzz_structured.log" "${CORPUS}"
    ;;
waveform)
    launch fuzz_waveform "${ROOT}/${BUILD_DIR}/fuzz_waveform.log" "${CORPUS}"
    ;;
clipboard)
    launch fuzz_clipboard    "${ROOT}/${BUILD_DIR}/fuzz_clipboard.log"    "${CORPUS}"
    ;;
old_format)
    launch fuzz_old_format   "${ROOT}/${BUILD_DIR}/fuzz_old_format.log"   "${CORPUS}"
    ;;
ic_registry)
    launch fuzz_ic_registry  "${ROOT}/${BUILD_DIR}/fuzz_ic_registry.log"  "${CORPUS}"
    ;;
codegen)
    launch fuzz_codegen      "${ROOT}/${BUILD_DIR}/fuzz_codegen.log"      "${CORPUS}"
    ;;
round_trip)
    launch fuzz_round_trip   "${ROOT}/${BUILD_DIR}/fuzz_round_trip.log"   "${CORPUS}"
    ;;
ic_file)
    launch fuzz_ic_file      "${ROOT}/${BUILD_DIR}/fuzz_ic_file.log"      "${CORPUS}"
    ;;
copy_panda)
    launch fuzz_copy_panda   "${ROOT}/${BUILD_DIR}/fuzz_copy_panda.log"   "${CORPUS}"
    ;;
undo)
    launch fuzz_undo         "${ROOT}/${BUILD_DIR}/fuzz_undo.log"         "${CORPUS}"
    ;;
all)
    launch fuzz_deserialize  "${ROOT}/${BUILD_DIR}/fuzz_deserialize.log"  "${CORPUS}"
    launch fuzz_ic_blob      "${ROOT}/${BUILD_DIR}/fuzz_ic_blob.log"      "${IC_CORPUS}"
    launch fuzz_structured   "${ROOT}/${BUILD_DIR}/fuzz_structured.log"   "${CORPUS}"
    launch fuzz_waveform     "${ROOT}/${BUILD_DIR}/fuzz_waveform.log"     "${CORPUS}"
    launch fuzz_clipboard    "${ROOT}/${BUILD_DIR}/fuzz_clipboard.log"    "${CORPUS}"
    launch fuzz_old_format   "${ROOT}/${BUILD_DIR}/fuzz_old_format.log"   "${CORPUS}"
    launch fuzz_ic_registry  "${ROOT}/${BUILD_DIR}/fuzz_ic_registry.log"  "${CORPUS}"
    launch fuzz_codegen      "${ROOT}/${BUILD_DIR}/fuzz_codegen.log"      "${CORPUS}"
    launch fuzz_round_trip   "${ROOT}/${BUILD_DIR}/fuzz_round_trip.log"   "${CORPUS}"
    launch fuzz_ic_file      "${ROOT}/${BUILD_DIR}/fuzz_ic_file.log"      "${CORPUS}"
    launch fuzz_copy_panda   "${ROOT}/${BUILD_DIR}/fuzz_copy_panda.log"   "${CORPUS}"
    launch fuzz_undo         "${ROOT}/${BUILD_DIR}/fuzz_undo.log"         "${CORPUS}"
    ;;
*)
    echo "Unknown harness '${HARNESS}'. Use: deserialize | ic_blob | structured | waveform | clipboard | old_format | ic_registry | codegen | round_trip | ic_file | copy_panda | undo | all" >&2
    exit 1
    ;;
esac

echo
echo "Findings: ${FINDINGS}/"
echo "Stop all: pkill -f fuzz_"
