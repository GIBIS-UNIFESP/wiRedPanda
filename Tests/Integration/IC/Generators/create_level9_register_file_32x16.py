#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Register File 32x16 IC (32 registers, 16 bits wide)

Built for level9_cpu_16bit_risc's register file, addressed by the full 5-bit
DestReg instruction field — a literal 32 registers, not an address-aliased
stand-in, since DestReg's 5 bits are a deliberate instruction-format
provision (unlike e.g. level8_memory_stage's incidental 8-bit address bus
over an 8-word RAM, where partial decode is a natural side effect of bus-
width standardization rather than a sized-and-promised field).

Only one read port is needed (the RISC CPU's OperandA is always the SrcBits
immediate, never a register), so num_read_ports=1 avoids generating a full
second 16-wide, fully-wired-but-unused read-mux array.

Inputs:
  - Write_Addr[0..4]: Write address (5-bit)
  - Read_Addr1[0..4]: Read port address (5-bit)
  - Data_In[0..15]: Write data (16-bit)
  - Write_Enable: Write control signal
  - Clock: Clock signal for synchronous write

Outputs:
  - Read_Data1[0..15]: Read port data (16-bit)

Architecture:
  - Write decoder: level2_decoder_5to32 IC
  - Write gates: 32 AND gates (decoder output × write enable)
  - Storage: 32×16 array of D flip-flops with hold muxes
  - Read mux: 16 multiplexers (one per bit, single read port)

Usage:
    python create_level9_register_file_32x16.py
"""

import asyncio

from ic_builder_base import run_ic_builder
from ic_builder_helpers import RegisterFileBuilder


async def build(mcp) -> bool:
    """Entry point for the builder"""
    return await RegisterFileBuilder(
        mcp, num_registers=32, data_width=16, address_bits=5, level=9, num_read_ports=1
    ).create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "Register File 32×16 IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
