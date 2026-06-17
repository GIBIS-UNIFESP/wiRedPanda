#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Comparator IC (Level 3)

Compares two 4-bit values A and B using cascading magnitude comparison.

Inputs: A[0-3], B[0-3] (two 4-bit values)
Outputs: Greater, Equal, Less (exactly one asserted)

Architecture:
  - 4 XNOR gates for bit-wise equality
  - 8 AND gates for greater/less conditions per bit
  - 4 cascade AND gates for equality propagation
  - OR gates to combine comparison results

Usage:
    python create_level3_comparator_4bit.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import ComparatorBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await ComparatorBuilder(mcp, level=3).create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "Comparator 4-bit IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
