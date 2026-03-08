#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Decode Stage IC

Implements the decode stage of a CPU pipeline.

Inputs:
  OpCode[0..4] (5-bit instruction opcode from fetch stage)
  Clock (synchronization signal)
  Reset (reset all outputs to 0)

Outputs:
  ALUOp[0..2] (3-bit ALU operation selector)
  RegWrite (register write enable)
  MemRead (memory read enable)
  MemWrite (memory write enable)
  InstrDecodedLines[0..31] (optional: one-hot instruction decode)

Instruction Set:
  000 (0): ADD - Add two registers
  001 (1): SUB - Subtract two registers
  010 (2): AND - Bitwise AND
  011 (3): OR  - Bitwise OR
  100 (4): XOR - Bitwise XOR
  101 (5): NOT - Bitwise NOT
  110 (6): SHL - Shift left
  111 (7): Shift right / Memory operation (opcode[3-4] determine variant)

Control Signal Derivation:
  ALUOp[0-2] = OpCode[0-2] (lower 3 bits select ALU operation)
  RegWrite = NOT(OpCode[4]) (most instructions write to register)
  MemRead = OpCode[4] AND NOT(OpCode[3]) (memory read when bits 4-3 = 10)
  MemWrite = OpCode[4] AND OpCode[3] (memory write when bits 4-3 = 11)

Architecture:
  - Combinational logic gates (no storage, no clock dependence for outputs)
  - Directly routes opcode bits with simple logic transformations
  - Outputs stable within propagation delay

Simplicity Note:
  This is a simple control logic generator suitable for basic CPUs.
  More complex instruction sets would require a decoder IC.

Usage:
    python create_level8_decode_stage.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class DecodeStageBuilder(ICBuilderBase):
    """Builder for Decode Stage IC"""

    async def create(self) -> bool:
        """Create the Decode Stage IC"""
        await self.begin_build('Decode Stage')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0
        input_y = 100.0

        # ---- Create OpCode input switches (5-bit) ----
        opcode_inputs = []
        for i in range(5):
            opcode_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), input_y, f"OpCode[{i}]")
            if opcode_id is None:
                return False
            opcode_inputs.append(opcode_id)
        await self.log("  ✓ Created 5 opcode inputs")

        # ---- Create clock input ----
        control_x = input_x + (6 * HORIZONTAL_GATE_SPACING)

        clock_id = await self.create_element("Clock", control_x, input_y, "Clock")
        if clock_id is None:
            return False

        reset_id = await self.create_element("InputSwitch", control_x, input_y + VERTICAL_STAGE_SPACING / 2, "Reset")
        if reset_id is None:
            return False

        await self.log("  ✓ Created control signals (Clock, Reset)")

        # ---- Create combinational logic for control signals ----

        # ALUOp[0-2]: Direct pass-through from OpCode[0-2]
        # (No gates needed - direct connection)

        # RegWrite = NOT(OpCode[4])
        # When bit 4 is 0, write to register; when 1, it's a memory operation
        regwrite_not_id = await self.create_element("Not", control_x + HORIZONTAL_GATE_SPACING, input_y, "RegWrite_NOT")
        if regwrite_not_id is None:
            return False

        if not await self.connect(opcode_inputs[4], regwrite_not_id):
            return False

        # MemRead = OpCode[4] AND NOT(OpCode[3])
        # Memory read when bits [4:3] = 10
        opcode3_not_id = await self.create_element("Not", control_x + (2 * HORIZONTAL_GATE_SPACING), input_y, "OpCode3_NOT")
        if opcode3_not_id is None:
            return False

        if not await self.connect(opcode_inputs[3], opcode3_not_id):
            return False

        memread_and_id = await self.create_element("And", control_x + (3 * HORIZONTAL_GATE_SPACING), input_y, "MemRead_AND")
        if memread_and_id is None:
            return False

        if not await self.connect(opcode_inputs[4], memread_and_id):
            return False

        if not await self.connect(opcode3_not_id, memread_and_id, target_port=1):
            return False

        # MemWrite = OpCode[4] AND OpCode[3]
        # Memory write when bits [4:3] = 11
        memwrite_and_id = await self.create_element("And", control_x + (4 * HORIZONTAL_GATE_SPACING), input_y, "MemWrite_AND")
        if memwrite_and_id is None:
            return False

        if not await self.connect(opcode_inputs[4], memwrite_and_id):
            return False

        if not await self.connect(opcode_inputs[3], memwrite_and_id, target_port=1):
            return False

        await self.log("  ✓ Created control signal logic")

        # ---- Create output LEDs ----
        output_x = control_x + (6 * HORIZONTAL_GATE_SPACING)

        # ALUOp outputs (direct pass-through from OpCode[0-2])
        for i in range(3):
            led_id = await self.create_element("Led", output_x, input_y + (i * (VERTICAL_STAGE_SPACING / 2)), f"ALUOp[{i}]")
            if led_id is None:
                return False

            if not await self.connect(opcode_inputs[i], led_id):
                return False

        await self.log("  ✓ Created ALUOp outputs")

        # RegWrite output
        regwrite_led_id = await self.create_element("Led", output_x + HORIZONTAL_GATE_SPACING, input_y, "RegWrite")
        if regwrite_led_id is None:
            return False

        if not await self.connect(regwrite_not_id, regwrite_led_id):
            return False

        # MemRead output
        memread_led_id = await self.create_element("Led", output_x + (2 * HORIZONTAL_GATE_SPACING), input_y, "MemRead")
        if memread_led_id is None:
            return False

        if not await self.connect(memread_and_id, memread_led_id):
            return False

        # MemWrite output
        memwrite_led_id = await self.create_element("Led", output_x + (3 * HORIZONTAL_GATE_SPACING), input_y, "MemWrite")
        if memwrite_led_id is None:
            return False

        if not await self.connect(memwrite_and_id, memwrite_led_id):
            return False

        await self.log("  ✓ Created control signal outputs")

        output_file = str(IC_COMPONENTS_DIR / "level8_decode_stage.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created Decode Stage IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = DecodeStageBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Decode Stage IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
