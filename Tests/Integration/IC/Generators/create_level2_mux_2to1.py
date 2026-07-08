#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 2-to-1 Multiplexer IC using a single Mux primitive

Inputs: Data[0], Data[1], Sel[0], Enable (74153-style strobe, defaults high)
Output: Output

Architecture:
- Single Mux element with inputSize=3 (2 data + 1 select)
- AND gate gates the Mux output with Enable

Usage:
    python create_level2_mux_2to1.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder


class Multiplexer2to1Builder(ICBuilderBase):
    """Builder for 2-to-1 Multiplexer IC"""

    async def create(self) -> bool:
        """Create 2-to-1 Multiplexer IC"""
        data_inputs = 2
        input_size = 3  # 2 data + 1 select

        await self.begin_build("Multiplexer 2-to-1")
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0
        input_y = 100.0

        # Create input switches for data
        data_inputs_list = []
        for i in range(data_inputs):
            data_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), input_y, f"Data[{i}]"
            )
            if data_id is None:
                return False
            data_inputs_list.append(data_id)
            await self.log(f"  ✓ Created Data[{i}] (id={data_id})")

        # Create input switch for select line
        select_inputs = []
        select_input_x = input_x + (data_inputs * HORIZONTAL_GATE_SPACING) + HORIZONTAL_GATE_SPACING
        sel_id = await self.create_element("InputSwitch", select_input_x, input_y, "Sel[0]")
        if sel_id is None:
            return False
        select_inputs.append(sel_id)
        await self.log(f"  ✓ Created Sel[0] (id={sel_id})")

        # Create single Mux element
        mux_x = select_input_x + (2 * HORIZONTAL_GATE_SPACING)
        mux_y = input_y + VERTICAL_STAGE_SPACING
        mux_id = await self.create_element("Mux", mux_x, mux_y, "Mux_2to1")
        if mux_id is None:
            return False
        await self.log(f"  ✓ Created Mux (id={mux_id})")

        # Set the Mux inputSize
        response = await self.mcp.send_command("change_input_size", {"element_id": mux_id, "size": input_size})
        if not response.success:
            self.log_error(f"Failed to set Mux inputSize: {response.error}")
            return False
        await self.log(f"  ✓ Set Mux inputSize to {input_size}")

        # Connect data inputs to Mux
        for i in range(data_inputs):
            if not await self.connect(data_inputs_list[i], mux_id, target_port_label=f"In{i}"):
                return False
            await self.log(f"  ✓ Connected Data[{i}] to Mux (In{i})")

        # Connect select input to Mux
        if not await self.connect(select_inputs[0], mux_id, target_port_label="S0"):
            return False
        await self.log("  ✓ Connected Sel[0] to Mux (S0)")

        # Active-high Enable, defaulted HIGH: the mux output is ANDed with Enable,
        # so Enable=0 forces the output low (74153-style strobe). Defaulting it
        # high means a mux embedded WITHOUT an explicit Enable connection behaves
        # exactly as before — both the engine and the SV/Arduino export resolve an
        # unconnected Enable to its high default.
        enable_x = mux_x + HORIZONTAL_GATE_SPACING
        enable_id = await self.create_element("InputSwitch", enable_x, input_y, "Enable")
        if enable_id is None:
            return False
        set_en = await self.mcp.send_command("set_input_value", {"element_id": enable_id, "value": True})
        if not set_en.success:
            self.log_error("Failed to default Enable high")
            return False

        en_and = await self.create_element("And", enable_x, mux_y, "out_enable")
        if en_and is None:
            return False
        if not await self.connect(mux_id, en_and, source_port_label="Out", target_port=0):
            return False
        if not await self.connect(enable_id, en_and, target_port=1):
            return False

        # Create output LED
        output_x = enable_x + HORIZONTAL_GATE_SPACING
        output_id = await self.create_element("Led", output_x, mux_y, "Output")
        if output_id is None:
            return False
        if not await self.connect(en_and, output_id):
            return False
        await self.log("  ✓ Connected Mux output through Enable gate to LED")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level2_mux_2to1.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 2:1 Multiplexer IC"
            f"({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = Multiplexer2to1Builder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "2-to-1 Multiplexer IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
