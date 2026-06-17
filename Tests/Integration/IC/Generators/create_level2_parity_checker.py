#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Parity Checker IC (8-bit)

Given up to 8 bits (data + parity), checks if odd parity is satisfied.
Output is 1 when the total number of 1-bits is odd (valid odd parity).

Inputs:
  - data[0] to data[7]: 8 input bits (data + parity bit)

Outputs:
  - parity: 1-bit check result (1 = valid odd parity, 0 = error)

Architecture:
  - Binary tree of XOR gates (3 stages)
  - Stage 1: 4 XOR gates (parallel pairs)
  - Stage 2: 2 XOR gates
  - Stage 3: 1 XOR gate (final)
  - Output LED

Usage:
    python create_level2_parity_checker.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import ParityBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await ParityBuilder(mcp, is_checker=True).create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "Parity Checker IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
