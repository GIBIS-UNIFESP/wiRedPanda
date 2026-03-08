#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Half Adder IC

Inputs: A, B
Outputs: Sum (A XOR B), Carry (A AND B)

Circuit:
- 1 XOR gate (Sum = A XOR B)
- 1 AND gate (Carry = A AND B)

Usage:
    python create_half_adder.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import (
    HORIZONTAL_GATE_SPACING,
    VERTICAL_STAGE_SPACING,
)


class HalfAdderBuilder(ICBuilderBase):
    """Builder for Half Adder IC"""

    async def create(self) -> bool:
        """Create the Half Adder IC"""
        await self.begin_build("Half Adder")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position calculation with proper spacing
        input_x = 50.0
        gate_x = input_x + HORIZONTAL_GATE_SPACING
        output_x = gate_x + HORIZONTAL_GATE_SPACING

        top_y = 100.0
        bottom_y = top_y + VERTICAL_STAGE_SPACING

        # Create input switches for A and B
        input_a_id = await self.create_element("InputSwitch", input_x, top_y, "A")
        if input_a_id is None:
            return False
        await self.log(f"  ✓ Created input A (id={input_a_id})")

        input_b_id = await self.create_element("InputSwitch", input_x, bottom_y, "B")
        if input_b_id is None:
            return False
        await self.log(f"  ✓ Created input B (id={input_b_id})")

        # Create XOR gate for Sum
        xor_id = await self.create_element("Xor", gate_x, top_y, "xor_sum")
        if xor_id is None:
            return False
        await self.log(f"  ✓ Created XOR gate (id={xor_id})")

        # Create AND gate for Carry
        and_id = await self.create_element("And", gate_x, bottom_y, "and_carry")
        if and_id is None:
            return False
        await self.log(f"  ✓ Created AND gate (id={and_id})")

        # Create output LEDs
        sum_id = await self.create_element("Led", output_x, top_y, "Sum")
        if sum_id is None:
            return False
        await self.log(f"  ✓ Created Sum output (id={sum_id})")

        carry_id = await self.create_element("Led", output_x, bottom_y, "Carry")
        if carry_id is None:
            return False
        await self.log(f"  ✓ Created Carry output (id={carry_id})")

        # Connect A to XOR port 0
        if not await self.connect(input_a_id, xor_id):
            return False

        # Connect B to XOR port 1
        if not await self.connect(input_b_id, xor_id, target_port=1):
            return False

        # Connect XOR output to Sum LED
        if not await self.connect(xor_id, sum_id):
            return False

        # Connect A to AND port 0
        if not await self.connect(input_a_id, and_id):
            return False

        # Connect B to AND port 1
        if not await self.connect(input_b_id, and_id, target_port=1):
            return False

        # Connect AND output to Carry LED
        if not await self.connect(and_id, carry_id):
            return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level2_half_adder.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created HalfAdder IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = HalfAdderBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Half Adder IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
