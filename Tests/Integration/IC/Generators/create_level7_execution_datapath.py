#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Execution Datapath IC

Implements a simplified CPU execution datapath combining operand inputs,
ALU operations, and status flag generation.

Inputs:
  OperandA[0..7] (8-bit first operand)
  OperandB[0..7] (8-bit second operand)
  OpCode[0..2] (3-bit operation select for ALU)

Outputs:
  Result[0..7] (8-bit operation result)
  Zero (Status: result is zero)
  Sign (Status: result is negative/sign bit set)

Architecture:
  - Instantiate level6_alu_8bit for arithmetic/logic operations
  - Generate status flags from result:
    - Zero: NOR of all result bits
    - Sign: Result MSB (bit 7)

Purpose:
  Core execution unit of the CPU that performs operations on operands
  and generates results and status flags for control flow decisions.

Usage:
    python create_level7_execution_datapath.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder


class ExecutionDatapathBuilder(ICBuilderBase):
    """Builder for Execution Datapath IC"""

    async def create(self) -> bool:
        """Create the Execution Datapath IC"""
        await self.begin_build("Execution Datapath")
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0
        alu_x = input_x + (6 * HORIZONTAL_GATE_SPACING)
        alu_y = 600.0

        # Instantiate the 8-bit ALU first so the operand rows above it can be
        # placed with guaranteed clearance from its real (port-count-dependent)
        # top edge -- level6_alu_8bit's height scales with its 20+ ports and is
        # far taller than the flat 64px assumed elsewhere (queried via
        # instantiate_ic_with_size rather than a flat spacing constant).
        alu_handle = await self.instantiate_ic_with_size("level6_alu_8bit", alu_x, alu_y, "ALU_8bit")
        if alu_handle is None:
            return False
        alu_id = alu_handle.element_id
        await self.log("  ✓ Instantiated 8-bit ALU")

        operand_b_y = alu_y - (alu_handle.height / 2) - VERTICAL_STAGE_SPACING
        operand_a_y = operand_b_y - VERTICAL_STAGE_SPACING

        # Create OperandA inputs (8-bit)
        operand_a_inputs = []
        for i in range(8):
            op_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), operand_a_y, f"OperandA[{i}]"
            )
            if op_id is None:
                return False
            operand_a_inputs.append(op_id)
        await self.log("  ✓ Created 8 OperandA inputs")

        # Create OperandB inputs (8-bit)
        operand_b_inputs = []
        for i in range(8):
            op_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), operand_b_y, f"OperandB[{i}]"
            )
            if op_id is None:
                return False
            operand_b_inputs.append(op_id)
        await self.log("  ✓ Created 8 OperandB inputs")

        # Create OpCode inputs (3-bit)
        opcode_inputs = []
        for i in range(3):
            op_id = await self.create_element(
                "InputSwitch",
                input_x + (8 * HORIZONTAL_GATE_SPACING) + (i * HORIZONTAL_GATE_SPACING),
                operand_a_y,
                f"OpCode[{i}]",
            )
            if op_id is None:
                return False
            opcode_inputs.append(op_id)
        await self.log("  ✓ Created 3 OpCode inputs")

        # Connect operands to ALU
        for i in range(8):
            if not await self.connect(operand_a_inputs[i], alu_id, target_port_label=f"A[{i}]"):
                return False

            if not await self.connect(operand_b_inputs[i], alu_id, target_port_label=f"B[{i}]"):
                return False

        # Connect OpCode to ALU
        for i in range(3):
            if not await self.connect(opcode_inputs[i], alu_id, target_port_label=f"OpCode[{i}]"):
                return False
        await self.log("  ✓ Connected operands and opcode to ALU")

        # Create output LEDs for Result (8-bit)
        output_x = input_x + (14 * HORIZONTAL_GATE_SPACING)
        result_leds = []
        for i in range(8):
            led_id = await self.create_element("Led", output_x, alu_y + (i * VERTICAL_STAGE_SPACING), f"Result[{i}]")
            if led_id is None:
                return False
            if not led_id:
                self.log_error(f"Failed to get ID for Result[{i}]")
                return False
            result_leds.append(led_id)

            # Connect ALU result to LED
            if not await self.connect(alu_id, led_id, source_port_label=f"Result[{i}]"):
                return False
        await self.log("  ✓ Created 8 Result output LEDs")

        # Zero/Sign are already computed by the embedded ALU (level6_alu_8bit
        # exposes both directly) -- forward them rather than re-deriving Zero
        # from Result bits with a redundant NOR/AND tree, matching how
        # level7_alu_16bit and level8_execute_stage both wire their own Zero
        # flag straight from their ALU.
        zero_sign_x = output_x + HORIZONTAL_GATE_SPACING
        zero_led_id = await self.create_element("Led", zero_sign_x, alu_y, "Zero")
        if zero_led_id is None:
            return False
        if not zero_led_id:
            self.log_error("Failed to get ID for Zero flag LED")
            return False

        # Connect Zero flag to LED
        if not await self.connect(alu_id, zero_led_id, source_port_label="Zero"):
            return False

        # Create Sign flag output LED (MSB of result)
        sign_led_id = await self.create_element("Led", zero_sign_x, alu_y + VERTICAL_STAGE_SPACING, "Sign")
        if sign_led_id is None:
            return False
        if not sign_led_id:
            self.log_error("Failed to get ID for Sign flag LED")
            return False

        # Connect Sign flag to LED (result bit 7, the MSB)
        if not await self.connect(alu_id, sign_led_id, source_port_label="Result[7]"):
            return False

        await self.log("  ✓ Created status flag generation logic")

        output_file = str(IC_COMPONENTS_DIR / "level7_execution_datapath.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created Execution Datapath IC"
            f"({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = ExecutionDatapathBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "Execution Datapath IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
