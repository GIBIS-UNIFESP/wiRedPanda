#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Clock-Gated Decoder IC (Level 5)

A 3-to-8 decoder with integrated clock gating for synchronized output selection.
Uses decoder logic composition with AND gates for clock gating control.

Inputs:
  - addr0, addr1, addr2 (3-bit address)
  - clock (clock signal for gating)
  - writeEnable (output enable control)

Outputs:
  - out0, out1, out2, out3, out4, out5, out6, out7 (8 one-hot outputs)

Architecture:
  - NOT gates to create address complements
  - AND gates for 3-to-8 decoder logic
  - AND gates combining decoder output with clock for gating
  - AND gates combining gated output with writeEnable for final control

Usage:
    python3 create_level5_clock_gated_decoder.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class ClockGatedDecoderBuilder(ICBuilderBase):
    """Builder for Clock-Gated Decoder IC (Level 5)"""

    async def create(self) -> bool:
        """Create the Clock-Gated Decoder IC"""
        await self.begin_build('Clock-Gated Decoder 3-to-8')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position hierarchy
        input_x = 50.0
        addr_y = 100.0
        control_y = addr_y + VERTICAL_STAGE_SPACING
        not_gates_x = input_x + HORIZONTAL_GATE_SPACING
        decoder_gates_x = not_gates_x + HORIZONTAL_GATE_SPACING
        gating_gates_x = decoder_gates_x + HORIZONTAL_GATE_SPACING * 1.5
        output_gates_x = gating_gates_x + HORIZONTAL_GATE_SPACING * 2
        output_x = output_gates_x + HORIZONTAL_GATE_SPACING

        # ========== Create Input Switches ==========
        # Address inputs (3 bits)
        addr_inputs = []
        for i in range(3):
            addr_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), addr_y, f"addr{i}")
            if addr_id is None:
                return False
            addr_inputs.append(addr_id)

        # Clock input
        clock_id = await self.create_element("InputSwitch", input_x, control_y, "clock")
        if clock_id is None:
            return False

        # WriteEnable input
        write_enable_id = await self.create_element("InputSwitch", input_x + HORIZONTAL_GATE_SPACING, control_y, "writeEnable")
        if write_enable_id is None:
            return False

        await self.log(f"  ✓ Created all inputs (3 address + 2 control)")

        # ========== Instantiate 3-to-8 Decoder IC ==========
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_decoder_3to8")):

            return False

        decoder_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_decoder_3to8"), decoder_gates_x, addr_y, "Decoder_3to8")
        if decoder_id is None:
            return False

        # Connect address inputs to decoder IC
        for i in range(3):
            if not await self.connect(addr_inputs[i], decoder_id, target_port_label=f"addr[{i}]"):
                return False

        await self.log("  ✓ Instantiated level2_decoder_3to8 IC with address connections")

        # ========== Create Gating AND Gates (decoder output AND clock) ==========
        gating_gates = []
        for i in range(8):
            gating_id = await self.create_element("And", gating_gates_x + (i % 4) * HORIZONTAL_GATE_SPACING * 0.6, addr_y + (i // 4) * VERTICAL_STAGE_SPACING, f"gating_and{i}")
            if gating_id is None:
                return False
            gating_gates.append(gating_id)

            # Connect decoder IC output to gating AND port 0
            if not await self.connect(decoder_id, gating_id, source_port_label=f"out[{i}]"):
                return False

            # Connect clock to gating AND port 1
            if not await self.connect(clock_id, gating_id, target_port=1):
                return False

        await self.log("  ✓ Created 8 gating AND gates (clock control)")

        # ========== Create Output ANDs for WriteEnable Control ==========
        output_gates = []
        for i in range(8):
            output_id = await self.create_element("And", output_gates_x + (i % 4) * HORIZONTAL_GATE_SPACING * 0.6, addr_y + (i // 4) * VERTICAL_STAGE_SPACING, f"output_and{i}")
            if output_id is None:
                return False
            output_gates.append(output_id)

            # Connect gating AND output to output AND port 0
            if not await self.connect(gating_gates[i], output_id):
                return False

            # Connect writeEnable to output AND port 1
            if not await self.connect(write_enable_id, output_id, target_port=1):
                return False

        await self.log("  ✓ Created 8 output AND gates (writeEnable control)")

        # ========== Create Output LEDs ==========
        output_leds = []
        for i in range(8):
            led_id = await self.create_element("Led", output_x + (i % 4) * HORIZONTAL_GATE_SPACING * 0.6, addr_y + (i // 4) * VERTICAL_STAGE_SPACING, f"out{i}")
            if led_id is None:
                return False
            output_leds.append(led_id)

            # Connect output AND to LED
            if not await self.connect(output_gates[i], led_id):
                return False

        await self.log("  ✓ Created 8 output LEDs")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level5_clock_gated_decoder.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 3-to-8 Clock-Gated Decoder IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Inputs: addr0, addr1, addr2, clock, writeEnable")
        await self.log(f"   Outputs: out0..out7 (8 one-hot)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = ClockGatedDecoderBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Clock-Gated Decoder IC (3-to-8, Level 5)"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
