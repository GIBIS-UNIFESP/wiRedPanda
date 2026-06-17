#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Register File 4x4 IC (4 registers, 4 bits wide)

Inputs:
  - Write_Addr[0..1]: Write address (2-bit)
  - Read_Addr1[0..1]: Read port 1 address (2-bit)
  - Read_Addr2[0..1]: Read port 2 address (2-bit)
  - Data_In[0..3]: Write data (4-bit)
  - Write_Enable: Write control signal
  - Clock: Clock signal for synchronous write

Outputs:
  - Read_Data1[0..3]: Read port 1 data (4-bit)
  - Read_Data2[0..3]: Read port 2 data (4-bit)

Architecture:
  - Write decoder: level2_decoder_2to4 IC
  - Write gates: 4 AND gates (decoder output × write enable)
  - Storage: 4×4 array of D flip-flops with hold muxes
  - Read muxes: 2×4 multiplexers (one per bit per read port)

Usage:
    python create_level5_register_file_4x4.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import RegisterFileBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await RegisterFileBuilder(mcp, num_registers=4, data_width=4, address_bits=2).create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "Register File 4×4 IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
