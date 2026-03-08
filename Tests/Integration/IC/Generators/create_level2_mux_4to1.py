#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-to-1 Multiplexer IC using a single Mux primitive

Inputs: Data[0..3], Sel[0..1]
Output: Output

Architecture:
- Single Mux element with inputSize=6 (4 data + 2 select)

Usage:
    python create_level2_mux_4to1.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class Multiplexer4to1Builder(ICBuilderBase):
    """Builder for 4-to-1 Multiplexer IC"""

    async def create(self) -> bool:
        """Create 4-to-1 Multiplexer IC"""
        data_inputs = 4
        select_bits = 2
        input_size = 6  # 4 data + 2 select

        await self.begin_build('Multiplexer 4-to-1')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0
        input_y = 100.0

        # Create input switches for data
        data_inputs_list = []
        for i in range(data_inputs):
            data_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), input_y, f"Data[{i}]")
            if data_id is None:
                return False
            data_inputs_list.append(data_id)
            await self.log(f"  ✓ Created Data[{i}] (id={data_id})")

        # Create input switches for select lines
        select_inputs = []
        select_input_x = input_x + (data_inputs * HORIZONTAL_GATE_SPACING) + HORIZONTAL_GATE_SPACING
        for i in range(select_bits):
            sel_id = await self.create_element("InputSwitch", select_input_x + (i * HORIZONTAL_GATE_SPACING), input_y, f"Sel[{i}]")
            if sel_id is None:
                return False
            select_inputs.append(sel_id)
            await self.log(f"  ✓ Created Sel[{i}] (id={sel_id})")

        # Create single Mux element
        mux_x = select_input_x + (2 * HORIZONTAL_GATE_SPACING)
        mux_y = input_y + VERTICAL_STAGE_SPACING
        mux_id = await self.create_element("Mux", mux_x, mux_y, "Mux_4to1")
        if mux_id is None:
            return False
        await self.log(f"  ✓ Created Mux (id={mux_id})")

        # Set the Mux inputSize
        response = await self.mcp.send_command("set_element_properties", {
            "element_id": mux_id,
            "input_size": input_size
        })
        if not response.success:
            self.log_error(f"Failed to set Mux inputSize: {response.error}")
            return False
        await self.log(f"  ✓ Set Mux inputSize to {input_size}")

        # Connect data inputs to Mux
        for i in range(data_inputs):
            if not await self.connect(data_inputs_list[i], mux_id, target_port_label=f"In{i}"):
                return False
            await self.log(f"  ✓ Connected Data[{i}] to Mux (In{i})")

        # Connect select inputs to Mux
        for i in range(select_bits):
            if not await self.connect(select_inputs[i], mux_id, target_port_label=f"S{i}"):
                return False
            await self.log(f"  ✓ Connected Sel[{i}] to Mux (S{i})")

        # Create output LED
        output_x = mux_x + HORIZONTAL_GATE_SPACING
        output_id = await self.create_element("Led", output_x, mux_y, "Output")
        if output_id is None:
            return False
        await self.log(f"  ✓ Created output LED (id={output_id})")

        # Connect Mux output to LED
        if not await self.connect(mux_id, output_id, source_port_label="Out"):
            return False
        await self.log(f"  ✓ Connected Mux output to LED")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level2_mux_4to1.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 4:1 Multiplexer IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = Multiplexer4to1Builder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-to-1 Multiplexer IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
