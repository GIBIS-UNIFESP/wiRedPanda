#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Modulo-N Counter IC

Inputs: CLK (Clock), Modulo[0-3] (4-bit modulo value)
Outputs: Q[0-3] (4-bit count value), Overflow (pulse when count wraps)

Architecture (built from level1_d_flip_flop ICs):
- 4 D flip-flops for counter state
- Increment logic (same carry chain as binary counter)
- level3_comparator_4bit_equality IC compares NEXT value with modulo
- When next_count == modulo, force D inputs to 0 (reset on next clock)
- D[i] = inc[i] AND NOT(equal)

Modulo Counter behavior (counts 0 to N-1, then wraps):
- Counts 0, 1, 2, ..., N-1, 0, 1, ...
- Overflow signals when counter wraps

Usage:
    python create_level5_modulo_counter_4bit.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import CounterBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await CounterBuilder(mcp, mode="modulo").create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Modulo Counter IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
