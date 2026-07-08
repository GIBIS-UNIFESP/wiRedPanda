#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 5-to-32 Decoder IC

Inputs: addr[0] to addr[4]
Outputs: out[0] to out[31]

Circuit:
- 5 NOT gates (invert address bits)
- 32 AND gates (6-input: 5 address bits + Enable, one-hot encoding)
  - Each AND combines 5 inputs (direct or inverted) plus Enable
  - For output i: bit pattern of i determines which inputs to use

Usage:
    python create_level2_decoder_5to32.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import DecoderBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await DecoderBuilder(mcp, address_bits=5).create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "5-to-32 Decoder IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
