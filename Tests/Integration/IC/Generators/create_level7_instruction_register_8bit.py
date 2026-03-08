#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create CPU Level 3: 8-bit Instruction Register IC

Stores an 8-bit instruction with opcode and register address extraction.

Inputs:
  Data[0..7] (instruction from memory)
  Clock, Load, Reset

Outputs:
  Instruction[0..7] (full instruction)
  OpCode[0..4] (upper 5 bits: bits 7-3)
  RegisterAddr[0..2] (lower 3 bits: bits 2-0)

Usage:
    python create_cpu_instruction_register_8bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class InstructionRegister8BitBuilder(ICBuilderBase):
    """Builder for 8-bit Instruction Register IC"""

    async def create(self) -> bool:
        """Create the 8-bit Instruction Register IC"""
        await self.begin_build("Instruction Register 8-bit")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        data_input_x = 50.0
        data_input_y = 100.0
        control_input_x = data_input_x + (8 * HORIZONTAL_GATE_SPACING) + HORIZONTAL_GATE_SPACING
        control_input_y = data_input_y

        # ---- Create input data bus (Data[0..7]) ----
        data_inputs = []
        for i in range(8):
            element_id = await self.create_element("InputSwitch", data_input_x + (i * HORIZONTAL_GATE_SPACING), data_input_y, f"Data[{i}]")
            if element_id is None:
                return False
            data_inputs.append(element_id)

        # ---- Create control signals ----
        clock = await self.create_element("InputSwitch", control_input_x, control_input_y, "Clock")
        if clock is None:
            return False

        load = await self.create_element("InputSwitch", control_input_x, control_input_y + VERTICAL_STAGE_SPACING, "Load")
        if load is None:
            return False

        reset = await self.create_element("InputSwitch", control_input_x, control_input_y + (2 * VERTICAL_STAGE_SPACING), "Reset")
        if reset is None:
            return False

        await self.log("  ✓ Created 8 data inputs + 3 control signals")

        # IC position
        register_x = data_input_x + (4 * HORIZONTAL_GATE_SPACING)
        register_y = control_input_y + (4 * VERTICAL_STAGE_SPACING)

        # ---- Instantiate 8-bit Register for instruction storage ----
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level6_register_8bit")):

            return False

        instr_register = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level6_register_8bit"), register_x, register_y, "InstructionReg")
        if instr_register is None:
            return False
        await self.log("  ✓ Instantiated 8-bit Register for instruction storage")

        # Connect Clock to register
        if not await self.connect(clock, instr_register, target_port_label="Clock"):
            return False

        # Connect Load to WriteEnable
        if not await self.connect(load, instr_register, target_port_label="WriteEnable"):
            return False

        # Connect Reset to register
        if not await self.connect(reset, instr_register, target_port_label="Reset"):
            return False

        # Connect Data to register inputs
        for i in range(8):
            if not await self.connect(data_inputs[i], instr_register, target_port_label=f"Data[{i}]"):
                return False
        await self.log("  ✓ Connected control signals to instruction register")

        # Output positions
        output_x = data_input_x
        output_y = register_y + (3 * VERTICAL_STAGE_SPACING)

        # ---- Create Instruction output LEDs (Instruction[0..7]) ----
        for i in range(8):
            instr_led = await self.create_element("Led", output_x + (i * HORIZONTAL_GATE_SPACING), output_y, f"Instruction[{i}]")
            if instr_led is None:
                return False

            if not await self.connect(instr_register, instr_led, source_port_label=f"Q[{i}]"):
                return False
        await self.log("  ✓ Created 8 Instruction outputs")

        # ---- Create OpCode output LEDs (OpCode[0..4] = Instruction[7:3]) ----
        for i in range(5):
            opcode_led = await self.create_element("Led", output_x + (i * HORIZONTAL_GATE_SPACING), output_y + VERTICAL_STAGE_SPACING, f"OpCode[{i}]")
            if opcode_led is None:
                return False

            if not await self.connect(instr_register, opcode_led, source_port_label=f"Q[{i + 3}]"):
                return False
        await self.log("  ✓ Created 5 OpCode outputs (bits 3-7 of instruction)")

        # ---- Create RegisterAddr output LEDs (RegisterAddr[0..2] = Instruction[2:0]) ----
        for i in range(3):
            regaddr_led = await self.create_element("Led", output_x + (i * HORIZONTAL_GATE_SPACING), output_y + (2 * VERTICAL_STAGE_SPACING), f"RegisterAddr[{i}]")
            if regaddr_led is None:
                return False

            if not await self.connect(instr_register, regaddr_led, source_port_label=f"Q[{i}]"):
                return False
        await self.log("  ✓ Created 3 RegisterAddr outputs (bits 0-2 of instruction)")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level7_instruction_register_8bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 8-bit Instruction Register IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = InstructionRegister8BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "8-bit Instruction Register IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
