#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit ALU IC (Composite) - All Operations Exposed

Computes all 4 operations (ADD, SUB, AND, OR) simultaneously.
Exposes all operation results without built-in selection.

Uses FullAdder4bit for ADD and FullSubtractor4bit for SUB.

Operations computed:
  ADD: A + B
  SUB: A - B (using 2's complement)
  AND: A & B
  OR:  A | B

Inputs: A[0-3], B[0-3], CarryIn (9 inputs)
Outputs: Result_ADD[0-3], Result_SUB[0-3], Result_AND[0-3], Result_OR[0-3], CarryOut (17 outputs)

Architecture:
    - 2 FullAdder4bit ICs for ADD and SUB operations
      (SUB uses same adder with NOT gates to invert B inputs)
    - 4 Mux2to1 ICs for AND operation (using Mux pattern)
    - 4 Mux2to1 ICs for OR operation (using Mux pattern)
    - Ground and Vcc for carry/borrow control
    - Direct outputs for all 4 operation results

Usage:
    python create_alu_4bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class ALU4bitBuilder(ICBuilderBase):
    """Builder for 4-bit ALU IC"""

    async def create(self) -> bool:
        """Create the 4-bit ALU IC"""
        await self.begin_build("ALU 4-bit")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position hierarchy for logical stages
        input_x_start = 50.0
        a_input_y = 100.0
        b_input_y = a_input_y + VERTICAL_STAGE_SPACING

        adder_x = input_x_start + HORIZONTAL_GATE_SPACING
        adder_y = 200.0
        subtractor_y = adder_y + VERTICAL_STAGE_SPACING
        and_or_y = subtractor_y + VERTICAL_STAGE_SPACING
        mux_y = adder_y + VERTICAL_STAGE_SPACING
        output_x = input_x_start + HORIZONTAL_GATE_SPACING * 5

        # Dense array spacing for 4-bit parallel elements
        dense_spacing = 40.0

        # Create input switches for A operand
        a_inputs = []
        for i in range(4):
            a_id = await self.create_element("InputSwitch", input_x_start + i * dense_spacing, a_input_y, f"A[{i}]")
            if a_id is None:
                return False
            a_inputs.append(a_id)

        # Create input switches for B operand
        b_inputs = []
        for i in range(4):
            b_id = await self.create_element("InputSwitch", input_x_start + i * dense_spacing, b_input_y, f"B[{i}]")
            if b_id is None:
                return False
            b_inputs.append(b_id)

        # Create CarryIn input port for adder chaining
        carryin_id = await self.create_element("InputSwitch", input_x_start, b_input_y + VERTICAL_STAGE_SPACING, "CarryIn")
        if carryin_id is None:
            return False

        # Create ground (always 0) for fallback/reference
        ground_id = await self.create_element("InputGnd", input_x_start + HORIZONTAL_GATE_SPACING, b_input_y + VERTICAL_STAGE_SPACING, "Ground")
        if ground_id is None:
            return False

        # Create Vcc (always 1) for subtractor Borrow input
        vcc_id = await self.create_element("InputVcc", input_x_start, b_input_y + VERTICAL_STAGE_SPACING * 1.5, "Vcc")
        if vcc_id is None:
            return False

        # Load FullAdder4bit IC for ADD operation
        adder_ic_name = str(IC_COMPONENTS_DIR / "level4_ripple_adder_4bit")
        if not self.check_dependency(adder_ic_name):

            return False

        adder_id = await self.instantiate_ic(adder_ic_name, adder_x, adder_y, "Adder")
        if adder_id is None:
            return False

        # Load FullAdder4bit IC for SUB operation (with inverted B inputs)
        # Subtraction = A - B = A + (~B + 1) = A + ~B + 1, so we invert B and add
        subtractor_ic_name = str(IC_COMPONENTS_DIR / "level4_ripple_adder_4bit")
        if not self.check_dependency(subtractor_ic_name):

            return False

        subtractor_id = await self.instantiate_ic(subtractor_ic_name, adder_x, subtractor_y, "Subtractor (Adder with ~B)")
        if subtractor_id is None:
            return False

        # Create NOT gates for inverting B inputs in subtraction
        not_gates = []
        for i in range(4):
            not_id = await self.create_element("Not", adder_x - HORIZONTAL_GATE_SPACING + i * dense_spacing, subtractor_y - VERTICAL_STAGE_SPACING / 2, f"NOT_B[{i}]")
            if not_id is None:
                return False
            not_gates.append(not_id)

        # Instantiate AND path Mux2to1 ICs (4 times for 4 bits)
        # Mux(GND, B[i], A[i]) = A[i] AND B[i]
        and_gates = []
        for i in range(4):
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_mux_2to1")):

                return False

            mux_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_mux_2to1"), adder_x + i * dense_spacing, and_or_y, f"mux_and[{i}]")
            if mux_id is None:
                return False
            and_gates.append(mux_id)

        # Instantiate OR path Mux2to1 ICs (4 times for 4 bits)
        # Mux(A[i], VCC, B[i]) = A[i] OR B[i]
        or_gates = []
        for i in range(4):
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_mux_2to1")):

                return False

            mux_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_mux_2to1"), adder_x + i * dense_spacing, and_or_y + VERTICAL_STAGE_SPACING, f"mux_or[{i}]")
            if mux_id is None:
                return False
            or_gates.append(mux_id)

        # Create output LEDs for all 4 operations (16 total)
        result_add_outputs = []
        result_sub_outputs = []
        result_and_outputs = []
        result_or_outputs = []

        output_y_add = mux_y
        output_y_sub = mux_y + VERTICAL_STAGE_SPACING
        output_y_and = mux_y + 2 * VERTICAL_STAGE_SPACING
        output_y_or = mux_y + 3 * VERTICAL_STAGE_SPACING

        for i in range(4):
            # Result_ADD[0-3]
            led_id = await self.create_element("Led", output_x + i * dense_spacing, output_y_add, f"Result_ADD[{i}]")
            if led_id is None:
                return False
            result_add_outputs.append(led_id)

            # Result_SUB[0-3]
            led_id = await self.create_element("Led", output_x + i * dense_spacing, output_y_sub, f"Result_SUB[{i}]")
            if led_id is None:
                return False
            result_sub_outputs.append(led_id)

            # Result_AND[0-3]
            led_id = await self.create_element("Led", output_x + i * dense_spacing, output_y_and, f"Result_AND[{i}]")
            if led_id is None:
                return False
            result_and_outputs.append(led_id)

            # Result_OR[0-3]
            led_id = await self.create_element("Led", output_x + i * dense_spacing, output_y_or, f"Result_OR[{i}]")
            if led_id is None:
                return False
            result_or_outputs.append(led_id)

        # Create CarryOut LED for adder overflow
        output_y_carry = mux_y + 4 * VERTICAL_STAGE_SPACING
        carryout_led = await self.create_element("Led", output_x, output_y_carry, "CarryOut")
        if carryout_led is None:
            return False

        # ============ CONNECT ADDER (ADD operation) ============
        # Connect A and B to FullAdder using semantic labels
        for i in range(4):
            if not await self.connect(a_inputs[i], adder_id, target_port_label=f"A[{i}]"):
                return False

            if not await self.connect(b_inputs[i], adder_id, target_port_label=f"B[{i}]"):
                return False

        # Connect CarryIn input to adder
        if not await self.connect(carryin_id, adder_id, target_port_label="CarryIn"):
            return False

        # ============ CONNECT SUBTRACTOR (SUB operation) ============
        # Connect A to Subtractor (Adder) using semantic labels
        for i in range(4):
            if not await self.connect(a_inputs[i], subtractor_id, target_port_label=f"A[{i}]"):
                return False

        # Connect B inputs through NOT gates to Subtractor
        for i in range(4):
            # B[i] -> NOT -> Subtractor B[i]
            if not await self.connect(b_inputs[i], not_gates[i]):
                return False

            if not await self.connect(not_gates[i], subtractor_id, target_port_label=f"B[{i}]"):
                return False

        # Connect CarryIn to Vcc (1) for SUB: A + (~B) + 1
        if not await self.connect(vcc_id, subtractor_id, target_port_label="CarryIn"):
            return False

        # ============ CONNECT AND MUXES (per bit) ============
        # Implement AND using Mux: Out = (A[i]==0) ? 0 : B[i] = A[i] AND B[i]
        # Mux(In0=0, In1=B[i], Sel=A[i])
        for i in range(4):
            # Connect ground (0) to Data[0] (selected when A[i]=0)
            if not await self.connect(ground_id, and_gates[i], target_port_label="Data[0]"):
                return False

            # Connect B[i] to Data[1] (selected when A[i]=1)
            if not await self.connect(b_inputs[i], and_gates[i], target_port_label="Data[1]"):
                return False

            # Connect A[i] to Sel[0] (multiplexer control)
            if not await self.connect(a_inputs[i], and_gates[i], target_port_label="Sel[0]"):
                return False

            # Connect AND result to output LED
            if not await self.connect(and_gates[i], result_and_outputs[i], source_port_label="Output"):
                return False

        # ============ CONNECT OR MUXES (per bit) ============
        # Implement OR using Mux: Out = (B[i]==0) ? A[i] : 1 = A[i] OR B[i]
        # Mux(In0=A[i], In1=1, Sel=B[i])
        for i in range(4):
            # Connect A[i] to Data[0] (selected when B[i]=0)
            if not await self.connect(a_inputs[i], or_gates[i], target_port_label="Data[0]"):
                return False

            # Connect Vcc (1) to Data[1] (selected when B[i]=1)
            if not await self.connect(vcc_id, or_gates[i], target_port_label="Data[1]"):
                return False

            # Connect B[i] to Sel[0] (multiplexer control)
            if not await self.connect(b_inputs[i], or_gates[i], target_port_label="Sel[0]"):
                return False

            # Connect OR result to output LED
            if not await self.connect(or_gates[i], result_or_outputs[i], source_port_label="Output"):
                return False

        # ============ CONNECT ADDER OUTPUTS ============
        for i in range(4):
            if not await self.connect(adder_id, result_add_outputs[i], source_port_label=f"Sum[{i}]"):
                return False

        # Connect adder CarryOut to output LED
        if not await self.connect(adder_id, carryout_led, source_port_label="CarryOut"):
            return False

        # ============ CONNECT SUBTRACTOR OUTPUTS ============
        for i in range(4):
            if not await self.connect(subtractor_id, result_sub_outputs[i], source_port_label=f"Sum[{i}]"):
                return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level4_ripple_alu_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 4-bit ALU IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = ALU4bitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "ALU 4-bit IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
