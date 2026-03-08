#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Controller IC

Decodes a 4-bit instruction opcode into 4-bit control signals.

Inputs:
  opcode[0..3] (4-bit instruction opcode)

Outputs:
  ctrl[0..3] (4-bit control word)
    ctrl[1:0]: ALU operation (00=ADD, 01=SUB, 10=AND, 11=OR)
    ctrl[2]: Register write enable
    ctrl[3]: Memory write enable

Control Mapping (by opcode range):
  0x0-0x3 (00xx): ALU operations only
    - register_write = 0, memory_write = 0
  0x4-0x7 (01xx): ALU with register write enabled
    - register_write = 1, memory_write = 0
  0x8-0xB (10xx): ALU with memory write enabled
    - register_write = 0, memory_write = 1
  0xC-0xF (11xx): ALU with register and memory write
    - register_write = 1, memory_write = 1

Decoding Logic:
  ctrl[1:0] = opcode[1:0]  (ALU operation from lower 2 bits)
  ctrl[2] = opcode[3] OR opcode[2]  (register write when bits [3:2] != 00)
  ctrl[3] = opcode[3]  (memory write only when bit [3] = 1)

Usage:
    python create_level5_controller_4bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class Controller4BitBuilder(ICBuilderBase):
    """Builder for 4-bit Controller IC"""

    async def create(self) -> bool:
        """Create the 4-bit Controller IC"""
        await self.begin_build('4-bit Controller')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0

        # Create opcode input switches (4-bit)
        opcode_inputs = []
        for i in range(4):
            opcode_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), 100.0, f"opcode[{i}]")
            if opcode_id is None:
                return False
            opcode_inputs.append(opcode_id)
        await self.log("  ✓ Created 4 opcode inputs")

        # ========== Decoding Logic ==========
        gates_x = input_x + HORIZONTAL_GATE_SPACING
        output_x = input_x + (4 * HORIZONTAL_GATE_SPACING)

        # ctrl[2] = opcode[3] OR opcode[2] (register write enable)
        regwrite_or_id = await self.create_element("Or", gates_x, 150.0, "RegWrite_OR")
        if regwrite_or_id is None:
            return False

        # Connect opcode[3] to OR port 0 and opcode[2] to OR port 1
        # ctrl[2] = opcode[3] OR opcode[2]
        if not await self.connect(opcode_inputs[3], regwrite_or_id, target_port=0):
            return False
        if not await self.connect(opcode_inputs[2], regwrite_or_id, target_port=1):
            return False

        await self.log("  ✓ Created decoding gates (OR gate for register write)")

        # ========== Create Output LEDs ==========
        control_outputs = []
        output_connections = [
            (0, opcode_inputs[0], None, "ctrl[0] = opcode[0]"),  # ALU op bit 0
            (1, opcode_inputs[1], None, "ctrl[1] = opcode[1]"),  # ALU op bit 1
            (2, None, regwrite_or_id, "ctrl[2] = opcode[3] OR opcode[2]"),  # Register write
            (3, opcode_inputs[3], None, "ctrl[3] = opcode[3]"),  # Memory write
        ]

        for ctrl_idx, opcode_source, gate_source, _ in output_connections:
            led_id = await self.create_element("Led", output_x, 100.0 + (ctrl_idx * VERTICAL_STAGE_SPACING), f"ctrl[{ctrl_idx}]")
            if led_id is None:
                return False
            control_outputs.append(led_id)

            # Connect to appropriate source
            # ctrl[0] and ctrl[1] pass through from opcode bits
            # ctrl[2] comes from OR gate, ctrl[3] from opcode[3]
            source_id = gate_source if gate_source else opcode_source
            if not await self.connect(source_id, led_id):
                return False

        await self.log("  ✓ Created 4 control outputs with decoding logic")

        output_file = str(IC_COMPONENTS_DIR / "level5_controller_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 4-bit Controller IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = Controller4BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Controller IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
