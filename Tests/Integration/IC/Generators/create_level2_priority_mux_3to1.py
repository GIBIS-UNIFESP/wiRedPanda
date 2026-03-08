#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 3-way Priority Multiplexer IC (1-bit)

Inputs: data0, data1, data2 (3 data inputs), sel0, sel1 (2 select signals)
Output: out

Priority: sel0 > sel1
Logic: sel0 ? data0 : (sel1 ? data1 : data2)

Circuit:
- 2 cascaded 2:1 multiplexers implementing priority logic
- Stage 1: mux(data1, data2, sel1) -> intermediate
- Stage 2: mux(data0, intermediate, sel0) -> out

Usage:
    python create_priority_mux_3to1.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import (
    HORIZONTAL_GATE_SPACING,
    VERTICAL_STAGE_SPACING,
)


class PriorityMux3to1Builder(ICBuilderBase):
    """Builder for 3-way Priority Multiplexer IC"""

    async def create(self) -> bool:
        """Create the 3-way Priority Multiplexer IC"""
        await self.begin_build('Priority Mux 3-to-1')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position calculation with proper spacing
        input_x = 50.0
        data_input_y = 100.0

        # Create input switches for data
        data_inputs = []
        for i in range(3):
            data_id = await self.create_element("InputSwitch", input_x, data_input_y + i * VERTICAL_STAGE_SPACING, f"data{i}")
            if data_id is None:
                return False
            data_inputs.append(data_id)
            await self.log(f"  Created data{i} (id={data_id})")

        # Create input switches for select signals (sel0, sel1)
        sel_input_x = input_x + 3 * HORIZONTAL_GATE_SPACING
        sel_input_y = 100.0

        sel_inputs = []
        for i in range(2):
            sel_id = await self.create_element("InputSwitch", sel_input_x, sel_input_y + i * VERTICAL_STAGE_SPACING, f"sel{i}")
            if sel_id is None:
                return False
            sel_inputs.append(sel_id)
            await self.log(f"  Created sel{i} (id={sel_id})")

        # Stage 1: Mux between data1 and data2 using sel1
        # (sel1 ? data1 : data2)
        stage1_mux_x = input_x + HORIZONTAL_GATE_SPACING
        stage1_mux_y = data_input_y + VERTICAL_STAGE_SPACING

        stage1_mux = await self.create_element("Mux", stage1_mux_x, stage1_mux_y, "stage1")
        if stage1_mux is None:
            return False

        # Connect data1 and data2 to stage1 (SWAPPED for correct Mux logic)
        # Mux logic: sel ? In1 : In0
        # We want: sel1 ? data1 : data2, so data2 goes to In0, data1 goes to In1
        if not await self.connect(data_inputs[2], stage1_mux, target_port_label="In0"):
            return False

        if not await self.connect(data_inputs[1], stage1_mux, target_port_label="In1"):
            return False

        # Connect sel1 to stage1 select line
        if not await self.connect(sel_inputs[1], stage1_mux, target_port_label="S0"):
            return False

        await self.log("  Created stage 1: mux(data1, data2, sel1)")

        # Stage 2: Mux between data0 and stage1 using sel0
        # (sel0 ? data0 : stage1)
        stage2_mux_x = stage1_mux_x + HORIZONTAL_GATE_SPACING
        stage2_mux_y = data_input_y

        stage2_mux = await self.create_element("Mux", stage2_mux_x, stage2_mux_y, "stage2")
        if stage2_mux is None:
            return False

        # Connect data0 and stage1 output to stage2 (SWAPPED for correct Mux logic)
        # Mux logic: sel ? In1 : In0
        # We want: sel0 ? data0 : stage1, so stage1 goes to In0, data0 goes to In1
        if not await self.connect(stage1_mux, stage2_mux, source_port_label="Out", target_port_label="In0"):
            return False

        if not await self.connect(data_inputs[0], stage2_mux, target_port_label="In1"):
            return False

        # Connect sel0 to stage2 select line
        if not await self.connect(sel_inputs[0], stage2_mux, target_port_label="S0"):
            return False

        await self.log("  Created stage 2: mux(data0, stage1, sel0)")

        # Create output LED
        output_x = stage2_mux_x + HORIZONTAL_GATE_SPACING
        output_y = data_input_y

        output_led = await self.create_element("Led", output_x, output_y, "out")
        if output_led is None:
            return False

        # Connect stage2 output to LED
        if not await self.connect(stage2_mux, output_led, source_port_label="Out"):
            return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level2_priority_mux_3to1.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"Successfully created PriorityMux3to1 IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = PriorityMux3to1Builder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Priority Mux 3-to-1 IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
