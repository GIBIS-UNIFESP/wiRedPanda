#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Comparator IC

Compares two 4-bit values A and B using cascading magnitude comparison.

Inputs: A[0-3], B[0-3] (two 4-bit values)
Outputs: Greater, Equal, Less (exactly one asserted)

Architecture:
  - 4 XNOR gates for bit-wise equality
  - 8 AND gates for greater/less conditions per bit
  - 4 cascade AND gates for equality propagation
  - OR gates to combine comparison results

Usage:
    python create_level3_comparator_4bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class Comparator4BitBuilder(ICBuilderBase):
    """Builder for 4-bit Comparator IC"""

    async def create(self) -> bool:
        """Create the 4-bit Comparator IC"""
        await self.begin_build("Comparator 4-bit")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position hierarchy
        input_x = 50.0
        not_x = input_x + HORIZONTAL_GATE_SPACING
        and_x = not_x + HORIZONTAL_GATE_SPACING
        cascade_x = and_x + HORIZONTAL_GATE_SPACING
        or_x = cascade_x + HORIZONTAL_GATE_SPACING
        output_x = or_x + HORIZONTAL_GATE_SPACING

        # ========== Create Input Switches ==========
        a_inputs = []
        for i in range(4):
            a_id = await self.create_element("InputSwitch", input_x, 100.0 + i * VERTICAL_STAGE_SPACING, f"A[{i}]")
            if a_id is None:
                return False
            a_inputs.append(a_id)

        b_inputs = []
        for i in range(4):
            b_id = await self.create_element("InputSwitch", input_x, 100.0 + (4 + i) * VERTICAL_STAGE_SPACING, f"B[{i}]")
            if b_id is None:
                return False
            b_inputs.append(b_id)

        await self.log(f"  ✓ Created inputs (A[0-3], B[0-3])")

        # ========== Create NOT Gates ==========
        not_a = []
        not_b = []
        for i in range(4):
            # NOT A[i]
            not_a_id = await self.create_element("Not", not_x, 100.0 + i * VERTICAL_STAGE_SPACING, f"notA[{i}]")
            if not_a_id is None:
                return False
            not_a.append(not_a_id)

            if not await self.connect(a_inputs[i], not_a_id):
                return False

            # NOT B[i]
            not_b_id = await self.create_element("Not", not_x, 100.0 + (4 + i) * VERTICAL_STAGE_SPACING, f"notB[{i}]")
            if not_b_id is None:
                return False
            not_b.append(not_b_id)

            if not await self.connect(b_inputs[i], not_b_id):
                return False

        await self.log(f"  ✓ Created NOT gates")

        # ========== Create Comparison Gates (XNOR, AND Greater, AND Less) ==========
        xnor_gates = []
        and_greater = []
        and_less = []

        for i in range(4):
            # XNOR: A[i] XNOR B[i]
            xnor_id = await self.create_element("Xnor", and_x + i * HORIZONTAL_GATE_SPACING, 100.0 + i * VERTICAL_STAGE_SPACING, f"xnor[{i}]")
            if xnor_id is None:
                return False
            xnor_gates.append(xnor_id)

            # Connect A[i] and B[i] to XNOR
            if not await self.connect(a_inputs[i], xnor_id):
                return False

            if not await self.connect(b_inputs[i], xnor_id, target_port=1):
                return False

            # AND: A[i] AND NOT B[i] (A greater)
            and_g_id = await self.create_element("And", and_x + i * HORIZONTAL_GATE_SPACING, 100.0 + (4 + i) * VERTICAL_STAGE_SPACING, f"andGreater[{i}]")
            if and_g_id is None:
                return False
            and_greater.append(and_g_id)

            if not await self.connect(a_inputs[i], and_g_id):
                return False

            if not await self.connect(not_b[i], and_g_id, target_port=1):
                return False

            # AND: NOT A[i] AND B[i] (A less)
            and_l_id = await self.create_element("And", and_x + i * HORIZONTAL_GATE_SPACING, 100.0 + (8 + i) * VERTICAL_STAGE_SPACING, f"andLess[{i}]")
            if and_l_id is None:
                return False
            and_less.append(and_l_id)

            if not await self.connect(not_a[i], and_l_id):
                return False

            if not await self.connect(b_inputs[i], and_l_id, target_port=1):
                return False

        await self.log(f"  ✓ Created comparison gates (XNOR, AND Greater, AND Less)")

        # ========== Create Cascade AND Gates for Equality ==========
        # OPTIMIZED: Single 4-input AND gate instead of cascading
        and_equal = await self.create_element("And", cascade_x + HORIZONTAL_GATE_SPACING, 100.0 + VERTICAL_STAGE_SPACING, "andEqual")
        if and_equal is None:
            return False

        # Set AND gate to 4-input size
        set_props = await self.mcp.send_command("change_input_size", {
            "element_id": and_equal,
            "size": 4
        })
        if not set_props.success:
            self.log_error("Failed to set input_size=4 for equality AND gate")
            return False

        # Connect all XNOR gates to equality AND gate
        for i in range(4):
            if not await self.connect(xnor_gates[i], and_equal, target_port=i):
                return False

        await self.log(f"  ✓ Created equality cascade (4-input AND)")

        # ========== Create Cascade Logic: OR gates combining comparison with equality cascade ==========
        # For proper magnitude comparison:
        # cascade[0] = andGreater[0]
        # cascade[i] = andGreater[i] OR (XNOR[i] AND cascade[i-1]) for i > 0

        cascade_greater = []
        cascade_less = []

        # Level 0: No cascade from below, just use andGreater[0] and andLess[0]
        cascade_greater.append(and_greater[0])
        cascade_less.append(and_less[0])

        # Levels 1-3: Create cascade with OR gates and intermediate AND gates
        for i in range(1, 4):
            # For each level, create an AND gate: XNOR[i] AND cascade[i-1]
            and_casc_g_id = await self.create_element("And", cascade_x + (i % 2) * HORIZONTAL_GATE_SPACING, 100.0 + (4 + (i // 2)) * VERTICAL_STAGE_SPACING, f"andCascadeGreater[{i}]")
            if and_casc_g_id is None:
                return False

            # Connect XNOR[i] to input 0
            if not await self.connect(xnor_gates[i], and_casc_g_id):
                return False

            # Connect cascade[i-1] to input 1
            if not await self.connect(cascade_greater[i - 1], and_casc_g_id, target_port=1):
                return False

            # Create OR gate: andGreater[i] OR andCascadeGreater[i]
            or_casc_g_id = await self.create_element("Or", or_x + (i % 2) * HORIZONTAL_GATE_SPACING, 100.0 + (4 + (i // 2)) * VERTICAL_STAGE_SPACING, f"orCascadeGreater[{i}]")
            if or_casc_g_id is None:
                return False
            cascade_greater.append(or_casc_g_id)

            # Connect andGreater[i] to input 0
            if not await self.connect(and_greater[i], or_casc_g_id):
                return False

            # Connect andCascadeGreater[i] to input 1
            if not await self.connect(and_casc_g_id, or_casc_g_id, target_port=1):
                return False

            # Same for Less
            and_casc_l_id = await self.create_element("And", cascade_x + (i % 2) * HORIZONTAL_GATE_SPACING, 100.0 + (6 + (i // 2)) * VERTICAL_STAGE_SPACING, f"andCascadeLess[{i}]")
            if and_casc_l_id is None:
                return False

            # Connect XNOR[i] to input 0
            if not await self.connect(xnor_gates[i], and_casc_l_id):
                return False

            # Connect cascade[i-1] to input 1
            if not await self.connect(cascade_less[i - 1], and_casc_l_id, target_port=1):
                return False

            # Create OR gate: andLess[i] OR andCascadeLess[i]
            or_casc_l_id = await self.create_element("Or", or_x + (i % 2) * HORIZONTAL_GATE_SPACING, 100.0 + (6 + (i // 2)) * VERTICAL_STAGE_SPACING, f"orCascadeLess[{i}]")
            if or_casc_l_id is None:
                return False
            cascade_less.append(or_casc_l_id)

            # Connect andLess[i] to input 0
            if not await self.connect(and_less[i], or_casc_l_id):
                return False

            # Connect andCascadeLess[i] to input 1
            if not await self.connect(and_casc_l_id, or_casc_l_id, target_port=1):
                return False

        await self.log(f"  ✓ Created cascade OR gates with proper comparison logic")

        # ========== Create Output LEDs ==========
        greater_led = await self.create_element("Led", output_x, 100.0 + 3 * VERTICAL_STAGE_SPACING, "Greater")
        if greater_led is None:
            return False

        if not await self.connect(cascade_greater[3], greater_led):
            return False

        equal_led = await self.create_element("Led", output_x, 100.0 + 5 * VERTICAL_STAGE_SPACING, "Equal")
        if equal_led is None:
            return False

        if not await self.connect(and_equal, equal_led):
            return False

        less_led = await self.create_element("Led", output_x, 100.0 + 7 * VERTICAL_STAGE_SPACING, "Less")
        if less_led is None:
            return False

        if not await self.connect(cascade_less[3], less_led):
            return False

        await self.log("  ✓ Created output LEDs")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level3_comparator_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created Comparator4Bit IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = Comparator4BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Comparator 4-bit IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
