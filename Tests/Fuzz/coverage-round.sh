#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# One coverage-guided round for one harness: fuzz it for a while on its own corpus, replay that corpus through the
# coverage build, and write the round's coverage of App/ per file and per function, so the code the harness misses
# can be read off and the next round compared with this one.
#
# Usage: coverage-round.sh <harness> [seconds] [fuzz-build-dir] [coverage-build-dir]
#   harness             deserialize | ic_blob | structured | waveform | clipboard | old_format | ic_registry |
#                       codegen | round_trip | ic_file | copy_panda | undo
#   seconds             fuzzing time, defaults to 300
#   fuzz-build-dir      defaults to build-fuzzer          (cmake --preset fuzzer)
#   coverage-build-dir  defaults to build-fuzzer-coverage (cmake --preset fuzzer-coverage)
#
# The corpus lives in <fuzz-build-dir>/rounds/<harness>/corpus and grows from round to round; its first round starts
# from the corpus run-unattended.sh gives the harness. Round n writes <fuzz-build-dir>/rounds/<harness>/round-<n>/:
# fuzz.log, files.txt (per file), functions.txt (per function), and any crash, leak, OOM or timeout input.
set -euo pipefail

HARNESS=${1:?usage: coverage-round.sh <harness> [seconds] [fuzz-build-dir] [coverage-build-dir]}
FUZZ_SECONDS=${2:-300}
FUZZ_DIR=${3:-build-fuzzer}
COV_DIR=${4:-build-fuzzer-coverage}

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT=$(cd "${SCRIPT_DIR}/../.." && pwd)

FUZZ_BIN="${ROOT}/${FUZZ_DIR}/fuzz_${HARNESS}"
COV_BIN="${ROOT}/${COV_DIR}/fuzz_${HARNESS}"
ROUNDS="${ROOT}/${FUZZ_DIR}/rounds/${HARNESS}"
CORPUS="${ROUNDS}/corpus"
IGNORE="(_deps|Qt|${COV_DIR}|Tests/)"

for binary in "${FUZZ_BIN}" "${COV_BIN}"; do
    if [[ ! -x "${binary}" ]]; then
        echo "Missing ${binary}: build the fuzzer and fuzzer-coverage presets first" >&2
        exit 1
    fi
done
for tool in llvm-profdata llvm-cov; do
    if ! command -v "${tool}" &>/dev/null; then
        echo "ERROR: ${tool} not found. Install llvm-tools." >&2
        exit 1
    fi
done

# The first round starts from the corpus run-unattended.sh would give this harness
mkdir -p "${CORPUS}"
if [[ -z "$(ls -A "${CORPUS}")" ]]; then
    if [[ "${HARNESS}" == "ic_blob" ]]; then
        find "${ROOT}/Tests/Integration/IC" -name "*.panda" -exec \
            sh -c 'cp "$1" "${2}/$(printf "%s" "$1" | sha1sum | cut -c1-12)-$(basename "$1")"' \
            _ {} "${CORPUS}" \;
    elif [[ -d "${ROOT}/${FUZZ_DIR}/seed-corpus" ]]; then
        cp "${ROOT}/${FUZZ_DIR}/seed-corpus/"* "${CORPUS}/"
    fi
    # fuzz_clipboard pastes clipboards, which no file holds: one pass over the .panda files writes the ones it copies
    if [[ "${HARNESS}" == "clipboard" ]]; then
        FUZZ_CLIPBOARD_SEED_DIR="${CORPUS}" "${FUZZ_BIN}" -runs=0 "${CORPUS}" > "${ROUNDS}/clipboard-seeds.log" 2>&1 || true
    fi
fi

# fuzz_codegen builds a circuit from any input that is not a .panda file, which the corpus holds none of: 64 inputs of
# fixed pseudo-random bytes, none starting with a .panda file's first byte, start that path
if [[ "${HARNESS}" == "codegen" ]] && [[ -z "$(find "${CORPUS}" -maxdepth 1 -name 'structured-seed-*' -print -quit)" ]]; then
    for i in $(seq 1 64); do
        seed="${CORPUS}/structured-seed-${i}"
        { printf '\001'
          for j in $(seq 1 12); do printf '%s' "${i}-${j}" | sha256sum | cut -c1-64 | xxd -r -p; done; } > "${seed}"
        truncate -s $(( 65 + 4 * i )) "${seed}"
    done
fi

ROUND=$(( $(find "${ROUNDS}" -maxdepth 1 -name 'round-*' | wc -l) + 1 ))
OUT="${ROUNDS}/round-${ROUND}"
PREVIOUS="${ROUNDS}/round-$(( ROUND - 1 ))"
mkdir -p "${OUT}"

export ASAN_OPTIONS=abort_on_error=1:halt_on_error=1
export UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1
export LSAN_OPTIONS=suppressions="${ROOT}/Tests/lsan_suppressions.txt"

count_inputs() { find "${CORPUS}" -maxdepth 1 -type f | wc -l; }

echo "=== Round ${ROUND} of fuzz_${HARNESS}: fuzzing ${FUZZ_SECONDS} s on $(count_inputs) inputs ==="
# A finding ends the run early; it is kept in the round's directory and reported below
"${FUZZ_BIN}" -max_total_time="${FUZZ_SECONDS}" -rss_limit_mb=2048 -timeout=30 \
    -dict="${SCRIPT_DIR}/panda.dict" -artifact_prefix="${OUT}/" "${CORPUS}" > "${OUT}/fuzz.log" 2>&1 || true
grep -E "^#[0-9]+ +DONE|^Done " "${OUT}/fuzz.log" || tail -3 "${OUT}/fuzz.log"

echo "=== Replaying $(count_inputs) inputs through the coverage build ==="
LLVM_PROFILE_FILE="${OUT}/coverage.profraw" "${COV_BIN}" -runs=0 "${CORPUS}" > "${OUT}/replay.log" 2>&1 || true
llvm-profdata merge -sparse "${OUT}/coverage.profraw" -o "${OUT}/coverage.profdata"
rm -f "${OUT}/coverage.profraw"

SOURCES=$(find "${ROOT}/App" \( -name '*.cpp' -o -name '*.h' \) | sort)
# shellcheck disable=SC2086
llvm-cov report "${COV_BIN}" -instr-profile="${OUT}/coverage.profdata" -ignore-filename-regex="${IGNORE}" \
    ${SOURCES} > "${OUT}/files.txt"
# shellcheck disable=SC2086
llvm-cov report "${COV_BIN}" -instr-profile="${OUT}/coverage.profdata" -ignore-filename-regex="${IGNORE}" \
    -show-functions ${SOURCES} > "${OUT}/functions.txt"

echo "=== Coverage of App/ (regions, functions, lines) ==="
grep '^TOTAL' "${OUT}/files.txt"
if [[ -f "${PREVIOUS}/files.txt" ]]; then
    echo "--- round $(( ROUND - 1 )):"
    grep '^TOTAL' "${PREVIOUS}/files.txt"
fi
FINDINGS=$(find "${OUT}" -maxdepth 1 \( -name 'crash-*' -o -name 'leak-*' -o -name 'oom-*' -o -name 'timeout-*' \))
if [[ -n "${FINDINGS}" ]]; then
    echo "=== Findings ==="
    echo "${FINDINGS}"
fi
echo "Per file: ${OUT}/files.txt"
echo "Per function: ${OUT}/functions.txt"
