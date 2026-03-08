#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Instruction Decoder IC

Decodes a 4-bit instruction opcode into 16 one-hot output lines.

Inputs:
  instr[0..3] (4-bit instruction opcode)

Outputs:
  op[0..15] (one-hot: only one line active per instruction)

Architecture:
  - Instantiates level2_decoder_4to16
  - Maps opcode bits to decoder address inputs
  - Outputs all 16 possible instruction lines

Usage:
    python create_level5_instruction_decoder_4bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class InstructionDecoder4BitBuilder(ICBuilderBase):
    """Builder for 4-bit Instruction Decoder IC"""

    async def create(self) -> bool:
        """Create the 4-bit Instruction Decoder IC"""
        await self.begin_build('4-bit Instruction Decoder')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0

        # Create instruction input switches (4-bit opcode)
        instr_inputs = []
        for i in range(4):
            instr_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), 100.0, f"instr[{i}]")
            if instr_id is None:
                return False
            instr_inputs.append(instr_id)
        await self.log("  ✓ Created 4 instruction inputs")

        # Instantiate level2_decoder_4to16
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_decoder_4to16")):

            return False

        decoder_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_decoder_4to16"), input_x + (2 * HORIZONTAL_GATE_SPACING), 200.0, "Decoder4to16")
        if decoder_id is None:
            return False
        await self.log("  ✓ Instantiated 4-to-16 Decoder")

        # Connect instruction inputs to decoder
        for i in range(4):
            if not await self.connect(instr_inputs[i], decoder_id, target_port_label=f"addr[{i}]"):
                return False
        await self.log("  ✓ Connected instruction inputs to decoder")

        # Create output LEDs for each instruction line (16 total)
        output_x = input_x + (4 * HORIZONTAL_GATE_SPACING)
        for i in range(16):
            led_id = await self.create_element("Led", output_x, 200.0 + (i * (VERTICAL_STAGE_SPACING / 2)), f"op[{i}]")
            if led_id is None:
                return False

            # Connect decoder output to LED
            if not await self.connect(decoder_id, led_id, source_port_label=f"out[{i}]"):
                return False
        await self.log("  ✓ Created 16 output lines and connected to decoder")

        output_file = str(IC_COMPONENTS_DIR / "level5_instruction_decoder_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 4-bit Instruction Decoder IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = InstructionDecoder4BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Instruction Decoder IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
