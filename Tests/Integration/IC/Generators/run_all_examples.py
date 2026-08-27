#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Regenerate every generated example circuit in Examples/.

Each generator drives the MCP server to build its circuit and save it, so they are run one
at a time in separate processes -- one server session per circuit, matching how the IC
fixture generators are invoked.

Usage:
    python3 run_all_examples.py
"""

import subprocess
import sys
from pathlib import Path

GENERATORS_DIR = Path(__file__).resolve().parent

EXAMPLE_GENERATORS = [
    "create_example_temporal_ring_oscillator.py",
    "create_example_temporal_static_hazard.py",
    "create_example_temporal_gate_delay_chain.py",
]


def main() -> int:
    """Run every example generator in turn; returns 0 only if all of them succeeded."""
    failures: list[str] = []

    for generator in EXAMPLE_GENERATORS:
        script = GENERATORS_DIR / generator
        print(f"\n=== {generator} ===")
        result = subprocess.run([sys.executable, str(script)], check=False)
        if result.returncode != 0:
            failures.append(generator)

    if failures:
        print(f"\nFailed: {', '.join(failures)}")
        return 1

    print(f"\nRegenerated {len(EXAMPLE_GENERATORS)} example circuits.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
