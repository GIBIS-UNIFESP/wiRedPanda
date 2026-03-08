#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Fetch Stage IC

Implements the fetch stage of a CPU pipeline.

Inputs:
  Clock (synchronization signal)
  Reset (reset program counter to 0)
  PCLoad (enable loading new PC value)
  PCInc (increment program counter)
  InstrLoad (enable loading instruction into register)
  PCData[0..7] (value to load into PC when PCLoad=1)

Outputs:
  PC[0..7] (current program counter value)
  Instruction[0..7] (currently stored instruction)
  OpCode[0..4] (upper 5 bits of instruction)
  RegisterAddr[0..2] (lower 3 bits of instruction)

Architecture:
  Composes three Level 7 building blocks:
  1. level7_program_counter_8bit
     - Manages program counter with load/increment/reset
     - Address output drives instruction memory
  2. level7_instruction_memory_interface
     - Fetches instruction from memory at address from PC
     - Returns 8-bit instruction word
  3. level7_instruction_register_8bit_cpu
     - Stores instruction with load control
     - Extracts opcode and register address fields

Data Flow:
  PCData → PC (on PCLoad)
  PC → InstrMem Address
  InstrMem → InstrReg Data Input (on InstrLoad)
  InstrReg → Instruction/OpCode/RegisterAddr outputs

Pipeline Behavior:
  - PC increments on each cycle (when PCInc=1) or loads new value (when PCLoad=1)
  - Instruction memory fetches instruction at current PC address
  - Instruction register stores fetched instruction
  - Instruction fields extracted for decode stage

Usage:
    python create_level8_fetch_stage.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class FetchStageBuilder(ICBuilderBase):
    """Builder for Fetch Stage IC"""

    async def create(self) -> bool:
        """Create the Fetch Stage IC"""
        await self.begin_build('Fetch Stage')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0
        input_y = 100.0

        # ---- Create PC data input (8-bit) ----
        pc_data_inputs = []
        for i in range(8):
            pc_data_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), input_y, f"PCData[{i}]")
            if pc_data_id is None:
                return False
            pc_data_inputs.append(pc_data_id)
        await self.log("  ✓ Created 8 PC data inputs")

        # ---- Create control signals ----
        control_x = input_x + (9 * HORIZONTAL_GATE_SPACING)

        clock_id = await self.create_element("Clock", control_x, input_y, "Clock")
        if clock_id is None:
            return False

        reset_id = await self.create_element("InputSwitch", control_x, input_y + VERTICAL_STAGE_SPACING / 2, "Reset")
        if reset_id is None:
            return False

        pc_load_id = await self.create_element("InputSwitch", control_x, input_y + VERTICAL_STAGE_SPACING, "PCLoad")
        if pc_load_id is None:
            return False

        pc_inc_id = await self.create_element("InputSwitch", control_x, input_y + (1.5 * VERTICAL_STAGE_SPACING), "PCInc")
        if pc_inc_id is None:
            return False

        instr_load_id = await self.create_element("InputSwitch", control_x, input_y + (2 * VERTICAL_STAGE_SPACING), "InstrLoad")
        if instr_load_id is None:
            return False

        await self.log("  ✓ Created control signals (Clock, Reset, PCLoad, PCInc, InstrLoad)")

        # ---- Instantiate Level 7 components ----

        # Program Counter
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level7_cpu_program_counter_8bit")):

            return False

        pc_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level7_cpu_program_counter_8bit"), input_x + (2 * HORIZONTAL_GATE_SPACING), 250.0, "PC")
        if pc_id is None:
            return False
        await self.log("  ✓ Instantiated Program Counter")

        # Instruction Memory Interface
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level7_instruction_memory_interface")):

            return False

        imem_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level7_instruction_memory_interface"), input_x + (6 * HORIZONTAL_GATE_SPACING), 250.0, "InstrMem")
        if imem_id is None:
            return False
        await self.log("  ✓ Instantiated Instruction Memory Interface")

        # Instruction Register
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level7_instruction_register_8bit")):

            return False

        ir_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level7_instruction_register_8bit"), input_x + (10 * HORIZONTAL_GATE_SPACING), 250.0, "InstrReg")
        if ir_id is None:
            return False
        await self.log("  ✓ Instantiated Instruction Register")

        # ---- Connect PC inputs ----
        for i in range(8):
            if not await self.connect(pc_data_inputs[i], pc_id, target_port_label=f"Data[{i}]"):
                return False

        if not await self.connect(clock_id, pc_id, target_port_label="Clock"):
            return False

        if not await self.connect(reset_id, pc_id, target_port_label="Reset"):
            return False

        if not await self.connect(pc_load_id, pc_id, target_port_label="Load"):
            return False

        if not await self.connect(pc_inc_id, pc_id, target_port_label="Inc"):
            return False

        # PC needs WriteEnable=1 always for normal operation
        vcc_id = await self.create_element("InputVcc", control_x + HORIZONTAL_GATE_SPACING, input_y, "WriteEn")
        if vcc_id is None:
            return False

        if not await self.connect(vcc_id, pc_id, target_port_label="WriteEnable"):
            return False

        await self.log("  ✓ Connected PC inputs")

        # ---- Connect PC to Instruction Memory ----
        for i in range(8):
            if not await self.connect(pc_id, imem_id, source_port_label=f"Address[{i}]", target_port_label=f"Address[{i}]"):
                return False

        if not await self.connect(clock_id, imem_id, target_port_label="Clock"):
            return False

        await self.log("  ✓ Connected PC to Instruction Memory")

        # ---- Connect Instruction Memory to Instruction Register ----
        for i in range(8):
            if not await self.connect(imem_id, ir_id, source_port_label=f"Instruction[{i}]", target_port_label=f"Data[{i}]"):
                return False

        if not await self.connect(clock_id, ir_id, target_port_label="Clock"):
            return False

        if not await self.connect(instr_load_id, ir_id, target_port_label="Load"):
            return False

        if not await self.connect(reset_id, ir_id, target_port_label="Reset"):
            return False

        await self.log("  ✓ Connected Instruction Memory to Instruction Register")

        # ---- Create output LEDs ----
        output_x = input_x + (14 * HORIZONTAL_GATE_SPACING)

        # PC outputs
        pc_outputs = []
        for i in range(8):
            led_id = await self.create_element("Led", output_x, 200.0 + (i * (VERTICAL_STAGE_SPACING / 2)), f"PC[{i}]")
            if led_id is None:
                return False
            pc_outputs.append(led_id)

            if not await self.connect(pc_id, led_id, source_port_label=f"Address[{i}]"):
                return False

        await self.log("  ✓ Created PC outputs")

        # Instruction outputs
        instr_outputs = []
        for i in range(8):
            led_id = await self.create_element("Led", output_x + HORIZONTAL_GATE_SPACING, 200.0 + (i * (VERTICAL_STAGE_SPACING / 2)), f"Instruction[{i}]")
            if led_id is None:
                return False
            instr_outputs.append(led_id)

            if not await self.connect(ir_id, led_id, source_port_label=f"Instruction[{i}]"):
                return False

        await self.log("  ✓ Created Instruction outputs")

        # OpCode outputs (5 bits)
        opcode_outputs = []
        for i in range(5):
            led_id = await self.create_element("Led", output_x + (2 * HORIZONTAL_GATE_SPACING), 200.0 + (i * (VERTICAL_STAGE_SPACING / 2)), f"OpCode[{i}]")
            if led_id is None:
                return False
            opcode_outputs.append(led_id)

            if not await self.connect(ir_id, led_id, source_port_label=f"OpCode[{i}]"):
                return False

        await self.log("  ✓ Created OpCode outputs")

        # RegisterAddr outputs (3 bits)
        regaddr_outputs = []
        for i in range(3):
            led_id = await self.create_element("Led", output_x + (3 * HORIZONTAL_GATE_SPACING), 200.0 + (i * (VERTICAL_STAGE_SPACING / 2)), f"RegisterAddr[{i}]")
            if led_id is None:
                return False
            regaddr_outputs.append(led_id)

            if not await self.connect(ir_id, led_id, source_port_label=f"RegisterAddr[{i}]"):
                return False

        await self.log("  ✓ Created RegisterAddr outputs")

        output_file = str(IC_COMPONENTS_DIR / "level8_fetch_stage.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created Fetch Stage IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = FetchStageBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Fetch Stage IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
