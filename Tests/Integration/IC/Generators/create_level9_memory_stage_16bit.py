#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 16-bit Memory Stage IC

Implements the data-memory stage for level9_cpu_16bit_risc's Load/Store
support, mirroring level8_memory_stage's exact contract (Address/DataIn/
Result-passthrough/MemRead/MemWrite/Clock/Reset -> DataOut) doubled to 16
bits via two level6_ram_8x8 lanes (low/high byte), the same way
level7_alu_16bit doubles level6_alu_8bit and level9_fetch_stage_16bit
doubles level7_instruction_memory_interface.

Inputs:
  Address[0..7] (8-bit memory address, shared by both lanes; the backing
                 RAM holds 8 words, so the high bits are partial decode --
                 addresses alias modulo 8, same precedent as
                 level8_memory_stage)
  DataIn[0..15] (16-bit data to write to memory)
  Result[0..15] (16-bit result from execute stage for pass-through)
  MemRead (memory read enable signal)
  MemWrite (memory write enable signal)
  Clock (synchronization signal)
  Reset (async clear of the backing memory, active HIGH)

Outputs:
  DataOut[0..15] (16-bit output: memory read data or pass-through result)

Memory Operations:
  MemRead=1, MemWrite=0: Load from memory at Address
    - DataOut = Memory[Address]
  MemRead=0, MemWrite=1: Store to memory at Address
    - Memory[Address] = DataIn
  MemRead=0, MemWrite=0: Pass-through result (ALU operation result)
    - DataOut = Result

Architecture:
  - Two level6_ram_8x8 lanes (low/high byte) for the data memory
  - 2-to-1 multiplexer (16 bits) to select between memory data and
    result pass-through
  - Synchronous write, asynchronous read
  - MemWrite controls both lanes' RAM write enable
  - MemRead controls the multiplexer selection

Usage:
    python create_level9_memory_stage_16bit.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder


class MemoryStage16bitBuilder(ICBuilderBase):
    """Builder for 16-bit Memory Stage IC"""

    async def create(self) -> bool:
        """Create the 16-bit Memory Stage IC"""
        await self.begin_build("16-bit Memory Stage")
        if not await self.create_new_circuit():
            return False

        input_x = 50.0
        ram_low_x = input_x + (3 * HORIZONTAL_GATE_SPACING)
        ram_y = 600.0

        # ---- Instantiate the two RAM lanes first ----
        # The Address/DataIn/Result rows above them share their X columns, so
        # they need clearance from the real (port-count-dependent) height --
        # level6_ram_8x8 is far taller than the flat 64px assumed elsewhere
        # (queried via instantiate_ic_with_size).
        ram_low_handle = await self.instantiate_ic_with_size("level6_ram_8x8", ram_low_x, ram_y, "RAM_Low")
        if ram_low_handle is None:
            return False
        ram_low_id = ram_low_handle.element_id

        ram_high_x = ram_low_x + max(HORIZONTAL_GATE_SPACING * 2, ram_low_handle.width + HORIZONTAL_GATE_SPACING)
        ram_high_handle = await self.instantiate_ic_with_size("level6_ram_8x8", ram_high_x, ram_y, "RAM_High")
        if ram_high_handle is None:
            return False
        ram_high_id = ram_high_handle.element_id
        await self.log("  ✓ Instantiated RAM_Low and RAM_High")

        ram_half_height = max(ram_low_handle.height, ram_high_handle.height) / 2
        result_y = ram_y - ram_half_height - VERTICAL_STAGE_SPACING
        datain_y = result_y - VERTICAL_STAGE_SPACING
        address_y = datain_y - VERTICAL_STAGE_SPACING

        # ---- Create Address input switches (8-bit, shared by both lanes) ----
        address_inputs = []
        for i in range(8):
            addr_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), address_y, f"Address[{i}]"
            )
            if addr_id is None:
                return False
            address_inputs.append(addr_id)
        await self.log("  ✓ Created 8 Address inputs")

        # ---- Create DataIn input switches (16-bit) ----
        datain_inputs = []
        for i in range(16):
            datain_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), datain_y, f"DataIn[{i}]"
            )
            if datain_id is None:
                return False
            datain_inputs.append(datain_id)
        await self.log("  ✓ Created 16 DataIn inputs")

        # ---- Create Result input switches (16-bit) ----
        result_inputs = []
        for i in range(16):
            result_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), result_y, f"Result[{i}]"
            )
            if result_id is None:
                return False
            result_inputs.append(result_id)
        await self.log("  ✓ Created 16 Result inputs")

        # ---- Create control signals ----
        control_x = input_x + (17 * HORIZONTAL_GATE_SPACING)

        memread_id = await self.create_element("InputSwitch", control_x, address_y, "MemRead")
        if memread_id is None:
            return False

        memwrite_id = await self.create_element(
            "InputSwitch", control_x, address_y + VERTICAL_STAGE_SPACING, "MemWrite"
        )
        if memwrite_id is None:
            return False

        clock_id = await self.create_element("Clock", control_x, address_y + (2 * VERTICAL_STAGE_SPACING), "Clock")
        if clock_id is None:
            return False

        reset_id = await self.create_element(
            "InputSwitch", control_x, address_y + (3 * VERTICAL_STAGE_SPACING), "Reset"
        )
        if reset_id is None:
            return False

        await self.log("  ✓ Created control signals")

        # ---- Connect Address inputs to both RAM lanes ----
        # 8-bit address bus over an 8-word RAM: the high bits are partial
        # decode (aliasing modulo 8), same precedent as level8_memory_stage.
        for i in range(3):
            if not await self.connect(address_inputs[i], ram_low_id, target_port_label=f"Address[{i}]"):
                return False
            if not await self.connect(address_inputs[i], ram_high_id, target_port_label=f"Address[{i}]"):
                return False

        # ---- Connect DataIn inputs to both RAM lanes ----
        for i in range(8):
            if not await self.connect(datain_inputs[i], ram_low_id, target_port_label=f"DataIn[{i}]"):
                return False
            if not await self.connect(datain_inputs[8 + i], ram_high_id, target_port_label=f"DataIn[{i}]"):
                return False

        # ---- Connect Clock, MemWrite, Reset to both RAM lanes ----
        for ram_id in (ram_low_id, ram_high_id):
            if not await self.connect(clock_id, ram_id, target_port_label="Clock"):
                return False
            if not await self.connect(memwrite_id, ram_id, target_port_label="WriteEnable"):
                return False
            if not await self.connect(reset_id, ram_id, target_port_label="Reset"):
                return False

        await self.log("  ✓ Connected RAM inputs")

        # ---- Create 16-bit 2-to-1 multiplexer for output selection ----
        # When MemRead=1, select RAM DataOut; when MemRead=0, select Result.
        mux_outputs = []
        mux_x = control_x + (3 * HORIZONTAL_GATE_SPACING)

        for i in range(16):
            mux_id = await self.create_element("Mux", mux_x, address_y + (i * VERTICAL_STAGE_SPACING), f"Mux[{i}]")
            if mux_id is None:
                return False
            mux_outputs.append(mux_id)

            # Connect Result (when MemRead=0) to Mux port 0
            if not await self.connect(result_inputs[i], mux_id, target_port_label="In0"):
                return False

            # Connect RAM DataOut (when MemRead=1) to Mux port 1
            ram_id = ram_low_id if i < 8 else ram_high_id
            if not await self.connect(ram_id, mux_id, source_port_label=f"DataOut[{i % 8}]", target_port_label="In1"):
                return False

            # Connect MemRead as selector to Mux port 2
            if not await self.connect(memread_id, mux_id, target_port_label="S0"):
                return False

        await self.log("  ✓ Created 16-bit output multiplexer")

        # ---- Create output LEDs ----
        output_x = mux_x + (2 * HORIZONTAL_GATE_SPACING)

        for i in range(16):
            led_id = await self.create_element(
                "Led", output_x, address_y + (i * VERTICAL_STAGE_SPACING), f"DataOut[{i}]"
            )
            if led_id is None:
                return False

            if not await self.connect(mux_outputs[i], led_id):
                return False

        await self.log("  ✓ Created DataOut outputs")

        output_file = str(IC_COMPONENTS_DIR / "level9_memory_stage_16bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 16-bit Memory Stage IC "
            f"({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = MemoryStage16bitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "16-bit Memory Stage IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
