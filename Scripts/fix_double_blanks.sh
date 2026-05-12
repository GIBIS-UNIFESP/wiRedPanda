#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Collapse consecutive blank lines in C++ source files.
# Usage: fix_double_blanks.sh [dir...]
# Defaults to the git repository root if no dirs given.

set -euo pipefail

dirs=("$@")

if [[ ${#dirs[@]} -eq 0 ]]; then
    if root=$(git rev-parse --show-toplevel 2>/dev/null); then
        dirs=("$root")
    else
        dirs=("$(cd "$(dirname "$0")/.." && pwd)")
    fi
fi

found=0

while IFS= read -r -d '' file; do
    if awk 'FNR==1{prev="x"} prev==""&&/^$/{found=1; exit} {prev=$0} END{exit !found}' "$file"; then
        echo "$file: consecutive blank lines found"
        found=1
        awk '/^$/{if(!blank)print; blank=1; next} {blank=0; print}' "$file" > "$file.tmp" && mv "$file.tmp" "$file"
        echo "$file: fixed"
    fi
done < <(find "${dirs[@]}" \
    -type d \( -name 'build*' -o -name '.git' -o -name '.venv' -o -name '.github' -o -name '_deps' -o -name '.vscode' -o -name '.idea' \) -prune -o \
    -type f \( -name '*.cpp' -o -name '*.h' \) -print0)

if [[ $found -eq 0 ]]; then
    echo "No consecutive blank lines found."
fi
