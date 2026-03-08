#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Barrel Rotator IC

Implements bidirectional circular rotation using 2-stage mux cascade.
Architecture:
  - LEFT path:  2-stage cascade (rotate by 1, rotate by 2) with wrap-around
  - RIGHT path: 2-stage cascade (rotate by 1, rotate by 2) with wrap-around
  - Direction selector: final mux layer selects between LEFT and RIGHT paths

Left rotation:  output[i] = input[(i+rotate) % 4]
Right rotation: output[i] = input[(i-rotate+4) % 4]

Inputs: Data[0-3], Rotate_Amount[0-1], Direction (7 inputs)
        Direction: 0=left, 1=right
Outputs: Result[0-3] (4 outputs)

Usage:
    python create_barrel_rotator.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class BarrelRotatorBuilder(ICBuilderBase):
    """Builder for 4-bit Barrel Rotator IC"""

    async def create(self) -> bool:
        """Create the 4-bit Barrel Rotator IC"""
        await self.begin_build("Barrel Rotator")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position hierarchy for logical stages
        input_x_start = 50.0
        data_input_y = 100.0
        data_input_dup_y = data_input_y + VERTICAL_STAGE_SPACING
        rotate_input_x = input_x_start + HORIZONTAL_GATE_SPACING * 2.5

        left_stage1_y = 200.0
        left_stage2_y = left_stage1_y + VERTICAL_STAGE_SPACING
        right_stage1_y = left_stage2_y + VERTICAL_STAGE_SPACING
        right_stage2_y = right_stage1_y + VERTICAL_STAGE_SPACING
        direction_y = 300.0
        output_x = input_x_start + HORIZONTAL_GATE_SPACING * 3

        # Dense array spacing for 4-bit parallel elements
        dense_spacing = 40.0

        # Create input switches for Data[0-3]
        data_inputs = []
        for i in range(4):
            data_id = await self.create_element("InputSwitch", input_x_start + i * dense_spacing, data_input_y, f"Data[{i}]")
            if data_id is None:
                return False
            data_inputs.append(data_id)

        # Create input for Rotate_Amount[0]
        rotate_amount_0_id = await self.create_element("InputSwitch", rotate_input_x, data_input_y, "Rotate_Amount[0]")
        if rotate_amount_0_id is None:
            return False

        # Create input for Rotate_Amount[1]
        rotate_amount_1_id = await self.create_element("InputSwitch", rotate_input_x, data_input_dup_y, "Rotate_Amount[1]")
        if rotate_amount_1_id is None:
            return False

        # Create input for Direction (0=left, 1=right)
        direction_id = await self.create_element("InputSwitch", rotate_input_x, data_input_dup_y + VERTICAL_STAGE_SPACING, "Direction")
        if direction_id is None:
            return False

        # ============ LEFT ROTATION PATH ============
        # Stage 1: Rotate by 0 or 1 (left) - using bus_mux_4bit
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit")):

            return False

        left_stage1_mux_ic = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit"), input_x_start + HORIZONTAL_GATE_SPACING, left_stage1_y, "BusMux_Left_S1")
        if left_stage1_mux_ic is None:
            return False

        # Stage 2: Rotate by 0 or 2 (left) - using bus_mux_4bit
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit")):

            return False

        left_stage2_mux_ic = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit"), input_x_start + HORIZONTAL_GATE_SPACING, left_stage2_y, "BusMux_Left_S2")
        if left_stage2_mux_ic is None:
            return False

        # ============ RIGHT ROTATION PATH ============
        # Stage 1: Rotate by 0 or 1 (right) - using bus_mux_4bit
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit")):

            return False

        right_stage1_mux_ic = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit"), input_x_start + HORIZONTAL_GATE_SPACING, right_stage1_y, "BusMux_Right_S1")
        if right_stage1_mux_ic is None:
            return False

        # Stage 2: Rotate by 0 or 2 (right) - using bus_mux_4bit
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit")):

            return False

        right_stage2_mux_ic = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit"), input_x_start + HORIZONTAL_GATE_SPACING, right_stage2_y, "BusMux_Right_S2")
        if right_stage2_mux_ic is None:
            return False

        # ============ DIRECTION SELECTOR ============
        # Final selector: Left or Right rotation path - using bus_mux_4bit
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit")):

            return False

        direction_mux_ic = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit"), output_x, direction_y, "BusMux_Direction")
        if direction_mux_ic is None:
            return False

        # Create output LEDs for Result[0-3]
        output_leds = []
        for i in range(4):
            led_id = await self.create_element("Led", output_x + HORIZONTAL_GATE_SPACING + i * dense_spacing, direction_y, f"Result[{i}]")
            if led_id is None:
                return False
            output_leds.append(led_id)

        # ============ WIRE LEFT ROTATION PATH ============
        # Left Stage 1: Rotate by 1 left using bus_mux_4bit
        # In0: pass through (rotate=0), In1: rotated by 1 with wrap-around
        for i in range(4):
            # In0[i]: pass through (rotate=0)
            if not await self.connect(data_inputs[i], left_stage1_mux_ic, target_port_label=f"In0[{i}]"):
                return False

            # In1[i]: rotated by 1 left - input[(i-1+4) % 4]
            src_bit = (i - 1 + 4) % 4
            if not await self.connect(data_inputs[src_bit], left_stage1_mux_ic, target_port_label=f"In1[{i}]"):
                return False

        # Select: Rotate_Amount[0]
        if not await self.connect(rotate_amount_0_id, left_stage1_mux_ic, target_port_label="Sel"):
            return False

        # Left Stage 2: Rotate by 2 left using bus_mux_4bit (cascaded on stage 1 output)
        for i in range(4):
            # In0[i]: pass through from stage 1
            if not await self.connect(left_stage1_mux_ic, left_stage2_mux_ic, source_port_label=f"Out[{i}]", target_port_label=f"In0[{i}]"):
                return False

            # In1[i]: rotated by 2 from stage 1 - stage1[(i-2+4) % 4]
            src_bit = (i - 2 + 4) % 4
            if not await self.connect(left_stage1_mux_ic, left_stage2_mux_ic, source_port_label=f"Out[{src_bit}]", target_port_label=f"In1[{i}]"):
                return False

        # Select: Rotate_Amount[1]
        if not await self.connect(rotate_amount_1_id, left_stage2_mux_ic, target_port_label="Sel"):
            return False

        # ============ WIRE RIGHT ROTATION PATH ============
        # Right Stage 1: Rotate by 1 right using bus_mux_4bit
        # In0: pass through (rotate=0), In1: rotated by 1 with wrap-around
        for i in range(4):
            # In0[i]: pass through (rotate=0)
            if not await self.connect(data_inputs[i], right_stage1_mux_ic, target_port_label=f"In0[{i}]"):
                return False

            # In1[i]: rotated by 1 right - input[(i+1) % 4]
            src_bit = (i + 1) % 4
            if not await self.connect(data_inputs[src_bit], right_stage1_mux_ic, target_port_label=f"In1[{i}]"):
                return False

        # Select: Rotate_Amount[0]
        if not await self.connect(rotate_amount_0_id, right_stage1_mux_ic, target_port_label="Sel"):
            return False

        # Right Stage 2: Rotate by 2 right using bus_mux_4bit (cascaded on stage 1 output)
        for i in range(4):
            # In0[i]: pass through from stage 1
            if not await self.connect(right_stage1_mux_ic, right_stage2_mux_ic, source_port_label=f"Out[{i}]", target_port_label=f"In0[{i}]"):
                return False

            # In1[i]: rotated by 2 from stage 1 - stage1[(i+2) % 4]
            src_bit = (i + 2) % 4
            if not await self.connect(right_stage1_mux_ic, right_stage2_mux_ic, source_port_label=f"Out[{src_bit}]", target_port_label=f"In1[{i}]"):
                return False

        # Select: Rotate_Amount[1]
        if not await self.connect(rotate_amount_1_id, right_stage2_mux_ic, target_port_label="Sel"):
            return False

        # ============ WIRE DIRECTION SELECTOR ============
        # Select between left and right paths based on Direction using bus_mux_4bit
        # In0: left path, In1: right path, Sel: Direction (0=left, 1=right)
        for i in range(4):
            # In0[i]: left path output
            if not await self.connect(left_stage2_mux_ic, direction_mux_ic, source_port_label=f"Out[{i}]", target_port_label=f"In0[{i}]"):
                return False

            # In1[i]: right path output
            if not await self.connect(right_stage2_mux_ic, direction_mux_ic, source_port_label=f"Out[{i}]", target_port_label=f"In1[{i}]"):
                return False

        # Select: Direction
        if not await self.connect(direction_id, direction_mux_ic, target_port_label="Sel"):
            return False

        # Connect outputs to LEDs
        for i in range(4):
            if not await self.connect(direction_mux_ic, output_leds[i], source_port_label=f"Out[{i}]"):
                return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level5_barrel_rotator.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created Barrel Rotator IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = BarrelRotatorBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Barrel Rotator IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
