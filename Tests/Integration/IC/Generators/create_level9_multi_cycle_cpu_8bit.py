#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 8-bit Multi-Cycle CPU IC

Implements an 8-bit multi-cycle CPU that sequences through execution phases:
- Cycle 1: Fetch stage (instruction retrieval)
- Cycle 2: Decode stage (control signal generation)
- Cycle 3: Execute stage (ALU operations)
- Cycle 4: Memory stage (memory read/write and write-back)

Inputs:
  Clock (main system clock)
  Reset (initialize to fetch cycle; held high during programming to freeze
         the cycle counter at phase 0)
  ProgAddr[0-7] (instruction memory programming address)
  ProgData[0-7] (instruction memory programming data)
  ProgWrite (instruction memory write enable)
  RegProgAddr[0-2] (register file programming address)
  RegProgData[0-7] (register file programming data)
  RegProgWrite (register file programming write enable)

Outputs:
  PC[0-7] (program counter value)
  Result[0-7] (final execution result)
  CycleCounter[0-1] (current execution phase: 0=Fetch, 1=Decode, 2=Execute, 3=Memory)
  Instruction[0-7] (current instruction being fetched)

Architecture:
  - 2-bit cycle counter for 4-cycle execution sequence
  - Each stage instantiated separately; decode/execute are purely
    combinational (no clock). Fetch/memory/regfile all run on the MAIN
    clock with phase-gated ENABLES (clock-enable, not gated clocks): the
    fetch PCInc/InstrLoad are gated by Phase 0 and the memory MemWrite by
    Phase 3, so a single clock drives every register
  - Register file provides operands (Read_Addr2 = Instruction[0..2], the
    REGISTERED instruction so addressing stays valid after PCInc advances
    PC during the fetch phase); execute stage computes on them; memory
    stage is addressed by Instruction[0..2]; write-back to the register
    file is enabled by OR(AND(RegWrite, Phase3), RegProgWrite)
  - Programming muxes let tests pre-load instruction memory (via ProgWrite)
    and registers (via RegProgWrite) while Reset holds the counter at phase 0

Execution Flow:
  Cycle 0 (Fetch): PC → Address, fetch instruction from memory
  Cycle 1 (Decode): Decode instruction bits to control signals
  Cycle 2 (Execute): Execute ALU operation with regfile operands
  Cycle 3 (Memory): Read/write memory, write-back result to registers

Usage:
    python create_level9_multi_cycle_cpu_8bit.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder
from ic_builder_helpers import build_cpu_register_programming_block

# Layout is organized into vertically-stacked bands, each on its own row (or
# pair of rows), separated by a fixed buffer far larger than any embedded
# IC's real height (the tallest, level8_decode_stage, is ~602px) so no
# height query can ever bridge the gap between two bands. Within a band,
# elements are chained left-to-right using their real width (queried via
# instantiate_ic_with_size) where the element is an embedded IC.
BAND_GAP = 1000.0


class CPU8BitMultiCycleBuilder(ICBuilderBase):
    """Builder for 8-bit Multi-Cycle CPU IC"""

    async def create(self) -> bool:
        """Create the 8-bit Multi-Cycle CPU IC"""
        await self.begin_build("8-bit Multi-Cycle CPU")
        if not await self.create_new_circuit():
            return False

        base_x = 50.0

        # ---- Band Y anchors (top to bottom) ----
        y_control = 100.0
        y_logic = y_control + BAND_GAP
        y_stages = y_logic + BAND_GAP
        y_regfile = y_stages + BAND_GAP
        y_prog = y_regfile + BAND_GAP
        y_regprog = y_prog + BAND_GAP
        y_output = y_regprog + BAND_GAP

        # ==== Band: Clock/Reset/Gnd (boundary control inputs) ====
        clock_id = await self.create_element("Clock", base_x, y_control, "Clock")
        if clock_id is None:
            return False

        reset_id = await self.create_element("InputSwitch", base_x + HORIZONTAL_GATE_SPACING, y_control, "Reset")
        if reset_id is None:
            return False

        gnd_id = await self.create_element("InputGnd", base_x + (2 * HORIZONTAL_GATE_SPACING), y_control, "Gnd")
        if gnd_id is None:
            return False

        await self.log("  ✓ Created Clock, Reset, and Gnd inputs")

        # ==== Band: internal control/counter logic (not boundary elements —
        # order among them doesn't affect the IC's port order) ====
        reset_not_id = await self.create_element("Not", base_x, y_logic, "ResetNot")
        if reset_not_id is None:
            return False

        counter_ids = []
        for i in range(2):
            ff_id = await self.create_element(
                "DFlipFlop", base_x + ((1 + i) * HORIZONTAL_GATE_SPACING), y_logic, f"CycleFF[{i}]"
            )
            if ff_id is None:
                return False
            counter_ids.append(ff_id)

        not_q0_id = await self.create_element("Not", base_x + (3 * HORIZONTAL_GATE_SPACING), y_logic, "NotQ0")
        if not_q0_id is None:
            return False
        not_q1_id = await self.create_element("Not", base_x + (4 * HORIZONTAL_GATE_SPACING), y_logic, "NotQ1")
        if not_q1_id is None:
            return False

        phase0_id = await self.create_element("And", base_x + (5 * HORIZONTAL_GATE_SPACING), y_logic, "Phase0")
        if phase0_id is None:
            return False

        phase3_id = await self.create_element("And", base_x + (6 * HORIZONTAL_GATE_SPACING), y_logic, "Phase3")
        if phase3_id is None:
            return False

        we_and_id = await self.create_element("And", base_x + (7 * HORIZONTAL_GATE_SPACING), y_logic, "WriteBackEnable")
        if we_and_id is None:
            return False

        we_or_id = await self.create_element("Or", base_x + (8 * HORIZONTAL_GATE_SPACING), y_logic, "WriteEnableOR")
        if we_or_id is None:
            return False

        xor_id = await self.create_element("Xor", base_x + (9 * HORIZONTAL_GATE_SPACING), y_logic, "XorFF")
        if xor_id is None:
            return False

        await self.log("  ✓ Created cycle counter and phase-decode logic")

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

        memwrite_gate_x = memory_x + max(HORIZONTAL_GATE_SPACING * 2, memory_handle.width + HORIZONTAL_GATE_SPACING)
        memwrite_gate_id = await self.create_element("And", memwrite_gate_x, y_stages, "MemWriteGate")
        if memwrite_gate_id is None:
            return False

        await self.log("  ✓ Instantiated Fetch, Decode, Execute, and Memory stages")

        # ==== Band: register file + its programming muxes, instruction/register
        # programming inputs (shared with the single-cycle CPU generator) ====
        block = await build_cpu_register_programming_block(self, base_x, y_regfile, y_prog, y_regprog)
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

        # ==== Band: output LEDs (four columns) ====
        pc_out_x = base_x
        result_out_x = pc_out_x + HORIZONTAL_GATE_SPACING
        cycle_out_x = result_out_x + HORIZONTAL_GATE_SPACING
        instr_out_x = cycle_out_x + HORIZONTAL_GATE_SPACING

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

        cycle_led_ids = []
        for i in range(2):
            led_id = await self.create_element(
                "Led", cycle_out_x, y_output + (i * VERTICAL_STAGE_SPACING), f"CycleCounter[{i}]"
            )
            if led_id is None:
                return False
            cycle_led_ids.append(led_id)

        instr_led_ids = []
        for i in range(8):
            led_id = await self.create_element(
                "Led", instr_out_x, y_output + (i * VERTICAL_STAGE_SPACING), f"Instruction[{i}]"
            )
            if led_id is None:
                return False
            instr_led_ids.append(led_id)

        await self.log("  ✓ Created output LEDs")

        # ================= Connections ================= #

        # ---- Connect Clock to cycle counter flip-flops ----
        for ff_id in counter_ids:
            if not await self.connect(clock_id, ff_id, target_port_label="Clock"):
                return False

        await self.log("  ✓ Connected Clock to cycle counter")

        # ---- Phase-decode enables (clock-ENABLE design, not clock gating) ----
        # Every sequential element runs on the MAIN clock; per-phase enables decide
        # when each stage updates. Gating the clock itself (Clock AND phase) would
        # glitch as the cycle counter transitions while the clock is high, and an
        # event-driven SystemVerilog export latches those glitches as spurious
        # edges — diverging from the cycle-based engine (e.g. an extra PC
        # increment). Clock enables are glitch-free: the enable is just a D-side
        # signal sampled at the (single, real) clock edge.
        if not await self.connect(counter_ids[0], not_q0_id, source_port_label="Q"):
            return False
        if not await self.connect(counter_ids[1], not_q1_id, source_port_label="Q"):
            return False

        # Phase 0 (Fetch) enable = NOT Q1 AND NOT Q0
        if not await self.connect(not_q1_id, phase0_id, target_port=0):
            return False
        if not await self.connect(not_q0_id, phase0_id, target_port=1):
            return False

        # Phase 3 (Memory write / register write-back) enable = Q1 AND Q0
        if not await self.connect(counter_ids[1], phase3_id, source_port_label="Q", target_port=0):
            return False
        if not await self.connect(counter_ids[0], phase3_id, source_port_label="Q", target_port=1):
            return False

        await self.log("  ✓ Created phase-decode enables (Phase0, Phase3)")

        # ---- All clocked stages run on the main clock (no gated clocks) ----
        # Decode/execute are combinational (no Clock port — F33). Fetch only
        # latches/increments when its enables are high (Phase 0); memory only
        # writes when MemWrite is high (gated by Phase 3 below); the register file
        # writes only when its Write_Enable is high.
        for stage_id in (fetch_id, memory_id, regfile_id):
            if not await self.connect(clock_id, stage_id, target_port_label="Clock"):
                return False

        await self.log("  ✓ Connected main clock to fetch, memory, and register file")

        # ---- Create NOT gate to invert reset signal (Clear is active-low) ----
        # Connect reset to NOT gate
        if not await self.connect(reset_id, reset_not_id):
            return False

        # ---- Connect Reset signals ----
        # Reset both cycle counter flip-flops using ~Clear port (active-low)
        for ff_id in counter_ids:
            if not await self.connect(reset_not_id, ff_id, target_port_label="~Clear"):
                return False

        # Reset to Fetch Stage
        if not await self.connect(reset_id, fetch_id, target_port_label="Reset"):
            return False

        # Reset clears the data memory too (F54 — the memory stage's Reset
        # used to be dead; the CPU now drives it)
        if not await self.connect(reset_id, memory_id, target_port_label="Reset"):
            return False

        await self.log("  ✓ Connected Reset signals")

        # ---- Connect Fetch to Decode (OpCode pass-through) ----
        for i in range(5):
            if not await self.connect(
                fetch_id, decode_id, source_port_label=f"OpCode[{i}]", target_port_label=f"OpCode[{i}]"
            ):
                return False

        await self.log("  ✓ Connected Fetch to Decode")

        # ---- Connect Decode to Execute (ALUOp pass-through) ----
        for i in range(3):
            if not await self.connect(
                decode_id, execute_id, source_port_label=f"ALUOp[{i}]", target_port_label=f"ALUOp[{i}]"
            ):
                return False

        await self.log("  ✓ Connected Decode to Execute")

        # ---- Connect Execute to Memory (Result pass-through) ----
        for i in range(8):
            if not await self.connect(
                execute_id, memory_id, source_port_label=f"Result[{i}]", target_port_label=f"Result[{i}]"
            ):
                return False

        # MemRead and MemWrite from Decode to Memory
        if not await self.connect(decode_id, memory_id, source_port_label="MemRead", target_port_label="MemRead"):
            return False

        # MemWrite is gated by Phase 3: the memory stage runs on the main clock
        # now, so the phase restriction moves from the clock to the write enable
        # — the data memory only writes during the memory phase. (MemRead stays
        # ungated: reads are combinational and select the output mux every cycle.)
        if not await self.connect(decode_id, memwrite_gate_id, source_port_label="MemWrite", target_port=0):
            return False
        if not await self.connect(phase3_id, memwrite_gate_id, target_port=1):
            return False
        if not await self.connect(memwrite_gate_id, memory_id, target_port_label="MemWrite"):
            return False

        await self.log("  ✓ Connected Execute to Memory")

        # ---- Connect fetch stage control enables ----
        # PCInc and InstrLoad are gated by Phase 0: the fetch stage runs on the
        # main clock, so it advances the PC and latches the instruction register
        # only during the fetch phase (clock-enable, replacing the old gated
        # clock). During programming the test holds Reset, which clears the fetch
        # PC/IR regardless of these enables.
        if not await self.connect(phase0_id, fetch_id, target_port_label="PCInc"):
            return False
        if not await self.connect(phase0_id, fetch_id, target_port_label="InstrLoad"):
            return False

        # PCLoad held low (no jumps); PCData unused. ProgWrite/ProgAddr/ProgData
        # are real programming inputs: tests hold Reset (clearing the fetch PC/IR)
        # and pulse the main clock to load instruction memory through ProgWrite.
        if not await self.connect(gnd_id, fetch_id, target_port_label="PCLoad"):
            return False
        for i in range(8):
            if not await self.connect(gnd_id, fetch_id, target_port_label=f"PCData[{i}]"):
                return False

        if not await self.connect(prog_write_id, fetch_id, target_port_label="ProgWrite"):
            return False
        for i in range(8):
            if not await self.connect(prog_addr_inputs[i], fetch_id, target_port_label=f"ProgAddr[{i}]"):
                return False
            if not await self.connect(prog_data_inputs[i], fetch_id, target_port_label=f"ProgData[{i}]"):
                return False

        # ---- Register file datapath (F26: previously unconnected) ----
        # Accumulator model: OperandA reads R0, OperandB reads
        # Register[Instruction[2:0]], results write back to R0. The operand
        # address comes from the REGISTERED instruction (not RawInstr): in the
        # multi-cycle design PCInc advances PC during the fetch phase, so by the
        # execute phase RawInstr already points at the next instruction. Decode
        # uses the registered OpCode, so operand/memory addressing must match.
        for i in range(3):
            if not await self.connect(gnd_id, regfile_id, target_port_label=f"Read_Addr1[{i}]"):
                return False
            if not await self.connect(
                fetch_id, regfile_id, source_port_label=f"Instruction[{i}]", target_port_label=f"Read_Addr2[{i}]"
            ):
                return False

        # Write address mux: In0=GND (normal: write R0 accumulator),
        # In1=RegProgAddr, Sel=RegProgWrite → regfile Write_Addr
        for i in range(3):
            if not await self.connect(gnd_id, write_addr_mux_ids[i], target_port=0):
                return False
            if not await self.connect(reg_prog_addr_inputs[i], write_addr_mux_ids[i], target_port=1):
                return False
            if not await self.connect(reg_prog_write_id, write_addr_mux_ids[i], target_port=2):
                return False
            if not await self.connect(write_addr_mux_ids[i], regfile_id, target_port_label=f"Write_Addr[{i}]"):
                return False

        for i in range(8):
            if not await self.connect(
                regfile_id, execute_id, source_port_label=f"Read_Data1[{i}]", target_port_label=f"OperandA[{i}]"
            ):
                return False
            if not await self.connect(
                regfile_id, execute_id, source_port_label=f"Read_Data2[{i}]", target_port_label=f"OperandB[{i}]"
            ):
                return False

        # Memory address comes from the REGISTERED instruction's register field
        # (same reasoning as Read_Addr2 above); data to store is the accumulator
        # (Read_Data1).
        for i in range(3):
            if not await self.connect(
                fetch_id, memory_id, source_port_label=f"Instruction[{i}]", target_port_label=f"Address[{i}]"
            ):
                return False
        for i in range(3, 8):
            if not await self.connect(gnd_id, memory_id, target_port_label=f"Address[{i}]"):
                return False
        for i in range(8):
            if not await self.connect(
                regfile_id, memory_id, source_port_label=f"Read_Data1[{i}]", target_port_label=f"DataIn[{i}]"
            ):
                return False

        # Writeback path through the write-data mux: In0=Memory DataOut (normal
        # write-back into R0 in the memory phase), In1=RegProgData (programming),
        # Sel=RegProgWrite → regfile Data_In.
        for i in range(8):
            if not await self.connect(
                memory_id, write_data_mux_id, source_port_label=f"DataOut[{i}]", target_port_label=f"In0[{i}]"
            ):
                return False
            if not await self.connect(reg_prog_data_inputs[i], write_data_mux_id, target_port_label=f"In1[{i}]"):
                return False
        if not await self.connect(reg_prog_write_id, write_data_mux_id, target_port_label="Sel"):
            return False
        for i in range(8):
            if not await self.connect(
                write_data_mux_id, regfile_id, source_port_label=f"Out[{i}]", target_port_label=f"Data_In[{i}]"
            ):
                return False

        # phase3_id (Q1 AND Q0) was created with the phase-decode enables above.
        if not await self.connect(decode_id, we_and_id, source_port_label="RegWrite"):
            return False
        if not await self.connect(phase3_id, we_and_id, target_port=1):
            return False

        # Write enable = normal write-back OR register programming. During
        # programming the test holds Reset (counter frozen at phase 0 → Phase3=0),
        # so the normal AND term is gated off and only RegProgWrite drives writes.
        if not await self.connect(we_and_id, we_or_id, target_port=0):
            return False
        if not await self.connect(reg_prog_write_id, we_or_id, target_port=1):
            return False
        if not await self.connect(we_or_id, regfile_id, target_port_label="WriteEnable"):
            return False

        await self.log("  ✓ Connected datapath and control signals")

        # ---- Connect output LEDs ----
        for i in range(8):
            if not await self.connect(fetch_id, pc_led_ids[i], source_port_label=f"PC[{i}]"):
                return False

        for i in range(8):
            if not await self.connect(memory_id, result_led_ids[i], source_port_label=f"DataOut[{i}]"):
                return False

        for i in range(2):
            if not await self.connect(counter_ids[i], cycle_led_ids[i], source_port_label="Q"):
                return False

        for i in range(8):
            if not await self.connect(fetch_id, instr_led_ids[i], source_port_label=f"Instruction[{i}]"):
                return False

        await self.log("  ✓ Connected output LEDs")

        # ---- Connect cycle counter feedback (simple ripple) ----
        # FF0 toggles each cycle: reuse the existing NotQ0 gate from the
        # clock gating (a duplicate "NotQ0" used to be created here).
        if not await self.connect(not_q0_id, counter_ids[0], target_port_label="Data"):
            return False

        # For FF1, use XOR(Q0, Q1) for ripple counting
        if not await self.connect(counter_ids[0], xor_id, source_port_label="Q"):
            return False

        if not await self.connect(counter_ids[1], xor_id, source_port_label="Q", target_port=1):
            return False

        # Connect XOR output to FF1 D input
        if not await self.connect(xor_id, counter_ids[1], target_port_label="Data"):
            return False

        await self.log("  ✓ Connected cycle counter feedback")

        output_file = str(IC_COMPONENTS_DIR / "level9_multi_cycle_cpu_8bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 8-bit Multi-Cycle CPU IC "
            f"({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = CPU8BitMultiCycleBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "8-bit Multi-Cycle CPU IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
