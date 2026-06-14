#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 8×8 Register File IC (Level 6)

The level-6 register-file fixture is the same circuit as the level-5
one (F32) — this script reuses the parameterized builder from
create_level5_register_file_8x8.py and only changes the output name.

Inputs:
  - Write_Addr[0..2] (3-bit write address for 8 registers)
  - Read_Addr1[0..2] (3-bit read address port 1)
  - Read_Addr2[0..2] (3-bit read address port 2)
  - Data_In[0..7] (8-bit write data)
  - WriteEnable (write control signal)
  - Clock (clock signal for synchronous write)

Outputs:
  - Read_Data1[0..7] (8-bit read port 1 output)
  - Read_Data2[0..7] (8-bit read port 2 output)

Usage:
    python3 create_level6_register_file_8x8.py
"""

import asyncio

from create_level5_register_file_8x8 import RegisterFile8x8Builder
from ic_builder_base import run_ic_builder

# Declared so run_all_generators.py still orders this script after the
# decoder generator (the builder lives in create_level5_register_file_8x8).
DEPENDENCIES = ["level2_decoder_3to8"]


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = RegisterFile8x8Builder(mcp, verbose=True, output_name="level6_register_file_8x8")
    return await builder.create()


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
