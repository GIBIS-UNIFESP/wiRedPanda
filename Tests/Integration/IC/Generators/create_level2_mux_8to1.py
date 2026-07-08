#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 8-to-1 Multiplexer IC

Selects one of 8 data inputs based on 3 select bits.

Inputs:
  - Data[0] to Data[7]: 8 data inputs
  - Sel[0], Sel[1], Sel[2]: 3 select bits
  - Enable: 74153-style strobe, defaults high

Outputs:
  - Output: selected data bit

Architecture:
  - Single Mux element with inputSize=11 (8 data + 3 select)
  - AND gate gates the Mux output with Enable

Usage:
    python create_level2_mux_8to1.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import MuxNto1Builder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await MuxNto1Builder(mcp, data_inputs=8).create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "8-to-1 Multiplexer IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
