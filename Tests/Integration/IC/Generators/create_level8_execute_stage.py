#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Execute Stage IC

Implements the execute stage of a CPU pipeline.

Inputs:
  OperandA[0..7] (first operand from register file)
  OperandB[0..7] (second operand from register file)
  ALUOp[0..2] (3-bit ALU operation selector from decode stage)
  Clock (synchronization signal)
  Reset (reset flag outputs)

Outputs:
  Result[0..7] (8-bit ALU result)
  Zero (zero flag: 1 if result is 0)
  Sign (sign flag: 1 if MSB is 1, indicates negative)

ALU Operations:
  000 (0): ADD - Add operands (A + B)
  001 (1): SUB - Subtract operands (A - B using 2's complement)
  010 (2): AND - Bitwise AND (A AND B)
  011 (3): OR  - Bitwise OR (A OR B)
  100 (4): XOR - Bitwise XOR (A XOR B)
  101 (5): NOT - Bitwise NOT (~A, B ignored)
  110 (6): SHL - Shift left (A << 1)
  111 (7): SHR - Shift right (A >> 1)

Architecture:
  - Instantiates level7_execution_datapath for ALU operations
  - Computes Zero flag from result (all bits 0)
  - Computes Sign flag from result MSB
  - Provides direct result output with flags

Pipeline Behavior:
  - Combinational ALU execution (no clock dependence for result)
  - Flag signals available immediately after operands change
  - No pipelining or delay between stages

Usage:
    python create_level8_execute_stage.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class ExecuteStageBuilder(ICBuilderBase):
    """Builder for Execute Stage IC"""

    async def create(self) -> bool:
        """Create the Execute Stage IC"""
        await self.begin_build('Execute Stage')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0
        input_y = 100.0

        # ---- Create OperandA input switches (8-bit) ----
        operandA_inputs = []
        for i in range(8):
            opA_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), input_y, f"OperandA[{i}]")
            if opA_id is None:
                return False
            operandA_inputs.append(opA_id)
        await self.log("  ✓ Created 8 OperandA inputs")

        # ---- Create OperandB input switches (8-bit) ----
        operandB_inputs = []
        for i in range(8):
            opB_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), input_y + VERTICAL_STAGE_SPACING / 2, f"OperandB[{i}]")
            if opB_id is None:
                return False
            operandB_inputs.append(opB_id)
        await self.log("  ✓ Created 8 OperandB inputs")

        # ---- Create ALUOp input switches (3-bit) ----
        aluop_inputs = []
        control_x = input_x + (9 * HORIZONTAL_GATE_SPACING)
        for i in range(3):
            aluop_id = await self.create_element("InputSwitch", control_x, input_y + (i * (VERTICAL_STAGE_SPACING / 2)), f"ALUOp[{i}]")
            if aluop_id is None:
                return False
            aluop_inputs.append(aluop_id)
        await self.log("  ✓ Created 3 ALUOp inputs")

        # ---- Create control signals ----
        clock_id = await self.create_element("Clock", control_x + HORIZONTAL_GATE_SPACING, input_y, "Clock")
        if clock_id is None:
            return False

        reset_id = await self.create_element("InputSwitch", control_x + HORIZONTAL_GATE_SPACING, input_y + VERTICAL_STAGE_SPACING / 2, "Reset")
        if reset_id is None:
            return False

        await self.log("  ✓ Created control signals (Clock, Reset)")

        # ---- Instantiate Execution Datapath ----
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level7_execution_datapath")):

            return False

        datapath_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level7_execution_datapath"), input_x + (3 * HORIZONTAL_GATE_SPACING), 300.0, "Datapath")
        if datapath_id is None:
            return False
        await self.log("  ✓ Instantiated Execution Datapath")

        # ---- Connect operands and ALUOp to datapath ----
        for i in range(8):
            if not await self.connect(operandA_inputs[i], datapath_id, target_port_label=f"OperandA[{i}]"):
                return False

            if not await self.connect(operandB_inputs[i], datapath_id, target_port_label=f"OperandB[{i}]"):
                return False

        for i in range(3):
            if not await self.connect(aluop_inputs[i], datapath_id, target_port_label=f"OpCode[{i}]"):
                return False

        await self.log("  ✓ Connected inputs to datapath")

        # ---- Create output LEDs ----
        output_x = control_x + (4 * HORIZONTAL_GATE_SPACING)

        # Result outputs (8-bit)
        result_outputs = []
        for i in range(8):
            led_id = await self.create_element("Led", output_x, input_y + (i * (VERTICAL_STAGE_SPACING / 2)), f"Result[{i}]")
            if led_id is None:
                return False
            result_outputs.append(led_id)

            if not await self.connect(datapath_id, led_id, source_port_label=f"Result[{i}]"):
                return False

        await self.log("  ✓ Created Result outputs")

        # Zero flag output
        zero_led_id = await self.create_element("Led", output_x + HORIZONTAL_GATE_SPACING, input_y, "Zero")
        if zero_led_id is None:
            return False

        if not await self.connect(datapath_id, zero_led_id, source_port_label="Zero"):
            return False

        # Sign flag output
        sign_led_id = await self.create_element("Led", output_x + HORIZONTAL_GATE_SPACING, input_y + (VERTICAL_STAGE_SPACING / 2), "Sign")
        if sign_led_id is None:
            return False

        if not await self.connect(datapath_id, sign_led_id, source_port_label="Sign"):
            return False

        await self.log("  ✓ Created flag outputs")

        output_file = str(IC_COMPONENTS_DIR / "level8_execute_stage.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created Execute Stage IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = ExecuteStageBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Execute Stage IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
