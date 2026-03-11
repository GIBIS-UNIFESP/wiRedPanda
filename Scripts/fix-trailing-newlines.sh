#!/bin/bash

# Script to check and fix trailing newlines in .cpp/.h files
# Usage: ./fix-trailing-newlines.sh [--check-only]

CHECK_ONLY=false
if [[ "$1" == "--check-only" ]]; then
    CHECK_ONLY=true
fi

FIXED_COUNT=0
MISSING_COUNT=0

# Find all .cpp and .h files
while IFS= read -r file; do
    # Skip empty files
    if [[ ! -s "$file" ]]; then
        continue
    fi

    # Check if file ends with empty line (double newline)
    last_two=$(tail -c 2 "$file" | od -An -tx1)
    if [[ "$last_two" != " 0a 0a" ]]; then
        MISSING_COUNT=$((MISSING_COUNT + 1))
        if [[ "$CHECK_ONLY" == true ]]; then
            echo "Missing empty line at end: $file"
        else
            echo "Fixing: $file"
            # Ensure file ends with newline, then add empty line
            if [[ -n $(tail -c 1 "$file") ]]; then
                printf '\n' >> "$file"
            fi
            printf '\n' >> "$file"
            FIXED_COUNT=$((FIXED_COUNT + 1))
        fi
    fi
done < <(find . -type f \( -name "*.cpp" -o -name "*.h" \) -not -path "./build*/*" -not -path "./.git/*")

if [[ "$CHECK_ONLY" == true ]]; then
    echo ""
    echo "Found $MISSING_COUNT files missing trailing newlines"
    exit $([[ $MISSING_COUNT -gt 0 ]] && echo 1 || echo 0)
else
    echo ""
    echo "Fixed $FIXED_COUNT files"
fi
