#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Register File 8x8 IC (8 registers, 8 bits wide)

Inputs:
  - Write_Addr[0..2]: Write address (3-bit)
  - Read_Addr1[0..2]: Read port 1 address (3-bit)
  - Read_Addr2[0..2]: Read port 2 address (3-bit)
  - Data_In[0..7]: Write data (8-bit)
  - Write_Enable: Write control signal
  - Clock: Clock signal for synchronous write

Outputs:
  - Read_Data1[0..7]: Read port 1 data (8-bit)
  - Read_Data2[0..7]: Read port 2 data (8-bit)

Architecture:
  - Write decoder: level2_decoder_3to8 IC
  - Write gates: 8 AND gates (decoder output × write enable)
  - Storage: 8×8 array of D flip-flops with hold muxes
  - Read muxes: 2×8 multiplexers (one per bit per read port)

Usage:
    python create_level5_register_file_8x8.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import RegisterFileBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await RegisterFileBuilder(
        mcp, num_registers=8, data_width=8, address_bits=3, decoder_name="level2_decoder_3to8"
    ).create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "Register File 8×8 IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
