#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 16-bit Fetch Stage IC

Implements the fetch stage for a 16-bit RISC CPU pipeline, mirroring the
level8_fetch_stage architecture (F53: this stage used to have a dead
InstrLoad input and no instruction register, no programming interface, and
its decoded fields were emitted MSB-first against the project-wide
LSB-first convention).

Inputs:
  Clock (synchronization signal)
  Reset (reset program counter and instruction register to 0)
  PCLoad (enable loading new PC value)
  PCInc (increment program counter)
  InstrLoad (enable loading the fetched word into the instruction register)
  PCData[0..7] (value to load into PC when PCLoad=1)
  ProgAddr[0..7] (address for programming instruction memory)
  ProgData[0..15] (16-bit word to write into instruction memory)
  ProgWrite (enable writing to instruction memory)

Outputs:
  PC[0..7] (program counter for memory addressing)
  Instruction[0..15] (registered 16-bit instruction word, from the IR)
  RawInstr[0..15] (unregistered word straight from memory — zero-delay
                   consumers such as single-cycle CPUs use this)
  OpCode[0..4]  = Instruction[11..15] (index 0 = field LSB)
  DestReg[0..4] = Instruction[6..10]  (index 0 = field LSB)
  SrcBits[0..5] = Instruction[0..5]   (index 0 = field LSB)

Architecture:
  1. level7_cpu_program_counter_8bit — PC with load/increment/reset
  2. level4_bus_mux_8bit — selects PC address (ProgWrite=0) or ProgAddr
     (ProgWrite=1), the level8_fetch_stage pattern
  3. 2× level7_instruction_memory_interface — low byte (Instruction[0..7])
     and high byte (Instruction[8..15]) at the muxed address
  4. 2× level6_register_8bit — the 16-bit instruction register
     (WriteEnable=InstrLoad, Reset=Reset)

16-bit Instruction Format:
  Bits [15:11] = OpCode (5 bits, 32 operations)
  Bits [10:6]  = DestReg (5 bits, 32 registers addressable)
  Bits [5:0]   = SrcBits (6 bits, 64 source/immediate modes)

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

        # ---- Create programming interface inputs (F53: previously absent —
        # the instruction memory could never be loaded, so the field-decode
        # tests were vacuous) ----
        prog_y = control_y + VERTICAL_STAGE_SPACING

        prog_addr_inputs = []
        for i in range(8):
            elem_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), prog_y, f"ProgAddr[{i}]")
            if elem_id is None:
                return False
            prog_addr_inputs.append(elem_id)

        prog_data_inputs = []
        for i in range(16):
            elem_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING / 2), prog_y + (VERTICAL_STAGE_SPACING / 2), f"ProgData[{i}]")
            if elem_id is None:
                return False
            prog_data_inputs.append(elem_id)

        prog_write_id = await self.create_element("InputSwitch", input_x + (9 * HORIZONTAL_GATE_SPACING), prog_y, "ProgWrite")
        if prog_write_id is None:
            return False

        await self.log("  ✓ Created programming interface inputs")

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

        # ---- Address mux: PC (fetch) vs ProgAddr (programming) ----
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_8bit")):

            return False

        addr_mux_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_8bit"), 350.0, 100.0, "AddrMux")
        if addr_mux_id is None:
            return False

        for i in range(8):
            if not await self.connect(pc_id, addr_mux_id, source_port_label=f"Address[{i}]", target_port_label=f"In0[{i}]"):
                return False
            if not await self.connect(prog_addr_inputs[i], addr_mux_id, target_port_label=f"In1[{i}]"):
                return False
        if not await self.connect(prog_write_id, addr_mux_id, target_port_label="Sel"):
            return False

        await self.log("  ✓ Instantiated Address Mux (PC vs ProgAddr)")

        # ---- Instantiate two Instruction Memory interfaces (low and high bytes) ----
        instr_mem_x = 450.0
        instr_mem_low_y = 100.0
        instr_mem_high_y = 250.0

        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level7_instruction_memory_interface")):

            return False

        instr_mem_low_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level7_instruction_memory_interface"), instr_mem_x, instr_mem_low_y, "InstrMem_Low")
        if instr_mem_low_id is None:
            return False

        instr_mem_high_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level7_instruction_memory_interface"), instr_mem_x, instr_mem_high_y, "InstrMem_High")
        if instr_mem_high_id is None:
            return False

        # Muxed address into both instruction memories
        for i in range(8):
            if not await self.connect(addr_mux_id, instr_mem_low_id, source_port_label=f"Out[{i}]", target_port_label=f"Address[{i}]"):
                return False

            if not await self.connect(addr_mux_id, instr_mem_high_id, source_port_label=f"Out[{i}]", target_port_label=f"Address[{i}]"):
                return False

        # Programming data and write enable
        for i in range(8):
            if not await self.connect(prog_data_inputs[i], instr_mem_low_id, target_port_label=f"DataIn[{i}]"):
                return False
            if not await self.connect(prog_data_inputs[8 + i], instr_mem_high_id, target_port_label=f"DataIn[{i}]"):
                return False

        if not await self.connect(prog_write_id, instr_mem_low_id, target_port_label="WriteEnable"):
            return False
        if not await self.connect(prog_write_id, instr_mem_high_id, target_port_label="WriteEnable"):
            return False

        # Clock to instruction memory interfaces
        if not await self.connect(control_signals["Clock"], instr_mem_low_id, target_port_label="Clock"):
            return False

        if not await self.connect(control_signals["Clock"], instr_mem_high_id, target_port_label="Clock"):
            return False

        await self.log("  ✓ Instantiated and wired instruction memory interfaces")

        # ---- Instruction Register: 2× level6_register_8bit (F53: InstrLoad
        # used to be a dead input — there was no IR to load) ----
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level6_register_8bit")):

            return False

        ir_low_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level6_register_8bit"), 600.0, 100.0, "IR_Low")
        if ir_low_id is None:
            return False

        ir_high_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level6_register_8bit"), 600.0, 250.0, "IR_High")
        if ir_high_id is None:
            return False

        for ir_id, mem_id in ((ir_low_id, instr_mem_low_id), (ir_high_id, instr_mem_high_id)):
            for i in range(8):
                if not await self.connect(mem_id, ir_id, source_port_label=f"Instruction[{i}]", target_port_label=f"Data[{i}]"):
                    return False

            if not await self.connect(control_signals["Clock"], ir_id, target_port_label="Clock"):
                return False

            if not await self.connect(control_signals["InstrLoad"], ir_id, target_port_label="WriteEnable"):
                return False

            if not await self.connect(control_signals["Reset"], ir_id, target_port_label="Reset"):
                return False

        await self.log("  ✓ Instantiated and wired the 16-bit instruction register")

        # ---- Create Output: registered 16-bit Instruction ----
        output_x = 750.0
        output_y = 100.0

        for i in range(16):
            ir_source = ir_low_id if i < 8 else ir_high_id

            led_id = await self.create_element("Led", output_x, output_y + (i * 30), f"Instruction[{i}]")
            if led_id is None:
                return False

            if not await self.connect(ir_source, led_id, source_port_label=f"Q[{i % 8}]"):
                return False

        await self.log("  ✓ Created 16-bit Instruction outputs (registered)")

        # ---- Create Output: RawInstr (unregistered, straight from memory) ----
        for i in range(16):
            mem_source = instr_mem_low_id if i < 8 else instr_mem_high_id

            led_id = await self.create_element("Led", output_x + (HORIZONTAL_GATE_SPACING / 2), output_y + (i * 30), f"RawInstr[{i}]")
            if led_id is None:
                return False

            if not await self.connect(mem_source, led_id, source_port_label=f"Instruction[{i % 8}]"):
                return False

        await self.log("  ✓ Created RawInstr outputs (unregistered)")

        # ---- Create Output: PC (for external use) ----
        for i in range(8):
            led_id = await self.create_element("Led", output_x + HORIZONTAL_GATE_SPACING, output_y + (i * 30), f"PC[{i}]")
            if led_id is None:
                return False

            if not await self.connect(pc_id, led_id, source_port_label=f"Address[{i}]"):
                return False

        await self.log("  ✓ Created PC outputs")

        # ---- Create Instruction Decode Outputs (registered, LSB-first) ----
        # F53: these used to be emitted MSB-first (OpCode[0]=Instruction[15]),
        # against the project-wide index-0-is-LSB convention (the 8-bit IR
        # maps OpCode[0]=Q[3], the field LSB).
        # OpCode[i]  = Instruction[11+i]
        # DestReg[i] = Instruction[6+i]
        # SrcBits[i] = Instruction[i]
        decode_x = output_x + (2 * HORIZONTAL_GATE_SPACING)

        def ir_source_for(instr_bit: int):
            return (ir_low_id if instr_bit < 8 else ir_high_id), f"Q[{instr_bit % 8}]"

        # OpCode (bits 11..15, LSB-first)
        for i in range(5):
            source_id, port = ir_source_for(11 + i)

            led_id = await self.create_element("Led", decode_x, output_y + (i * 30), f"OpCode[{i}]")
            if led_id is None:
                return False

            if not await self.connect(source_id, led_id, source_port_label=port):
                return False

        # DestReg (bits 6..10, LSB-first)
        for i in range(5):
            source_id, port = ir_source_for(6 + i)

            led_id = await self.create_element("Led", decode_x + HORIZONTAL_GATE_SPACING, output_y + (i * 30), f"DestReg[{i}]")
            if led_id is None:
                return False

            if not await self.connect(source_id, led_id, source_port_label=port):
                return False

        # SrcBits (bits 0..5, LSB-first)
        for i in range(6):
            source_id, port = ir_source_for(i)

            led_id = await self.create_element("Led", decode_x + (2 * HORIZONTAL_GATE_SPACING), output_y + (i * 30), f"SrcBits[{i}]")
            if led_id is None:
                return False

            if not await self.connect(source_id, led_id, source_port_label=port):
                return False

        await self.log("  ✓ Created instruction decode outputs (LSB-first)")

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
