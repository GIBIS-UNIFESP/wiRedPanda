#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Barrel Shifter IC

Implements bidirectional linear shift with zero-fill using 2-stage mux cascade.
Architecture:
  - LEFT path:  2-stage cascade (shift by 1, shift by 2) with zero-fill from right
  - RIGHT path: 2-stage cascade (shift by 1, shift by 2) with zero-fill from left
  - Direction selector: final mux layer selects between LEFT and RIGHT paths

Left shift:  output[i] = input[i-shift] (or 0 if i-shift < 0)
Right shift: output[i] = input[i+shift] (or 0 if i+shift >= 4)

Inputs: Data[0-3], ShiftAmount[0-1], ShiftDirection (7 inputs)
        ShiftDirection: 0=left, 1=right
Outputs: ShiftedData[0-3] (4 outputs)

Usage:
    python create_barrel_shifter_4bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class BarrelShifter4bitBuilder(ICBuilderBase):
    """Builder for 4-bit Barrel Shifter IC"""

    async def create(self) -> bool:
        """Create the 4-bit Barrel Shifter IC"""
        await self.begin_build("Barrel Shifter 4-bit")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position hierarchy for logical stages
        input_x_start = 50.0
        data_input_y = 100.0
        shift_input_x = input_x_start + HORIZONTAL_GATE_SPACING * 2.5
        shift_input_y = data_input_y

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

        # Create input for ShiftAmount[0]
        shift_amount_0_id = await self.create_element("InputSwitch", shift_input_x, shift_input_y, "ShiftAmount[0]")
        if shift_amount_0_id is None:
            return False

        # Create input for ShiftAmount[1]
        shift_amount_1_id = await self.create_element("InputSwitch", shift_input_x, shift_input_y + VERTICAL_STAGE_SPACING, "ShiftAmount[1]")
        if shift_amount_1_id is None:
            return False

        # Create input for ShiftDirection (0=left, 1=right)
        shift_direction_id = await self.create_element("InputSwitch", shift_input_x, shift_input_y + VERTICAL_STAGE_SPACING * 1.5, "ShiftDirection")
        if shift_direction_id is None:
            return False

        # Create ground input (always 0) for zero-fill
        ground_id = await self.create_element("InputGnd", input_x_start, shift_input_y + VERTICAL_STAGE_SPACING * 1.5, "Ground")
        if ground_id is None:
            return False

        # ============ LEFT SHIFT PATH ============
        # Stage 1: Shift by 0 or 1 (left) - using bus_mux_4bit
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit")):

            return False

        left_stage1_mux_ic = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit"), input_x_start + HORIZONTAL_GATE_SPACING, left_stage1_y, "BusMux_Left_S1")
        if left_stage1_mux_ic is None:
            return False

        # Stage 2: Shift by 0 or 2 (left) - using bus_mux_4bit
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit")):

            return False

        left_stage2_mux_ic = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit"), input_x_start + HORIZONTAL_GATE_SPACING, left_stage2_y, "BusMux_Left_S2")
        if left_stage2_mux_ic is None:
            return False

        # ============ RIGHT SHIFT PATH ============
        # Stage 1: Shift by 0 or 1 (right) - using bus_mux_4bit
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit")):

            return False

        right_stage1_mux_ic = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit"), input_x_start + HORIZONTAL_GATE_SPACING, right_stage1_y, "BusMux_Right_S1")
        if right_stage1_mux_ic is None:
            return False

        # Stage 2: Shift by 0 or 2 (right) - using bus_mux_4bit
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit")):

            return False

        right_stage2_mux_ic = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit"), input_x_start + HORIZONTAL_GATE_SPACING, right_stage2_y, "BusMux_Right_S2")
        if right_stage2_mux_ic is None:
            return False

        # ============ DIRECTION SELECTOR ============
        # Final selector: Left or Right shift path - using bus_mux_4bit
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit")):

            return False

        direction_mux_ic = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit"), output_x, direction_y, "BusMux_Direction")
        if direction_mux_ic is None:
            return False

        # Create output LEDs for ShiftedData[0-3]
        output_leds = []
        for i in range(4):
            led_id = await self.create_element("Led", output_x + HORIZONTAL_GATE_SPACING + i * dense_spacing, direction_y, f"ShiftedData[{i}]")
            if led_id is None:
                return False
            output_leds.append(led_id)

        # ============ WIRE LEFT SHIFT PATH ============
        # Left Stage 1: Shift by 1 left using bus_mux_4bit
        # In0: pass through (shift=0), In1: shifted by 1 or zero-fill
        for i in range(4):
            # In0[i]: pass through (shift=0)
            if not await self.connect(data_inputs[i], left_stage1_mux_ic, target_port_label=f"In0[{i}]"):
                return False

            # In1[i]: shifted value or zero
            if i > 0:
                if not await self.connect(data_inputs[i - 1], left_stage1_mux_ic, target_port_label=f"In1[{i}]"):
                    return False
            else:
                if not await self.connect(ground_id, left_stage1_mux_ic, target_port_label=f"In1[{i}]"):
                    return False

        # Select: ShiftAmount[0]
        if not await self.connect(shift_amount_0_id, left_stage1_mux_ic, target_port_label="Sel"):
            return False

        # Left Stage 2: Shift by 2 left using bus_mux_4bit (cascaded on stage 1 output)
        for i in range(4):
            # In0[i]: pass through from stage 1
            if not await self.connect(left_stage1_mux_ic, left_stage2_mux_ic, source_port_label=f"Out[{i}]", target_port_label=f"In0[{i}]"):
                return False

            # In1[i]: shifted by 2 or zero
            if i >= 2:
                if not await self.connect(left_stage1_mux_ic, left_stage2_mux_ic, source_port_label=f"Out[{i - 2}]", target_port_label=f"In1[{i}]"):
                    return False
            else:
                if not await self.connect(ground_id, left_stage2_mux_ic, target_port_label=f"In1[{i}]"):
                    return False

        # Select: ShiftAmount[1]
        if not await self.connect(shift_amount_1_id, left_stage2_mux_ic, target_port_label="Sel"):
            return False

        # ============ WIRE RIGHT SHIFT PATH ============
        # Right Stage 1: Shift by 1 right using bus_mux_4bit
        # When shifting right by 1: output[i] gets input[i+1] (higher index)
        for i in range(4):
            # In0[i]: pass through (shift=0)
            if not await self.connect(data_inputs[i], right_stage1_mux_ic, target_port_label=f"In0[{i}]"):
                return False

            # In1[i]: shifted value or zero
            if i < 3:
                if not await self.connect(data_inputs[i + 1], right_stage1_mux_ic, target_port_label=f"In1[{i}]"):
                    return False
            else:
                if not await self.connect(ground_id, right_stage1_mux_ic, target_port_label=f"In1[{i}]"):
                    return False

        # Select: ShiftAmount[0]
        if not await self.connect(shift_amount_0_id, right_stage1_mux_ic, target_port_label="Sel"):
            return False

        # Right Stage 2: Shift by 2 right using bus_mux_4bit (cascaded on stage 1 output)
        for i in range(4):
            # In0[i]: pass through from stage 1
            if not await self.connect(right_stage1_mux_ic, right_stage2_mux_ic, source_port_label=f"Out[{i}]", target_port_label=f"In0[{i}]"):
                return False

            # In1[i]: shifted by 2 or zero
            if i < 2:
                if not await self.connect(right_stage1_mux_ic, right_stage2_mux_ic, source_port_label=f"Out[{i + 2}]", target_port_label=f"In1[{i}]"):
                    return False
            else:
                if not await self.connect(ground_id, right_stage2_mux_ic, target_port_label=f"In1[{i}]"):
                    return False

        # Select: ShiftAmount[1]
        if not await self.connect(shift_amount_1_id, right_stage2_mux_ic, target_port_label="Sel"):
            return False

        # ============ WIRE DIRECTION SELECTOR ============
        # Select between left and right paths based on ShiftDirection using bus_mux_4bit
        # In0: left path, In1: right path, Sel: ShiftDirection (0=left, 1=right)
        for i in range(4):
            # In0[i]: left path output
            if not await self.connect(left_stage2_mux_ic, direction_mux_ic, source_port_label=f"Out[{i}]", target_port_label=f"In0[{i}]"):
                return False

            # In1[i]: right path output
            if not await self.connect(right_stage2_mux_ic, direction_mux_ic, source_port_label=f"Out[{i}]", target_port_label=f"In1[{i}]"):
                return False

        # Select: ShiftDirection
        if not await self.connect(shift_direction_id, direction_mux_ic, target_port_label="Sel"):
            return False

        # Connect outputs to LEDs
        for i in range(4):
            if not await self.connect(direction_mux_ic, output_leds[i], source_port_label=f"Out[{i}]"):
                return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level5_barrel_shifter_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 4-bit Barrel Shifter IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = BarrelShifter4bitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Barrel Shifter 4-bit IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
