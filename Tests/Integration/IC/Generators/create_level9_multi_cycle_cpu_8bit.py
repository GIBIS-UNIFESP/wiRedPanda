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
    python create_level9_cpu_8bit_multi_cycle.py
"""

import asyncio
from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING


class CPU8BitMultiCycleBuilder(ICBuilderBase):
    """Builder for 8-bit Multi-Cycle CPU IC"""

    async def create(self) -> bool:
        """Create the 8-bit Multi-Cycle CPU IC"""
        await self.begin_build('8-bit Multi-Cycle CPU')
        if not await self.create_new_circuit():
            return False

        # Layout positions
        stage_x_offsets = [50.0, 300.0, 550.0, 800.0]
        stage_y = 100.0
        counter_y = 50.0
        reg_file_y = 500.0

        # ---- Create Clock and Reset inputs ----
        control_x = 50.0
        control_y = counter_y

        clock_id = await self.create_element("Clock", control_x, control_y, "Clock")
        if clock_id is None:
            return False

        reset_id = await self.create_element("InputSwitch", control_x + HORIZONTAL_GATE_SPACING, control_y, "Reset")
        if reset_id is None:
            return False

        await self.log("  ✓ Created Clock and Reset inputs")

        # ---- Create instruction memory programming inputs ----
        # Tests hold Reset (which freezes the cycle counter at phase 0, so the
        # fetch stage's gated clock passes the full clock) and pulse the clock to
        # write instruction memory at ProgAddr.
        prog_x = 50.0
        prog_y = control_y + 600.0

        prog_addr_inputs = []
        for i in range(8):
            pid = await self.create_element("InputSwitch", prog_x + (i * HORIZONTAL_GATE_SPACING), prog_y, f"ProgAddr[{i}]")
            if pid is None:
                return False
            prog_addr_inputs.append(pid)

        prog_data_inputs = []
        for i in range(8):
            pid = await self.create_element("InputSwitch", prog_x + (i * HORIZONTAL_GATE_SPACING), prog_y + 50.0, f"ProgData[{i}]")
            if pid is None:
                return False
            prog_data_inputs.append(pid)

        prog_write_id = await self.create_element("InputSwitch", prog_x + (8 * HORIZONTAL_GATE_SPACING), prog_y, "ProgWrite")
        if prog_write_id is None:
            return False

        await self.log("  ✓ Created instruction memory programming inputs")

        # ---- Create register file programming inputs ----
        reg_prog_y = prog_y + 150.0

        reg_prog_addr_inputs = []
        for i in range(3):
            pid = await self.create_element("InputSwitch", prog_x + (i * HORIZONTAL_GATE_SPACING), reg_prog_y, f"RegProgAddr[{i}]")
            if pid is None:
                return False
            reg_prog_addr_inputs.append(pid)

        reg_prog_data_inputs = []
        for i in range(8):
            pid = await self.create_element("InputSwitch", prog_x + (i * HORIZONTAL_GATE_SPACING), reg_prog_y + 50.0, f"RegProgData[{i}]")
            if pid is None:
                return False
            reg_prog_data_inputs.append(pid)

        reg_prog_write_id = await self.create_element("InputSwitch", prog_x + (8 * HORIZONTAL_GATE_SPACING), reg_prog_y, "RegProgWrite")
        if reg_prog_write_id is None:
            return False

        await self.log("  ✓ Created register file programming inputs")

        # ---- Create 2-bit Cycle Counter (using 2 D flip-flops) ----
        # This counter sequences through 4 cycles: 00->01->10->11->00
        counter_ids = []
        for i in range(2):
            ff_id = await self.create_element("DFlipFlop", 150.0 + (i * HORIZONTAL_GATE_SPACING / 2), counter_y, f"CycleFF[{i}]")
            if ff_id is None:
                return False
            counter_ids.append(ff_id)

        await self.log("  ✓ Created 2-bit cycle counter")

        # ---- Create Gnd for default control signals (held-low inputs / mux In0) ----
        gnd_id = await self.create_element("InputGnd", 150.0, stage_y - 50.0, "Gnd")
        if gnd_id is None:
            return False

        await self.log("  ✓ Created Gnd for control signals")

        # ---- Instantiate Fetch Stage ----
        fetch_id = await self.instantiate_ic("level8_fetch_stage", stage_x_offsets[0], stage_y, "Fetch")
        if fetch_id is None:
            return False
        await self.log("  ✓ Instantiated Fetch Stage")

        # ---- Instantiate Decode Stage ----
        decode_id = await self.instantiate_ic("level8_decode_stage", stage_x_offsets[1], stage_y, "Decode")
        if decode_id is None:
            return False
        await self.log("  ✓ Instantiated Decode Stage")

        # ---- Instantiate Execute Stage ----
        execute_id = await self.instantiate_ic("level8_execute_stage", stage_x_offsets[2], stage_y, "Execute")
        if execute_id is None:
            return False
        await self.log("  ✓ Instantiated Execute Stage")

        # ---- Instantiate Memory Stage ----
        memory_id = await self.instantiate_ic("level8_memory_stage", stage_x_offsets[3], stage_y, "Memory")
        if memory_id is None:
            return False
        await self.log("  ✓ Instantiated Memory Stage")

        # ---- Instantiate Register File ----
        regfile_id = await self.instantiate_ic("level6_register_file_8x8", 425.0, reg_file_y, "RegFile")
        if regfile_id is None:
            return False
        await self.log("  ✓ Instantiated Register File")

        # ---- Instantiate register file programming muxes (mirrors single-cycle) ----
        # Write data mux: In0=Memory DataOut (normal writeback), In1=RegProgData
        write_data_mux_id = await self.instantiate_ic("level4_bus_mux_8bit", 425.0, reg_file_y - 150.0, "WriteDataMux")
        if write_data_mux_id is None:
            return False

        # Write address muxes (3 individual Mux elements for the 3-bit address)
        write_addr_mux_ids = []
        for i in range(3):
            mux_id = await self.create_element("Mux", 350.0 + (i * HORIZONTAL_GATE_SPACING), reg_file_y - 150.0, f"WriteAddrMux{i}")
            if mux_id is None:
                return False
            write_addr_mux_ids.append(mux_id)

        await self.log("  ✓ Instantiated register file programming muxes")

        # ---- Connect Clock to cycle counter flip-flops ----
        for i, ff_id in enumerate(counter_ids):
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

        # Counter-bit inverters (also feed the FF0 toggle further below).
        not_q0_id = await self.create_element("Not", 200.0, counter_y, "NotQ0")
        if not_q0_id is None:
            return False
        not_q1_id = await self.create_element("Not", 250.0, counter_y, "NotQ1")
        if not_q1_id is None:
            return False
        if not await self.connect(counter_ids[0], not_q0_id, source_port_label="Q"):
            return False
        if not await self.connect(counter_ids[1], not_q1_id, source_port_label="Q"):
            return False

        # Phase 0 (Fetch) enable = NOT Q1 AND NOT Q0
        phase0_id = await self.create_element("And", 300.0, counter_y, "Phase0")
        if phase0_id is None:
            return False
        if not await self.connect(not_q1_id, phase0_id, target_port=0):
            return False
        if not await self.connect(not_q0_id, phase0_id, target_port=1):
            return False

        # Phase 3 (Memory write / register write-back) enable = Q1 AND Q0
        phase3_id = await self.create_element("And", 340.0, counter_y, "Phase3")
        if phase3_id is None:
            return False
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
        reset_not_id = await self.create_element("Not", 100.0, counter_y + 50.0, "ResetNot")
        if reset_not_id is None:
            return False

        # Connect reset to NOT gate
        if not await self.connect(reset_id, reset_not_id):
            return False

        # ---- Connect Reset signals ----
        # Reset both cycle counter flip-flops using ~Clear port (active-low)
        for i, ff_id in enumerate(counter_ids):
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
            if not await self.connect(fetch_id, decode_id, source_port_label=f"OpCode[{i}]", target_port_label=f"OpCode[{i}]"):
                return False

        await self.log("  ✓ Connected Fetch to Decode")

        # ---- Connect Decode to Execute (ALUOp pass-through) ----
        for i in range(3):
            if not await self.connect(decode_id, execute_id, source_port_label=f"ALUOp[{i}]", target_port_label=f"ALUOp[{i}]"):
                return False

        await self.log("  ✓ Connected Decode to Execute")

        # ---- Connect Execute to Memory (Result pass-through) ----
        for i in range(8):
            if not await self.connect(execute_id, memory_id, source_port_label=f"Result[{i}]", target_port_label=f"Result[{i}]"):
                return False

        # MemRead and MemWrite from Decode to Memory
        if not await self.connect(decode_id, memory_id, source_port_label="MemRead", target_port_label="MemRead"):
            return False

        # MemWrite is gated by Phase 3: the memory stage runs on the main clock
        # now, so the phase restriction moves from the clock to the write enable
        # — the data memory only writes during the memory phase. (MemRead stays
        # ungated: reads are combinational and select the output mux every cycle.)
        memwrite_gate_id = await self.create_element("And", 880.0, stage_y, "MemWriteGate")
        if memwrite_gate_id is None:
            return False
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
            if not await self.connect(fetch_id, regfile_id, source_port_label=f"Instruction[{i}]", target_port_label=f"Read_Addr2[{i}]"):
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
            if not await self.connect(regfile_id, execute_id, source_port_label=f"Read_Data1[{i}]", target_port_label=f"OperandA[{i}]"):
                return False
            if not await self.connect(regfile_id, execute_id, source_port_label=f"Read_Data2[{i}]", target_port_label=f"OperandB[{i}]"):
                return False

        # Memory address comes from the REGISTERED instruction's register field
        # (same reasoning as Read_Addr2 above); data to store is the accumulator
        # (Read_Data1).
        for i in range(3):
            if not await self.connect(fetch_id, memory_id, source_port_label=f"Instruction[{i}]", target_port_label=f"Address[{i}]"):
                return False
        for i in range(3, 8):
            if not await self.connect(gnd_id, memory_id, target_port_label=f"Address[{i}]"):
                return False
        for i in range(8):
            if not await self.connect(regfile_id, memory_id, source_port_label=f"Read_Data1[{i}]", target_port_label=f"DataIn[{i}]"):
                return False

        # Writeback path through the write-data mux: In0=Memory DataOut (normal
        # write-back into R0 in the memory phase), In1=RegProgData (programming),
        # Sel=RegProgWrite → regfile Data_In.
        for i in range(8):
            if not await self.connect(memory_id, write_data_mux_id, source_port_label=f"DataOut[{i}]", target_port_label=f"In0[{i}]"):
                return False
            if not await self.connect(reg_prog_data_inputs[i], write_data_mux_id, target_port_label=f"In1[{i}]"):
                return False
        if not await self.connect(reg_prog_write_id, write_data_mux_id, target_port_label="Sel"):
            return False
        for i in range(8):
            if not await self.connect(write_data_mux_id, regfile_id, source_port_label=f"Out[{i}]", target_port_label=f"Data_In[{i}]"):
                return False

        # phase3_id (Q1 AND Q0) was created with the phase-decode enables above.
        we_and_id = await self.create_element("And", 420.0, counter_y, "WriteBackEnable")
        if we_and_id is None:
            return False
        if not await self.connect(decode_id, we_and_id, source_port_label="RegWrite"):
            return False
        if not await self.connect(phase3_id, we_and_id, target_port=1):
            return False

        # Write enable = normal write-back OR register programming. During
        # programming the test holds Reset (counter frozen at phase 0 → Phase3=0),
        # so the normal AND term is gated off and only RegProgWrite drives writes.
        we_or_id = await self.create_element("Or", 460.0, counter_y, "WriteEnableOR")
        if we_or_id is None:
            return False
        if not await self.connect(we_and_id, we_or_id, target_port=0):
            return False
        if not await self.connect(reg_prog_write_id, we_or_id, target_port=1):
            return False
        if not await self.connect(we_or_id, regfile_id, target_port_label="Write_Enable"):
            return False

        await self.log("  ✓ Connected datapath and control signals")

        # ---- Create output LEDs ----
        output_x = stage_x_offsets[3] + HORIZONTAL_GATE_SPACING

        # PC outputs
        for i in range(8):
            led_id = await self.create_element("Led", output_x, stage_y + (i * 50.0), f"PC[{i}]")
            if led_id is None:
                return False

            if not await self.connect(fetch_id, led_id, source_port_label=f"PC[{i}]"):
                return False

        # Result outputs
        for i in range(8):
            led_id = await self.create_element("Led", output_x, 500.0 + (i * 50.0), f"Result[{i}]")
            if led_id is None:
                return False

            if not await self.connect(memory_id, led_id, source_port_label=f"DataOut[{i}]"):
                return False

        # Cycle counter outputs (connect flip-flop Q outputs)
        for i in range(2):
            led_id = await self.create_element("Led", output_x + 100.0, counter_y + (i * 50.0), f"CycleCounter[{i}]")
            if led_id is None:
                return False

            if not await self.connect(counter_ids[i], led_id, source_port_label="Q"):
                return False

        # Instruction outputs (from Fetch stage)
        for i in range(8):
            led_id = await self.create_element("Led", output_x + 200.0, stage_y + (i * 50.0), f"Instruction[{i}]")
            if led_id is None:
                return False

            if not await self.connect(fetch_id, led_id, source_port_label=f"Instruction[{i}]"):
                return False

        await self.log("  ✓ Created output LEDs")

        # ---- Connect cycle counter feedback (simple ripple) ----
        # FF0 toggles each cycle: reuse the existing NotQ0 gate from the
        # clock gating (a duplicate "NotQ0" used to be created here).
        if not await self.connect(not_q0_id, counter_ids[0], target_port_label="Data"):
            return False

        # For FF1, use XOR(Q0, Q1) for ripple counting
        xor_id = await self.create_element("Xor", 250.0, counter_y, "XorFF")
        if xor_id is None:
            return False

        # Connect Q0 and Q1 to XOR
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

        await self.log(f"✅ Successfully created 8-bit Multi-Cycle CPU IC ({self.element_count} elements, {self.connection_count} connections)")
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
