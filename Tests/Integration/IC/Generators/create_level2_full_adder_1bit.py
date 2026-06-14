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
    python3 create_level2_full_adder_1bit.py
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

        # Instantiate two half-adders (textbook full-adder decomposition):
        # HA1(A,B) -> (Sum=s1, Carry=c1); HA2(s1,Cin) -> (Sum, Carry=c2).
        ha1_id = await self.instantiate_ic("level2_half_adder", stage1_x, top_y, "HA1")
        if ha1_id is None:
            return False
        await self.log(f"  ✓ Instantiated half-adder HA1 (id={ha1_id})")

        ha2_id = await self.instantiate_ic("level2_half_adder", stage2_x, top_y, "HA2")
        if ha2_id is None:
            return False
        await self.log(f"  ✓ Instantiated half-adder HA2 (id={ha2_id})")

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

        # ========== Connect HA1: A, B -> (Sum=s1, Carry=c1) ==========

        if not await self.connect(input_a_id, ha1_id, target_port_label="A"):
            return False

        if not await self.connect(input_b_id, ha1_id, target_port_label="B"):
            return False

        # ========== Connect HA2: s1, Cin -> (Sum, Carry=c2) ==========

        # HA1 Sum (s1) drives HA2 A
        if not await self.connect(ha1_id, ha2_id, source_port_label="Sum", target_port_label="A"):
            return False

        # Cin drives HA2 B
        if not await self.connect(input_cin_id, ha2_id, target_port_label="B"):
            return False

        # ========== Carry propagation: c1 OR c2 -> Cout ==========

        # HA1 Carry (c1) to OR port 0
        if not await self.connect(ha1_id, or_id, source_port_label="Carry", target_port=0):
            return False

        # HA2 Carry (c2) to OR port 1
        if not await self.connect(ha2_id, or_id, source_port_label="Carry", target_port=1):
            return False

        # ========== Connect outputs ==========

        # Connect HA2 Sum output to Sum LED
        if not await self.connect(ha2_id, sum_id, source_port_label="Sum"):
            return False

        # Connect OR output to Cout LED
        if not await self.connect(or_id, cout_id):
            return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level2_full_adder_1bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created FullAdder 1-bit IC ({self.element_count} elements, {self.connection_count} connections)"
        )
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
