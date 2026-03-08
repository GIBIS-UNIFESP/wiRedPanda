#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Equality Comparator IC

Inputs: A[0-3], B[0-3] (two 4-bit values)
Outputs: Equal (1 if A==B, 0 otherwise)

Circuit:
- 4 XNOR gates (equality check per bit)
- AND gate tree (all bits equal)

Usage:
    python create_comparator_4bit_equality.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class Comparator4BitEqualityBuilder(ICBuilderBase):
    """Builder for 4-bit Equality Comparator IC"""

    async def create(self) -> bool:
        """Create the 4-bit Equality Comparator IC"""
        await self.begin_build("Comparator 4-bit Equality")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Create input switches for A bits
        a_inputs = []
        input_x = 50.0
        for i in range(4):
            a_id = await self.create_element("InputSwitch", input_x, 100.0 + i * VERTICAL_STAGE_SPACING, f"A[{i}]")
            if a_id is None:
                return False
            a_inputs.append(a_id)
            await self.log(f"  ✓ Created A[{i}] (id={a_id})")

        # Create input switches for B bits
        b_inputs = []
        for i in range(4):
            b_id = await self.create_element("InputSwitch", input_x, 100.0 + (4 + i) * VERTICAL_STAGE_SPACING, f"B[{i}]")
            if b_id is None:
                return False
            b_inputs.append(b_id)
            await self.log(f"  ✓ Created B[{i}] (id={b_id})")

        # Create XNOR gates for bit-wise equality
        xnor_gates = []
        xnor_x = input_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            xnor_id = await self.create_element("Xnor", xnor_x + i * HORIZONTAL_GATE_SPACING, 100.0 + i * VERTICAL_STAGE_SPACING, f"xnor[{i}]")
            if xnor_id is None:
                return False
            xnor_gates.append(xnor_id)

            # Connect A[i] and B[i] to XNOR
            if not await self.connect(a_inputs[i], xnor_id):
                return False

            if not await self.connect(b_inputs[i], xnor_id, target_port=1):
                return False

        await self.log("  ✓ Created 4 XNOR gates")

        # OPTIMIZED: Create single 4-input AND gate instead of cascading 3 AND gates
        and_x = xnor_x + HORIZONTAL_GATE_SPACING * 4
        final_and_x = and_x + HORIZONTAL_GATE_SPACING
        and_gate_final = await self.create_element("And", final_and_x, 100.0 + VERTICAL_STAGE_SPACING, "andGateEqual")
        if and_gate_final is None:
            return False

        # Set AND gate to 4-input size
        set_props = await self.mcp.send_command("change_input_size", {
            "element_id": and_gate_final,
            "size": 4
        })
        if not set_props.success:
            self.log_error("Failed to set input_size=4 for equality AND gate")
            return False

        # Connect all 4 XNOR gates directly to the 4-input AND
        for i in range(4):
            if not await self.connect(xnor_gates[i], and_gate_final, target_port=i):
                return False

        await self.log("  ✓ Created optimized 4-input AND gate for equality")

        # Create output LED
        output_x = final_and_x + HORIZONTAL_GATE_SPACING
        output_led = await self.create_element("Led", output_x, 100.0 + VERTICAL_STAGE_SPACING, "Equal")
        if output_led is None:
            return False

        # Connect final AND to output
        if not await self.connect(and_gate_final, output_led):
            return False

        await self.log("  ✓ Created output LED")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level3_comparator_4bit_equality.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created Comparator4BitEquality IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = Comparator4BitEqualityBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Comparator 4-bit Equality IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
