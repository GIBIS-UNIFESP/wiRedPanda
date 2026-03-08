#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-to-16 Decoder IC

Inputs: addr[0], addr[1], addr[2], addr[3]
Outputs: out[0] to out[15]

Circuit:
- 4 NOT gates (invert address bits)
- 16 AND gates (4-input, one-hot encoding)
  - Each AND combines 4 inputs (direct or inverted)
  - For output i: bit pattern of i determines which inputs to use

Usage:
    python create_level2_decoder_4to16.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class Decoder4to16Builder(ICBuilderBase):
    """Builder for 4-to-16 Decoder IC"""

    async def create(self) -> bool:
        """Create 4-to-16 Decoder IC"""
        width = 4
        num_outputs = 16

        await self.begin_build("Decoder 4-to-16")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position calculation
        input_x = 50.0
        not_x = input_x + HORIZONTAL_GATE_SPACING
        and_x = not_x + HORIZONTAL_GATE_SPACING
        output_x = and_x + HORIZONTAL_GATE_SPACING

        # Create input switches for address bits
        addr_inputs = []
        for i in range(width):
            addr_id = await self.create_element("InputSwitch", input_x, 100.0 + i * VERTICAL_STAGE_SPACING, f"addr[{i}]")
            if addr_id is None:
                return False
            addr_inputs.append(addr_id)
            await self.log(f"  ✓ Created addr[{i}]")

        # Create NOT gates for inverted address bits
        not_gates = []
        for i in range(width):
            not_id = await self.create_element("Not", not_x, 100.0 + i * VERTICAL_STAGE_SPACING, f"not_addr[{i}]")
            if not_id is None:
                return False
            not_gates.append(not_id)

            # Connect addr[i] to NOT gate
            if not await self.connect(addr_inputs[i], not_id):
                return False

        await self.log(f"  ✓ Created {width} NOT gates")

        # Create 4-input AND gates for one-hot encoding
        output_leds = []
        and_gates = []

        for i in range(num_outputs):
            gate_id = await self.create_element("And", and_x + (i % 4) * HORIZONTAL_GATE_SPACING, 100.0 + (i // 4) * VERTICAL_STAGE_SPACING, f"and[{i}]")
            if gate_id is None:
                return False
            and_gates.append(gate_id)

            # Set AND gate to 4-input size
            set_props = await self.mcp.send_command("change_input_size", {
                "element_id": gate_id,
                "size": width
            })
            if not set_props.success:
                self.log_error(f"Failed to set input_size={width} for AND gate {i}")
                return False

            # Determine which inputs to use based on binary pattern of i
            for bit_pos in range(width):
                if (i & (1 << bit_pos)) != 0:
                    source_id = addr_inputs[bit_pos]
                else:
                    source_id = not_gates[bit_pos]

                if not await self.connect(source_id, gate_id, target_port=bit_pos):
                    return False

            # Create output LED
            led_id = await self.create_element("Led", output_x + (i % 4) * HORIZONTAL_GATE_SPACING, 100.0 + (i // 4) * VERTICAL_STAGE_SPACING, f"out[{i}]")
            if led_id is None:
                return False
            output_leds.append(led_id)

            # Connect AND gate to LED
            if not await self.connect(gate_id, led_id):
                return False

        await self.log(f"  ✓ Created {num_outputs} AND gates with {num_outputs} output LEDs")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level2_decoder_4to16.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 4-to-16 Decoder IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = Decoder4to16Builder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-to-16 Decoder IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
