#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Loadable Counter IC

Inputs: CLK (Clock), Load, D[0-3] (load value)
Outputs: Q[0-3] (4-bit count value)

Architecture (built from level1_d_flip_flop ICs):
- 4 D flip-flops for counter state
- Increment logic (same carry chain as binary counter)
- Load/Count mux: ff_d[i] = Mux(inc[i], D[i], Load)
  - Load=1: loads data from D inputs
  - Load=0: increments counter

Increment Logic:
- Bit 0: inc[0] = NOT(Q0)
- Bit 1: inc[1] = Q1 XOR Q0
- Bit 2: inc[2] = Q2 XOR (Q0 AND Q1)
- Bit 3: inc[3] = Q3 XOR (Q0 AND Q1 AND Q2)

Usage:
    python create_level5_loadable_counter_4bit.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import CounterBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await CounterBuilder(mcp, mode="loadable", dff_name="level1_d_flip_flop").create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Loadable Counter IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
