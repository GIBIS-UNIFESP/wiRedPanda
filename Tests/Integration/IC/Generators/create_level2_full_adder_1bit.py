#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Full Adder IC

Inputs: A, B, Cin
Outputs: Sum, Cout

Circuit:
- 2 XOR gates (Sum generation)
- 2 AND gates (Carry generation)
- 1 OR gate (Carry propagation)

Logic:
- HA1: A XOR B -> s1, A AND B -> c1
- HA2: s1 XOR Cin -> Sum, s1 AND Cin -> c2
- Cout = c1 OR c2

Usage:
    python create_full_adder.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import (
    HORIZONTAL_GATE_SPACING,
    VERTICAL_STAGE_SPACING,
)


class FullAdderBuilder(ICBuilderBase):
    """Builder for Full Adder IC"""

    async def create(self) -> bool:
        """Create the Full Adder IC"""
        await self.begin_build("Full Adder")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position calculation with proper spacing (5 horizontal stages)
        input_x = 50.0
        stage1_x = input_x + HORIZONTAL_GATE_SPACING
        stage2_x = stage1_x + HORIZONTAL_GATE_SPACING
        or_x = stage2_x + HORIZONTAL_GATE_SPACING
        output_x = or_x + HORIZONTAL_GATE_SPACING

        # Vertical positions
        top_y = 100.0
        mid_y = top_y + VERTICAL_STAGE_SPACING
        bottom_y = mid_y + VERTICAL_STAGE_SPACING

        # Create input switches for A, B, and Cin
        input_a_id = await self.create_element("InputSwitch", input_x, top_y, "A")
        if input_a_id is None:
            return False
        await self.log(f"  ✓ Created input A (id={input_a_id})")

        input_b_id = await self.create_element("InputSwitch", input_x, mid_y, "B")
        if input_b_id is None:
            return False
        await self.log(f"  ✓ Created input B (id={input_b_id})")

        input_cin_id = await self.create_element("InputSwitch", input_x, bottom_y, "Cin")
        if input_cin_id is None:
            return False
        await self.log(f"  ✓ Created input Cin (id={input_cin_id})")

        # Create XOR1 gate (A XOR B -> s1)
        xor1_id = await self.create_element("Xor", stage1_x, top_y, "xor1")
        if xor1_id is None:
            return False
        await self.log(f"  ✓ Created XOR1 gate (id={xor1_id})")

        # Create AND1 gate (A AND B -> c1)
        and1_id = await self.create_element("And", stage1_x, mid_y, "and1")
        if and1_id is None:
            return False
        await self.log(f"  ✓ Created AND1 gate (id={and1_id})")

        # Create XOR2 gate (s1 XOR Cin -> Sum)
        xor2_id = await self.create_element("Xor", stage2_x, top_y, "xor2")
        if xor2_id is None:
            return False
        await self.log(f"  ✓ Created XOR2 gate (id={xor2_id})")

        # Create AND2 gate (s1 AND Cin -> c2)
        and2_id = await self.create_element("And", stage2_x, mid_y, "and2")
        if and2_id is None:
            return False
        await self.log(f"  ✓ Created AND2 gate (id={and2_id})")

        # Create OR gate (c1 OR c2 -> Cout)
        or_id = await self.create_element("Or", or_x, mid_y, "or_cout")
        if or_id is None:
            return False
        await self.log(f"  ✓ Created OR gate (id={or_id})")

        # Create output LEDs
        sum_id = await self.create_element("Led", output_x, top_y, "Sum")
        if sum_id is None:
            return False
        await self.log(f"  ✓ Created Sum output (id={sum_id})")

        cout_id = await self.create_element("Led", output_x, mid_y, "Cout")
        if cout_id is None:
            return False
        await self.log(f"  ✓ Created Cout output (id={cout_id})")

        # ========== Connect HA1: A XOR B -> s1, A AND B -> c1 ==========

        # Connect A to XOR1 port 0
        if not await self.connect(input_a_id, xor1_id):
            return False

        # Connect B to XOR1 port 1
        if not await self.connect(input_b_id, xor1_id, target_port=1):
            return False

        # Connect A to AND1 port 0
        if not await self.connect(input_a_id, and1_id):
            return False

        # Connect B to AND1 port 1
        if not await self.connect(input_b_id, and1_id, target_port=1):
            return False

        # ========== Connect HA2: s1 XOR Cin -> Sum, s1 AND Cin -> c2 ==========

        # Connect XOR1 output to XOR2 port 0
        if not await self.connect(xor1_id, xor2_id):
            return False

        # Connect Cin to XOR2 port 1
        if not await self.connect(input_cin_id, xor2_id, target_port=1):
            return False

        # Connect XOR1 output to AND2 port 0
        if not await self.connect(xor1_id, and2_id):
            return False

        # Connect Cin to AND2 port 1
        if not await self.connect(input_cin_id, and2_id, target_port=1):
            return False

        # ========== Connect Carry propagation: c1 OR c2 -> Cout ==========

        # Connect AND1 output to OR port 0
        if not await self.connect(and1_id, or_id):
            return False

        # Connect AND2 output to OR port 1
        if not await self.connect(and2_id, or_id, target_port=1):
            return False

        # ========== Connect outputs ==========

        # Connect XOR2 output to Sum LED
        if not await self.connect(xor2_id, sum_id):
            return False

        # Connect OR output to Cout LED
        if not await self.connect(or_id, cout_id):
            return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level2_full_adder_1bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created FullAdder 1-bit IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = FullAdderBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Full Adder IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
