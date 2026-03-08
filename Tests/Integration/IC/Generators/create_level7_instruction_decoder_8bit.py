#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 8-bit Instruction Decoder IC

Decodes an 8-bit instruction opcode into 256 one-hot output lines.

Inputs:
  instr[0..7] (8-bit instruction opcode)

Outputs:
  op[0..255] (one-hot: only one line active per instruction)

Architecture:
  - Two 4-to-16 decoders (hierarchical composition):
    - Decoder1: instr[3:0] → 16 low outputs
    - Decoder2: instr[7:4] → 16 high outputs
  - 256 2-input AND gates: one per instruction code
    - AND gate for instruction N combines:
      - high decoder output [N >> 4]
      - low decoder output [N & 0xF]
  - Output is 1 when the 8-bit instruction matches

Example: Instruction 0x42 (binary 01000010)
  - Low nibble 0x2 activates low_out[2]
  - High nibble 0x4 activates high_out[4]
  - AND(high_out[4], low_out[2]) activates op[66]

Purpose:
  Decodes instruction opcode via hierarchical decoder composition.
  Much more efficient than 256 8-input AND gates.

Usage:
    python create_level7_instruction_decoder_8bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class InstructionDecoder8BitBuilder(ICBuilderBase):
    """Builder for 8-bit Instruction Decoder IC"""

    async def create(self) -> bool:
        """Create the 8-bit Instruction Decoder IC"""
        await self.begin_build('8-bit Instruction Decoder')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0

        # Create instruction input switches (8-bit opcode)
        instr_inputs = []
        for i in range(8):
            instr_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), 100.0, f"instr[{i}]")
            if instr_id is None:
                return False
            instr_inputs.append(instr_id)
        await self.log("  ✓ Created 8 instruction inputs")

        # Instantiate two 4-to-16 decoders for hierarchical decoding
        # Decoder 1: bits [3:0] → 16 outputs (low nibble)
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_decoder_4to16")):

            return False

        decoder1_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_decoder_4to16"), input_x + (2 * HORIZONTAL_GATE_SPACING), 200.0, "Decoder4to16_low")
        if decoder1_id is None:
            return False
        await self.log("  ✓ Instantiated decoder for low nibble [3:0]")

        # Decoder 2: bits [7:4] → 16 outputs (high nibble)
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_decoder_4to16")):

            return False

        decoder2_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_decoder_4to16"), input_x + (6 * HORIZONTAL_GATE_SPACING), 200.0, "Decoder4to16_high")
        if decoder2_id is None:
            return False
        await self.log("  ✓ Instantiated decoder for high nibble [7:4]")

        # Connect low nibble inputs to decoder 1
        for i in range(4):
            if not await self.connect(instr_inputs[i], decoder1_id, target_port_label=f"addr[{i}]"):
                return False
        await self.log("  ✓ Connected low nibble to decoder 1")

        # Connect high nibble inputs to decoder 2
        for i in range(4):
            if not await self.connect(instr_inputs[i + 4], decoder2_id, target_port_label=f"addr[{i}]"):
                return False
        await self.log("  ✓ Connected high nibble to decoder 2")

        # Create 256 2-input AND gates to combine decoder outputs
        # AND gate for instruction N uses outputs from decoder2[N>>4] and decoder1[N&0xF]
        and_y_base = 350.0
        op_outputs = []

        for instruction_code in range(256):
            # Calculate decoder output indices
            high_idx = instruction_code >> 4  # bits [7:4]
            low_idx = instruction_code & 0xF  # bits [3:0]

            # Calculate position for this AND gate
            and_col = instruction_code % 16
            and_row = instruction_code // 16
            and_x = input_x + (and_col * HORIZONTAL_GATE_SPACING / 2)
            and_y = and_y_base + (and_row * VERTICAL_STAGE_SPACING)

            # Create AND gate (2 inputs)
            and_id = await self.create_element("And", and_x, and_y, f"dec[{instruction_code}]")
            if and_id is None:
                return False

            # Connect high decoder output
            if not await self.connect(decoder2_id, and_id, source_port_label=f"out[{high_idx}]"):
                return False

            # Connect low decoder output to AND input 1
            if not await self.connect(decoder1_id, and_id, source_port_label=f"out[{low_idx}]", target_port=1):
                return False

            # Create output LED
            op_id = await self.create_element("Led", and_x, and_y + 40.0, f"op[{instruction_code}]")
            if op_id is None:
                return False
            op_outputs.append(op_id)

            # Connect AND gate output to LED
            if not await self.connect(and_id, op_id):
                return False

        await self.log("  ✓ Created 256 2-input AND gates combining decoder outputs")

        output_file = str(IC_COMPONENTS_DIR / "level7_instruction_decoder_8bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 8-bit Instruction Decoder IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = InstructionDecoder8BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "8-bit Instruction Decoder IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
