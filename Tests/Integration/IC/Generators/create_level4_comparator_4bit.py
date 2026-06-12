#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Comparator IC (Level 4)

The level-4 comparator fixture is the same circuit as the level-3 one
(F32) — this script reuses the parameterized builder from
create_level3_comparator_4bit.py and only changes the output name.

Inputs: A[0-3], B[0-3] (two 4-bit values)
Outputs: Greater, Equal, Less (exactly one asserted)

Usage:
    python create_level4_comparator_4bit.py
"""

import asyncio

from create_level3_comparator_4bit import Comparator4BitBuilder
from ic_builder_base import run_ic_builder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = Comparator4BitBuilder(mcp, verbose=True, output_name="level4_comparator_4bit")
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Comparator IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
