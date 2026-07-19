#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
"""Generate the English-source Weblate translation catalog for Exercise/Tour content.

Scans App/Resources/Exercises/*.json and App/Resources/Tours/*.json and extracts every
translatable field (title, description, and per-step instruction/hint or title/body, keyed by
each step's "key") into one deterministic JSON catalog consumed by Weblate as
App/Resources/Translations/ExerciseTour/en.json. Non-English ExerciseTour/<lang>.json files
are managed by Weblate and never touched by this script.

Usage:
    python3 Scripts/generate_exercise_tour_catalog.py [--check]

--check: exit 1 if regenerating would change the committed file, without writing it (used by CI
  to verify the catalog is up to date with the content JSON).
"""

import argparse
import json
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
EXERCISES_DIR = ROOT / "App" / "Resources" / "Exercises"
TOURS_DIR = ROOT / "App" / "Resources" / "Tours"
CATALOG_PATH = ROOT / "App" / "Resources" / "Translations" / "ExerciseTour" / "en.json"


def extract_entries(content_dir, is_tour):
    """Build the {fileId: {...}} catalog fragment for every *.json file in content_dir."""
    catalog = {}
    for json_file in sorted(content_dir.glob("*.json")):
        data = json.loads(json_file.read_text(encoding="utf-8"))
        file_id = data.get("id")
        title = data.get("title")
        if not file_id or not title:
            raise SystemExit(f"{json_file}: missing required 'id' or 'title'")

        entry = {"title": title}
        if data.get("description"):
            entry["description"] = data["description"]

        seen_keys = set()
        for step in data.get("steps", []):
            key = step.get("key")
            if not key:
                continue
            if key in seen_keys:
                raise SystemExit(f"{json_file}: duplicate step key '{key}'")
            seen_keys.add(key)
            if is_tour:
                step_entry = {"title": step.get("title", "")}
                if step.get("body"):
                    step_entry["body"] = step["body"]
            else:
                step_entry = {"instruction": step.get("instruction", "")}
                if step.get("hint"):
                    step_entry["hint"] = step["hint"]
            entry[key] = step_entry
        catalog[file_id] = entry
    return catalog


def build_catalog():
    """Merge the Exercises and Tours catalog fragments, rejecting id collisions between them."""
    exercises = extract_entries(EXERCISES_DIR, is_tour=False)
    tours = extract_entries(TOURS_DIR, is_tour=True)
    collisions = set(exercises) & set(tours)
    if collisions:
        raise SystemExit(f"Duplicate content id(s) across Exercises/ and Tours/: {sorted(collisions)}")
    return {**exercises, **tours}


def main():
    """Regenerate (or, with --check, verify) the committed English-source catalog."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="verify catalog is up to date; do not write")
    args = parser.parse_args()

    rendered = json.dumps(build_catalog(), indent=2, sort_keys=True, ensure_ascii=False) + "\n"

    if args.check:
        current = CATALOG_PATH.read_text(encoding="utf-8") if CATALOG_PATH.exists() else ""
        if current != rendered:
            print(
                f"{CATALOG_PATH} is out of date; run: python3 Scripts/generate_exercise_tour_catalog.py",
                file=sys.stderr,
            )
            return 1
        print(f"{CATALOG_PATH} is up to date.")
        return 0

    CATALOG_PATH.parent.mkdir(parents=True, exist_ok=True)
    CATALOG_PATH.write_text(rendered, encoding="utf-8")
    print(f"Wrote {CATALOG_PATH}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
