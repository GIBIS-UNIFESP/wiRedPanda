#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Cross-pollinate and minimise corpora across all fuzz harnesses.
#
# Usage: merge-corpora.sh [build-dir]
#   build-dir  defaults to build-fuzzer
#
# What it does:
#   1. Collects inputs from every harness corpus into a shared pool.
#   2. For each harness, runs libFuzzer -merge=1 to distill the pool
#      down to only the inputs that add new coverage for that harness.
#      This cross-pollinates: a file discovered by fuzz_structured that
#      also improves fuzz_deserialize coverage gets added to its corpus.
#   3. Reports the before/after corpus sizes.
#
# Run after a fuzzing session (or on a schedule) to keep corpora lean
# and maximally informative.  The existing seed corpora are replaced
# in-place; git-tracked files are unaffected (corpora live in build-/).
set -euo pipefail

BUILD_DIR=${1:-build-fuzzer}

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT=$(cd "${SCRIPT_DIR}/../.." && pwd)

CORPORA=(
    "${ROOT}/${BUILD_DIR}/seed-corpus"
    "${ROOT}/${BUILD_DIR}/seed-corpus-ic"
)

HARNESSES=(
    "fuzz_deserialize:${ROOT}/${BUILD_DIR}/seed-corpus"
    "fuzz_ic_blob:${ROOT}/${BUILD_DIR}/seed-corpus-ic"
    "fuzz_structured:${ROOT}/${BUILD_DIR}/seed-corpus"
    "fuzz_waveform:${ROOT}/${BUILD_DIR}/seed-corpus"
    "fuzz_clipboard:${ROOT}/${BUILD_DIR}/seed-corpus"
    "fuzz_old_format:${ROOT}/${BUILD_DIR}/seed-corpus"
    "fuzz_ic_registry:${ROOT}/${BUILD_DIR}/seed-corpus"
    "fuzz_codegen:${ROOT}/${BUILD_DIR}/seed-corpus"
    "fuzz_round_trip:${ROOT}/${BUILD_DIR}/seed-corpus"
    "fuzz_ic_file:${ROOT}/${BUILD_DIR}/seed-corpus"
    "fuzz_copy_panda:${ROOT}/${BUILD_DIR}/seed-corpus"
)

SUPPRESSIONS="${ROOT}/Tests/lsan_suppressions.txt"
POOL="${ROOT}/${BUILD_DIR}/corpus-pool"

export ASAN_OPTIONS=abort_on_error=0:halt_on_error=0
export LSAN_OPTIONS=suppressions="${SUPPRESSIONS}"

# ---- Step 1: collect all inputs into a shared pool ----
echo "=== Building shared corpus pool ==="
mkdir -p "${POOL}"
pool_before=$(find "${POOL}" -type f | wc -l)

for corpus in "${CORPORA[@]}"; do
    if [[ -d "${corpus}" ]]; then
        cp -n "${corpus}"/* "${POOL}/" 2>/dev/null || true
    fi
done

pool_after=$(find "${POOL}" -type f | wc -l)
echo "  Pool: ${pool_before} → ${pool_after} inputs"

# ---- Step 2: for each harness, merge pool → corpus ----
echo ""
echo "=== Merging pool into each harness corpus ==="

for entry in "${HARNESSES[@]}"; do
    name="${entry%%:*}"
    corpus="${entry##*:}"
    binary="${ROOT}/${BUILD_DIR}/${name}"

    if [[ ! -x "${binary}" ]]; then
        echo "  SKIP ${name}: binary not found"
        continue
    fi
    if [[ ! -d "${corpus}" ]]; then
        echo "  SKIP ${name}: corpus directory missing"
        continue
    fi

    before=$(find "${corpus}" -type f | wc -l)

    # -merge=1: add inputs from pool to corpus, keeping only those that
    # increase coverage for this specific harness binary.
    "${binary}" -merge=1 "${corpus}" "${POOL}" 2>/dev/null || true

    after=$(find "${corpus}" -type f | wc -l)
    echo "  ${name}: ${before} → ${after} inputs"
done

# ---- Step 3: minimise the shared seed corpus using the primary harness ----
# Important: minimise against a single reference harness only (fuzz_deserialize).
# Running sequential per-harness minimisation compounds coverage loss because each
# step removes inputs that are useful to later harnesses.  One-pass minimisation
# on the primary harness keeps the corpus small while preserving overall coverage.
echo ""
echo "=== Minimising shared seed corpus ==="

PRIMARY="${ROOT}/${BUILD_DIR}/fuzz_deserialize"
SHARED="${ROOT}/${BUILD_DIR}/seed-corpus"

if [[ -x "${PRIMARY}" && -d "${SHARED}" ]]; then
    before=$(find "${SHARED}" -type f | wc -l)
    tmp="${SHARED}-minimised"
    mkdir -p "${tmp}"

    "${PRIMARY}" -merge=1 "${tmp}" "${POOL}" 2>/dev/null || true

    after=$(find "${tmp}" -type f | wc -l)
    if [[ "${after}" -gt 0 && "${after}" -lt "${before}" ]]; then
        rm -rf "${SHARED}"
        mv "${tmp}" "${SHARED}"
        echo "  seed-corpus: ${before} → ${after} inputs (minimised)"
    else
        rm -rf "${tmp}"
        echo "  seed-corpus: ${before} inputs (no reduction)"
    fi
fi

echo ""
echo "Done.  Re-run the harnesses to verify coverage is maintained."
