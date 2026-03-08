#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 8-bit Single-Cycle CPU IC

Implements a complete 8-bit single-cycle CPU with 4 pipeline stages integrated:
- Fetch Stage: Instruction retrieval and program counter management
- Decode Stage: Opcode to control signal conversion
- Execute Stage: ALU operations with 8 supported operations
- Memory Stage: Memory read/write and result pass-through
- Register File: 8 general-purpose 8-bit registers

Inputs:
  Clock (synchronization signal)
  Reset (initialize program counter to 0)
  ProgAddr[0-7] (instruction memory programming address)
  ProgData[0-7] (instruction memory programming data)
  ProgWrite (instruction memory write enable)
  RegProgAddr[0-2] (register file programming address)
  RegProgData[0-7] (register file programming data)
  RegProgWrite (register file programming write enable)

Outputs:
  PC[0-7] (current program counter value)
  Result[0-7] (ALU/Memory result)
  Instruction[0-7] (current instruction being executed)
  Zero (zero flag from ALU)
  Sign (sign flag from ALU)

Architecture:
  - Integrates level8_fetch_stage for instruction retrieval (with programming port)
  - Integrates level8_decode_stage for control signal generation
  - Integrates level8_execute_stage for ALU operations
  - Integrates level8_memory_stage for memory operations
  - Instantiates level6_register_file_8x8 for data storage
  - Register file wired with programming mux for external initialization
  - All stages connected in sequence with proper data paths

Execution Model (Accumulator-based):
  - OperandA = R0 (accumulator, always read from register 0)
  - OperandB = Register[RegisterAddr] (from instruction bits [2:0])
  - ALU result written back to R0 via Memory stage
  - Programming ports allow tests to pre-load instruction memory and registers

Usage:
    python create_level9_cpu_8bit_single_cycle.py
"""

import asyncio
from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class CPU8BitSingleCycleBuilder(ICBuilderBase):
    """Builder for 8-bit Single-Cycle CPU IC"""

    async def create(self) -> bool:
        """Create the 8-bit Single-Cycle CPU IC"""
        await self.begin_build('8-bit Single-Cycle CPU')
        if not await self.create_new_circuit():
            return False

        # Layout positions
        stage_x_offsets = [50.0, 300.0, 550.0, 800.0]
        stage_y = 100.0
        reg_file_y = 500.0

        # ---- Create Clock and Reset inputs ----
        control_x = 50.0
        control_y = 50.0

        clock_id = await self.create_element("Clock", control_x, control_y, "Clock")
        if clock_id is None:
            return False

        reset_id = await self.create_element("InputSwitch", control_x + HORIZONTAL_GATE_SPACING, control_y, "Reset")
        if reset_id is None:
            return False

        await self.log("  ✓ Created Clock and Reset inputs")

        # ---- Create instruction memory programming inputs ----
        prog_x = 50.0
        prog_y = control_y + VERTICAL_STAGE_SPACING

        prog_addr_inputs = []
        for i in range(8):
            pid = await self.create_element("InputSwitch", prog_x + (i * HORIZONTAL_GATE_SPACING), prog_y, f"ProgAddr[{i}]")
            if pid is None:
                return False
            prog_addr_inputs.append(pid)

        prog_data_inputs = []
        for i in range(8):
            pid = await self.create_element("InputSwitch", prog_x + (i * HORIZONTAL_GATE_SPACING), prog_y + VERTICAL_STAGE_SPACING / 2, f"ProgData[{i}]")
            if pid is None:
                return False
            prog_data_inputs.append(pid)

        prog_write_id = await self.create_element("InputSwitch", prog_x + (8 * HORIZONTAL_GATE_SPACING), prog_y, "ProgWrite")
        if prog_write_id is None:
            return False

        await self.log("  ✓ Created instruction memory programming inputs")

        # ---- Create register file programming inputs ----
        reg_prog_y = prog_y + VERTICAL_STAGE_SPACING * 1.5

        reg_prog_addr_inputs = []
        for i in range(3):
            pid = await self.create_element("InputSwitch", prog_x + (i * HORIZONTAL_GATE_SPACING), reg_prog_y, f"RegProgAddr[{i}]")
            if pid is None:
                return False
            reg_prog_addr_inputs.append(pid)

        reg_prog_data_inputs = []
        for i in range(8):
            pid = await self.create_element("InputSwitch", prog_x + (i * HORIZONTAL_GATE_SPACING), reg_prog_y + VERTICAL_STAGE_SPACING / 2, f"RegProgData[{i}]")
            if pid is None:
                return False
            reg_prog_data_inputs.append(pid)

        reg_prog_write_id = await self.create_element("InputSwitch", prog_x + (8 * HORIZONTAL_GATE_SPACING), reg_prog_y, "RegProgWrite")
        if reg_prog_write_id is None:
            return False

        await self.log("  ✓ Created register file programming inputs")

        # ---- Instantiate pipeline stages ----
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level8_fetch_stage")):
            return False
        fetch_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level8_fetch_stage"), stage_x_offsets[0], stage_y, "Fetch")
        if fetch_id is None:
            return False
        await self.log("  ✓ Instantiated Fetch Stage")

        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level8_decode_stage")):
            return False
        decode_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level8_decode_stage"), stage_x_offsets[1], stage_y, "Decode")
        if decode_id is None:
            return False
        await self.log("  ✓ Instantiated Decode Stage")

        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level8_execute_stage")):
            return False
        execute_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level8_execute_stage"), stage_x_offsets[2], stage_y, "Execute")
        if execute_id is None:
            return False
        await self.log("  ✓ Instantiated Execute Stage")

        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level8_memory_stage")):
            return False
        memory_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level8_memory_stage"), stage_x_offsets[3], stage_y, "Memory")
        if memory_id is None:
            return False
        await self.log("  ✓ Instantiated Memory Stage")

        # ---- Instantiate Register File ----
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level6_register_file_8x8")):
            return False
        regfile_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level6_register_file_8x8"), 425.0, reg_file_y, "RegFile")
        if regfile_id is None:
            return False
        await self.log("  ✓ Instantiated Register File")

        # ---- Instantiate register file programming muxes ----
        # Write data mux: In0=Memory DataOut (normal), In1=RegProgData (programming)
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_8bit")):
            return False
        write_data_mux_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_8bit"), 425.0, reg_file_y - 150.0, "WriteDataMux")
        if write_data_mux_id is None:
            return False

        # Write address muxes (3 individual Mux elements for 3-bit address)
        write_addr_mux_ids = []
        for i in range(3):
            mux_id = await self.create_element("Mux", 350.0 + (i * HORIZONTAL_GATE_SPACING), reg_file_y - 150.0, f"WriteAddrMux{i}")
            if mux_id is None:
                return False
            write_addr_mux_ids.append(mux_id)

        # Write enable logic: (RegWrite AND NOT(ProgWrite) AND NOT(Reset)) OR RegProgWrite
        # This prevents the normal pipeline from writing registers during programming or reset
        prog_not_id = await self.create_element("Not", 500.0, reg_file_y - 200.0, "ProgWriteNOT")
        if prog_not_id is None:
            return False
        reset_not_id = await self.create_element("Not", 530.0, reg_file_y - 200.0, "ResetNOT")
        if reset_not_id is None:
            return False
        we_and1_id = await self.create_element("And", 520.0, reg_file_y - 150.0, "WriteEnableAND1")
        if we_and1_id is None:
            return False
        we_and2_id = await self.create_element("And", 540.0, reg_file_y - 150.0, "WriteEnableAND2")
        if we_and2_id is None:
            return False
        we_or_id = await self.create_element("Or", 560.0, reg_file_y - 150.0, "WriteEnableOR")
        if we_or_id is None:
            return False

        await self.log("  ✓ Instantiated register file programming muxes")

        # ---- Create Vcc and Gnd for default control signals ----
        vcc_id = await self.create_element("InputVcc", 100.0, stage_y - 50.0, "Vcc")
        if vcc_id is None:
            return False

        gnd_id = await self.create_element("InputGnd", 150.0, stage_y - 50.0, "Gnd")
        if gnd_id is None:
            return False

        await self.log("  ✓ Created Vcc and Gnd for control signals")

        # ==== CONNECT CLOCK AND RESET ====
        if not await self.connect(clock_id, fetch_id, target_port_label="Clock"):
            return False
        if not await self.connect(reset_id, fetch_id, target_port_label="Reset"):
            return False

        # PCLoad = 0 (no jumps), InstrLoad = 1 (load instruction every cycle)
        if not await self.connect(gnd_id, fetch_id, target_port_label="PCLoad"):
            return False
        if not await self.connect(vcc_id, fetch_id, target_port_label="InstrLoad"):
            return False
        # PCInc = 1 (increment PC each cycle)
        if not await self.connect(vcc_id, fetch_id, target_port_label="PCInc"):
            return False
        # PCData = 0 (unused since PCLoad=0)
        for i in range(8):
            if not await self.connect(gnd_id, fetch_id, target_port_label=f"PCData[{i}]"):
                return False

        # Connect programming interface to Fetch stage
        for i in range(8):
            if not await self.connect(prog_addr_inputs[i], fetch_id, target_port_label=f"ProgAddr[{i}]"):
                return False
            if not await self.connect(prog_data_inputs[i], fetch_id, target_port_label=f"ProgData[{i}]"):
                return False
        if not await self.connect(prog_write_id, fetch_id, target_port_label="ProgWrite"):
            return False

        # Clock to other stages
        if not await self.connect(clock_id, decode_id, target_port_label="Clock"):
            return False
        if not await self.connect(clock_id, execute_id, target_port_label="Clock"):
            return False
        if not await self.connect(clock_id, memory_id, target_port_label="Clock"):
            return False
        if not await self.connect(clock_id, regfile_id, target_port_label="Clock"):
            return False

        await self.log("  ✓ Connected Clock, Reset, and programming inputs")

        # ==== CONNECT FETCH → DECODE (using raw unregistered instruction) ====
        # RawInstr[3..7] → OpCode[0..4] (bypass instruction register for zero-delay decode)
        for i in range(5):
            if not await self.connect(fetch_id, decode_id, source_port_label=f"RawInstr[{i + 3}]", target_port_label=f"OpCode[{i}]"):
                return False
        await self.log("  ✓ Connected Fetch to Decode (via RawInstr)")

        # ==== CONNECT DECODE → EXECUTE ====
        for i in range(3):
            if not await self.connect(decode_id, execute_id, source_port_label=f"ALUOp[{i}]", target_port_label=f"ALUOp[{i}]"):
                return False
        await self.log("  ✓ Connected Decode to Execute")

        # ==== WIRE REGISTER FILE ====
        # Read_Addr1 = GND (always read R0 as accumulator → OperandA)
        for i in range(3):
            if not await self.connect(gnd_id, regfile_id, target_port_label=f"Read_Addr1[{i}]"):
                return False

        # Read_Addr2 = RawInstr[0..2] (instruction-addressed register → OperandB)
        for i in range(3):
            if not await self.connect(fetch_id, regfile_id, source_port_label=f"RawInstr[{i}]", target_port_label=f"Read_Addr2[{i}]"):
                return False

        # Connect register file reads to Execute stage operands
        for i in range(8):
            if not await self.connect(regfile_id, execute_id, source_port_label=f"Read_Data1[{i}]", target_port_label=f"OperandA[{i}]"):
                return False
            if not await self.connect(regfile_id, execute_id, source_port_label=f"Read_Data2[{i}]", target_port_label=f"OperandB[{i}]"):
                return False

        await self.log("  ✓ Wired register file read ports to Execute stage")

        # Write address mux: In0=GND (normal: write to R0), In1=RegProgAddr, Sel=RegProgWrite
        for i in range(3):
            if not await self.connect(gnd_id, write_addr_mux_ids[i], target_port=0):            # In0 = GND
                return False
            if not await self.connect(reg_prog_addr_inputs[i], write_addr_mux_ids[i], target_port=1):  # In1 = RegProgAddr
                return False
            if not await self.connect(reg_prog_write_id, write_addr_mux_ids[i], target_port=2):  # Sel = RegProgWrite
                return False
            if not await self.connect(write_addr_mux_ids[i], regfile_id, target_port_label=f"Write_Addr[{i}]"):
                return False

        # Write data mux: In0=Memory DataOut (normal writeback), In1=RegProgData (programming)
        for i in range(8):
            if not await self.connect(reg_prog_data_inputs[i], write_data_mux_id, target_port_label=f"In1[{i}]"):
                return False
        if not await self.connect(reg_prog_write_id, write_data_mux_id, target_port_label="Sel"):
            return False
        # In0 connected after Memory stage wiring (needs memory_id DataOut)

        # Write enable: (RegWrite AND NOT(ProgWrite) AND NOT(Reset)) OR RegProgWrite
        # NOT gates
        if not await self.connect(prog_write_id, prog_not_id):
            return False
        if not await self.connect(reset_id, reset_not_id):
            return False
        # AND1: RegWrite AND NOT(ProgWrite)
        if not await self.connect(decode_id, we_and1_id, source_port_label="RegWrite", target_port=0):
            return False
        if not await self.connect(prog_not_id, we_and1_id, target_port=1):
            return False
        # AND2: AND1 output AND NOT(Reset)
        if not await self.connect(we_and1_id, we_and2_id, target_port=0):
            return False
        if not await self.connect(reset_not_id, we_and2_id, target_port=1):
            return False
        # OR: AND2 output OR RegProgWrite
        if not await self.connect(we_and2_id, we_or_id, target_port=0):
            return False
        if not await self.connect(reg_prog_write_id, we_or_id, target_port=1):
            return False
        if not await self.connect(we_or_id, regfile_id, target_port_label="Write_Enable"):
            return False

        await self.log("  ✓ Wired register file write ports with programming mux")

        # ==== CONNECT EXECUTE → MEMORY ====
        for i in range(8):
            if not await self.connect(execute_id, memory_id, source_port_label=f"Result[{i}]", target_port_label=f"Result[{i}]"):
                return False

        if not await self.connect(decode_id, memory_id, source_port_label="MemRead", target_port_label="MemRead"):
            return False
        if not await self.connect(decode_id, memory_id, source_port_label="MemWrite", target_port_label="MemWrite"):
            return False

        # Memory Address: RawInstr[0-2] for lower 3 bits, GND for upper 5
        for i in range(3):
            if not await self.connect(fetch_id, memory_id, source_port_label=f"RawInstr[{i}]", target_port_label=f"Address[{i}]"):
                return False
        for i in range(3, 8):
            if not await self.connect(gnd_id, memory_id, target_port_label=f"Address[{i}]"):
                return False

        # Memory DataIn: R0 value (accumulator) for STORE operations
        for i in range(8):
            if not await self.connect(regfile_id, memory_id, source_port_label=f"Read_Data1[{i}]", target_port_label=f"DataIn[{i}]"):
                return False

        await self.log("  ✓ Connected Execute and Memory stages")

        # ==== CONNECT MEMORY WRITEBACK → REGISTER FILE ====
        # Memory DataOut → Write Data Mux In0 (normal writeback path)
        for i in range(8):
            if not await self.connect(memory_id, write_data_mux_id, source_port_label=f"DataOut[{i}]", target_port_label=f"In0[{i}]"):
                return False
        # Mux output → Register File Data_In
        for i in range(8):
            if not await self.connect(write_data_mux_id, regfile_id, source_port_label=f"Out[{i}]", target_port_label=f"Data_In[{i}]"):
                return False

        await self.log("  ✓ Connected Memory writeback to Register File")

        # ---- Create output LEDs for external visibility ----
        output_x = stage_x_offsets[3] + HORIZONTAL_GATE_SPACING

        # PC outputs
        pc_outputs = []
        for i in range(8):
            led_id = await self.create_element("Led", output_x, stage_y + (i * 50.0), f"PC[{i}]")
            if led_id is None:
                return False
            pc_outputs.append(led_id)

            if not await self.connect(fetch_id, led_id, source_port_label=f"PC[{i}]"):
                return False

        await self.log("  ✓ Created PC outputs")

        # Result outputs
        result_outputs = []
        for i in range(8):
            led_id = await self.create_element("Led", output_x, 500.0 + (i * 50.0), f"Result[{i}]")
            if led_id is None:
                return False
            result_outputs.append(led_id)

            if not await self.connect(memory_id, led_id, source_port_label=f"DataOut[{i}]"):
                return False

        await self.log("  ✓ Created Result outputs")

        # Instruction outputs (from Fetch stage)
        for i in range(8):
            led_id = await self.create_element("Led", output_x + 100.0, stage_y + (i * 50.0), f"Instruction[{i}]")
            if led_id is None:
                return False

            if not await self.connect(fetch_id, led_id, source_port_label=f"Instruction[{i}]"):
                return False

        await self.log("  ✓ Created Instruction outputs")

        # Zero flag output (from Execute stage)
        zero_led_id = await self.create_element("Led", output_x + 200.0, stage_y, "Zero")
        if zero_led_id is None:
            return False

        if not await self.connect(execute_id, zero_led_id, source_port_label="Zero"):
            return False

        # Sign flag output (from Execute stage)
        sign_led_id = await self.create_element("Led", output_x + 200.0, stage_y + 50.0, "Sign")
        if sign_led_id is None:
            return False

        if not await self.connect(execute_id, sign_led_id, source_port_label="Sign"):
            return False

        await self.log("  ✓ Created Zero and Sign flag outputs")

        output_file = str(IC_COMPONENTS_DIR / "level9_single_cycle_cpu_8bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 8-bit Single-Cycle CPU IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = CPU8BitSingleCycleBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "8-bit Single-Cycle CPU IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
