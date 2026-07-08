#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 3-to-8 Decoder IC

Inputs: addr[0], addr[1], addr[2], Enable (74138-style chip select, defaults high)
Outputs: out[0] to out[7]

Circuit:
- 3 NOT gates (invert address bits)
- 8 AND gates (4-input, one-hot encoding)
  - Each AND combines 4 inputs: 3 address bits (direct or inverted) + Enable
  - For output i: bit pattern of i determines which address inputs to use

Usage:
    python create_level2_decoder_3to8.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import DecoderBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await DecoderBuilder(mcp, address_bits=3).create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "3-to-8 Decoder IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
