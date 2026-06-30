#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Barrel Rotator IC

Implements bidirectional circular rotation using 2-stage mux cascade.
Architecture:
  - LEFT path:  2-stage cascade (rotate by 1, rotate by 2) with wrap-around
  - RIGHT path: 2-stage cascade (rotate by 1, rotate by 2) with wrap-around
  - Direction selector: final mux layer selects between LEFT and RIGHT paths

Left rotation:  output[i] = input[(i-rotate+4) % 4]
Right rotation: output[i] = input[(i+rotate) % 4]

Inputs: Data[0-3], Rotate_Amount[0-1], Direction (7 inputs)
        Direction: 0=left, 1=right
Outputs: Result[0-3] (4 outputs)

Usage:
    python create_level5_barrel_rotator.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import BarrelShiftBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await BarrelShiftBuilder(mcp, is_rotator=True).create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "Barrel Rotator IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
