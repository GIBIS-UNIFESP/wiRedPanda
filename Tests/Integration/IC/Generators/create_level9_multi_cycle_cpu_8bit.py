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
  Reset (initialize to fetch cycle)

Outputs:
  PC[0-7] (program counter value)
  Result[0-7] (final execution result)
  CycleCounter[0-1] (current execution phase: 0=Fetch, 1=Decode, 2=Execute, 3=Memory)
  Instruction[0-7] (current instruction being fetched)

Architecture:
  - 2-bit cycle counter for 4-cycle execution sequence
  - Each stage instantiated separately; decode/execute are purely
    combinational (no clock), only fetch/memory/regfile are clocked,
    each gated to its own phase of the cycle counter
  - Register file provides operands (Read_Addr2 = RawInstr[0..2]);
    execute stage computes on them; memory stage is addressed by
    RawInstr[0..2]; write-back to the register file is gated by
    AND(RegWrite, Phase3)

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

        # ---- Create 2-bit Cycle Counter (using 2 D flip-flops) ----
        # This counter sequences through 4 cycles: 00->01->10->11->00
        counter_ids = []
        for i in range(2):
            ff_id = await self.create_element("DFlipFlop", 150.0 + (i * HORIZONTAL_GATE_SPACING / 2), counter_y, f"CycleFF[{i}]")
            if ff_id is None:
                return False
            counter_ids.append(ff_id)

        await self.log("  ✓ Created 2-bit cycle counter")

        # ---- Create Vcc and Gnd for default control signals ----
        vcc_id = await self.create_element("InputVcc", 100.0, stage_y - 50.0, "Vcc")
        if vcc_id is None:
            return False

        gnd_id = await self.create_element("InputGnd", 150.0, stage_y - 50.0, "Gnd")
        if gnd_id is None:
            return False

        await self.log("  ✓ Created Vcc and Gnd for control signals")

        # ---- Instantiate Fetch Stage ----
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level8_fetch_stage")):

            return False

        fetch_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level8_fetch_stage"), stage_x_offsets[0], stage_y, "Fetch")
        if fetch_id is None:
            return False
        await self.log("  ✓ Instantiated Fetch Stage")

        # ---- Instantiate Decode Stage ----
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level8_decode_stage")):

            return False

        decode_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level8_decode_stage"), stage_x_offsets[1], stage_y, "Decode")
        if decode_id is None:
            return False
        await self.log("  ✓ Instantiated Decode Stage")

        # ---- Instantiate Execute Stage ----
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level8_execute_stage")):

            return False

        execute_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level8_execute_stage"), stage_x_offsets[2], stage_y, "Execute")
        if execute_id is None:
            return False
        await self.log("  ✓ Instantiated Execute Stage")

        # ---- Instantiate Memory Stage ----
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

        # ---- Connect Clock to cycle counter flip-flops ----
        for i, ff_id in enumerate(counter_ids):
            if not await self.connect(clock_id, ff_id, target_port_label="Clock"):
                return False

        await self.log("  ✓ Connected Clock to cycle counter")

        # ---- Create clock gating logic for each stage ----
        # For proper multi-cycle operation, each stage gets clock only on its designated cycle:
        # Cycle 0 (Fetch):   00 -> NOT Q1 AND NOT Q0
        # Cycle 1 (Decode):  01 -> NOT Q1 AND Q0
        # Cycle 2 (Execute): 10 -> Q1 AND NOT Q0
        # Cycle 3 (Memory):  11 -> Q1 AND Q0

        # Create NOT gates for counter bits
        not_q0_id = await self.create_element("Not", 200.0, counter_y, "NotQ0")
        if not_q0_id is None:
            return False

        not_q1_id = await self.create_element("Not", 250.0, counter_y, "NotQ1")
        if not_q1_id is None:
            return False

        # Connect counter outputs to NOT gates
        if not await self.connect(counter_ids[0], not_q0_id, source_port_label="Q"):
            return False

        if not await self.connect(counter_ids[1], not_q1_id, source_port_label="Q"):
            return False

        await self.log("  ✓ Created clock gate inverters")

        # ---- Create 3-input AND gates for clock gating ----
        # Gate0 (Fetch):   Clock AND NOT Q1 AND NOT Q0
        # Gate1 (Decode):  Clock AND NOT Q1 AND Q0
        # Gate2 (Execute): Clock AND Q1 AND NOT Q0
        # Gate3 (Memory):  Clock AND Q1 AND Q0

        gated_clocks = []
        gate_configs = [
            # (stage_name, input1, input2, input3, use_q_port_for_in2, use_q_port_for_in3)
            ("Fetch", clock_id, not_q1_id, not_q0_id, False, False),
            ("Decode", clock_id, not_q1_id, counter_ids[0], False, True),
            ("Execute", clock_id, counter_ids[1], not_q0_id, True, False),
            ("Memory", clock_id, counter_ids[1], counter_ids[0], True, True),
        ]

        for i, (label, in1, in2, in3, use_q_for_in2, use_q_for_in3) in enumerate(gate_configs):
            # Create AND gate
            and_id = await self.create_element("And", 300.0 + (i * 80.0), counter_y, f"Gate{label}")
            if and_id is None:
                return False
            gated_clocks.append(and_id)

            # Set AND gate to 3 inputs
            set_size = await self.mcp.send_command("change_input_size", {
                "element_id": and_id,
                "size": 3
            })
            if not set_size.success:
                self.log_error(f"Failed to set input_size=3 for {label} AND gate: {set_size.error}")
                return False

            # Connect three inputs to 3-input AND (ports 0, 1, 2)
            for port_num, (input_id, use_q_port) in enumerate([(in1, False), (in2, use_q_for_in2), (in3, use_q_for_in3)]):
                if use_q_port:
                    if not await self.connect(input_id, and_id, source_port_label="Q", target_port=port_num):
                        return False
                else:
                    if not await self.connect(input_id, and_id, target_port=port_num):
                        return False

        await self.log("  ✓ Created 3-input clock gate AND gates")

        # ---- Connect gated clocks to the clocked stages ----
        # Decode and execute are combinational (no Clock port — F33); fetch
        # latches on the cycle-0 gated clock and memory writes on cycle 3.
        stage_info = [
            (fetch_id, "Fetch", gated_clocks[0]),
            (memory_id, "Memory", gated_clocks[3]),
            (regfile_id, "RegFile", clock_id)  # Register file gets main clock for synchronous write
        ]

        for stage_id, _, clock_source in stage_info:
            if not await self.connect(clock_source, stage_id, target_port_label="Clock"):
                return False

        await self.log("  ✓ Connected gated clocks to pipeline stages")

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

        if not await self.connect(decode_id, memory_id, source_port_label="MemWrite", target_port_label="MemWrite"):
            return False

        await self.log("  ✓ Connected Execute to Memory")

        # ---- Connect default control signals ----
        # Fetch stage controls. InstrLoad is held high (F26: it was tied to
        # GND, so the instruction register could never load) — the fetch
        # stage's gated cycle-0 clock confines the actual latching to the
        # fetch phase.
        if not await self.connect(vcc_id, fetch_id, target_port_label="PCInc"):
            return False
        if not await self.connect(vcc_id, fetch_id, target_port_label="InstrLoad"):
            return False

        for port in ["PCLoad", "ProgWrite"]:
            if not await self.connect(gnd_id, fetch_id, target_port_label=port):
                return False

        # PCData, ProgAddr, ProgData default to 0 (must be connected to avoid floating inputs)
        for i in range(8):
            for port_prefix in ["PCData", "ProgAddr", "ProgData"]:
                if not await self.connect(gnd_id, fetch_id, target_port_label=f"{port_prefix}[{i}]"):
                    return False

        # ---- Register file datapath (F26: previously unconnected) ----
        # Accumulator model matching the single-cycle CPU: OperandA reads R0,
        # OperandB reads Register[RawInstr[2:0]], results write back to R0.
        for i in range(3):
            if not await self.connect(gnd_id, regfile_id, target_port_label=f"Read_Addr1[{i}]"):
                return False
            if not await self.connect(fetch_id, regfile_id, source_port_label=f"RawInstr[{i}]", target_port_label=f"Read_Addr2[{i}]"):
                return False
            if not await self.connect(gnd_id, regfile_id, target_port_label=f"Write_Addr[{i}]"):
                return False

        for i in range(8):
            if not await self.connect(regfile_id, execute_id, source_port_label=f"Read_Data1[{i}]", target_port_label=f"OperandA[{i}]"):
                return False
            if not await self.connect(regfile_id, execute_id, source_port_label=f"Read_Data2[{i}]", target_port_label=f"OperandB[{i}]"):
                return False

        # Memory address comes from the instruction's register field; data to
        # store is the accumulator (Read_Data1).
        for i in range(3):
            if not await self.connect(fetch_id, memory_id, source_port_label=f"RawInstr[{i}]", target_port_label=f"Address[{i}]"):
                return False
        for i in range(3, 8):
            if not await self.connect(gnd_id, memory_id, target_port_label=f"Address[{i}]"):
                return False
        for i in range(8):
            if not await self.connect(regfile_id, memory_id, source_port_label=f"Read_Data1[{i}]", target_port_label=f"DataIn[{i}]"):
                return False

        # Writeback: memory stage output (memory data or ALU pass-through)
        # into R0, enabled only in the write-back phase (cycle 3) when the
        # decoded instruction writes a register.
        for i in range(8):
            if not await self.connect(memory_id, regfile_id, source_port_label=f"DataOut[{i}]", target_port_label=f"Data_In[{i}]"):
                return False

        phase3_id = await self.create_element("And", 380.0, counter_y, "Phase3")
        if phase3_id is None:
            return False
        if not await self.connect(counter_ids[1], phase3_id, source_port_label="Q"):
            return False
        if not await self.connect(counter_ids[0], phase3_id, source_port_label="Q", target_port=1):
            return False

        we_and_id = await self.create_element("And", 420.0, counter_y, "WriteBackEnable")
        if we_and_id is None:
            return False
        if not await self.connect(decode_id, we_and_id, source_port_label="RegWrite"):
            return False
        if not await self.connect(phase3_id, we_and_id, target_port=1):
            return False
        if not await self.connect(we_and_id, regfile_id, target_port_label="Write_Enable"):
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
