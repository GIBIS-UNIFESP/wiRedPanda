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
  - level2_decoder_3to8 IC (now has an active-high enable)
  - 1 AND gate: clock AND writeEnable, driven into the decoder's enable
  - out[i] = decode[i] AND clock AND writeEnable, taken straight off the decoder
    (the decoder's enable makes a separate external gating stage redundant)

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
        await self.begin_build("Clock-Gated Decoder 3-to-8")
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
            addr_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), addr_y, f"addr{i}"
            )
            if addr_id is None:
                return False
            addr_inputs.append(addr_id)

        # Clock input
        clock_id = await self.create_element("InputSwitch", input_x, control_y, "clock")
        if clock_id is None:
            return False

        # WriteEnable input
        write_enable_id = await self.create_element(
            "InputSwitch", input_x + HORIZONTAL_GATE_SPACING, control_y, "WriteEnable"
        )
        if write_enable_id is None:
            return False

        await self.log(f"  ✓ Created all inputs (3 address + 2 control)")

        # ========== Instantiate 3-to-8 Decoder IC ==========
        decoder_id = await self.instantiate_ic("level2_decoder_3to8", decoder_gates_x, addr_y, "Decoder_3to8")
        if decoder_id is None:
            return False

        # Connect address inputs to decoder IC
        for i in range(3):
            if not await self.connect(addr_inputs[i], decoder_id, target_port_label=f"addr[{i}]"):
                return False

        await self.log("  ✓ Instantiated level2_decoder_3to8 IC with address connections")

        # ========== Gate the decoder via its enable (clock AND writeEnable) ==========
        # The decoder now has an active-high enable, so out[i] = decode[i] AND
        # enable. Driving enable = clock AND writeEnable yields
        # out[i] = decode[i] AND clock AND writeEnable directly — the 16 external
        # gating/output AND gates this used to need are redundant.
        gate_id = await self.create_element("And", gating_gates_x, control_y, "clock_we_gate")
        if gate_id is None:
            return False
        if not await self.connect(clock_id, gate_id, target_port=0):
            return False
        if not await self.connect(write_enable_id, gate_id, target_port=1):
            return False
        if not await self.connect(gate_id, decoder_id, target_port_label="Enable"):
            return False

        await self.log("  ✓ Gated decoder enable with clock AND writeEnable")

        # ========== Create Output LEDs (straight off the gated decoder) ==========
        output_leds = []
        for i in range(8):
            led_id = await self.create_element(
                "Led",
                output_x + (i % 4) * HORIZONTAL_GATE_SPACING * 0.6,
                addr_y + (i // 4) * VERTICAL_STAGE_SPACING,
                f"out{i}",
            )
            if led_id is None:
                return False
            output_leds.append(led_id)

            # Connect decoder output directly to LED
            if not await self.connect(decoder_id, led_id, source_port_label=f"out[{i}]"):
                return False

        await self.log("  ✓ Created 8 output LEDs")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level5_clock_gated_decoder.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 3-to-8 Clock-Gated Decoder IC ({self.element_count} elements, {self.connection_count} connections)"
        )
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
