#!/usr/bin/env python3
"""
Fix C++ include ordering violations in the wiRedPanda project.

Correct order:
1. Related header (for .cpp files) - "quotes"
2. [blank line]
3. C/C++ Standard Library headers alphabetically sorted <...> not starting with Q
4. [blank line]
5. Qt/Third-party headers alphabetically sorted (<Q...>, <nlohmann/...>, etc.)
6. [blank line]
7. Project-local headers alphabetically sorted "..."

For .h files: groups 3, 5, 7 only (no related header).

Handles #ifdef blocks by:
- Extracting includes from within #ifdef...#endif blocks
- Separating mixed-type includes into different conditional blocks
- Reordering all includes while preserving conditional compilation directives

Usage:
    python3 Scripts/fix_includes.py [root_dir] [OPTIONS]

Options:
    --check         Only report violations, don't fix them
    --fix           Fix violations (default)
    --verbose       Show detailed output per file
    --force         Reprocess all files even if unchanged

If root_dir is not provided, uses the git repository root (or script's parent directory).
"""

import re
import subprocess
import sys
from pathlib import Path
from dataclasses import dataclass
from typing import Optional, List


@dataclass
class IncludeEntry:
    """Represents a single include with optional conditional context."""
    kind: str  # 'related', 'stdlib', 'qt', 'project'
    header: str  # The include path/name
    line: str  # The raw #include line
    condition: Optional[str] = None  # E.g., "Q_OS_WIN", "HAVE_SENTRY", etc.


def find_repo_root():
    try:
        result = subprocess.run(
            ["git", "rev-parse", "--show-toplevel"],
            capture_output=True, text=True, check=True
        )
        return Path(result.stdout.strip())
    except (subprocess.CalledProcessError, FileNotFoundError):
        return Path(__file__).parent.parent


EXCLUDE_DIRS = {"build", ".venv", ".git", "Scripts"}
EXCLUDE_FILES: set = set()


def should_exclude_dir(dirname):
    """Check if a directory should be excluded from scanning."""
    if dirname in EXCLUDE_DIRS:
        return True
    if dirname.startswith("."):
        return True
    if dirname.lower().startswith("build"):
        return True
    return False


def classify_include(line):
    """
    Classify an #include line into one of:
      'stdlib'   - angle-bracket include NOT starting with Q
      'qt'       - angle-bracket include starting with Q (also nlohmann, etc.)
      'project'  - double-quoted include

    Returns (kind, header_name) or (None, None) if not an include.
    """
    m = re.match(r'^\s*#\s*include\s*(<([^>]+)>|"([^"]+)")', line)
    if not m:
        return None, None
    angle = m.group(2)
    quote = m.group(3)
    if angle is not None:
        if angle.startswith("Q") or angle.startswith("nlohmann"):
            return "qt", angle
        else:
            return "stdlib", angle
    else:
        return "project", quote


def extract_condition(line):
    """Extract preprocessor condition name from #ifdef or #if defined lines."""
    m = re.match(r'^\s*#ifdef\s+(\w+)', line)
    if m:
        return m.group(1)
    m = re.match(r'^\s*#if\s+defined\s*\(\s*(\w+)\s*\)', line)
    if m:
        return m.group(1)
    return None


def parse_include_section_with_ifdef(lines, start_idx):
    """
    Parse the include section, handling #ifdef blocks intelligently.
    Extracts all includes from #ifdef blocks, tracking their conditions.
    If an #ifdef block contains non-include code lines, those are collected
    as a trailing block to be re-emitted after the includes section.

    Returns (end_idx, include_entries, trailing_blocks) where:
      - include_entries is a list of IncludeEntry objects
      - trailing_blocks is a list of (condition, [code_lines]) for #ifdef blocks
        that contained non-include code (to be placed right after the includes)
    """
    i = start_idx
    n = len(lines)
    entries: List[IncludeEntry] = []
    trailing_blocks: List[tuple] = []

    while i < n:
        line = lines[i]

        # Skip blank lines
        if line.strip() == "":
            i += 1
            continue

        # Check for preprocessor conditional block (#ifdef FOO or #if defined(FOO))
        stripped = line.strip()
        if stripped.startswith("#ifdef") or (stripped.startswith("#if ") and "defined" in stripped):
            # Pre-scan to find #endif and check if block contains any includes
            j = i + 1
            depth = 1
            has_includes = False
            while j < n and depth > 0:
                s = lines[j].strip()
                if s.startswith("#ifdef") or s.startswith("#if "):
                    depth += 1
                elif s == "#endif":
                    depth -= 1
                    if depth == 0:
                        break
                if depth > 0 and classify_include(lines[j])[0]:
                    has_includes = True
                j += 1

            # If the block contains no includes, treat it as end-of-section
            if not has_includes:
                return i, entries, trailing_blocks

            # Block has includes — process it
            condition = extract_condition(line)

            # If we can't extract a named condition (e.g. "#if defined __cplusplus"),
            # treat the block as end-of-section to preserve the wrapper
            if condition is None:
                return i, entries, trailing_blocks

            i += 1

            block_includes = []
            block_code_lines = []

            while i < n:
                block_line = lines[i]
                bstripped = block_line.strip()

                if bstripped == "#endif":
                    i += 1
                    break

                kind, header = classify_include(block_line)
                if kind:
                    # Extract the include — it will be reordered into the right section
                    block_includes.append(IncludeEntry(
                        kind=kind,
                        header=header,
                        line=block_line.rstrip(),
                        condition=condition
                    ))
                elif bstripped and not bstripped.startswith("//"):
                    # Non-include code inside #ifdef — collect to re-emit later
                    block_code_lines.append(block_line.rstrip())

                i += 1

            entries.extend(block_includes)

            # If the block had non-include code, preserve it as a trailing block
            if block_code_lines:
                trailing_blocks.append((condition, block_code_lines))

            continue

        # Check if it's a direct include (not in #ifdef)
        kind, header = classify_include(line)
        if kind is None:
            # Non-include line, end of section
            return i, entries, trailing_blocks

        # Add direct include with no condition
        entries.append(IncludeEntry(
            kind=kind,
            header=header,
            line=line.rstrip(),
            condition=None
        ))
        i += 1

    return i, entries, trailing_blocks


def skip_initial_comments(lines):
    """
    Skip copyright/license block comments, blank lines, and preprocessor directives at top.
    Returns the index of the first include directive.
    """
    i = 0
    n = len(lines)

    while i < n:
        while i < n and lines[i].strip() == "":
            i += 1
        if i >= n:
            break

        if lines[i].strip().startswith("/*"):
            while i < n and "*/" not in lines[i]:
                i += 1
            i += 1
            continue

        if lines[i].strip().startswith("//"):
            i += 1
            continue

        s = lines[i].strip()

        # Stop at #ifdef/#if defined — they may contain includes
        if s.startswith("#ifdef") or (s.startswith("#if ") and "defined" in s):
            break

        # Skip other preprocessor directives (#pragma once, include guards, #define, etc.)
        if s.startswith("#") and not s.startswith("#include"):
            i += 1
            continue

        break

    return i


def is_cpp_file(filepath):
    return str(filepath).endswith(".cpp")


def is_header_file(filepath):
    return str(filepath).endswith(".h")


def detect_violations(entries: List[IncludeEntry], is_cpp, filepath, original_lines, end_idx):
    """Detect include ordering violations."""
    violations = []

    # The parser skips blank lines, so end_idx always points to a non-blank line.
    # A missing blank line means the line immediately before end_idx is not blank.
    if end_idx < len(original_lines) and end_idx > 0 and original_lines[end_idx - 1].strip() != "":
        violations.append("Missing blank line after includes")

    if not entries:
        return violations

    # Check for duplicates
    seen = set()
    for entry in entries:
        key = (entry.kind, entry.header)
        if key in seen:
            violations.append(f"Duplicate include: {entry.header}")
        seen.add(key)

    # Compare what the formatter would produce vs the actual file content
    formatted = format_includes_from_entries(entries, is_cpp, filepath)

    # Reconstruct what the actual include section looks like (including blank lines)
    start = skip_initial_comments(original_lines)
    actual_section_lines = []
    for i in range(start, end_idx):
        s = original_lines[i].rstrip()
        actual_section_lines.append(s)

    # Strip trailing blank lines
    while actual_section_lines and not actual_section_lines[-1]:
        actual_section_lines.pop()

    # If formatted and actual differ, there's a violation
    if formatted != actual_section_lines:
        violations.append("Include ordering/spacing incorrect")

    return violations


def format_includes_from_entries(entries: List[IncludeEntry], is_cpp, filepath):
    """
    Format includes from IncludeEntry list in correct order.
    Preserves #ifdef conditions and separates mixed-type conditional blocks.
    """
    result = []

    # Find related header for .cpp files
    related_header = None
    if is_cpp:
        cpp_name = Path(filepath).stem
        for entry in entries:
            if entry.kind == "project" and Path(entry.header).stem == cpp_name:
                related_header = entry
                break

    # Group entries by kind, preserving conditions
    groups = {}
    for kind in ["related", "stdlib", "qt", "project"]:
        groups[kind] = []

    for entry in entries:
        # Skip related header in project group, will add it separately
        if entry == related_header:
            groups["related"].append(entry)
        else:
            groups[entry.kind].append(entry)

    # Output related header first (for .cpp files)
    if is_cpp and related_header:
        result.append(related_header.line)
        result.append("")

    # Output other kinds in order, with each kind's unconditional then conditional
    kinds_to_output = []
    for kind in ["stdlib", "qt", "project"]:
        if groups[kind]:
            kinds_to_output.append(kind)

    for idx, kind in enumerate(kinds_to_output):
        # Separate unconditional and conditional entries
        unconditional = [e for e in groups[kind] if e.condition is None]
        conditional_by_cond = {}
        for e in groups[kind]:
            if e.condition:
                if e.condition not in conditional_by_cond:
                    conditional_by_cond[e.condition] = []
                conditional_by_cond[e.condition].append(e)

        # Sort unconditional by header
        unconditional.sort(key=lambda e: e.header.lower())
        for entry in unconditional:
            result.append(entry.line)

        # Output conditional blocks (sorted by condition name), each separated by blank lines
        for condition in sorted(conditional_by_cond.keys()):
            cond_entries = conditional_by_cond[condition]
            cond_entries.sort(key=lambda e: e.header.lower())

            # Blank line before each #ifdef block
            if result and result[-1] != "":
                result.append("")

            result.append(f"#ifdef {condition}")
            for entry in cond_entries:
                result.append(entry.line)
            result.append("#endif")

        # Add blank line after this group if not the last group
        if idx < len(kinds_to_output) - 1:
            if result and result[-1] != "":
                result.append("")

    # Remove trailing blank lines
    while result and result[-1] == "":
        result.pop()

    return result


def fix_file(filepath, verbose=False, force=False, check_only=False):
    """Fix includes in a single file."""
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            lines = f.readlines()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return False

    start_idx = skip_initial_comments(lines)

    if start_idx >= len(lines):
        return False

    end_idx, entries, trailing_blocks = parse_include_section_with_ifdef(lines, start_idx)

    if not entries:
        return False

    is_cpp = is_cpp_file(filepath)
    is_header = is_header_file(filepath)

    if not is_cpp and not is_header:
        return False

    violations = detect_violations(entries, is_cpp, filepath, lines, end_idx)

    # Trailing blocks (code split out from mixed #ifdef) also count as a violation
    if trailing_blocks:
        violations.append("Mixed #ifdef block (includes + code) needs splitting")

    if check_only:
        if violations:
            print(f"{filepath.relative_to(Path.cwd())}:")
            for violation in violations:
                print(f"  - {violation}")
            return True
        return False

    if not violations and not force:
        return False

    formatted = format_includes_from_entries(entries, is_cpp, filepath)

    if not formatted:
        return False

    new_lines = lines[:start_idx]
    for line in formatted:
        new_lines.append(line + "\n")

    # Emit trailing blocks (code that was inside mixed #ifdef blocks)
    # each gets its own #ifdef...#endif wrapper, separated by blank lines
    for condition, code_lines in trailing_blocks:
        new_lines.append("\n")
        new_lines.append(f"#ifdef {condition}\n")
        for code_line in code_lines:
            new_lines.append(code_line + "\n")
        new_lines.append("#endif\n")

    if end_idx < len(lines) and lines[end_idx].strip() != "":
        new_lines.append("\n")

    new_lines.extend(lines[end_idx:])

    old_content = "".join(lines)
    new_content = "".join(new_lines)
    changed = old_content != new_content

    if not changed and not force:
        return False

    try:
        with open(filepath, "w", encoding="utf-8") as f:
            f.writelines(new_lines)
        if verbose:
            status = "[MODIFIED]" if changed else "[UNCHANGED]"
            if violations:
                print(f"  {status} {filepath.relative_to(Path.cwd())} ({len(violations)} violations fixed)")
            else:
                print(f"  {status} {filepath.relative_to(Path.cwd())}")
        return True
    except Exception as e:
        print(f"Error writing {filepath}: {e}")
        return False


def main():
    root = None
    verbose = False
    force = False
    check_only = False

    for arg in sys.argv[1:]:
        if arg == "--verbose":
            verbose = True
        elif arg == "--force":
            force = True
        elif arg == "--check":
            check_only = True
        elif not arg.startswith("--"):
            root = Path(arg)

    if root is None:
        root = find_repo_root()

    if not root.exists():
        print(f"Error: root directory {root} does not exist")
        sys.exit(1)

    if check_only:
        print(f"Checking include ordering in {root}...\n")
    else:
        print(f"Fixing include ordering in {root}...")
        if force:
            print("(Force mode: reprocessing all files)\n")
        else:
            print()

    cpp_files = []
    for ext in ["*.cpp", "*.h"]:
        cpp_files.extend(root.glob(f"**/{ext}"))

    cpp_files = [f for f in cpp_files if not any(
        should_exclude_dir(part) for part in f.relative_to(root).parts
    ) and f.name not in EXCLUDE_FILES]

    cpp_files.sort()

    processed_count = 0
    for filepath in cpp_files:
        if fix_file(filepath, verbose=verbose, force=force, check_only=check_only):
            if not verbose and not check_only:
                print(f"Fixed: {filepath.relative_to(root)}")
            processed_count += 1

    print(f"\n--- Summary ---")
    print(f"Files checked:    {len(cpp_files)}")
    if check_only:
        print(f"Files with issues: {processed_count}")
    else:
        print(f"Files processed:  {processed_count}")


if __name__ == "__main__":
    main()
