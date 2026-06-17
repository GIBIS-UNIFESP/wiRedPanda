#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Binary Counter IC

Inputs: CLK (Clock)
Outputs: Q[0:3] (Counter Output, 0-15)

Circuit:
- 4 D flip-flops for counter state (4-bit value)
- Ripple-carry increment logic using AND/OR gates
- NOT gates for signal inversion

Binary Counter behavior (0→1→2→...→15→0):
- Increments by 1 on each clock rising edge
- Wraps around from 15 to 0
- No load/reset controls for simplicity

Increment Logic:
- Bit 0: D0 = NOT Q0 (always toggle)
- Bit 1: D1 = Q1 XOR Q0
- Bit 2: D2 = Q2 XOR (Q0 AND Q1)
- Bit 3: D3 = Q3 XOR (Q0 AND Q1 AND Q2)

Usage:
    python create_binary_counter_4bit.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import CounterBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await CounterBuilder(mcp, mode="binary").create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Binary Counter IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
