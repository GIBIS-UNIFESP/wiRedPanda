#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Barrel Shifter IC

Implements bidirectional linear shift with zero-fill using 2-stage mux cascade.
Architecture:
  - LEFT path:  2-stage cascade (shift by 1, shift by 2) with zero-fill from right
  - RIGHT path: 2-stage cascade (shift by 1, shift by 2) with zero-fill from left
  - Direction selector: final mux layer selects between LEFT and RIGHT paths

Left shift:  output[i] = input[i-shift] (or 0 if i-shift < 0)
Right shift: output[i] = input[i+shift] (or 0 if i+shift >= 4)

Inputs: Data[0-3], ShiftAmount[0-1], ShiftDirection (7 inputs)
        ShiftDirection: 0=left, 1=right
Outputs: ShiftedData[0-3] (4 outputs)

Usage:
    python create_level5_barrel_shifter_4bit.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import BarrelShiftBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await BarrelShiftBuilder(mcp, is_rotator=False).create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "Barrel Shifter 4-bit IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
