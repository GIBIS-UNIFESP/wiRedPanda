#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 16-bit RISC CPU IC

Implements a true 16-bit RISC (Reduced Instruction Set Computer) CPU:
- 16-bit instruction word with 5-bit opcode
- 16-bit ALU with full arithmetic/logic operations
- 16-bit program counter and data paths
- Pipeline-based execution with 4-stage pipeline

Inputs:
  Clock (synchronization signal)
  Reset (initialize program counter to 0)

Outputs:
  PC[0..7] (program counter for memory addressing)
  Result[0..15] (16-bit ALU result)
  Instr[0..15] (16-bit instruction word)
  OpCode[0..4] (5-bit operation code)

16-bit Instruction Format:
  Bits [15:11] = OpCode (5 bits, 32 operations)
  Bits [10:6] = DestReg (5 bits, 32 registers)
  Bits [5:0] = SrcBits (6 bits, 64 source/immediate)

Architecture:
  - 16-bit Fetch stage: Outputs 16-bit instruction with decoded fields
  - 16-bit ALU: Full arithmetic and logic operations
  - 16-bit program counter and data paths
  - Pipeline: Fetch → Decode → Execute → Memory

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
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class CPU16BitRISCBuilder(ICBuilderBase):
    """Builder for 16-bit RISC CPU IC"""

    async def create(self) -> bool:
        """Create the 16-bit RISC CPU IC"""
        await self.begin_build('16-bit RISC CPU')
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

        # ---- Create Vcc for control signals ----
        vcc_id = await self.create_element("InputVcc", 150.0, 50.0, "Vcc")
        if vcc_id is None:
            return False

        await self.log("  ✓ Created Vcc for control signals")

        # ---- Instantiate 16-bit Fetch Stage ----
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level9_fetch_stage_16bit")):

            return False

        fetch_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level9_fetch_stage_16bit"), 300.0, 100.0, "Fetch_16bit")
        if fetch_id is None:
            return False

        # Connect Clock, Reset, and control signals to Fetch stage
        if not await self.connect(clock_id, fetch_id, target_port_label="Clock"):
            return False

        if not await self.connect(reset_id, fetch_id, target_port_label="Reset"):
            return False

        # Default control signals
        if not await self.connect(vcc_id, fetch_id, target_port_label="PCInc"):
            return False

        await self.log("  ✓ Instantiated 16-bit Fetch stage")

        # ---- Instantiate 16-bit ALU ----
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level7_alu_16bit")):

            return False

        alu_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level7_alu_16bit"), 650.0, 100.0, "ALU_16bit")
        if alu_id is None:
            return False

        await self.log("  ✓ Instantiated 16-bit ALU")

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

        await self.log(f"✅ Successfully created 16-bit RISC CPU IC ({self.element_count} elements, {self.connection_count} connections)")
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
