#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Bus Multiplexer IC

Implements a 4-bit wide 2:1 multiplexer by composing 4× level2_mux_2to1 ICs.

Inputs:  In0[0-3], In1[0-3], Sel
Outputs: Out[0-3]
Logic:   Out[i] = (Sel == 0) ? In0[i] : In1[i]

Usage:
    python create_level4_bus_mux_4bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class BusMux4BitBuilder(ICBuilderBase):
    """Builder for 4-bit Bus Multiplexer IC"""

    async def create(self) -> bool:
        """Create 4-bit Bus Multiplexer IC"""
        bit_width = 4

        await self.begin_build('4-bit Bus Multiplexer')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0
        in0_y = 100.0
        in1_y = in0_y + VERTICAL_STAGE_SPACING
        sel_y = in1_y + VERTICAL_STAGE_SPACING

        # Create input switches for In0 (4 bits)
        in0_inputs = []
        for i in range(bit_width):
            in0_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), in0_y, f"In0[{i}]")
            if in0_id is None:
                return False
            in0_inputs.append(in0_id)
            await self.log(f"  ✓ Created In0[{i}] (id={in0_id})")

        # Create input switches for In1 (4 bits)
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

        # Instantiate 4× level2_mux_2to1 ICs (one per bit)
        mux_ic_ids = []
        mux_x = input_x + ((bit_width + 1) * HORIZONTAL_GATE_SPACING)

        for i in range(4):
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_mux_2to1")):

                return False

            mux_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_mux_2to1"), mux_x + (i * HORIZONTAL_GATE_SPACING), in0_y + VERTICAL_STAGE_SPACING, f"Mux[{i}]")
            if mux_id is None:
                return False
            mux_ic_ids.append(mux_id)
            await self.log(f"  ✓ Instantiated Mux[{i}] (id={mux_id})")

        # Connect inputs to 2to1 muxes
        for i in range(4):
            # In0[i] → Mux[i] Data[0] (select when Sel=0)
            if not await self.connect(in0_inputs[i], mux_ic_ids[i], target_port_label="Data[0]"):
                return False

            # In1[i] → Mux[i] Data[1] (select when Sel=1)
            if not await self.connect(in1_inputs[i], mux_ic_ids[i], target_port_label="Data[1]"):
                return False

            # Sel → Mux[i] Sel[0]
            if not await self.connect(sel_id, mux_ic_ids[i], target_port_label="Sel[0]"):
                return False

        # Create output LEDs
        output_led_ids = []
        output_x = mux_x + (5 * HORIZONTAL_GATE_SPACING)
        for i in range(bit_width):
            output_id = await self.create_element("Led", output_x + ((i % 4) * HORIZONTAL_GATE_SPACING), (in0_y + VERTICAL_STAGE_SPACING) + ((i // 4) * VERTICAL_STAGE_SPACING), f"Out[{i}]")
            if output_id is None:
                return False
            output_led_ids.append(output_id)

        # Connect 2to1 mux outputs to LEDs
        for i in range(4):
            if not await self.connect(mux_ic_ids[i], output_led_ids[i], source_port_label="Output"):
                return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 4-bit Bus Multiplexer IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = BusMux4BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Bus Multiplexer IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
