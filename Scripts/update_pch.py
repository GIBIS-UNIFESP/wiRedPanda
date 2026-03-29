#!/usr/bin/env python3
"""
Script to analyze Qt includes used in the codebase and update the PCH file.
Finds all Qt includes used and adds missing ones, removes unused ones.
"""

import re
import os
from pathlib import Path
from collections import defaultdict

def find_source_files(root_dir):
    """Find all C++ source and header files (excluding pch.h)."""
    extensions = {'.cpp', '.h', '.hpp', '.cc', '.cxx'}
    source_files = []

    for root, dirs, files in os.walk(root_dir):
        # Skip build directories and external deps
        dirs[:] = [d for d in dirs if d not in {'build', 'build_no', '.git', '.github', 'cmake', '_deps', '.vscode'}]

        for file in files:
            if Path(file).suffix in extensions and file != 'pch.h':  # Don't scan the PCH itself
                source_files.append(os.path.join(root, file))

    return source_files

def extract_qt_includes(file_path):
    """Extract all Qt includes from a file."""
    qt_includes = set()

    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

            # Find all #include <Q...> patterns (but not module-based paths like QtCore/)
            pattern = r'#include\s+<(Q[^/\n>]+)>'
            matches = re.findall(pattern, content)

            for match in matches:
                qt_includes.add(f'<{match}>')

    except Exception as e:
        print(f"Warning: Could not read {file_path}: {e}")

    return qt_includes

def get_pch_includes(pch_path):
    """Extract current Qt includes from PCH file."""
    qt_includes = set()

    try:
        with open(pch_path, 'r', encoding='utf-8') as f:
            content = f.read()

            # Find all #include <Q...> lines
            pattern = r'#include\s+(<Q[^>]+>)'
            matches = re.findall(pattern, content)

            for match in matches:
                qt_includes.add(match)

    except Exception as e:
        print(f"Error reading PCH: {e}")

    return qt_includes

def update_pch_file(pch_path, used_includes):
    """Update the PCH file with the correct includes."""

    # Read the current PCH
    with open(pch_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    # Find the Qt includes section
    start_idx = None
    end_idx = None

    for i, line in enumerate(lines):
        if '// Qt includes' in line:
            start_idx = i
        elif start_idx is not None and '#endif' in line:
            end_idx = i
            break

    if start_idx is None:
        print("Could not find Qt includes section in PCH")
        return False

    if end_idx is None:
        end_idx = len(lines)

    # Sort includes for consistency
    sorted_includes = sorted(used_includes)

    # Build new content: keep header, add new includes, keep footer
    new_lines = lines[:start_idx + 1]  # Keep everything up to and including "// Qt includes"
    new_lines.append('\n')

    for include in sorted_includes:
        new_lines.append(f'#include {include}\n')

    new_lines.append('\n')
    new_lines.extend(lines[end_idx:])  # Add the #endif and beyond

    # Write back
    with open(pch_path, 'w', encoding='utf-8') as f:
        f.writelines(new_lines)

    return True

def main():
    root_dir = '/home/torres/wiredpanda'
    pch_path = os.path.join(root_dir, 'pch.h')

    print("Scanning source files for Qt includes (excluding pch.h)...")
    source_files = find_source_files(root_dir)
    print(f"Found {len(source_files)} source files")

    # Collect all Qt includes used
    all_includes = set()
    file_includes = defaultdict(set)

    for file_path in source_files:
        includes = extract_qt_includes(file_path)
        if includes:
            file_includes[file_path] = includes
            all_includes.update(includes)

    # Get current PCH includes
    current_pch = get_pch_includes(pch_path)

    # Analyze differences
    missing = all_includes - current_pch
    unused = current_pch - all_includes

    print(f"\nQt includes analysis:")
    print(f"Total unique Qt includes used: {len(all_includes)}")
    print(f"Current PCH includes: {len(current_pch)}")

    if missing:
        print(f"\nMissing from PCH ({len(missing)}):")
        for include in sorted(missing):
            print(f"  {include}")

    if unused:
        print(f"\nUnused in PCH ({len(unused)}):")
        for include in sorted(unused):
            print(f"  {include}")

    if not missing and not unused:
        print("\nPCH is already up to date!")
        return

    # Update the PCH
    if update_pch_file(pch_path, all_includes):
        print(f"\n✓ PCH updated successfully!")
        print(f"  Total includes: {len(all_includes)}")

        if missing:
            print(f"  Added: {len(missing)}")
        if unused:
            print(f"  Removed: {len(unused)}")
    else:
        print("Failed to update PCH")

if __name__ == '__main__':
    main()
