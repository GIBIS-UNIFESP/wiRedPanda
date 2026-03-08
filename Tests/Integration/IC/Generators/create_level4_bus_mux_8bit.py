#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 8-bit Bus Multiplexer IC

Implements an 8-bit wide 2:1 multiplexer by composing 2× level4_bus_mux_4bit ICs.

Inputs:  In0[0-7], In1[0-7], Sel
Outputs: Out[0-7]
Logic:   Out[i] = (Sel == 0) ? In0[i] : In1[i]

Usage:
    python create_level4_bus_mux_8bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class BusMux8BitBuilder(ICBuilderBase):
    """Builder for 8-bit Bus Multiplexer IC"""

    async def create(self) -> bool:
        """Create 8-bit Bus Multiplexer IC"""
        bit_width = 8

        await self.begin_build('8-bit Bus Multiplexer')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0
        in0_y = 100.0
        in1_y = in0_y + VERTICAL_STAGE_SPACING
        sel_y = in1_y + VERTICAL_STAGE_SPACING

        # Create input switches for In0 (8 bits)
        in0_inputs = []
        for i in range(bit_width):
            in0_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), in0_y, f"In0[{i}]")
            if in0_id is None:
                return False
            in0_inputs.append(in0_id)
            await self.log(f"  ✓ Created In0[{i}] (id={in0_id})")

        # Create input switches for In1 (8 bits)
        in1_inputs = []
        for i in range(bit_width):
            in1_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), in1_y, f"In1[{i}]")
            if in1_id is None:
                return False
            in1_inputs.append(in1_id)
            await self.log(f"  ✓ Created In1[{i}] (id={in1_id})")

        # Create input switch for Sel
        sel_id = await self.create_element("InputSwitch", input_x + (bit_width * HORIZONTAL_GATE_SPACING), sel_y, "Sel")
        if sel_id is None:
            return False
        await self.log(f"  ✓ Created Sel (id={sel_id})")

        # Instantiate 2× level4_bus_mux_4bit ICs
        mux_ic_ids = []
        mux_x = input_x + ((bit_width + 1) * HORIZONTAL_GATE_SPACING)
        num_bus_muxes = 2

        for block_idx in range(num_bus_muxes):
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit")):

                return False

            mux_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit"), mux_x + (block_idx * HORIZONTAL_GATE_SPACING), in0_y + VERTICAL_STAGE_SPACING, f"BusMux[{block_idx*4}-{block_idx*4+3}]")
            if mux_id is None:
                return False
            mux_ic_ids.append(mux_id)
            await self.log(f"  ✓ Instantiated BusMux[{block_idx}] (id={mux_id})")

        # Connect 4-bit blocks to bus_mux_4bit ICs
        for block_idx, mux_id in enumerate(mux_ic_ids):
            for bit_in_block in range(4):
                global_bit = block_idx * 4 + bit_in_block

                # In0[global_bit] → BusMux In0[bit_in_block]
                if not await self.connect(in0_inputs[global_bit], mux_id, target_port_label=f"In0[{bit_in_block}]"):
                    return False

                # In1[global_bit] → BusMux In1[bit_in_block]
                if not await self.connect(in1_inputs[global_bit], mux_id, target_port_label=f"In1[{bit_in_block}]"):
                    return False

            # Sel → BusMux Sel
            if not await self.connect(sel_id, mux_id, target_port_label="Sel"):
                return False

        # Create output LEDs
        output_led_ids = []
        output_x = mux_x + (3 * HORIZONTAL_GATE_SPACING)
        for i in range(bit_width):
            output_id = await self.create_element("Led", output_x + ((i % 4) * HORIZONTAL_GATE_SPACING), (in0_y + VERTICAL_STAGE_SPACING) + ((i // 4) * VERTICAL_STAGE_SPACING), f"Out[{i}]")
            if output_id is None:
                return False
            output_led_ids.append(output_id)

        # Connect bus_mux outputs to LEDs
        for block_idx, mux_id in enumerate(mux_ic_ids):
            for bit_in_block in range(4):
                global_bit = block_idx * 4 + bit_in_block
                if not await self.connect(mux_id, output_led_ids[global_bit], source_port_label=f"Out[{bit_in_block}]"):
                    return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level4_bus_mux_8bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 8-bit Bus Multiplexer IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = BusMux8BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "8-bit Bus Multiplexer IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
