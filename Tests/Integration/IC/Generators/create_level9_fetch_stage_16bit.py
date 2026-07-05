#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 16-bit Fetch Stage IC

Implements the fetch stage for the 16-bit RISC CPU, mirroring the
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

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder


class FetchStage16bitBuilder(ICBuilderBase):
    """Builder for 16-bit Fetch Stage IC"""

    async def create(self) -> bool:
        """Create the 16-bit Fetch Stage IC"""
        await self.begin_build("16-bit Fetch Stage")
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0

        # ---- Instantiate all embedded ICs first, chained left to right ----
        # PCData[]/control signals sit above this whole row, and
        # ProgAddr[]/ProgData[] sit below it, sharing its wide X range -- these
        # ICs are far taller than the flat 64px assumed elsewhere, so their
        # real height (queried via instantiate_ic_with_size) drives the
        # clearance above and below, and their real width drives the
        # horizontal chaining between them.
        ic_row_y = 800.0

        pc_x = input_x + (2 * HORIZONTAL_GATE_SPACING)
        pc_handle = await self.instantiate_ic_with_size("level7_cpu_program_counter_8bit", pc_x, ic_row_y, "PC")
        if pc_handle is None:
            return False
        pc_id = pc_handle.element_id

        addr_mux_x = pc_x + max(HORIZONTAL_GATE_SPACING * 2, pc_handle.width + HORIZONTAL_GATE_SPACING)
        addr_mux_handle = await self.instantiate_ic_with_size("level4_bus_mux_8bit", addr_mux_x, ic_row_y, "AddrMux")
        if addr_mux_handle is None:
            return False
        addr_mux_id = addr_mux_handle.element_id

        instr_mem_low_x = addr_mux_x + max(HORIZONTAL_GATE_SPACING * 2, addr_mux_handle.width + HORIZONTAL_GATE_SPACING)
        instr_mem_low_handle = await self.instantiate_ic_with_size(
            "level7_instruction_memory_interface", instr_mem_low_x, ic_row_y, "InstrMem_Low"
        )
        if instr_mem_low_handle is None:
            return False
        instr_mem_low_id = instr_mem_low_handle.element_id

        instr_mem_high_x = instr_mem_low_x + max(
            HORIZONTAL_GATE_SPACING * 2, instr_mem_low_handle.width + HORIZONTAL_GATE_SPACING
        )
        instr_mem_high_handle = await self.instantiate_ic_with_size(
            "level7_instruction_memory_interface", instr_mem_high_x, ic_row_y, "InstrMem_High"
        )
        if instr_mem_high_handle is None:
            return False
        instr_mem_high_id = instr_mem_high_handle.element_id

        ir_low_x = instr_mem_high_x + max(
            HORIZONTAL_GATE_SPACING * 2, instr_mem_high_handle.width + HORIZONTAL_GATE_SPACING
        )
        ir_low_handle = await self.instantiate_ic_with_size("level6_register_8bit", ir_low_x, ic_row_y, "IR_Low")
        if ir_low_handle is None:
            return False
        ir_low_id = ir_low_handle.element_id

        ir_high_x = ir_low_x + max(HORIZONTAL_GATE_SPACING * 2, ir_low_handle.width + HORIZONTAL_GATE_SPACING)
        ir_high_handle = await self.instantiate_ic_with_size("level6_register_8bit", ir_high_x, ic_row_y, "IR_High")
        if ir_high_handle is None:
            return False
        ir_high_id = ir_high_handle.element_id

        ic_row_half_height = (
            max(
                pc_handle.height,
                addr_mux_handle.height,
                instr_mem_low_handle.height,
                instr_mem_high_handle.height,
                ir_low_handle.height,
                ir_high_handle.height,
            )
            / 2
        )
        control_y = ic_row_y - ic_row_half_height - VERTICAL_STAGE_SPACING
        pc_data_y = control_y - VERTICAL_STAGE_SPACING
        # "InstrMem_Low"'s (and "InstrMem_High"'s) side label is rotated 90
        # degrees and hangs well below the IC's own ports/pixmap bounding box
        # (not captured by ic_row_half_height), reaching down into the
        # ProgAddr[]/ProgData[] row below -- instr_mem_low_x lands in the
        # same column as ProgAddr[6] (both input_x + 6 * HORIZONTAL_GATE_SPACING),
        # so clearing that label reach needs more than one
        # VERTICAL_STAGE_SPACING of clearance below the IC row.
        prog_addr_y = ic_row_y + ic_row_half_height + (2 * VERTICAL_STAGE_SPACING)
        prog_data_y = prog_addr_y + VERTICAL_STAGE_SPACING

        # ---- Create PC data input (8-bit) ----
        pc_data_inputs = []
        for i in range(8):
            elem_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), pc_data_y, f"PCData[{i}]"
            )
            if elem_id is None:
                return False
            pc_data_inputs.append(elem_id)

        await self.log("  ✓ Created PCData inputs")

        # ---- Create control signal inputs ----
        control_x = input_x

        control_signals = {}
        for signal_name in ["Clock", "Reset", "PCLoad", "PCInc", "InstrLoad"]:
            elem_id = await self.create_element(
                "InputSwitch" if signal_name != "Clock" else "Clock", control_x, control_y, signal_name
            )
            if elem_id is None:
                return False
            control_signals[signal_name] = elem_id
            control_x += HORIZONTAL_GATE_SPACING

        await self.log("  ✓ Created control signals")

        # ---- Create programming interface inputs (F53: previously absent —
        # the instruction memory could never be loaded, so the field-decode
        # tests were vacuous) ----
        prog_addr_inputs = []
        for i in range(8):
            elem_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), prog_addr_y, f"ProgAddr[{i}]"
            )
            if elem_id is None:
                return False
            prog_addr_inputs.append(elem_id)

        # ProgData[10..15]'s two-digit index makes the label long enough to
        # reach into its neighbor at a standard 1x step on platforms that
        # render the label a bit wider than the default Linux font (observed
        # on Windows CI), so this row gets extra clearance.
        prog_data_col_spacing = HORIZONTAL_GATE_SPACING + 32
        prog_data_inputs = []
        for i in range(16):
            elem_id = await self.create_element(
                "InputSwitch",
                input_x + (i * prog_data_col_spacing),
                prog_data_y,
                f"ProgData[{i}]",
            )
            if elem_id is None:
                return False
            prog_data_inputs.append(elem_id)

        prog_write_id = await self.create_element(
            "InputSwitch", input_x + (9 * HORIZONTAL_GATE_SPACING), prog_addr_y, "ProgWrite"
        )
        if prog_write_id is None:
            return False

        await self.log("  ✓ Created programming interface inputs")

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
        for i in range(8):
            if not await self.connect(
                pc_id, addr_mux_id, source_port_label=f"Address[{i}]", target_port_label=f"In0[{i}]"
            ):
                return False
            if not await self.connect(prog_addr_inputs[i], addr_mux_id, target_port_label=f"In1[{i}]"):
                return False
        if not await self.connect(prog_write_id, addr_mux_id, target_port_label="Sel"):
            return False

        await self.log("  ✓ Instantiated Address Mux (PC vs ProgAddr)")

        # ---- Two Instruction Memory interfaces (low and high bytes) ----
        # Muxed address into both instruction memories
        for i in range(8):
            if not await self.connect(
                addr_mux_id, instr_mem_low_id, source_port_label=f"Out[{i}]", target_port_label=f"Address[{i}]"
            ):
                return False

            if not await self.connect(
                addr_mux_id, instr_mem_high_id, source_port_label=f"Out[{i}]", target_port_label=f"Address[{i}]"
            ):
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
        for ir_id, mem_id in ((ir_low_id, instr_mem_low_id), (ir_high_id, instr_mem_high_id)):
            for i in range(8):
                if not await self.connect(
                    mem_id, ir_id, source_port_label=f"Instruction[{i}]", target_port_label=f"Data[{i}]"
                ):
                    return False

            if not await self.connect(control_signals["Clock"], ir_id, target_port_label="Clock"):
                return False

            if not await self.connect(control_signals["InstrLoad"], ir_id, target_port_label="WriteEnable"):
                return False

            if not await self.connect(control_signals["Reset"], ir_id, target_port_label="Reset"):
                return False

        await self.log("  ✓ Instantiated and wired the 16-bit instruction register")

        # ---- Create output LED columns ----
        # Placed clear of both the last chained IC's (IR_High) real width and
        # the 16-wide ProgData[] row above/below it (whose rightmost switch
        # reaches out to input_x + 15 * prog_data_col_spacing).
        ic_chain_output_x = ir_high_x + max(HORIZONTAL_GATE_SPACING * 2, ir_high_handle.width + HORIZONTAL_GATE_SPACING)
        prog_data_row_end = input_x + (15 * prog_data_col_spacing)
        # "ProgData[15]"'s label (fixed offset, doesn't scale with the row's
        # own spacing) reaches past a single HORIZONTAL_GATE_SPACING gap into
        # the output column, so this margin needs to be wider.
        output_x = max(ic_chain_output_x, prog_data_row_end + (1.5 * HORIZONTAL_GATE_SPACING))
        output_y = ic_row_y
        instruction_x = output_x
        # "Instruction[N]" labels are wide enough (especially double-digit N)
        # that the standard HORIZONTAL_GATE_SPACING isn't enough to keep them
        # clear of the "RawInstr[N]" column beside them.
        rawinstr_x = instruction_x + (1.5 * HORIZONTAL_GATE_SPACING)
        pc_out_x = rawinstr_x + HORIZONTAL_GATE_SPACING
        decode_x = pc_out_x + HORIZONTAL_GATE_SPACING

        # ---- Create Output: registered 16-bit Instruction ----
        for i in range(16):
            ir_source = ir_low_id if i < 8 else ir_high_id

            led_id = await self.create_element(
                "Led", instruction_x, output_y + (i * VERTICAL_STAGE_SPACING), f"Instruction[{i}]"
            )
            if led_id is None:
                return False

            if not await self.connect(ir_source, led_id, source_port_label=f"Q[{i % 8}]"):
                return False

        await self.log("  ✓ Created 16-bit Instruction outputs (registered)")

        # ---- Create Output: RawInstr (unregistered, straight from memory) ----
        for i in range(16):
            mem_source = instr_mem_low_id if i < 8 else instr_mem_high_id

            led_id = await self.create_element(
                "Led", rawinstr_x, output_y + (i * VERTICAL_STAGE_SPACING), f"RawInstr[{i}]"
            )
            if led_id is None:
                return False

            if not await self.connect(mem_source, led_id, source_port_label=f"Instruction[{i % 8}]"):
                return False

        await self.log("  ✓ Created RawInstr outputs (unregistered)")

        # ---- Create Output: PC (for external use) ----
        for i in range(8):
            led_id = await self.create_element("Led", pc_out_x, output_y + (i * VERTICAL_STAGE_SPACING), f"PC[{i}]")
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

        def ir_source_for(instr_bit: int):
            return (ir_low_id if instr_bit < 8 else ir_high_id), f"Q[{instr_bit % 8}]"

        # OpCode (bits 11..15, LSB-first)
        for i in range(5):
            source_id, port = ir_source_for(11 + i)

            led_id = await self.create_element("Led", decode_x, output_y + (i * VERTICAL_STAGE_SPACING), f"OpCode[{i}]")
            if led_id is None:
                return False

            if not await self.connect(source_id, led_id, source_port_label=port):
                return False

        # DestReg (bits 6..10, LSB-first)
        for i in range(5):
            source_id, port = ir_source_for(6 + i)

            led_id = await self.create_element(
                "Led",
                decode_x + HORIZONTAL_GATE_SPACING,
                output_y + (i * VERTICAL_STAGE_SPACING),
                f"DestReg[{i}]",
            )
            if led_id is None:
                return False

            if not await self.connect(source_id, led_id, source_port_label=port):
                return False

        # SrcBits (bits 0..5, LSB-first)
        for i in range(6):
            source_id, port = ir_source_for(i)

            led_id = await self.create_element(
                "Led",
                decode_x + (2 * HORIZONTAL_GATE_SPACING),
                output_y + (i * VERTICAL_STAGE_SPACING),
                f"SrcBits[{i}]",
            )
            if led_id is None:
                return False

            if not await self.connect(source_id, led_id, source_port_label=port):
                return False

        await self.log("  ✓ Created instruction decode outputs (LSB-first)")

        output_file = str(IC_COMPONENTS_DIR / "level9_fetch_stage_16bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 16-bit Fetch Stage IC "
            f"({self.element_count} elements, {self.connection_count} connections)"
        )
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
