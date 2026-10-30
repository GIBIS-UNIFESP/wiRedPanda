#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
"""Check the exception inventory against the sources.

Every throw site (a ``throw`` or a ``PANDACEPTION``), every ``catch`` and every ``guardedSlot()`` use in the
application's and the MCP server's C++ sources must have a row at its file and line in the matching table of the
inventory (.claude/LEDGER_exception_boundaries.md, "Appendix: the inventory"), and every row must still name such a
site. A new site is classified there before it lands; a site that moves takes its row with it. Comments and string
literals are not read.

Usage:
    python3 Scripts/check_exception_inventory.py [--ledger PATH]

Exits 1, listing each difference, when the sources and the inventory disagree.
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path


def find_repo_root():
    """Return the git repository root, falling back to the script's grandparent directory."""
    try:
        result = subprocess.run(["git", "rev-parse", "--show-toplevel"], capture_output=True, text=True, check=True)
        return Path(result.stdout.strip())
    except (subprocess.CalledProcessError, FileNotFoundError):
        return Path(__file__).resolve().parent.parent


ROOT = find_repo_root()
LEDGER_PATH = ROOT / ".claude" / "LEDGER_exception_boundaries.md"
SOURCE_DIRS = ("App", "MCP/Server")
SOURCE_SUFFIXES = (".cpp", ".h")

# Each kind of site, the inventory's table that holds it, and how a line of code shows it
KINDS = (
    ("throw", "### Throw sites", re.compile(r"\bthrow\b|\bPANDACEPTION(?:_WITH_CONTEXT|_LITERAL)?\s*\(")),
    ("catch", "### Catch sites", re.compile(r"\bcatch\s*\(")),
    ("guardedSlot", "### `guardedSlot()` uses", re.compile(r"\bguardedSlot\s*\(")),
)
DEFINE = re.compile(r"^\s*#\s*define\b")
ROW = re.compile(r"^\| `([^`]+:\d+)` \|", re.MULTILINE)


def code_lines(text):
    """Return the lines of C++ source *text* with comments and string and character literals blanked."""
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if text.startswith("//", i):
            end = text.find("\n", i)
            i = n if end < 0 else end
        elif text.startswith("/*", i):
            end = text.find("*/", i + 2)
            end = n if end < 0 else end + 2
            out.append("\n" * text.count("\n", i, end))
            i = end
        elif c == "R" and text.startswith('R"', i) and (i == 0 or not (text[i - 1].isalnum() or text[i - 1] == "_")):
            open_paren = text.find("(", i + 2)
            delimiter = text[i + 2 : open_paren]
            close = text.find(")" + delimiter + '"', open_paren)
            end = n if close < 0 else close + len(delimiter) + 2
            out.append('""' + "\n" * text.count("\n", i, end))
            i = end
        elif c == "'" and i > 0 and text[i - 1].isalnum():
            # A digit separator (1'000), not a character literal
            out.append(c)
            i += 1
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            out.append(c + c)
            i = j + 1
        else:
            out.append(c)
            i += 1
    return "".join(out).split("\n")


def source_sites():
    """Return, for each kind, the set of ``path:line`` sites in the sources."""
    sites = {kind: set() for kind, _, _ in KINDS}
    for directory in SOURCE_DIRS:
        for path in sorted((ROOT / directory).rglob("*")):
            if path.suffix not in SOURCE_SUFFIXES or not path.is_file():
                continue
            relative = path.relative_to(ROOT).as_posix()
            for number, line in enumerate(code_lines(path.read_text(encoding="utf-8")), 1):
                if DEFINE.match(line):
                    continue
                for kind, _, pattern in KINDS:
                    if pattern.search(line):
                        sites[kind].add(f"{relative}:{number}")
    return sites


def inventory_sites(ledger):
    """Return, for each kind, the set of ``path:line`` sites the inventory's tables hold."""
    text = ledger.read_text(encoding="utf-8")
    appendix = text.split("## Appendix: the inventory", 1)
    if len(appendix) != 2:
        raise SystemExit(f"{ledger}: no inventory appendix")
    sections = re.split(r"^(?=### )", appendix[1], flags=re.MULTILINE)
    sites = {}
    for kind, heading, _ in KINDS:
        matching = [section for section in sections if section.startswith(heading + "\n")]
        if len(matching) != 1:
            raise SystemExit(f"{ledger}: expected one '{heading}' table")
        sites[kind] = set(ROW.findall(matching[0]))
    return sites


def site_key(site):
    """Order ``path:line`` sites by path, then by line number."""
    path, line = site.rsplit(":", 1)
    return path, int(line)


def main():
    """Compare the sources with the inventory and report every difference."""
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--ledger", type=Path, default=LEDGER_PATH, help="the ledger holding the inventory")
    args = parser.parse_args()

    in_sources = source_sites()
    in_inventory = inventory_sites(args.ledger)
    failed = False
    for kind, heading, _ in KINDS:
        for site in sorted(in_sources[kind] - in_inventory[kind], key=site_key):
            print(f"{site}: a {kind} site the inventory's '{heading[4:]}' table does not hold")
            failed = True
        for site in sorted(in_inventory[kind] - in_sources[kind], key=site_key):
            print(f"{site}: in the inventory's '{heading[4:]}' table, but no {kind} site is there")
            failed = True
    if failed:
        print("Classify each new site in the inventory (below a boundary, an expected condition, a broken invariant),")
        print("and move or remove the row of each site that moved or went.")
        return 1
    counts = ", ".join(f"{len(in_sources[kind])} {kind}" for kind, _, _ in KINDS)
    print(f"The inventory holds every site: {counts}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
