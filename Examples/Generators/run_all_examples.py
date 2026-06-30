#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Regenerate every temporal example circuit in Examples/.

Each generator spawns its own headless wiRedPanda MCP server, builds the circuit,
and saves the .panda into Examples/. Run after changing a generator to refresh the
committed .panda artifacts.

Usage:
    python3 run_all_examples.py
"""

import subprocess
import sys
from pathlib import Path

GENERATORS = [
    "create_temporal_ring_oscillator.py",
    "create_temporal_static_hazard.py",
    "create_temporal_gate_delay_chain.py",
]


def main() -> int:
    """Run every generator in GENERATORS, returning a non-zero exit code on any failure."""
    here = Path(__file__).resolve().parent
    failures = []
    for script in GENERATORS:
        print(f"\n=== {script} ===")
        result = subprocess.run([sys.executable, str(here / script)], check=False)
        if result.returncode != 0:
            failures.append(script)

    print()
    if failures:
        print(f"❌ {len(failures)} generator(s) failed: {', '.join(failures)}")
        return 1
    print(f"✅ All {len(GENERATORS)} temporal examples generated.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
