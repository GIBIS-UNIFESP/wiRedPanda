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
  ProgAddr[0..7] (address for programming instruction memory)
  ProgData[0..7] (data to write into instruction memory)
  ProgWrite (enable writing to instruction memory)

Outputs:
  PC[0..7] (current program counter value)
  Instruction[0..7] (currently stored instruction)
  OpCode[0..4] (upper 5 bits of instruction)
  RegisterAddr[0..2] (lower 3 bits of instruction)

Architecture:
  Composes three Level 7 building blocks plus an address mux:
  1. level7_program_counter_8bit
     - Manages program counter with load/increment/reset
     - Address output drives instruction memory (via mux)
  2. level4_bus_mux_8bit
     - Selects between PC address (normal) and ProgAddr (programming)
     - Sel=ProgWrite: 0=PC address, 1=ProgAddr
  3. level7_instruction_memory_interface
     - Fetches instruction from memory at mux output address
     - Write port exposed for external programming
  4. level7_instruction_register_8bit_cpu
     - Stores instruction with load control
     - Extracts opcode and register address fields

Data Flow:
  PCData → PC (on PCLoad)
  PC Address / ProgAddr → Mux → InstrMem Address
  ProgData → InstrMem DataIn (on ProgWrite)
  InstrMem → InstrReg Data Input (on InstrLoad)
  InstrReg → Instruction/OpCode/RegisterAddr outputs

Pipeline Behavior:
  - PC increments on each cycle (when PCInc=1) or loads new value (when PCLoad=1)
  - Address mux selects PC (ProgWrite=0) or ProgAddr (ProgWrite=1)
  - Instruction memory fetches or writes at mux output address
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

        # ---- Create programming interface inputs ----
        prog_x = input_x
        prog_y = input_y + (3 * VERTICAL_STAGE_SPACING)

        prog_addr_inputs = []
        for i in range(8):
            prog_addr_id = await self.create_element("InputSwitch", prog_x + (i * HORIZONTAL_GATE_SPACING), prog_y, f"ProgAddr[{i}]")
            if prog_addr_id is None:
                return False
            prog_addr_inputs.append(prog_addr_id)
        await self.log("  ✓ Created 8 ProgAddr inputs")

        prog_data_inputs = []
        for i in range(8):
            prog_data_id = await self.create_element("InputSwitch", prog_x + (i * HORIZONTAL_GATE_SPACING), prog_y + VERTICAL_STAGE_SPACING / 2, f"ProgData[{i}]")
            if prog_data_id is None:
                return False
            prog_data_inputs.append(prog_data_id)
        await self.log("  ✓ Created 8 ProgData inputs")

        prog_write_id = await self.create_element("InputSwitch", control_x, prog_y, "ProgWrite")
        if prog_write_id is None:
            return False
        await self.log("  ✓ Created ProgWrite input")

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

        # Address Mux: selects between PC address (In0) and ProgAddr (In1)
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_8bit")):

            return False

        addr_mux_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_8bit"), input_x + (4 * HORIZONTAL_GATE_SPACING), 250.0, "AddrMux")
        if addr_mux_id is None:
            return False
        await self.log("  ✓ Instantiated Address Mux (PC vs ProgAddr)")

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

        # ---- Connect Address Mux ----
        # In0 = PC address (normal fetch), In1 = ProgAddr (programming), Sel = ProgWrite
        for i in range(8):
            if not await self.connect(pc_id, addr_mux_id, source_port_label=f"Address[{i}]", target_port_label=f"In0[{i}]"):
                return False
            if not await self.connect(prog_addr_inputs[i], addr_mux_id, target_port_label=f"In1[{i}]"):
                return False
        if not await self.connect(prog_write_id, addr_mux_id, target_port_label="Sel"):
            return False
        await self.log("  ✓ Connected Address Mux inputs")

        # ---- Connect Mux output to Instruction Memory Address ----
        for i in range(8):
            if not await self.connect(addr_mux_id, imem_id, source_port_label=f"Out[{i}]", target_port_label=f"Address[{i}]"):
                return False

        # Connect programming data and write enable to Instruction Memory
        for i in range(8):
            if not await self.connect(prog_data_inputs[i], imem_id, target_port_label=f"DataIn[{i}]"):
                return False
        if not await self.connect(prog_write_id, imem_id, target_port_label="WriteEnable"):
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

        # Raw (unregistered) instruction outputs — directly from instruction memory,
        # bypassing the instruction register's 1-cycle delay.
        # Used by single-cycle CPUs that need zero-delay instruction decode.
        for i in range(8):
            led_id = await self.create_element("Led", output_x + (4 * HORIZONTAL_GATE_SPACING), 200.0 + (i * (VERTICAL_STAGE_SPACING / 2)), f"RawInstr[{i}]")
            if led_id is None:
                return False
            if not await self.connect(imem_id, led_id, source_port_label=f"Instruction[{i}]"):
                return False

        await self.log("  ✓ Created RawInstr outputs (unregistered)")

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
