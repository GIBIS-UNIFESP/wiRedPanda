#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Generate an LLVM source-based line-coverage report for the fuzz harnesses.
#
# Usage: coverage-report.sh [build-dir]
#   build-dir  defaults to build-fuzzer-coverage
#
# Prerequisites:
#   cmake --preset fuzzer-coverage && cmake --build --preset fuzzer-coverage
#
# Output: build-dir/coverage-report/index.html
#         Open in a browser to see which lines are never reached by the corpus.
set -euo pipefail

BUILD_DIR=${1:-build-fuzzer-coverage}

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT=$(cd "${SCRIPT_DIR}/../.." && pwd)

CORPUS="${ROOT}/build-fuzzer/seed-corpus"
IC_CORPUS="${ROOT}/build-fuzzer/seed-corpus-ic"
REPORT_DIR="${ROOT}/${BUILD_DIR}/coverage-report"
PROF_DIR="${ROOT}/${BUILD_DIR}/profiles"

mkdir -p "${PROF_DIR}" "${REPORT_DIR}"

# Check for required tools
for tool in llvm-profdata llvm-cov; do
    if ! command -v ${tool} &>/dev/null; then
        echo "ERROR: ${tool} not found. Install llvm-tools." >&2
        exit 1
    fi
done

declare -a BINARIES=()
declare -a PROF_FILES=()

run_harness() {
    local name="$1"
    local binary="${ROOT}/${BUILD_DIR}/${name}"
    local corpus="$2"
    local profraw="${PROF_DIR}/${name}.profraw"

    if [[ ! -x "${binary}" ]]; then
        echo "  SKIP: ${name} not built" >&2
        return
    fi
    if [[ ! -d "${corpus}" ]]; then
        echo "  SKIP: corpus ${corpus} missing" >&2
        return
    fi

    echo "  Running ${name} on corpus..."
    LLVM_PROFILE_FILE="${profraw}" \
        "${binary}" -runs=0 "${corpus}" 2>/dev/null || true

    BINARIES+=("${binary}")
    PROF_FILES+=("${profraw}")
}

echo "=== Collecting coverage from harnesses ==="
run_harness fuzz_deserialize  "${CORPUS}"
run_harness fuzz_ic_blob      "${IC_CORPUS:-${CORPUS}}"
run_harness fuzz_structured   "${CORPUS}"
run_harness fuzz_waveform     "${CORPUS}"
run_harness fuzz_clipboard    "${CORPUS}"
run_harness fuzz_old_format   "${CORPUS}"
run_harness fuzz_ic_registry  "${CORPUS}"
run_harness fuzz_codegen      "${CORPUS}"
run_harness fuzz_round_trip   "${CORPUS}"
run_harness fuzz_ic_file      "${CORPUS}"
run_harness fuzz_copy_panda   "${CORPUS}"
run_harness fuzz_undo         "${CORPUS}"

if [[ ${#PROF_FILES[@]} -eq 0 ]]; then
    echo "No harnesses ran — nothing to report." >&2
    exit 1
fi

echo "=== Merging profiles ==="
llvm-profdata merge -sparse "${PROF_FILES[@]}" \
    -o "${PROF_DIR}/merged.profdata"

echo "=== Generating HTML report ==="
# Build -object flags for all binaries
OBJECT_FLAGS=()
for bin in "${BINARIES[@]}"; do
    OBJECT_FLAGS+=(-object "${bin}")
done

llvm-cov show \
    "${BINARIES[0]}" \
    "${OBJECT_FLAGS[@]:1}" \
    -instr-profile="${PROF_DIR}/merged.profdata" \
    -format=html \
    -output-dir="${REPORT_DIR}" \
    -show-line-counts-or-regions \
    -show-instantiations=false \
    -ignore-filename-regex="(_deps|Qt|build-fuzzer-coverage)" \
    "${ROOT}/App"

echo ""
echo "=== Summary ==="
llvm-cov report \
    "${BINARIES[0]}" \
    "${OBJECT_FLAGS[@]:1}" \
    -instr-profile="${PROF_DIR}/merged.profdata" \
    -ignore-filename-regex="(_deps|Qt|build-fuzzer-coverage)" \
    "${ROOT}/App" \
    | tail -20

echo ""
echo "HTML report: ${REPORT_DIR}/index.html"
