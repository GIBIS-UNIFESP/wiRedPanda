#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 5-way ALU Result Selector IC (1-bit)

Inputs: result0, result1, result2, result3, result4 (5 ALU result bits)
        op0, op1, op2 (3-bit opcode for selecting result)
Output: out (selected_result - 1 output bit)

Circuit:
- 3-level cascaded mux structure to select from 5 inputs
  - Level 1: 2 muxes to reduce first 4 inputs to 2 using op0
  - Level 2: 1 mux to reduce level 1 outputs to 1 using op1
  - Level 3: 1 mux to select between level 2 output and result4 using op2

Logic: Select one of 5 inputs based on 3-bit opcode
- Used for selecting between 5 different ALU operations per bit

Usage:
    python create_alu_selector_5way.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class ALUSelector5wayBuilder(ICBuilderBase):
    """Builder for 5-way ALU Result Selector IC"""

    async def create(self) -> bool:
        """Create the 5-way ALU Result Selector IC"""
        await self.begin_build("ALU Selector 5-way")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position hierarchy
        input_x = 50.0
        input_y = 100.0
        opcode_x = input_x + (5 * HORIZONTAL_GATE_SPACING) + HORIZONTAL_GATE_SPACING
        level1_x = input_x + HORIZONTAL_GATE_SPACING
        level2_x = level1_x + HORIZONTAL_GATE_SPACING
        level3_x = level2_x + HORIZONTAL_GATE_SPACING
        output_x = level3_x + HORIZONTAL_GATE_SPACING

        # Create input switches for 5 ALU result bits
        result_inputs = []
        for i in range(5):
            result_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), input_y, f"result{i}")
            if result_id is None:
                return False
            result_inputs.append(result_id)

        # Create input switches for 3-bit opcode
        op_inputs = []
        for i in range(3):
            op_id = await self.create_element("InputSwitch", opcode_x + (i * HORIZONTAL_GATE_SPACING), input_y, f"op{i}")
            if op_id is None:
                return False
            op_inputs.append(op_id)

        # Build 3-level cascaded mux to select from 5 inputs
        # Level 1: Two 2:1 muxes to reduce 4 inputs to 2
        # Level 2: One 2:1 mux to reduce 2 inputs to 1
        # Result selected: opcode[2:0] determines which of 5 inputs (only need 5 of 8 possible positions)

        # Level 1: Mux between result0/result1 using op0
        l1_mux1 = await self.create_element("Mux", level1_x, input_y + VERTICAL_STAGE_SPACING, "l1_mux1")
        if l1_mux1 is None:
            return False

        # Level 1: Mux between result2/result3 using op0
        l1_mux2 = await self.create_element("Mux", level1_x, input_y + (2 * VERTICAL_STAGE_SPACING), "l1_mux2")
        if l1_mux2 is None:
            return False

        # Connect level 1 muxes
        port_labels = ["In0", "In1"]
        for source_id, result_idx, mux_id, port in [
            (result_inputs[0], 0, l1_mux1, 0),  # result0 to l1_mux1 In0
            (result_inputs[1], 1, l1_mux1, 1),  # result1 to l1_mux1 In1
            (result_inputs[2], 2, l1_mux2, 0),  # result2 to l1_mux2 In0
            (result_inputs[3], 3, l1_mux2, 1),  # result3 to l1_mux2 In1
        ]:
            if not await self.connect(source_id, mux_id, target_port_label=port_labels[port]):
                return False

        # Connect op0 to both level 1 muxes select line
        for mux_id in [l1_mux1, l1_mux2]:
            if not await self.connect(op_inputs[0], mux_id, target_port_label="S0"):
                return False

        await self.log("  Created level 1: two 2:1 muxes")

        # Level 2: Mux between l1_mux1, l1_mux2, result4 (need special logic for 5 inputs)
        # Use cascaded structure: first select between l1_mux1 and l1_mux2 using op1
        l2_mux1 = await self.create_element("Mux", level2_x, input_y + (1.5 * VERTICAL_STAGE_SPACING), "l2_mux1")
        if l2_mux1 is None:
            return False

        # Connect l1 mux outputs to l2_mux1
        if not await self.connect(l1_mux1, l2_mux1, source_port_label="Out", target_port_label="In0"):
            return False

        if not await self.connect(l1_mux2, l2_mux1, source_port_label="Out", target_port_label="In1"):
            return False

        if not await self.connect(op_inputs[1], l2_mux1, target_port_label="S0"):
            return False

        await self.log("  Created level 2: select between 4-input results")

        # Level 3: Mux between l2_mux1 and result4 using op2
        l3_mux = await self.create_element("Mux", level3_x, input_y + (1.5 * VERTICAL_STAGE_SPACING), "l3_mux")
        if l3_mux is None:
            return False

        # Connect final level
        if not await self.connect(l2_mux1, l3_mux, source_port_label="Out", target_port_label="In0"):
            return False

        if not await self.connect(result_inputs[4], l3_mux, target_port_label="In1"):
            return False

        if not await self.connect(op_inputs[2], l3_mux, target_port_label="S0"):
            return False

        await self.log("  Created level 3: final 2:1 mux for result4")

        # Create output LED
        output_led = await self.create_element("Led", output_x, input_y + (1.5 * VERTICAL_STAGE_SPACING), "out")
        if output_led is None:
            return False

        # Connect final output
        if not await self.connect(l3_mux, output_led, source_port_label="Out"):
            return False

        await self.log("  Created output LED")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level3_alu_selector_5way.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created ALUSelector5way IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = ALUSelector5wayBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "ALU Selector 5-way IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
