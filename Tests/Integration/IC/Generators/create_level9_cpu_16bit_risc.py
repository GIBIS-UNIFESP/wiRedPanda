#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 16-bit RISC CPU IC

Implements a true 16-bit RISC (Reduced Instruction Set Computer) CPU:
- 16-bit instruction word with 5-bit opcode
- 16-bit ALU with full arithmetic/logic operations
- 16-bit program counter and data paths
- Single-cycle execution: combinational decode straight from the fetched
  instruction word (no pipeline; the level9_single_cycle pattern)

Inputs:
  Clock (synchronization signal)
  Reset (initialize program counter to 0)
  ProgAddr[0..7] (instruction memory programming address — F53)
  ProgData[0..15] (16-bit instruction word to program)
  ProgWrite (instruction memory write enable)

Outputs:
  PC[0..7] (program counter for memory addressing)
  Result[0..15] (16-bit ALU result)
  Instr[0..15] (registered 16-bit instruction word)
  OpCode[0..4] (5-bit operation code, LSB-first)

16-bit Instruction Format:
  Bits [15:11] = OpCode (5 bits, 32 operations)
  Bits [10:6] = DestReg (5 bits, 32 registers)
  Bits [5:0] = SrcBits (6 bits, 64 source/immediate)

Architecture:
  - 16-bit Fetch stage with programming port and instruction register
    (InstrLoad held high; the CPU decodes from RawInstr for zero-delay
    single-cycle execution, the level9_single_cycle pattern — F53)
  - 16-bit ALU computing ALUOp(SrcBits, DestReg): OperandA = zero-extended
    SrcBits (RawInstr[0..5]), OperandB = zero-extended DestReg
    (RawInstr[6..10]), ALUOp = RawInstr[11..13] — all LSB-first
  - PCLoad/PCData tied to explicit GND (no jumps; F34 convention)

RISC Characteristics:
  - Load/Store architecture
  - Fixed 16-bit instructions
  - Large register addressable (32 registers via 5-bit field)
  - Orthogonal instruction set
  - Simple control logic

Usage:
    python create_level9_cpu_16bit_risc.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder


class CPU16BitRISCBuilder(ICBuilderBase):
    """Builder for 16-bit RISC CPU IC"""

    async def create(self) -> bool:
        """Create the 16-bit RISC CPU IC"""
        await self.begin_build("16-bit RISC CPU")
        if not await self.create_new_circuit():
            return False

        input_x = 50.0

        # ---- Instantiate the Fetch and ALU ICs first ----
        # The ProgAddr[]/ProgData[] input rows above them (and both ICs'
        # own X columns) share the same wide X range, so the input rows
        # need clearance from whichever IC is taller -- level9_fetch_stage_16bit
        # and level7_alu_16bit are both far taller than the flat 64px assumed
        # elsewhere (queried via instantiate_ic_with_size).
        fetch_x = input_x + (5 * HORIZONTAL_GATE_SPACING)
        ic_row_y = 1200.0

        fetch_handle = await self.instantiate_ic_with_size("level9_fetch_stage_16bit", fetch_x, ic_row_y, "Fetch_16bit")
        if fetch_handle is None:
            return False
        fetch_id = fetch_handle.element_id

        alu_x = fetch_x + max(HORIZONTAL_GATE_SPACING * 2, fetch_handle.width + HORIZONTAL_GATE_SPACING)
        alu_handle = await self.instantiate_ic_with_size("level7_alu_16bit", alu_x, ic_row_y, "ALU_16bit")
        if alu_handle is None:
            return False
        alu_id = alu_handle.element_id
        await self.log("  ✓ Instantiated 16-bit ALU")

        ic_row_half_height = max(fetch_handle.height, alu_handle.height) / 2
        prog_data_y = ic_row_y - ic_row_half_height - VERTICAL_STAGE_SPACING
        prog_addr_y = prog_data_y - VERTICAL_STAGE_SPACING
        control_y = prog_addr_y - VERTICAL_STAGE_SPACING

        # ---- Create Clock and Reset inputs ----
        clock_id = await self.create_element("Clock", input_x, control_y, "Clock")
        if clock_id is None:
            return False

        reset_id = await self.create_element("InputSwitch", input_x + HORIZONTAL_GATE_SPACING, control_y, "Reset")
        if reset_id is None:
            return False

        await self.log("  ✓ Created Clock and Reset inputs")

        # ---- Create Vcc/Gnd for control signals ----
        vcc_id = await self.create_element("InputVcc", input_x + (2 * HORIZONTAL_GATE_SPACING), control_y, "Vcc")
        if vcc_id is None:
            return False

        gnd_id = await self.create_element("InputGnd", input_x + (3 * HORIZONTAL_GATE_SPACING), control_y, "Gnd")
        if gnd_id is None:
            return False

        await self.log("  ✓ Created Vcc and Gnd for control signals")

        # ---- Create instruction memory programming inputs (F53) ----
        prog_addr_inputs = []
        for i in range(8):
            elem_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), prog_addr_y, f"ProgAddr[{i}]"
            )
            if elem_id is None:
                return False
            prog_addr_inputs.append(elem_id)

        prog_data_inputs = []
        for i in range(16):
            elem_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), prog_data_y, f"ProgData[{i}]"
            )
            if elem_id is None:
                return False
            prog_data_inputs.append(elem_id)

        prog_write_id = await self.create_element(
            "InputSwitch", input_x + (4 * HORIZONTAL_GATE_SPACING), control_y, "ProgWrite"
        )
        if prog_write_id is None:
            return False

        await self.log("  ✓ Created programming interface inputs")

        # Connect Clock, Reset, and control signals to Fetch stage
        if not await self.connect(clock_id, fetch_id, target_port_label="Clock"):
            return False

        if not await self.connect(reset_id, fetch_id, target_port_label="Reset"):
            return False

        # Default control signals: free-running fetch (PCInc=1, InstrLoad=1),
        # no jumps (PCLoad/PCData = explicit GND, F34 convention)
        if not await self.connect(vcc_id, fetch_id, target_port_label="PCInc"):
            return False

        if not await self.connect(vcc_id, fetch_id, target_port_label="InstrLoad"):
            return False

        if not await self.connect(gnd_id, fetch_id, target_port_label="PCLoad"):
            return False

        for i in range(8):
            if not await self.connect(gnd_id, fetch_id, target_port_label=f"PCData[{i}]"):
                return False

        # Programming interface pass-through
        for i in range(8):
            if not await self.connect(prog_addr_inputs[i], fetch_id, target_port_label=f"ProgAddr[{i}]"):
                return False

        for i in range(16):
            if not await self.connect(prog_data_inputs[i], fetch_id, target_port_label=f"ProgData[{i}]"):
                return False

        if not await self.connect(prog_write_id, fetch_id, target_port_label="ProgWrite"):
            return False

        await self.log("  ✓ Instantiated and wired 16-bit Fetch stage")

        # ---- Wire fetch instruction fields into the ALU (F26, F53) ----
        # Single-cycle decode uses the UNREGISTERED RawInstr word (the
        # level9_single_cycle pattern — the IR's registered fields would add
        # a cycle of latency). Fields are LSB-first per project convention:
        #   OperandA = zero-extended SrcBits = RawInstr[0..5]
        #   OperandB = zero-extended DestReg = RawInstr[6..10]
        #   ALUOp    = low 3 bits of OpCode = RawInstr[11..13]
        # The unconnected high OperandA/OperandB bits default to the ALU
        # IC's saved-off switches (0), giving the zero extension implicitly.
        for i in range(6):
            if not await self.connect(
                fetch_id, alu_id, source_port_label=f"RawInstr[{i}]", target_port_label=f"OperandA[{i}]"
            ):
                return False

        for i in range(5):
            if not await self.connect(
                fetch_id, alu_id, source_port_label=f"RawInstr[{6 + i}]", target_port_label=f"OperandB[{i}]"
            ):
                return False

        for i in range(3):
            if not await self.connect(
                fetch_id, alu_id, source_port_label=f"RawInstr[{11 + i}]", target_port_label=f"ALUOp[{i}]"
            ):
                return False

        await self.log("  ✓ Wired RawInstr fields to ALU operands (LSB-first)")

        # ---- Create output LED columns ----
        # Placed clear of the ALU's real width so none of the four columns
        # below encroach on it.
        output_x = alu_x + max(HORIZONTAL_GATE_SPACING * 2, alu_handle.width + HORIZONTAL_GATE_SPACING)
        pc_x = output_x
        result_x = pc_x + HORIZONTAL_GATE_SPACING
        instr_x = result_x + HORIZONTAL_GATE_SPACING
        opcode_x = instr_x + HORIZONTAL_GATE_SPACING

        # ---- Create Output: PC ----
        for i in range(8):
            led_id = await self.create_element("Led", pc_x, ic_row_y + (i * VERTICAL_STAGE_SPACING), f"PC[{i}]")
            if led_id is None:
                return False

            if not await self.connect(fetch_id, led_id, source_port_label=f"PC[{i}]"):
                return False

        await self.log("  ✓ Created PC outputs")

        # ---- Create Output: 16-bit Result ----
        for i in range(16):
            led_id = await self.create_element("Led", result_x, ic_row_y + (i * VERTICAL_STAGE_SPACING), f"Result[{i}]")
            if led_id is None:
                return False

            if not await self.connect(alu_id, led_id, source_port_label=f"Result[{i}]"):
                return False

        await self.log("  ✓ Created 16-bit Result outputs")

        # ---- Create Output: 16-bit Instruction ----
        for i in range(16):
            led_id = await self.create_element("Led", instr_x, ic_row_y + (i * VERTICAL_STAGE_SPACING), f"Instr[{i}]")
            if led_id is None:
                return False

            if not await self.connect(fetch_id, led_id, source_port_label=f"Instruction[{i}]"):
                return False

        await self.log("  ✓ Created Instruction outputs")

        # ---- Create Output: OpCode ----
        for i in range(5):
            led_id = await self.create_element("Led", opcode_x, ic_row_y + (i * VERTICAL_STAGE_SPACING), f"OpCode[{i}]")
            if led_id is None:
                return False

            if not await self.connect(fetch_id, led_id, source_port_label=f"OpCode[{i}]"):
                return False

        await self.log("  ✓ Created OpCode outputs")

        output_file = str(IC_COMPONENTS_DIR / "level9_cpu_16bit_risc.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 16-bit RISC CPU IC "
            f"({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log("   Architecture: 16-bit Fetch + 16-bit ALU")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = CPU16BitRISCBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "16-bit RISC CPU"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
