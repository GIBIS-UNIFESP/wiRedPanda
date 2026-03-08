#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Full Adder IC (Level 4)

Implements a 4-bit ripple-carry adder using IC composition pattern.
Uses 4× level2_full_adder ICs connected in a carry chain.

Inputs: A[0-3] (4-bit operand A), B[0-3] (4-bit operand B), CarryIn
Outputs: Sum[0-3] (4-bit sum), CarryOut (final carry out)

Architecture:
- 4× Full Adder ICs (level2_full_adder) in ripple-carry configuration
- Carry propagation: FA[i].CarryOut → FA[i+1].CarryIn
- First stage CarryIn from external input
- Final CarryOut from FA[3]

This refactoring demonstrates IC composition hierarchy:
- level1: Basic flip-flops and latches
- level2: Single-bit building blocks (full_adder, half_adder)
- level4: Multi-bit composed ICs (4-bit adder using 4× FA ICs)

Usage:
    python create_level4_adder_4bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class FullAdder4bitBuilder(ICBuilderBase):
    """Builder for 4-bit Full Adder IC using IC composition"""

    async def create(self) -> bool:
        """
        Create 4-bit Full Adder IC using ripple-carry composition.

        Returns True on success, False on failure.
        """
        await self.begin_build("4-bit Full Adder")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position calculation
        input_x = 50.0
        a_input_y = 100.0
        b_input_y = a_input_y + VERTICAL_STAGE_SPACING
        carry_in_y = b_input_y + VERTICAL_STAGE_SPACING

        # Create A[0-3] inputs
        a_inputs = []
        for i in range(4):
            a_id = await self.create_element("InputSwitch", input_x, a_input_y + i * VERTICAL_STAGE_SPACING, f"A[{i}]")
            if a_id is None:
                return False
            a_inputs.append(a_id)
            await self.log(f"  ✓ Created A[{i}]")

        # Create B[0-3] inputs
        b_inputs = []
        for i in range(4):
            b_id = await self.create_element("InputSwitch", input_x, b_input_y + i * VERTICAL_STAGE_SPACING, f"B[{i}]")
            if b_id is None:
                return False
            b_inputs.append(b_id)
            await self.log(f"  ✓ Created B[{i}]")

        # Create CarryIn input
        carry_in_id = await self.create_element("InputSwitch", input_x, carry_in_y, "CarryIn")
        if carry_in_id is None:
            return False
        await self.log("  ✓ Created CarryIn")

        # Instantiate 4 Full Adder ICs in ripple-carry chain
        full_adders = []
        fa_x = input_x + HORIZONTAL_GATE_SPACING * 2
        fa_y = 100.0

        for bit in range(4):
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_full_adder_1bit")):

                return False

            fa_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_full_adder_1bit"), fa_x + (bit * HORIZONTAL_GATE_SPACING * 1.5), fa_y + (bit * VERTICAL_STAGE_SPACING * 0.3), f"FA[{bit}]")
            if fa_id is None:
                return False
            full_adders.append(fa_id)
            await self.log(f"  ✓ Instantiated FA[{bit}] (id={fa_id})")

        # Create Sum[0-3] output LEDs
        sum_outputs = []
        sum_output_x = fa_x + (4 * HORIZONTAL_GATE_SPACING * 1.5)
        for i in range(4):
            sum_id = await self.create_element("Led", sum_output_x, a_input_y + i * VERTICAL_STAGE_SPACING, f"Sum[{i}]")
            if sum_id is None:
                return False
            sum_outputs.append(sum_id)
            await self.log(f"  ✓ Created Sum[{i}] output")

        # Create CarryOut output LED
        carry_out_id = await self.create_element("Led", sum_output_x, carry_in_y, "CarryOut")
        if carry_out_id is None:
            return False
        await self.log("  ✓ Created CarryOut output")

        # Connect FA[i].A ← A[i]
        for bit in range(4):
            if not await self.connect(a_inputs[bit], full_adders[bit], target_port_label="A"):
                return False
            await self.log(f"  ✓ Connected A[{bit}] → FA[{bit}].A")

        # Connect FA[i].B ← B[i]
        for bit in range(4):
            if not await self.connect(b_inputs[bit], full_adders[bit], target_port_label="B"):
                return False
            await self.log(f"  ✓ Connected B[{bit}] → FA[{bit}].B")

        # Connect carry chain: FA[i].Cout → FA[i+1].Cin
        for bit in range(4):
            if bit == 0:
                if not await self.connect(carry_in_id, full_adders[bit], target_port_label="Cin"):
                    return False
            else:
                if not await self.connect(full_adders[bit - 1], full_adders[bit], source_port_label="Cout", target_port_label="Cin"):
                    return False
            await self.log(f"  ✓ Connected carry chain FA[{bit}]")

        # Connect Sum outputs: FA[i].Sum → Sum[i]
        for bit in range(4):
            if not await self.connect(full_adders[bit], sum_outputs[bit], source_port_label="Sum"):
                return False
            await self.log(f"  ✓ Connected Sum[{bit}] output")

        # Connect final Cout from FA[3]
        if not await self.connect(full_adders[3], carry_out_id, source_port_label="Cout"):
            return False
        await self.log("  ✓ Connected CarryOut output")

        # Save circuit to file
        output_file = IC_COMPONENTS_DIR / "level4_ripple_adder_4bit.panda"
        if not await self.save_circuit(str(output_file)):
            return False

        await self.log("✅ Successfully created 4-bit Full Adder IC!")
        await self.log(f"   Total elements: {self.element_count}")
        await self.log(f"   Total connections: {self.connection_count}")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for IC builder"""
    builder = FullAdder4bitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Full Adder IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)

