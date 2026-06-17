#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Ring Counter IC

Ring counter with 4-state one-hot sequence:
  Q3=0,Q2=0,Q1=0,Q0=1 → Q3=0,Q2=0,Q1=1,Q0=0 →
  Q3=0,Q2=1,Q1=0,Q0=0 → Q3=1,Q2=0,Q1=0,Q0=0 → (repeat)

Inputs: CLK (Clock), PRESET (active-LOW async preset for FF[0])
Outputs: Q0, Q1, Q2, Q3

Implementation:
- 4 D Flip-Flops (DFlipFlop)
- Simple circular interconnections
- PRESET signal connected to FF[0] PRESET input (port 2)

Usage:
    python create_ring_counter.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import RingJohnsonCounterBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await RingJohnsonCounterBuilder(mcp, is_johnson=False).create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Ring Counter IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
