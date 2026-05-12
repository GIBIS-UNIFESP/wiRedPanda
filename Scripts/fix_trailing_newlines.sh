#!/usr/bin/env bash
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Ensure C++ source files end with a final blank line (i.e. terminate with two
# newline bytes). Usage: fix_trailing_newlines.sh [dir...]
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

fixed=0

while IFS= read -r -d '' file; do
    if [[ ! -s "$file" ]]; then
        continue
    fi

    last_two=$(tail -c 2 "$file" | od -An -tx1)
    if [[ "$last_two" != " 0a 0a" ]]; then
        echo "Fixing: $file"
        if [[ -n $(tail -c 1 "$file") ]]; then
            printf '\n' >> "$file"
        fi
        printf '\n' >> "$file"
        fixed=$((fixed + 1))
    fi
done < <(find "${dirs[@]}" \
    -type d \( -name 'build*' -o -name '.git' -o -name '.venv' -o -name '.github' -o -name '_deps' -o -name '.vscode' -o -name '.idea' \) -prune -o \
    -type f \( -name '*.cpp' -o -name '*.h' \) -print0)

echo "Fixed $fixed files"
