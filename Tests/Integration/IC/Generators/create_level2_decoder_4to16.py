#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-to-16 Decoder IC

Inputs: addr[0], addr[1], addr[2], addr[3], Enable (74138-style chip select, defaults high)
Outputs: out[0] to out[15]

Circuit:
- 4 NOT gates (invert address bits)
- 16 AND gates (5-input, one-hot encoding)
  - Each AND combines 5 inputs: 4 address bits (direct or inverted) + Enable
  - For output i: bit pattern of i determines which address inputs to use

Usage:
    python create_level2_decoder_4to16.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import DecoderBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await DecoderBuilder(mcp, address_bits=4).create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-to-16 Decoder IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
