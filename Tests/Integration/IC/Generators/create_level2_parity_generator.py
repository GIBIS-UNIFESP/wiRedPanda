#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Parity Generator IC (4-bit)

Given 4 data bits, generates the parity bit (XOR of all inputs).
Output is 1 when the number of 1-bits is odd (odd parity).

Inputs:
  - Data[0] to Data[3]: 4 data input bits

Outputs:
  - Parity: 1-bit parity output

Architecture:
  - Binary tree of XOR gates (2 stages)
  - Stage 1: 2 XOR gates (pairs)
  - Stage 2: 1 XOR gate (final)
  - Output LED

Usage:
    python create_level2_parity_generator.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import ParityBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await ParityBuilder(mcp, is_checker=False).create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "Parity Generator IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
