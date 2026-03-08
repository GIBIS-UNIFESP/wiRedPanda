#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Instruction Memory Interface IC

Provides a clean interface to instruction memory (ROM/RAM) for the CPU.

Inputs:
  Address[0..7] (8-bit memory address from program counter)
  Clock (clock signal for read/write synchronization)

Outputs:
  Instruction[0..7] (8-bit instruction fetched from memory)

Architecture:
  - Instantiates level6_ram_256x8 as instruction memory
  - Asynchronous read: instruction available immediately based on address
  - Pre-loaded with CPU instruction set (simulated)
  - Simple wrapper for clean CPU interface

Purpose:
  Provides instruction fetch stage for CPU pipeline.
  Decouples instruction memory from program counter logic.

Usage:
    python create_level7_instruction_memory_interface.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class InstructionMemoryInterfaceBuilder(ICBuilderBase):
    """Builder for Instruction Memory Interface IC"""

    async def create(self) -> bool:
        """Create the Instruction Memory Interface IC"""
        await self.begin_build('Instruction Memory Interface')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0

        # Create address inputs (8-bit)
        address_inputs = []
        for i in range(8):
            addr_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), 100.0, f"Address[{i}]")
            if addr_id is None:
                return False
            address_inputs.append(addr_id)
        await self.log("  ✓ Created 8 address inputs")

        # Create clock input
        clock_id = await self.create_element("InputSwitch", input_x + (8 * HORIZONTAL_GATE_SPACING), 100.0, "Clock")
        if clock_id is None:
            return False
        await self.log("  ✓ Created clock input")

        # Instantiate 256×8 RAM for instruction memory
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level6_ram_256x8")):

            return False

        ram_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level6_ram_256x8"), input_x + (4 * HORIZONTAL_GATE_SPACING), 250.0, "InstructionMemory")
        if ram_id is None:
            return False
        await self.log("  ✓ Instantiated 256×8 RAM for instruction memory")

        # Connect address inputs to RAM
        for i in range(8):
            if not await self.connect(address_inputs[i], ram_id, target_port_label=f"Address[{i}]"):
                return False

        # Connect clock to RAM (for synchronization)
        if not await self.connect(clock_id, ram_id, target_port_label="Clock"):
            return False

        # Create ground for WriteEnable (read-only: WriteEnable=0)
        gnd_id = await self.create_element("InputGnd", input_x + (9 * HORIZONTAL_GATE_SPACING), 250.0, "GND_WriteEnable")
        if gnd_id is None:
            return False

        # Connect ground to RAM WriteEnable (read-only)
        if not await self.connect(gnd_id, ram_id, target_port_label="WriteEnable"):
            return False
        await self.log("  ✓ Connected address, clock, and read-only controls to RAM")

        # Create instruction output LEDs (8-bit)
        output_x = input_x + (14 * HORIZONTAL_GATE_SPACING)
        for i in range(8):
            led_id = await self.create_element("Led", output_x, 250.0 + (i * (VERTICAL_STAGE_SPACING / 2)), f"Instruction[{i}]")
            if led_id is None:
                return False

            # Connect RAM output to LED
            if not await self.connect(ram_id, led_id, source_port_label=f"DataOut[{i}]"):
                return False

        await self.log("  ✓ Created 8 instruction outputs")

        output_file = str(IC_COMPONENTS_DIR / "level7_instruction_memory_interface.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created Instruction Memory Interface IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = InstructionMemoryInterfaceBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Instruction Memory Interface IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
