#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 8×8 RAM IC

Implements an 8-word, 8-bit-wide RAM by composing 8 single-bit RAMs.
(F26: formerly named "256×8" while only ever wiring 3 address bits — a true
256×8 needs 2048 flip-flops, which is infeasible to simulate at this scale.
The name now matches the actual capacity; consumers treat the unused high
address bits as partial decode.)

Inputs:
  Address[0..2] (3-bit address: selects one of 8 words)
  DataIn[0..7] (8-bit data to write)
  WriteEnable (Write control signal)
  Clock (Clock signal for synchronous write)

Outputs:
  DataOut[0..7] (8-bit data read from selected address)

Architecture:
  - Instantiate level4_ram_8x1 8 times
  - One RAM instance for each data bit
  - All share the same address, write enable, and clock
  - Each outputs one bit of the result

Usage:
    python create_level6_ram_8x8.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder


class RAM8x8Builder(ICBuilderBase):
    """Builder for 8×8 RAM IC"""

    async def create(self) -> bool:
        """Create the 8×8 RAM IC"""
        await self.begin_build("8×8 RAM")
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0

        # Create Address inputs (3-bit: 8 words)
        address_inputs = []
        for i in range(3):
            addr_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), 100.0, f"Address[{i}]"
            )
            if addr_id is None:
                return False
            address_inputs.append(addr_id)
        await self.log("  ✓ Created 3 Address inputs")

        # Create DataIn inputs (8-bit)
        data_in_inputs = []
        for i in range(8):
            data_in_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), 100.0 + VERTICAL_STAGE_SPACING, f"DataIn[{i}]"
            )
            if data_in_id is None:
                return False
            data_in_inputs.append(data_in_id)
        await self.log("  ✓ Created 8 DataIn inputs")

        # Create control inputs
        we_id = await self.create_element(
            "InputSwitch", input_x + (8 * HORIZONTAL_GATE_SPACING), 100.0 + VERTICAL_STAGE_SPACING, "WriteEnable"
        )
        if we_id is None:
            return False

        clk_id = await self.create_element(
            "InputSwitch", input_x + (8 * HORIZONTAL_GATE_SPACING), 100.0 + (2 * VERTICAL_STAGE_SPACING), "Clock"
        )
        if clk_id is None:
            return False

        # Reset input: async clear of all cells, fanned out to every ram_8x1
        # column's own Reset (F54, matching the RAM family).
        reset_id = await self.create_element(
            "InputSwitch", input_x + (8 * HORIZONTAL_GATE_SPACING), 100.0 + (3 * VERTICAL_STAGE_SPACING), "Reset"
        )
        if reset_id is None:
            return False
        await self.log("  ✓ Created WriteEnable, Clock and Reset inputs")

        # Instantiate 8 RAM 8×1 modules (one for each data bit)
        # This creates an 8×1 by 8 parallel structure for 8×8 total
        ram_ics = []
        ram_x = input_x + (10 * HORIZONTAL_GATE_SPACING)
        for bit_idx in range(8):
            ram_id = await self.instantiate_ic(
                "level4_ram_8x1",
                ram_x + (bit_idx * HORIZONTAL_GATE_SPACING),
                250.0,
                f"RAM_Bit{bit_idx}",
            )
            if ram_id is None:
                return False
            ram_ics.append(ram_id)
        await self.log("  ✓ Instantiated 8 RAM 8×1 modules")

        # Connect the 3 address bits to all RAM modules (8 words each)
        for bit_idx, ram_id in enumerate(ram_ics):
            # Connect address bits (using available address lines)
            for addr_bit in range(3):  # 8x1 RAM uses 3 address bits
                if not await self.connect(address_inputs[addr_bit], ram_id, target_port_label=f"Address[{addr_bit}]"):
                    return False

            # Connect data input
            if not await self.connect(data_in_inputs[bit_idx], ram_id, target_port_label="DataIn"):
                return False

            # Connect write enable
            if not await self.connect(we_id, ram_id, target_port_label="WriteEnable"):
                return False

            # Connect clock
            if not await self.connect(clk_id, ram_id, target_port_label="Clock"):
                return False

            # Connect Reset (async clear of every column — F54)
            if not await self.connect(reset_id, ram_id, target_port_label="Reset"):
                return False
        await self.log("  ✓ Connected control signals to RAM modules")

        # Create output LEDs for DataOut (8-bit)
        output_x = ram_x + (10 * HORIZONTAL_GATE_SPACING)
        for bit_idx, ram_id in enumerate(ram_ics):
            led_id = await self.create_element(
                "Led", output_x, 250.0 + (bit_idx * VERTICAL_STAGE_SPACING), f"DataOut[{bit_idx}]"
            )
            if led_id is None:
                return False

            # Connect RAM output to LED
            if not await self.connect(ram_id, led_id, source_port_label="DataOut"):
                return False
        await self.log("  ✓ Created 8 DataOut outputs and connected to RAM modules")

        output_file = str(IC_COMPONENTS_DIR / "level6_ram_8x8.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 8×8 RAM IC ({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log("   Note: Uses level4_ram_8x1 as building blocks (simplified version)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = RAM8x8Builder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "8x8 RAM IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
