#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 8-bit Single-Cycle CPU IC

Implements a complete 8-bit single-cycle CPU with 4 combinational datapath
stages integrated (no pipeline — one instruction completes per clock):
- Fetch Stage: Instruction retrieval and program counter management
- Decode Stage: Opcode to control signal conversion
- Execute Stage: ALU operations with 8 supported operations
- Memory Stage: Memory read/write and result pass-through
- Register File: 8 general-purpose 8-bit registers

Inputs:
  Clock (synchronization signal)
  Reset (initialize program counter to 0 — must be held asserted for the whole
         instruction-memory programming window, not just pulsed before it: PCInc/
         InstrLoad are unconditionally tied to Vcc, so every clock pulse used to
         write a word would otherwise also advance the fetch PC. Reset's async
         override on the PC/instruction register is what prevents that drift —
         the same mechanism level9_multi_cycle_cpu_8bit documents for its own
         phase counter, applied directly here since there's no phase counter)
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
    python create_level9_single_cycle_cpu_8bit.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder
from ic_builder_helpers import build_cpu_register_programming_block

# Layout is organized into vertically-stacked bands, each on its own row (or
# pair of rows). Within a band, elements are chained left-to-right using
# their real width (queried via instantiate_ic_with_size) where the element
# is an embedded IC. Gaps between bands are likewise derived from real
# content height (queried the same way, or the known 64px raw-gate row
# height where a band has no embedded IC), not a flat worst-case constant --
# each band's start can only be computed once the band above it has actually
# been placed and its real height learned.
RAW_ROW_HEIGHT = 64.0

# The stage ICs (Fetch/Decode/Execute/Memory) are placed with many ports
# spread symmetrically above and below their nominal y, not a simple
# top-anchored 64px box -- level8_decode_stage, the tallest, is ~602px real
# height. The gap immediately above y_stages can't be derived from a query
# (nothing is instantiated there yet), so use half that documented worst
# case plus margin; every other gap in this file is a real measurement.
STAGE_CLEARANCE_ABOVE = (602.0 / 2) + VERTICAL_STAGE_SPACING


class CPU8BitSingleCycleBuilder(ICBuilderBase):
    """Builder for 8-bit Single-Cycle CPU IC"""

    async def create(self) -> bool:
        """Create the 8-bit Single-Cycle CPU IC"""
        await self.begin_build("8-bit Single-Cycle CPU")
        if not await self.create_new_circuit():
            return False

        base_x = 50.0

        # ---- Band Y anchors (top to bottom): control and logic are both a
        # single raw-gate row (known 64px height), so their gaps are flat;
        # y_stages' gap to what follows depends on the stage ICs' real
        # height, learned once they're instantiated below. ----
        y_control = 100.0
        y_logic = y_control + RAW_ROW_HEIGHT + VERTICAL_STAGE_SPACING
        y_stages = y_logic + RAW_ROW_HEIGHT + STAGE_CLEARANCE_ABOVE

        # ==== Band: Clock/Reset/Vcc/Gnd (boundary control inputs) ====
        clock_id = await self.create_element("Clock", base_x, y_control, "Clock")
        if clock_id is None:
            return False

        reset_id = await self.create_element("InputSwitch", base_x + HORIZONTAL_GATE_SPACING, y_control, "Reset")
        if reset_id is None:
            return False

        vcc_id = await self.create_element("InputVcc", base_x + (2 * HORIZONTAL_GATE_SPACING), y_control, "Vcc")
        if vcc_id is None:
            return False

        gnd_id = await self.create_element("InputGnd", base_x + (3 * HORIZONTAL_GATE_SPACING), y_control, "Gnd")
        if gnd_id is None:
            return False

        await self.log("  ✓ Created Clock, Reset, Vcc, and Gnd inputs")

        # ==== Band: register-file write-enable logic (not boundary elements
        # — order among them doesn't affect the IC's port order). Labels here
        # (ProgWriteNOT, WriteEnableAND1/2, WriteEnableOR) are long enough
        # that the standard 1x step isn't enough clearance -- widen the
        # whole row's step. ====
        logic_row_step = 1.5 * HORIZONTAL_GATE_SPACING
        prog_not_id = await self.create_element("Not", base_x, y_logic, "ProgWriteNOT")
        if prog_not_id is None:
            return False
        reset_not_id = await self.create_element("Not", base_x + logic_row_step, y_logic, "ResetNOT")
        if reset_not_id is None:
            return False
        we_and1_id = await self.create_element("And", base_x + (2 * logic_row_step), y_logic, "WriteEnableAND1")
        if we_and1_id is None:
            return False
        we_and2_x = base_x + (3 * logic_row_step)
        we_and2_id = await self.create_element("And", we_and2_x, y_logic, "WriteEnableAND2")
        if we_and2_id is None:
            return False
        we_or_id = await self.create_element("Or", we_and2_x + logic_row_step, y_logic, "WriteEnableOR")
        if we_or_id is None:
            return False

        await self.log("  ✓ Created register file write-enable logic")

        # ==== Band: pipeline stage ICs, chained left to right by real width ====
        fetch_x = base_x
        fetch_handle = await self.instantiate_ic_with_size("level8_fetch_stage", fetch_x, y_stages, "Fetch")
        if fetch_handle is None:
            return False
        fetch_id = fetch_handle.element_id

        decode_x = fetch_x + max(HORIZONTAL_GATE_SPACING * 2, fetch_handle.width + HORIZONTAL_GATE_SPACING)
        decode_handle = await self.instantiate_ic_with_size("level8_decode_stage", decode_x, y_stages, "Decode")
        if decode_handle is None:
            return False
        decode_id = decode_handle.element_id

        execute_x = decode_x + max(HORIZONTAL_GATE_SPACING * 2, decode_handle.width + HORIZONTAL_GATE_SPACING)
        execute_handle = await self.instantiate_ic_with_size("level8_execute_stage", execute_x, y_stages, "Execute")
        if execute_handle is None:
            return False
        execute_id = execute_handle.element_id

        memory_x = execute_x + max(HORIZONTAL_GATE_SPACING * 2, execute_handle.width + HORIZONTAL_GATE_SPACING)
        memory_handle = await self.instantiate_ic_with_size("level8_memory_stage", memory_x, y_stages, "Memory")
        if memory_handle is None:
            return False
        memory_id = memory_handle.element_id

        await self.log("  ✓ Instantiated Fetch, Decode, Execute, and Memory stages")

        stages_height = max(fetch_handle.height, decode_handle.height, execute_handle.height, memory_handle.height)
        y_regfile = y_stages + stages_height + VERTICAL_STAGE_SPACING

        # ==== Band: register file + its programming muxes, instruction/register
        # programming inputs (shared with the multi-cycle CPU generator) ====
        block = await build_cpu_register_programming_block(
            self, base_x, y_regfile, regfile_name="level6_register_file_8x8", bus_mux_name="level4_bus_mux_8bit"
        )
        if block is None:
            return False
        regfile_id = block.regfile_id
        write_data_mux_id = block.write_data_mux_id
        write_addr_mux_ids = block.write_addr_mux_ids
        prog_addr_inputs = block.prog_addr_inputs
        prog_data_inputs = block.prog_data_inputs
        prog_write_id = block.prog_write_id
        reg_prog_addr_inputs = block.reg_prog_addr_inputs
        reg_prog_data_inputs = block.reg_prog_data_inputs
        reg_prog_write_id = block.reg_prog_write_id
        y_output = block.next_y

        # ==== Band: output LEDs ====
        pc_out_x = base_x
        result_out_x = pc_out_x + HORIZONTAL_GATE_SPACING
        instr_out_x = result_out_x + HORIZONTAL_GATE_SPACING
        # Instruction[i]'s label is long enough that the standard 1x step
        # into the Zero/Sign flag column isn't enough clearance.
        flags_out_x = instr_out_x + 1.5 * HORIZONTAL_GATE_SPACING

        pc_led_ids = []
        for i in range(8):
            led_id = await self.create_element("Led", pc_out_x, y_output + (i * VERTICAL_STAGE_SPACING), f"PC[{i}]")
            if led_id is None:
                return False
            pc_led_ids.append(led_id)

        result_led_ids = []
        for i in range(8):
            led_id = await self.create_element(
                "Led", result_out_x, y_output + (i * VERTICAL_STAGE_SPACING), f"Result[{i}]"
            )
            if led_id is None:
                return False
            result_led_ids.append(led_id)

        instr_led_ids = []
        for i in range(8):
            led_id = await self.create_element(
                "Led", instr_out_x, y_output + (i * VERTICAL_STAGE_SPACING), f"Instruction[{i}]"
            )
            if led_id is None:
                return False
            instr_led_ids.append(led_id)

        zero_led_id = await self.create_element("Led", flags_out_x, y_output, "Zero")
        if zero_led_id is None:
            return False

        sign_led_id = await self.create_element("Led", flags_out_x, y_output + VERTICAL_STAGE_SPACING, "Sign")
        if sign_led_id is None:
            return False

        await self.log("  ✓ Created output LEDs")

        # ================= Connections ================= #

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

        # Clock to the clocked stages (decode/execute are combinational and
        # no longer expose a Clock port — F33)
        if not await self.connect(clock_id, memory_id, target_port_label="Clock"):
            return False
        if not await self.connect(clock_id, regfile_id, target_port_label="Clock"):
            return False

        # Reset clears the data memory too (F54 — the memory stage's Reset
        # used to be dead; the CPU now drives it)
        if not await self.connect(reset_id, memory_id, target_port_label="Reset"):
            return False

        await self.log("  ✓ Connected Clock, Reset, and programming inputs")

        # ==== CONNECT FETCH → DECODE (using raw unregistered instruction) ====
        # RawInstr[3..7] → OpCode[0..4] (bypass instruction register for zero-delay decode)
        for i in range(5):
            if not await self.connect(
                fetch_id, decode_id, source_port_label=f"RawInstr[{i + 3}]", target_port_label=f"OpCode[{i}]"
            ):
                return False
        await self.log("  ✓ Connected Fetch to Decode (via RawInstr)")

        # ==== CONNECT DECODE → EXECUTE ====
        for i in range(3):
            if not await self.connect(
                decode_id, execute_id, source_port_label=f"ALUOp[{i}]", target_port_label=f"ALUOp[{i}]"
            ):
                return False
        await self.log("  ✓ Connected Decode to Execute")

        # ==== WIRE REGISTER FILE ====
        # Read_Addr1 = GND (always read R0 as accumulator → OperandA)
        for i in range(3):
            if not await self.connect(gnd_id, regfile_id, target_port_label=f"Read_Addr1[{i}]"):
                return False

        # Read_Addr2 = RawInstr[0..2] (instruction-addressed register → OperandB)
        for i in range(3):
            if not await self.connect(
                fetch_id, regfile_id, source_port_label=f"RawInstr[{i}]", target_port_label=f"Read_Addr2[{i}]"
            ):
                return False

        # Connect register file reads to Execute stage operands
        for i in range(8):
            if not await self.connect(
                regfile_id, execute_id, source_port_label=f"Read_Data1[{i}]", target_port_label=f"OperandA[{i}]"
            ):
                return False
            if not await self.connect(
                regfile_id, execute_id, source_port_label=f"Read_Data2[{i}]", target_port_label=f"OperandB[{i}]"
            ):
                return False

        await self.log("  ✓ Wired register file read ports to Execute stage")

        # Write address mux: In0=GND (normal: write to R0), In1=RegProgAddr, Sel=RegProgWrite
        for i in range(3):
            if not await self.connect(gnd_id, write_addr_mux_ids[i], target_port=0):  # In0 = GND
                return False
            if not await self.connect(
                reg_prog_addr_inputs[i], write_addr_mux_ids[i], target_port=1
            ):  # In1 = RegProgAddr
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
        if not await self.connect(we_or_id, regfile_id, target_port_label="WriteEnable"):
            return False

        await self.log("  ✓ Wired register file write ports with programming mux")

        # ==== CONNECT EXECUTE → MEMORY ====
        for i in range(8):
            if not await self.connect(
                execute_id, memory_id, source_port_label=f"Result[{i}]", target_port_label=f"Result[{i}]"
            ):
                return False

        if not await self.connect(decode_id, memory_id, source_port_label="MemRead", target_port_label="MemRead"):
            return False
        if not await self.connect(decode_id, memory_id, source_port_label="MemWrite", target_port_label="MemWrite"):
            return False

        # Memory Address: RawInstr[0-2] for lower 3 bits, GND for upper 5
        for i in range(3):
            if not await self.connect(
                fetch_id, memory_id, source_port_label=f"RawInstr[{i}]", target_port_label=f"Address[{i}]"
            ):
                return False
        for i in range(3, 8):
            if not await self.connect(gnd_id, memory_id, target_port_label=f"Address[{i}]"):
                return False

        # Memory DataIn: R0 value (accumulator) for STORE operations
        for i in range(8):
            if not await self.connect(
                regfile_id, memory_id, source_port_label=f"Read_Data1[{i}]", target_port_label=f"DataIn[{i}]"
            ):
                return False

        await self.log("  ✓ Connected Execute and Memory stages")

        # ==== CONNECT MEMORY WRITEBACK → REGISTER FILE ====
        # Memory DataOut → Write Data Mux In0 (normal writeback path)
        for i in range(8):
            if not await self.connect(
                memory_id, write_data_mux_id, source_port_label=f"DataOut[{i}]", target_port_label=f"In0[{i}]"
            ):
                return False
        # Mux output → Register File Data_In
        for i in range(8):
            if not await self.connect(
                write_data_mux_id, regfile_id, source_port_label=f"Out[{i}]", target_port_label=f"Data_In[{i}]"
            ):
                return False

        await self.log("  ✓ Connected Memory writeback to Register File")

        # ---- Connect output LEDs ----
        for i in range(8):
            if not await self.connect(fetch_id, pc_led_ids[i], source_port_label=f"PC[{i}]"):
                return False
        await self.log("  ✓ Created PC outputs")

        for i in range(8):
            if not await self.connect(memory_id, result_led_ids[i], source_port_label=f"DataOut[{i}]"):
                return False
        await self.log("  ✓ Created Result outputs")

        for i in range(8):
            if not await self.connect(fetch_id, instr_led_ids[i], source_port_label=f"Instruction[{i}]"):
                return False
        await self.log("  ✓ Created Instruction outputs")

        if not await self.connect(execute_id, zero_led_id, source_port_label="Zero"):
            return False
        if not await self.connect(execute_id, sign_led_id, source_port_label="Sign"):
            return False
        await self.log("  ✓ Created Zero and Sign flag outputs")

        output_file = str(IC_COMPONENTS_DIR / "level9_single_cycle_cpu_8bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 8-bit Single-Cycle CPU IC "
            f"({self.element_count} elements, {self.connection_count} connections)"
        )
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
