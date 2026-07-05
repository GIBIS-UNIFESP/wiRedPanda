#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Instruction Memory Interface IC

Provides a clean interface to instruction memory (ROM/RAM) for the CPU.

Inputs:
  Address[0..7] (8-bit memory address from program counter)
  DataIn[0..7] (8-bit data to write into memory)
  WriteEnable (enable writing to memory at current address)
  Clock (clock signal for read/write synchronization)

Outputs:
  Instruction[0..7] (8-bit instruction fetched from memory)

Architecture:
  - Instantiates level6_ram_8x8 as instruction memory
  - Asynchronous read: instruction available immediately based on address
  - Write port exposed for external programming of instruction memory
  - Simple wrapper for clean CPU interface

Purpose:
  Provides instruction fetch stage for CPU pipeline.
  Decouples instruction memory from program counter logic.
  Write port enables tests and external logic to program the instruction memory.

Usage:
    python create_level7_instruction_memory_interface.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder


class InstructionMemoryInterfaceBuilder(ICBuilderBase):
    """Builder for Instruction Memory Interface IC"""

    async def create(self) -> bool:
        """Create the Instruction Memory Interface IC"""
        await self.begin_build("Instruction Memory Interface")
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0
        ram_x = input_x + (4 * HORIZONTAL_GATE_SPACING)
        ram_y = 400.0

        # Instantiate the 8×8 RAM first so the Address/DataIn input rows
        # above it can be placed with guaranteed clearance from its real
        # (port-count-dependent) height -- level6_ram_8x8 is far taller than
        # the flat 64px assumed elsewhere (queried via instantiate_ic_with_size).
        ram_handle = await self.instantiate_ic_with_size("level6_ram_8x8", ram_x, ram_y, "InstructionMemory")
        if ram_handle is None:
            return False
        ram_id = ram_handle.element_id
        await self.log("  ✓ Instantiated 8×8 RAM for instruction memory")

        data_y = ram_y - (ram_handle.height / 2) - VERTICAL_STAGE_SPACING
        address_y = data_y - VERTICAL_STAGE_SPACING

        # Create address inputs (8-bit)
        address_inputs = []
        for i in range(8):
            addr_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), address_y, f"Address[{i}]"
            )
            if addr_id is None:
                return False
            address_inputs.append(addr_id)
        await self.log("  ✓ Created 8 address inputs")

        # Create data inputs (8-bit) for writing to memory
        data_inputs = []
        for i in range(8):
            data_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), data_y, f"DataIn[{i}]"
            )
            if data_id is None:
                return False
            data_inputs.append(data_id)
        await self.log("  ✓ Created 8 data inputs")

        # Create write enable input
        write_enable_id = await self.create_element(
            "InputSwitch", input_x + (8 * HORIZONTAL_GATE_SPACING), data_y, "WriteEnable"
        )
        if write_enable_id is None:
            return False
        await self.log("  ✓ Created WriteEnable input")

        # Create clock input
        clock_id = await self.create_element("InputSwitch", input_x + (9 * HORIZONTAL_GATE_SPACING), address_y, "Clock")
        if clock_id is None:
            return False
        await self.log("  ✓ Created clock input")

        # Connect the low address bits to the RAM. The interface keeps a full
        # 8-bit address bus for the CPU; the backing memory holds 8 words, so
        # the high bits are partial decode (addresses alias modulo 8).
        for i in range(3):
            if not await self.connect(address_inputs[i], ram_id, target_port_label=f"Address[{i}]"):
                return False

        # Connect data inputs to RAM
        for i in range(8):
            if not await self.connect(data_inputs[i], ram_id, target_port_label=f"DataIn[{i}]"):
                return False

        # Connect write enable to RAM
        if not await self.connect(write_enable_id, ram_id, target_port_label="WriteEnable"):
            return False

        # Connect clock to RAM (for synchronization)
        if not await self.connect(clock_id, ram_id, target_port_label="Clock"):
            return False
        await self.log("  ✓ Connected address, data, write enable, and clock to RAM")

        # Create instruction output LEDs (8-bit)
        output_x = input_x + (14 * HORIZONTAL_GATE_SPACING)
        for i in range(8):
            led_id = await self.create_element(
                "Led", output_x, ram_y + (i * VERTICAL_STAGE_SPACING), f"Instruction[{i}]"
            )
            if led_id is None:
                return False

            # Connect RAM output to LED
            if not await self.connect(ram_id, led_id, source_port_label=f"DataOut[{i}]"):
                return False

        await self.log("  ✓ Created 8 instruction outputs")

        output_file = str(IC_COMPONENTS_DIR / "level7_instruction_memory_interface.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created Instruction Memory Interface IC"
            f"({self.element_count} elements, {self.connection_count} connections)"
        )
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
