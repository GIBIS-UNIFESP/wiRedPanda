#!/usr/bin/env bash
# Find (and optionally fix) consecutive blank lines in source files.
# Usage: ~/find_double_blanks.sh [--fix] [dir...]
# Defaults to ~/wiredpanda if no dirs given.

set -euo pipefail

WIREDPANDA="$HOME/wiredpanda"
FIX=0
dirs=()

for arg in "$@"; do
    if [[ "$arg" == "--fix" ]]; then
        FIX=1
    else
        dirs+=("$arg")
    fi
done

if [[ ${#dirs[@]} -eq 0 ]]; then
    dirs=("$WIREDPANDA")
fi

found=0

while IFS= read -r -d '' file; do
    if awk 'FNR==1{prev="x"} prev==""&&/^$/{found=1; exit} {prev=$0} END{exit !found}' "$file"; then
        echo "$file: consecutive blank lines found"
        found=1
        if [[ $FIX -eq 1 ]]; then
            awk '/^$/{if(!blank)print; blank=1; next} {blank=0; print}' "$file" > "$file.tmp" && mv "$file.tmp" "$file"
            echo "$file: fixed"
        fi
    fi
done < <(find "${dirs[@]}" -type d -name "build*" -prune -o -type f \( -name "*.cpp" -o -name "*.h" \) -print0)

if [[ $found -eq 0 ]]; then
    echo "No consecutive blank lines found."
fi
