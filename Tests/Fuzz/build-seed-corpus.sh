#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Bootstrap a libFuzzer seed corpus for fuzz_deserialize.
#
# Usage: build-seed-corpus.sh <output-dir>
#
# Pulls every .panda file from Examples/, Tests/BackwardCompatibility/,
# Tests/Fixtures/, and Tests/Integration/ into a flat directory.
# libFuzzer treats each entry as one input.
#
# Including Integration fixtures exercises element-specific load() paths
# (Clock, TruthTable, Buzzer, AudioBox, Node, IC, etc.) that BackwardCompatibility
# files do not always cover.
set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <output-dir>" >&2
    exit 1
fi

OUT_DIR=$1
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT=$(cd "${SCRIPT_DIR}/../.." && pwd)

mkdir -p "${OUT_DIR}"

count=0
while IFS= read -r -d '' src; do
    # Hash the path to give every seed a unique flat name even when the
    # source has duplicate basenames (e.g. dflipflop.panda exists in many
    # BackwardCompatibility/vN.M/ subdirs).
    rel=${src#${ROOT}/}
    hash=$(printf '%s' "${rel}" | sha1sum | cut -c1-12)
    cp "${src}" "${OUT_DIR}/${hash}-$(basename "${src}")"
    count=$((count + 1))
done < <(find "${ROOT}/Examples" \
              "${ROOT}/Tests/BackwardCompatibility" \
              "${ROOT}/Tests/Fixtures" \
              "${ROOT}/Tests/Integration" \
              -type f -name '*.panda' -print0 2>/dev/null)

echo "Wrote ${count} seed files to ${OUT_DIR}"
