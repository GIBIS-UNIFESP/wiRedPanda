#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Memory Stage IC

Implements the memory stage of a CPU pipeline.

Inputs:
  Address[0..7] (8-bit memory address)
  DataIn[0..7] (8-bit data to write to memory)
  Result[0..7] (8-bit result from execute stage for pass-through)
  MemRead (memory read enable signal)
  MemWrite (memory write enable signal)
  Clock (synchronization signal)
  Reset (reset signal)

Outputs:
  DataOut[0..7] (8-bit output: memory read data or pass-through result)

Memory Operations:
  MemRead=1, MemWrite=0: Load from memory at Address
    - DataOut = Memory[Address]
  MemRead=0, MemWrite=1: Store to memory at Address
    - Memory[Address] = DataIn
  MemRead=0, MemWrite=0: Pass-through result (ALU operation result)
    - DataOut = Result

Architecture:
  - Instantiates level6_ram_256x8 for 256×8 data memory
  - Uses 2-to-1 multiplexer to select between memory data and result pass-through
  - Synchronous write, asynchronous read
  - MemWrite controls RAM write enable
  - MemRead controls multiplexer selection

Pipeline Behavior:
  - Handles both load and store operations
  - Passes through ALU results for non-memory operations
  - Memory writes are synchronous (on clock edge)
  - Memory reads are asynchronous (immediate)
  - Completes pipeline: Fetch → Decode → Execute → Memory

Usage:
    python create_level8_memory_stage.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class MemoryStageBuilder(ICBuilderBase):
    """Builder for Memory Stage IC"""

    async def create(self) -> bool:
        """Create the Memory Stage IC"""
        await self.begin_build('Memory Stage')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0
        input_y = 100.0

        # ---- Create Address input switches (8-bit) ----
        address_inputs = []
        for i in range(8):
            addr_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), input_y, f"Address[{i}]")
            if addr_id is None:
                return False
            address_inputs.append(addr_id)
        await self.log("  ✓ Created 8 Address inputs")

        # ---- Create DataIn input switches (8-bit) ----
        datain_inputs = []
        for i in range(8):
            datain_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), input_y + VERTICAL_STAGE_SPACING / 2, f"DataIn[{i}]")
            if datain_id is None:
                return False
            datain_inputs.append(datain_id)
        await self.log("  ✓ Created 8 DataIn inputs")

        # ---- Create Result input switches (8-bit) ----
        result_inputs = []
        for i in range(8):
            result_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), input_y + VERTICAL_STAGE_SPACING, f"Result[{i}]")
            if result_id is None:
                return False
            result_inputs.append(result_id)
        await self.log("  ✓ Created 8 Result inputs")

        # ---- Create control signals ----
        control_x = input_x + (9 * HORIZONTAL_GATE_SPACING)

        memread_id = await self.create_element("InputSwitch", control_x, input_y, "MemRead")
        if memread_id is None:
            return False

        memwrite_id = await self.create_element("InputSwitch", control_x, input_y + VERTICAL_STAGE_SPACING / 2, "MemWrite")
        if memwrite_id is None:
            return False

        clock_id = await self.create_element("Clock", control_x, input_y + VERTICAL_STAGE_SPACING, "Clock")
        if clock_id is None:
            return False

        reset_id = await self.create_element("InputSwitch", control_x, input_y + (1.5 * VERTICAL_STAGE_SPACING), "Reset")
        if reset_id is None:
            return False

        await self.log("  ✓ Created control signals")

        # ---- Instantiate RAM ----
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level6_ram_256x8")):

            return False

        ram_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level6_ram_256x8"), input_x + (3 * HORIZONTAL_GATE_SPACING), 300.0, "RAM")
        if ram_id is None:
            return False
        await self.log("  ✓ Instantiated RAM")

        # ---- Connect Address inputs to RAM ----
        for i in range(8):
            if not await self.connect(address_inputs[i], ram_id, target_port_label=f"Address[{i}]"):
                return False

        # ---- Connect DataIn inputs to RAM ----
        for i in range(8):
            if not await self.connect(datain_inputs[i], ram_id, target_port_label=f"DataIn[{i}]"):
                return False

        # ---- Connect Clock to RAM ----
        if not await self.connect(clock_id, ram_id, target_port_label="Clock"):
            return False

        # ---- Connect MemWrite to RAM WriteEnable ----
        if not await self.connect(memwrite_id, ram_id, target_port_label="WriteEnable"):
            return False

        await self.log("  ✓ Connected RAM inputs")

        # ---- Create 8-bit 2-to-1 Multiplexer for output selection ----
        # When MemRead=1, select RAM DataOut; when MemRead=0, select Result
        # We'll create 8 individual 2-to-1 muxes (one per bit)

        mux_outputs = []
        mux_x = control_x + (3 * HORIZONTAL_GATE_SPACING)

        for i in range(8):
            # Create mux for bit i
            mux_id = await self.create_element("Mux", mux_x, input_y + (i * (VERTICAL_STAGE_SPACING / 2)), f"Mux[{i}]")
            if mux_id is None:
                return False
            mux_outputs.append(mux_id)

            # Connect Result (when MemRead=0) to Mux port 0
            if not await self.connect(result_inputs[i], mux_id, target_port_label="In0"):
                return False

            # Connect RAM DataOut (when MemRead=1) to Mux port 1
            if not await self.connect(ram_id, mux_id, source_port_label=f"DataOut[{i}]", target_port_label="In1"):
                return False

            # Connect MemRead as selector to Mux port 2
            if not await self.connect(memread_id, mux_id, target_port_label="S0"):
                return False

        await self.log("  ✓ Created 8-bit output multiplexer")

        # ---- Create output LEDs ----
        output_x = mux_x + (2 * HORIZONTAL_GATE_SPACING)

        for i in range(8):
            led_id = await self.create_element("Led", output_x, input_y + (i * (VERTICAL_STAGE_SPACING / 2)), f"DataOut[{i}]")
            if led_id is None:
                return False

            # Connect mux output to LED
            if not await self.connect(mux_outputs[i], led_id):
                return False

        await self.log("  ✓ Created DataOut outputs")

        output_file = str(IC_COMPONENTS_DIR / "level8_memory_stage.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created Memory Stage IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = MemoryStageBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Memory Stage IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
