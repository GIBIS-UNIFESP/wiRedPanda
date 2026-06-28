#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 16-bit ALU IC

Implements a 16-bit ALU by composing two 8-bit ALUs with proper carry propagation:

Inputs:
  OperandA[0..15] (16-bit operand A)
  OperandB[0..15] (16-bit operand B)
  ALUOp[0..2] (3-bit operation selector - same for both halves)

Outputs:
  Result[0..15] (16-bit result)
  Zero (zero flag - true if entire result is 0)
  Sign (sign flag - MSB of high byte result, same as Result[15])
  Carry (carry-out from high byte ALU)

Architecture:
  - Low byte ALU: Handles bits [7:0], provides carry-out to high byte
  - High byte ALU: Handles bits [15:8], receives carry-in from low byte
  - Both ALUs perform same operation (ALUOp is common)
  - Zero flag: Result[0..15] == 0
  - Sign flag: Result[15] (MSB of result)

Operations (same as 8-bit ALU):
  000 (0): ADD - Add operands
  001 (1): SUB - Subtract operands (A - B)
  010 (2): AND - Bitwise AND
  011 (3): OR  - Bitwise OR
  100 (4): XOR - Bitwise XOR
  101 (5): NOT - Bitwise NOT (~A, B ignored)
  110 (6): SHL - Shift left (A << 1)
  111 (7): SHR - Shift right (A >> 1)

Usage:
    python create_level7_alu_16bit.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder


class ALU16bitBuilder(ICBuilderBase):
    """Builder for 16-bit ALU IC"""

    async def create(self) -> bool:
        """Create the 16-bit ALU IC"""
        await self.begin_build("16-bit ALU")
        if not await self.create_new_circuit():
            return False

        # Layout positions
        input_x = 50.0
        alu_low_x = 250.0
        alu_high_x = 450.0
        output_x = 650.0

        # ---- Create Input Switches ----
        # OperandA[0-15]
        op_a_inputs = []
        for i in range(16):
            elem_id = await self.create_element("InputSwitch", input_x, 100.0 + (i * 40.0), f"OperandA[{i}]")
            if elem_id is None:
                return False
            op_a_inputs.append(elem_id)

        # OperandB[0-15]
        op_b_inputs = []
        for i in range(16):
            elem_id = await self.create_element(
                "InputSwitch", input_x + HORIZONTAL_GATE_SPACING, 100.0 + (i * 40.0), f"OperandB[{i}]"
            )
            if elem_id is None:
                return False
            op_b_inputs.append(elem_id)

        await self.log("  ✓ Created operand inputs")

        # ALUOp[0-2]
        aluop_inputs = []
        for i in range(3):
            elem_id = await self.create_element(
                "InputSwitch", input_x + (2 * HORIZONTAL_GATE_SPACING), 100.0 + (i * 40.0), f"ALUOp[{i}]"
            )
            if elem_id is None:
                return False
            aluop_inputs.append(elem_id)

        await self.log("  ✓ Created ALUOp inputs")

        # ---- Instantiate two 8-bit ALUs ----
        # Low byte ALU (bits 0-7)
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level6_alu_8bit")):
            return False

        alu_low_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level6_alu_8bit"), alu_low_x, 150.0, "ALU_Low")
        if alu_low_id is None:
            return False

        # High byte ALU (bits 8-15)
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level6_alu_8bit")):
            return False

        alu_high_id = await self.instantiate_ic(
            str(IC_COMPONENTS_DIR / "level6_alu_8bit"), alu_high_x, 150.0, "ALU_High"
        )
        if alu_high_id is None:
            return False

        await self.log("  ✓ Instantiated two 8-bit ALUs")

        # ---- Connect operands to ALUs ----
        # Low byte: connect A[0-7], B[0-7]
        for i in range(8):
            if not await self.connect(op_a_inputs[i], alu_low_id, target_port_label=f"A[{i}]"):
                return False

            if not await self.connect(op_b_inputs[i], alu_low_id, target_port_label=f"B[{i}]"):
                return False

        # High byte: connect A[8-15], B[8-15]
        for i in range(8):
            if not await self.connect(op_a_inputs[8 + i], alu_high_id, target_port_label=f"A[{i}]"):
                return False

            if not await self.connect(op_b_inputs[8 + i], alu_high_id, target_port_label=f"B[{i}]"):
                return False

        await self.log("  ✓ Connected operands to ALUs")

        # ---- Connect OpCode to both ALUs ----
        for i in range(3):
            # Low ALU
            if not await self.connect(aluop_inputs[i], alu_low_id, target_port_label=f"OpCode[{i}]"):
                return False

            # High ALU
            if not await self.connect(aluop_inputs[i], alu_high_id, target_port_label=f"OpCode[{i}]"):
                return False

        await self.log("  ✓ Connected ALUOp to both ALUs")

        # ---- Chain the carries between the halves (F26) ----
        # The low half keeps its unconnected defaults (CarryIn=0,
        # SubCarryIn=1); the high half continues both chains, making 16-bit
        # ADD and SUB correct across the byte boundary.
        if not await self.connect(alu_low_id, alu_high_id, source_port_label="Carry", target_port_label="CarryIn"):
            return False
        if not await self.connect(
            alu_low_id, alu_high_id, source_port_label="SubCarryOut", target_port_label="SubCarryIn"
        ):
            return False

        await self.log("  ✓ Chained ADD and SUB carries between the byte halves")

        # ---- Create output LEDs ----
        # Result[0-15]
        for i in range(16):
            led_id = await self.create_element("Led", output_x, 100.0 + (i * 40.0), f"Result[{i}]")
            if led_id is None:
                return False

            # Connect low byte results
            if i < 8:
                if not await self.connect(alu_low_id, led_id, source_port_label=f"Result[{i}]"):
                    return False
            else:
                if not await self.connect(alu_high_id, led_id, source_port_label=f"Result[{i - 8}]"):
                    return False

        await self.log("  ✓ Created Result outputs")

        # ---- Create flag outputs ----
        # Zero flag: the 16-bit result is zero iff BOTH halves are zero —
        # AND of the per-half Zero flags (F26: this was a NOR, which asserted
        # exactly when both halves were non-zero).
        zero_id = await self.create_element("Led", output_x + HORIZONTAL_GATE_SPACING, 100.0, "Zero")
        if zero_id is None:
            return False

        zero_and_id = await self.create_element("And", output_x + (2 * HORIZONTAL_GATE_SPACING), 100.0, "Zero_AND")
        if zero_and_id is None:
            return False

        # Connect both ALUs' Zero flags to the AND gate
        if not await self.connect(alu_low_id, zero_and_id, source_port_label="Zero"):
            return False

        if not await self.connect(alu_high_id, zero_and_id, source_port_label="Zero", target_port=1):
            return False

        # Connect AND output to Zero LED
        if not await self.connect(zero_and_id, zero_id):
            return False

        # Sign flag (MSB of result, from high ALU's Negative output)
        sign_id = await self.create_element("Led", output_x + HORIZONTAL_GATE_SPACING, 150.0, "Sign")
        if sign_id is None:
            return False

        if not await self.connect(alu_high_id, sign_id, source_port_label="Negative"):
            return False

        # Carry flag (carry-out from high ALU)
        carry_id = await self.create_element("Led", output_x + HORIZONTAL_GATE_SPACING, 200.0, "Carry")
        if carry_id is None:
            return False

        if not await self.connect(alu_high_id, carry_id, source_port_label="Carry"):
            return False

        await self.log("  ✓ Created flag outputs")

        output_file = str(IC_COMPONENTS_DIR / "level7_alu_16bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 16-bit ALU IC({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = ALU16bitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "16-bit ALU IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
