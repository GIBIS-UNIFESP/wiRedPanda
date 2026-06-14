#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
"""Analyze Qt includes used in the codebase and update the PCH file.

Finds all Qt includes used and adds missing ones, removes unused ones.

Usage:
    python3 Scripts/update_pch.py [root_dir]

If root_dir is not provided, uses the git repository root.
"""

import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

SOURCE_EXTENSIONS = {".cpp", ".h", ".hpp", ".cc", ".cxx"}
EXCLUDE_DIRS = {".git", ".venv", ".github", ".vscode", ".idea", "_deps", "cmake", "Scripts"}


def find_repo_root():
    try:
        result = subprocess.run(["git", "rev-parse", "--show-toplevel"], capture_output=True, text=True, check=True)
        return Path(result.stdout.strip())
    except (subprocess.CalledProcessError, FileNotFoundError):
        return Path(__file__).resolve().parent.parent


def should_exclude_dir(dirname):
    if dirname in EXCLUDE_DIRS:
        return True
    if dirname.startswith("."):
        return True
    if dirname.lower().startswith("build"):
        return True
    return False


def find_source_files(root):
    """Find all C++ source and header files (excluding pch.h)."""
    files = []
    for ext in SOURCE_EXTENSIONS:
        files.extend(root.glob(f"**/*{ext}"))

    return [
        f
        for f in files
        if f.name != "pch.h" and not any(should_exclude_dir(part) for part in f.relative_to(root).parts)
    ]


def extract_qt_includes(filepath):
    """Extract all Qt includes from a file."""
    qt_includes = set()

    try:
        content = filepath.read_text(encoding="utf-8", errors="ignore")
        # Find all #include <Q...> patterns (but not module-based paths like QtCore/)
        for match in re.findall(r"#include\s+<(Q[^/\n>]+)>", content):
            qt_includes.add(f"<{match}>")
    except Exception as e:
        print(f"Warning: Could not read {filepath}: {e}")

    return qt_includes


def get_pch_includes(pch_path):
    """Extract current Qt includes from PCH file."""
    qt_includes = set()

    try:
        content = pch_path.read_text(encoding="utf-8")
        for match in re.findall(r"#include\s+(<Q[^>]+>)", content):
            qt_includes.add(match)
    except Exception as e:
        print(f"Error reading PCH: {e}")

    return qt_includes


def update_pch_file(pch_path, used_includes):
    """Update the PCH file with the correct includes."""
    lines = pch_path.read_text(encoding="utf-8").splitlines(keepends=True)

    # Find the Qt includes section
    start_idx = None
    end_idx = None

    for i, line in enumerate(lines):
        if "// Qt includes" in line:
            start_idx = i
        elif start_idx is not None and "#endif" in line:
            end_idx = i
            break

    if start_idx is None:
        print("Could not find Qt includes section in PCH")
        return False

    if end_idx is None:
        end_idx = len(lines)

    sorted_includes = sorted(used_includes)

    new_lines = lines[: start_idx + 1]
    new_lines.append("\n")
    for include in sorted_includes:
        new_lines.append(f"#include {include}\n")
    new_lines.append("\n")
    new_lines.extend(lines[end_idx:])

    pch_path.write_text("".join(new_lines), encoding="utf-8")
    return True


def main():
    root = Path(sys.argv[1]) if len(sys.argv) > 1 else find_repo_root()

    if not root.exists():
        print(f"Error: root directory {root} does not exist", file=sys.stderr)
        sys.exit(1)

    pch_path = root / "pch.h"
    if not pch_path.exists():
        print(f"Error: PCH file not found at {pch_path}", file=sys.stderr)
        sys.exit(1)

    print("Scanning source files for Qt includes (excluding pch.h)...")
    source_files = find_source_files(root)
    print(f"Found {len(source_files)} source files")

    all_includes = set()
    file_includes = defaultdict(set)

    for filepath in source_files:
        includes = extract_qt_includes(filepath)
        if includes:
            file_includes[filepath] = includes
            all_includes.update(includes)

    current_pch = get_pch_includes(pch_path)

    missing = all_includes - current_pch
    unused = current_pch - all_includes

    print("\nQt includes analysis:")
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
        print("\nPCH is already up to date.")
        return

    if update_pch_file(pch_path, all_includes):
        print("\nPCH updated successfully.")
        print(f"  Total includes: {len(all_includes)}")
        if missing:
            print(f"  Added: {len(missing)}")
        if unused:
            print(f"  Removed: {len(unused)}")
    else:
        print("Failed to update PCH")


if __name__ == "__main__":
    main()
