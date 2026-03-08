#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create CPU Level 2: 8-bit Adder IC

Composes 8 instances of 1-bit Full Adder with ripple carry chain.

Inputs: A[0..7], B[0..7], CarryIn (17 inputs)
Outputs: Sum[0..7], CarryOut (9 outputs)

Architecture:
- Bit 0: Full Adder(A[0], B[0], CarryIn) → Sum[0], Carry[0]
- Bit 1: Full Adder(A[1], B[1], Carry[0]) → Sum[1], Carry[1]
- ...
- Bit 7: Full Adder(A[7], B[7], Carry[6]) → Sum[7], CarryOut

Usage:
    python create_cpu_8bit_adder.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class Adder8BitBuilder(ICBuilderBase):
    """Builder for 8-bit Adder IC"""

    async def create(self) -> bool:
        """Create the 8-bit Adder IC"""
        await self.begin_build("8-bit Adder")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Create 8-bit A input switches (vertically stacked)
        a_inputs = []
        input_a_x = 50.0
        for i in range(8):
            element_id = await self.create_element("InputSwitch", input_a_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"A[{i}]")
            if element_id is None:
                return False
            a_inputs.append(element_id)

        # Create 8-bit B input switches (vertically stacked)
        b_inputs = []
        input_b_x = input_a_x + HORIZONTAL_GATE_SPACING
        for i in range(8):
            element_id = await self.create_element("InputSwitch", input_b_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"B[{i}]")
            if element_id is None:
                return False
            b_inputs.append(element_id)

        # Create CarryIn input
        carry_in = await self.create_element("InputSwitch", input_b_x + HORIZONTAL_GATE_SPACING, 100.0, "CarryIn")
        if carry_in is None:
            return False

        await self.log("  ✓ Created 16 data inputs + 1 carry input")

        # Load 1-bit Full Adder IC instances and chain with ripple carry
        full_adders = []
        fa_x = input_b_x + HORIZONTAL_GATE_SPACING
        fa_y = 100.0

        for bit in range(8):
            # Instantiate 1-bit Full Adder IC from file
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_full_adder_1bit")):

                return False

            fa_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_full_adder_1bit"), fa_x + bit * HORIZONTAL_GATE_SPACING, fa_y, f"FA[{bit}]")
            if fa_id is None:
                return False
            full_adders.append(fa_id)
            await self.log(f"  ✓ Instantiated Full Adder IC for bit {bit}")

            # Connect A[bit] to FA A input
            if not await self.connect(a_inputs[bit], fa_id, target_port_label="A"):
                return False

            # Connect B[bit] to FA B input
            if not await self.connect(b_inputs[bit], fa_id, target_port_label="B"):
                return False

            # Connect carry from previous stage (or Cin for bit 0)
            if bit == 0:
                if not await self.connect(carry_in, fa_id, target_port_label="Cin"):
                    return False
            else:
                if not await self.connect(full_adders[bit - 1], fa_id, source_port_label="Cout", target_port_label="Cin"):
                    return False

        await self.log("  ✓ Created and chained 8 Full Adders with ripple carry")

        # Create output LEDs for Sum bits
        output_x = fa_x + (8 * HORIZONTAL_GATE_SPACING)
        for bit in range(8):
            sum_led = await self.create_element("Led", fa_x + bit * HORIZONTAL_GATE_SPACING, fa_y + VERTICAL_STAGE_SPACING, f"Sum[{bit}]")
            if sum_led is None:
                return False

            if not await self.connect(full_adders[bit], sum_led, source_port_label="Sum"):
                return False

        # Create output LED for final CarryOut
        carry_out_led = await self.create_element("Led", fa_x + (7 * HORIZONTAL_GATE_SPACING), fa_y + 2 * VERTICAL_STAGE_SPACING, "CarryOut")
        if carry_out_led is None:
            return False

        if not await self.connect(full_adders[7], carry_out_led, source_port_label="Cout"):
            return False
        await self.log("  ✓ Created 8 Sum outputs + 1 CarryOut")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level6_ripple_adder_8bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 8-bit Adder IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = Adder8BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "8-bit Adder IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
