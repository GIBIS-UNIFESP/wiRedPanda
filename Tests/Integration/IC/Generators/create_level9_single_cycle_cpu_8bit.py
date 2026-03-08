#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 8-bit Single-Cycle CPU IC

Implements a complete 8-bit single-cycle CPU with 4 pipeline stages integrated:
- Fetch Stage: Instruction retrieval and program counter management
- Decode Stage: Opcode to control signal conversion
- Execute Stage: ALU operations with 8 supported operations
- Memory Stage: Memory read/write and result pass-through
- Register File: 8 general-purpose 8-bit registers

Inputs:
  Clock (synchronization signal)
  Reset (initialize program counter to 0)

Outputs:
  PC[0-7] (current program counter value)
  Result[0-7] (ALU/Memory result)
  Instruction[0-7] (current instruction being executed)
  Zero (zero flag from ALU)
  Sign (sign flag from ALU)

Architecture:
  - Integrates level8_fetch_stage for instruction retrieval
  - Integrates level8_decode_stage for control signal generation
  - Integrates level8_execute_stage for ALU operations
  - Integrates level8_memory_stage for memory operations
  - Instantiates level6_register_file_8x8 for data storage
  - All stages connected in sequence with proper data paths

Single-Cycle Execution:
  - All stages active simultaneously (instruction entering fetch while previous executes)
  - Register file provides operands to Execute stage
  - Execute result flows to Memory stage
  - Memory output flows back to register file for write-back

Usage:
    python create_level9_cpu_8bit_single_cycle.py
"""

import asyncio
from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class CPU8BitSingleCycleBuilder(ICBuilderBase):
    """Builder for 8-bit Single-Cycle CPU IC"""

    async def create(self) -> bool:
        """Create the 8-bit Single-Cycle CPU IC"""
        await self.begin_build('8-bit Single-Cycle CPU')
        if not await self.create_new_circuit():
            return False

        # Layout positions
        stage_x_offsets = [50.0, 300.0, 550.0, 800.0]
        stage_y = 100.0
        reg_file_y = 500.0

        # ---- Create Clock and Reset inputs ----
        control_x = 50.0
        control_y = 50.0

        clock_id = await self.create_element("Clock", control_x, control_y, "Clock")
        if clock_id is None:
            return False

        reset_id = await self.create_element("InputSwitch", control_x + HORIZONTAL_GATE_SPACING, control_y, "Reset")
        if reset_id is None:
            return False

        await self.log("  ✓ Created Clock and Reset inputs")

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

        # ---- Create Vcc and Gnd for default control signals ----
        vcc_id = await self.create_element("InputVcc", 100.0, stage_y - 50.0, "Vcc")
        if vcc_id is None:
            return False

        gnd_id = await self.create_element("InputGnd", 150.0, stage_y - 50.0, "Gnd")
        if gnd_id is None:
            return False

        await self.log("  ✓ Created Vcc and Gnd for control signals")

        # ---- Connect Clock and Reset through all stages ----
        # Clock to Fetch
        if not await self.connect(clock_id, fetch_id, target_port_label="Clock"):
            return False

        # Reset to Fetch
        if not await self.connect(reset_id, fetch_id, target_port_label="Reset"):
            return False

        # PCLoad = 0 (don't load new address)
        for port in ["PCLoad", "InstrLoad"]:
            if not await self.connect(gnd_id, fetch_id, target_port_label=port):
                return False

        # PCInc = 1 (increment PC each cycle)
        if not await self.connect(vcc_id, fetch_id, target_port_label="PCInc"):
            return False

        # Clock to Decode
        if not await self.connect(clock_id, decode_id, target_port_label="Clock"):
            return False

        # Clock to Execute
        if not await self.connect(clock_id, execute_id, target_port_label="Clock"):
            return False

        # Clock to Memory
        if not await self.connect(clock_id, memory_id, target_port_label="Clock"):
            return False

        # Clock to Register File
        if not await self.connect(clock_id, regfile_id, target_port_label="Clock"):
            return False

        await self.log("  ✓ Connected Clock and Reset")

        # ---- Connect Fetch to Decode ----
        # OpCode[0-4] from Fetch to Decode OpCode[0-4]
        for i in range(5):
            if not await self.connect(fetch_id, decode_id, source_port_label=f"OpCode[{i}]", target_port_label=f"OpCode[{i}]"):
                return False

        await self.log("  ✓ Connected Fetch to Decode")

        # ---- Connect Decode to Execute ----
        # ALUOp[0-2] from Decode to Execute ALUOp[0-2]
        for i in range(3):
            if not await self.connect(decode_id, execute_id, source_port_label=f"ALUOp[{i}]", target_port_label=f"ALUOp[{i}]"):
                return False

        await self.log("  ✓ Connected Decode to Execute")

        # ---- Connect Execute to Memory ----
        # Result[0-7] from Execute to Memory Result[0-7]
        for i in range(8):
            if not await self.connect(execute_id, memory_id, source_port_label=f"Result[{i}]", target_port_label=f"Result[{i}]"):
                return False

        # MemRead and MemWrite from Decode to Memory
        if not await self.connect(decode_id, memory_id, source_port_label="MemRead", target_port_label="MemRead"):
            return False

        if not await self.connect(decode_id, memory_id, source_port_label="MemWrite", target_port_label="MemWrite"):
            return False

        # Connect Memory stage inputs (Address and DataIn) to ground for now
        # In a full CPU, these would come from registers or instruction bits
        for i in range(8):
            if not await self.connect(gnd_id, memory_id, target_port_label=f"Address[{i}]"):
                return False

            if not await self.connect(gnd_id, memory_id, target_port_label=f"DataIn[{i}]"):
                return False

        await self.log("  ✓ Connected Execute to Memory")

        # ---- Create output LEDs for external visibility ----
        output_x = stage_x_offsets[3] + HORIZONTAL_GATE_SPACING

        # PC outputs
        pc_outputs = []
        for i in range(8):
            led_id = await self.create_element("Led", output_x, stage_y + (i * 50.0), f"PC[{i}]")
            if led_id is None:
                return False
            pc_outputs.append(led_id)

            if not await self.connect(fetch_id, led_id, source_port_label=f"PC[{i}]"):
                return False

        await self.log("  ✓ Created PC outputs")

        # Result outputs
        result_outputs = []
        for i in range(8):
            led_id = await self.create_element("Led", output_x, 500.0 + (i * 50.0), f"Result[{i}]")
            if led_id is None:
                return False
            result_outputs.append(led_id)

            if not await self.connect(memory_id, led_id, source_port_label=f"DataOut[{i}]"):
                return False

        await self.log("  ✓ Created Result outputs")

        # Instruction outputs (from Fetch stage)
        for i in range(8):
            led_id = await self.create_element("Led", output_x + 100.0, stage_y + (i * 50.0), f"Instruction[{i}]")
            if led_id is None:
                return False

            if not await self.connect(fetch_id, led_id, source_port_label=f"Instruction[{i}]"):
                return False

        await self.log("  ✓ Created Instruction outputs")

        # Zero flag output (from Execute stage)
        zero_led_id = await self.create_element("Led", output_x + 200.0, stage_y, "Zero")
        if zero_led_id is None:
            return False

        if not await self.connect(execute_id, zero_led_id, source_port_label="Zero"):
            return False

        # Sign flag output (from Execute stage)
        sign_led_id = await self.create_element("Led", output_x + 200.0, stage_y + 50.0, "Sign")
        if sign_led_id is None:
            return False

        if not await self.connect(execute_id, sign_led_id, source_port_label="Sign"):
            return False

        await self.log("  ✓ Created Zero and Sign flag outputs")

        output_file = str(IC_COMPONENTS_DIR / "level9_single_cycle_cpu_8bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 8-bit Single-Cycle CPU IC ({self.element_count} elements, {self.connection_count} connections)")
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
