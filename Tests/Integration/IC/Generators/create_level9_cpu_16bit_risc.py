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
from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder


class CPU16BitRISCBuilder(ICBuilderBase):
    """Builder for 16-bit RISC CPU IC"""

    async def create(self) -> bool:
        """Create the 16-bit RISC CPU IC"""
        await self.begin_build("16-bit RISC CPU")
        if not await self.create_new_circuit():
            return False

        # ---- Create Clock and Reset inputs ----
        clock_id = await self.create_element("Clock", 50.0, 50.0, "Clock")
        if clock_id is None:
            return False

        reset_id = await self.create_element("InputSwitch", 100.0, 50.0, "Reset")
        if reset_id is None:
            return False

        await self.log("  ✓ Created Clock and Reset inputs")

        # ---- Create Vcc/Gnd for control signals ----
        vcc_id = await self.create_element("InputVcc", 150.0, 50.0, "Vcc")
        if vcc_id is None:
            return False

        gnd_id = await self.create_element("InputGnd", 200.0, 50.0, "Gnd")
        if gnd_id is None:
            return False

        await self.log("  ✓ Created Vcc and Gnd for control signals")

        # ---- Create instruction memory programming inputs (F53) ----
        prog_addr_inputs = []
        for i in range(8):
            elem_id = await self.create_element("InputSwitch", 50.0 + (i * 40.0), 100.0, f"ProgAddr[{i}]")
            if elem_id is None:
                return False
            prog_addr_inputs.append(elem_id)

        prog_data_inputs = []
        for i in range(16):
            elem_id = await self.create_element("InputSwitch", 50.0 + (i * 20.0), 140.0, f"ProgData[{i}]")
            if elem_id is None:
                return False
            prog_data_inputs.append(elem_id)

        prog_write_id = await self.create_element("InputSwitch", 400.0, 100.0, "ProgWrite")
        if prog_write_id is None:
            return False

        await self.log("  ✓ Created programming interface inputs")

        # ---- Instantiate 16-bit Fetch Stage ----
        fetch_id = await self.instantiate_ic("level9_fetch_stage_16bit", 300.0, 100.0, "Fetch_16bit")
        if fetch_id is None:
            return False

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

        # ---- Instantiate 16-bit ALU ----
        alu_id = await self.instantiate_ic("level7_alu_16bit", 650.0, 100.0, "ALU_16bit")
        if alu_id is None:
            return False

        await self.log("  ✓ Instantiated 16-bit ALU")

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

        # ---- Create Output: PC ----
        for i in range(8):
            led_id = await self.create_element("Led", 900.0 + (i * 20), 100.0, f"PC[{i}]")
            if led_id is None:
                return False

            if not await self.connect(fetch_id, led_id, source_port_label=f"PC[{i}]"):
                return False

        await self.log("  ✓ Created PC outputs")

        # ---- Create Output: 16-bit Result ----
        for i in range(16):
            led_id = await self.create_element("Led", 900.0, 150.0 + (i * 20), f"Result[{i}]")
            if led_id is None:
                return False

            if not await self.connect(alu_id, led_id, source_port_label=f"Result[{i}]"):
                return False

        await self.log("  ✓ Created 16-bit Result outputs")

        # ---- Create Output: 16-bit Instruction ----
        for i in range(16):
            led_id = await self.create_element("Led", 1100.0, 100.0 + (i * 20), f"Instr[{i}]")
            if led_id is None:
                return False

            if not await self.connect(fetch_id, led_id, source_port_label=f"Instruction[{i}]"):
                return False

        await self.log("  ✓ Created Instruction outputs")

        # ---- Create Output: OpCode ----
        for i in range(5):
            led_id = await self.create_element("Led", 1200.0, 100.0 + (i * 20), f"OpCode[{i}]")
            if led_id is None:
                return False

            if not await self.connect(fetch_id, led_id, source_port_label=f"OpCode[{i}]"):
                return False

        await self.log("  ✓ Created OpCode outputs")

        output_file = str(IC_COMPONENTS_DIR / "level9_cpu_16bit_risc.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 16-bit RISC CPU IC ({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Architecture: 16-bit Fetch + 16-bit ALU")
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
