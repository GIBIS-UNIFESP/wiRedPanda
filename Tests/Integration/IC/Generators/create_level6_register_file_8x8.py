#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 8×8 Register File IC (Level 6)

The level-6 register-file fixture is the same circuit as the level-5 one
(F32) — this script reuses the shared RegisterFileBuilder and only changes
the output level prefix.

Inputs:
  - Write_Addr[0..2] (3-bit write address for 8 registers)
  - Read_Addr1[0..2] (3-bit read address port 1)
  - Read_Addr2[0..2] (3-bit read address port 2)
  - Data_In[0..7] (8-bit write data)
  - Write_Enable (write control signal)
  - Clock (clock signal for synchronous write)

Outputs:
  - Read_Data1[0..7] (8-bit read port 1 output)
  - Read_Data2[0..7] (8-bit read port 2 output)

Usage:
    python3 create_level6_register_file_8x8.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import RegisterFileBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await RegisterFileBuilder(
        mcp,
        num_registers=8,
        data_width=8,
        address_bits=3,
        decoder_name="level2_decoder_3to8",
        level=6,
    ).create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "Register File 8×8 IC (Level 6)"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
