#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 16-bit Fetch Stage IC

Implements the fetch stage for a 16-bit RISC CPU pipeline.

Inputs:
  Clock (synchronization signal)
  Reset (reset program counter to 0)
  PCLoad (enable loading new PC value)
  PCInc (increment program counter)
  InstrLoad (enable loading instruction into register)
  PCData[0..7] (value to load into PC when PCLoad=1)

Outputs:
  PC[0..7] (program counter for memory addressing)
  Instruction[0..15] (16-bit instruction word)
  OpCode[0..4] (bits [15:11] - 5-bit operation code)
  DestReg[0..3] (bits [10:6] - 5-bit destination register)
  SrcBits[0..5] (bits [5:0] - 6-bit source/immediate bits)

Architecture:
  Composes two Level 7 instruction memory interfaces to fetch 16-bit word:
  1. Low byte fetch (Instruction[0..7])  at PC address
  2. High byte fetch (Instruction[8..15]) at PC address
  Both driven by the same PC from a shared program counter.

Data Flow:
  PC → InstrMem[0] Address → Instr[0..7]
  PC → InstrMem[1] Address → Instr[8..15]

16-bit Instruction Format:
  Bits [15:11] = OpCode (5 bits, 32 operations)
  Bits [10:6] = DestReg (5 bits, 32 registers addressable)
  Bits [5:0] = SrcBits (6 bits, 64 source/immediate modes)

Usage:
    python create_level9_fetch_stage_16bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class FetchStage16bitBuilder(ICBuilderBase):
    """Builder for 16-bit Fetch Stage IC"""

    async def create(self) -> bool:
        """Create the 16-bit Fetch Stage IC"""
        await self.begin_build('16-bit Fetch Stage')
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0
        input_y = 100.0

        # ---- Create PC data input (8-bit) ----
        pc_data_inputs = []
        for i in range(8):
            elem_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), input_y, f"PCData[{i}]")
            if elem_id is None:
                return False
            pc_data_inputs.append(elem_id)

        await self.log("  ✓ Created PCData inputs")

        # ---- Create control signal inputs ----
        control_x = input_x
        control_y = input_y + VERTICAL_STAGE_SPACING

        control_signals = {}
        for signal_name in ["Clock", "Reset", "PCLoad", "PCInc", "InstrLoad"]:
            elem_id = await self.create_element("InputSwitch" if signal_name != "Clock" else "Clock", control_x, control_y, signal_name)
            if elem_id is None:
                return False
            control_signals[signal_name] = elem_id
            control_x += HORIZONTAL_GATE_SPACING

        await self.log("  ✓ Created control signals")

        # ---- Instantiate Program Counter ----
        pc_x = 250.0
        pc_y = 100.0

        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level7_cpu_program_counter_8bit")):


            return False


        pc_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level7_cpu_program_counter_8bit"), pc_x, pc_y, "PC")
        if pc_id is None:
            return False

        # Connect PC inputs
        for i in range(8):
            if not await self.connect(pc_data_inputs[i], pc_id, target_port_label=f"Data[{i}]"):
                return False

        # Connect PC control signals
        if not await self.connect(control_signals["Clock"], pc_id, target_port_label="Clock"):
            return False

        if not await self.connect(control_signals["Reset"], pc_id, target_port_label="Reset"):
            return False

        if not await self.connect(control_signals["PCLoad"], pc_id, target_port_label="Load"):
            return False

        if not await self.connect(control_signals["PCInc"], pc_id, target_port_label="Inc"):
            return False

        await self.log("  ✓ Instantiated Program Counter")

        # ---- Instantiate two Instruction Memory interfaces (low and high bytes) ----
        instr_mem_x = 450.0
        instr_mem_low_y = 100.0
        instr_mem_high_y = 250.0

        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level7_instruction_memory_interface")):


            return False


        instr_mem_low_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level7_instruction_memory_interface"), instr_mem_x, instr_mem_low_y, "InstrMem_Low")
        if instr_mem_low_id is None:
            return False

        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level7_instruction_memory_interface")):


            return False


        instr_mem_high_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level7_instruction_memory_interface"), instr_mem_x, instr_mem_high_y, "InstrMem_High")
        if instr_mem_high_id is None:
            return False

        # Connect PC output to both instruction memory address inputs
        for i in range(8):
            if not await self.connect(pc_id, instr_mem_low_id, source_port_label=f"Address[{i}]", target_port_label=f"Address[{i}]"):
                return False

            if not await self.connect(pc_id, instr_mem_high_id, source_port_label=f"Address[{i}]", target_port_label=f"Address[{i}]"):
                return False

        await self.log("  ✓ Instantiated instruction memory interfaces")

        # Connect Clock to instruction memory interfaces
        if not await self.connect(control_signals["Clock"], instr_mem_low_id, target_port_label="Clock"):
            return False

        if not await self.connect(control_signals["Clock"], instr_mem_high_id, target_port_label="Clock"):
            return False

        await self.log("  ✓ Connected Clock to instruction memories")

        # ---- Create Output: 16-bit Instruction ----
        # Low byte (Instruction[0..7]) from low memory
        # High byte (Instruction[8..15]) from high memory
        output_x = 700.0
        output_y = 100.0

        instr_output_leds = []
        instr_mem_connections = []  # Store (mem_id, port) tuples for direct field extraction

        for i in range(16):
            mem_source = instr_mem_low_id if i < 8 else instr_mem_high_id
            mem_port = f"Instruction[{i % 8}]"
            instr_mem_connections.append((mem_source, mem_port, i))

            led_id = await self.create_element("Led", output_x, output_y + (i * 30), f"Instruction[{i}]")
            if led_id is None:
                return False
            instr_output_leds.append(led_id)

            if not await self.connect(mem_source, led_id, source_port_label=mem_port):
                return False

        await self.log("  ✓ Created 16-bit Instruction outputs")

        # ---- Create Output: PC (for external use) ----
        for i in range(8):
            led_id = await self.create_element("Led", output_x + HORIZONTAL_GATE_SPACING, output_y + (i * 30), f"PC[{i}]")
            if led_id is None:
                return False

            if not await self.connect(pc_id, led_id, source_port_label=f"Address[{i}]"):
                return False

        await self.log("  ✓ Created PC outputs")

        # ---- Create Instruction Decode Outputs ----
        # Connect directly from instruction memory outputs
        # OpCode[0..4] = Instruction[15:11]
        # DestReg[0..4] = Instruction[10:6]
        # SrcBits[0..5] = Instruction[5:0]

        decode_x = output_x + (2 * HORIZONTAL_GATE_SPACING)

        # OpCode (bits 15-11)
        for i in range(5):
            instr_bit = 15 - i  # 15, 14, 13, 12, 11
            mem_source, mem_port, _ = instr_mem_connections[instr_bit]

            led_id = await self.create_element("Led", decode_x, output_y + (i * 30), f"OpCode[{i}]")
            if led_id is None:
                return False

            if not await self.connect(mem_source, led_id, source_port_label=mem_port):
                return False

        # DestReg (bits 10-6)
        for i in range(5):
            instr_bit = 10 - i  # 10, 9, 8, 7, 6
            mem_source, mem_port, _ = instr_mem_connections[instr_bit]

            led_id = await self.create_element("Led", decode_x + HORIZONTAL_GATE_SPACING, output_y + (i * 30), f"DestReg[{i}]")
            if led_id is None:
                return False

            if not await self.connect(mem_source, led_id, source_port_label=mem_port):
                return False

        # SrcBits (bits 5-0)
        for i in range(6):
            instr_bit = 5 - i  # 5, 4, 3, 2, 1, 0
            mem_source, mem_port, _ = instr_mem_connections[instr_bit]

            led_id = await self.create_element("Led", decode_x + (2 * HORIZONTAL_GATE_SPACING), output_y + (i * 30), f"SrcBits[{i}]")
            if led_id is None:
                return False

            if not await self.connect(mem_source, led_id, source_port_label=mem_port):
                return False

        await self.log("  ✓ Created instruction decode outputs")

        output_file = str(IC_COMPONENTS_DIR / "level9_fetch_stage_16bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 16-bit Fetch Stage IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = FetchStage16bitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "16-bit Fetch Stage IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
