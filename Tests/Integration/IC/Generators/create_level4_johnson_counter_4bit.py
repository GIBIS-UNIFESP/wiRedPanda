#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Johnson Counter IC (Twisted Ring Counter)

Inputs: CLK (Clock), PRESET (Preset/Initialize signal - active LOW)
Outputs: Q[0:3] (Counter Output)

Circuit:
- 4 D flip-flops cascaded with inverted feedback
- Preset input to initialize FF[0] to 1
- Output of each FF feeds to next FF's D input
- FF[3] output feeds through NOT gate back to FF[0] input (twisted ring)

Johnson Counter behavior (fill/drain sequence):
- Initialize with PRESET: Sets Q[0]=1, all others=0
- Fills with 1's: 0001→0011→0111→1111
- Then drains: 1110→1100→1000→0000
- Then repeats: 0001
- Period: 8 clock cycles (vs 4 for regular ring counter)
- Used for sequencing and state machines

Implementation:
- 4 D Flip-Flops (DFlipFlop)
- 1 NOT gate (invert Q[3] for twisted feedback)
- Simple cascade interconnections

Usage:
    python create_johnson_counter.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class JohnsonCounterBuilder(ICBuilderBase):
    """Builder for 4-bit Johnson Counter IC"""

    async def create(self) -> bool:
        """Create the 4-bit Johnson Counter IC"""
        await self.begin_build("4-bit Johnson Counter")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Create input controls
        input_x = 50.0
        clk_id = await self.create_element("InputSwitch", input_x, 100.0, "CLK")
        if clk_id is None:
            return False
        await self.log(f"  ✓ Created input CLK")

        preset_id = await self.create_element("InputSwitch", input_x, 100.0 + VERTICAL_STAGE_SPACING, "PRESET")
        if preset_id is None:
            return False
        await self.log(f"  ✓ Created input PRESET")

        # Create D flip-flops (4-bit) using level1_d_flip_flop IC
        dff_ids = []
        dff_x = input_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level1_d_flip_flop")):

                return False

            ff_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level1_d_flip_flop"), dff_x + (i * HORIZONTAL_GATE_SPACING), 100.0, f"FF{i}")
            if ff_id is None:
                return False
            dff_ids.append(ff_id)
            await self.log(f"  ✓ Instantiated level1_d_flip_flop {i}")

        # Create NOT gate for twisted feedback
        not_id = await self.create_element("Not", dff_x + (3 * HORIZONTAL_GATE_SPACING), 100.0 + VERTICAL_STAGE_SPACING, "not_q3")
        if not_id is None:
            return False
        await self.log(f"  ✓ Created NOT gate")

        # Create output LEDs (4-bit)
        output_led_ids = []
        output_y = 100.0 + VERTICAL_STAGE_SPACING
        for i in range(4):
            led_id = await self.create_element("Led", dff_x + (i * HORIZONTAL_GATE_SPACING), output_y, f"Q{i}")
            if led_id is None:
                return False
            output_led_ids.append(led_id)
            await self.log(f"  ✓ Created output Q{i}")

        # ========== Connect clock to all FFs ==========
        for i in range(4):
            if not await self.connect(clk_id, dff_ids[i], target_port_label="Clock"):
                return False

        # ========== Connect cascade path ==========
        for i in range(3):
            # Q[i] → D[i+1]
            if not await self.connect(dff_ids[i], dff_ids[i + 1], source_port_label="Q", target_port_label="D"):
                return False

        # ========== Connect twisted feedback path ==========
        # Q[3] → NOT → D[0]
        if not await self.connect(dff_ids[3], not_id, source_port_label="Q"):
            return False

        if not await self.connect(not_id, dff_ids[0], target_port_label="D"):
            return False

        # ========== Connect FF outputs to LEDs ==========
        for i in range(4):
            if not await self.connect(dff_ids[i], output_led_ids[i], source_port_label="Q"):
                return False

        # ========== Connect PRESET input directly to FF[0] Preset ==========
        if not await self.connect(preset_id, dff_ids[0], target_port_label="Preset"):
            return False

        # Create Vcc element for inactive Preset/Clear pins
        vcc_id = await self.create_element("InputVcc", dff_x - HORIZONTAL_GATE_SPACING, 100.0 + 2 * VERTICAL_STAGE_SPACING, "Vcc")
        if vcc_id is None:
            return False
        await self.log(f"  ✓ Created Vcc element")

        # ========== Connect Vcc/PRESET to Preset/Clear pins ==========
        for i in range(1, 4):
            # Connect Vcc to Preset (active-LOW, so HIGH keeps it inactive)
            if not await self.connect(vcc_id, dff_ids[i], target_port_label="Preset"):
                return False

            # Connect PRESET to Clear (when PRESET goes LOW, clear FF[1-3] to Q=0)
            if not await self.connect(preset_id, dff_ids[i], target_port_label="Clear"):
                return False

        # Also connect Vcc to FF[0]'s Clear (keep it inactive)
        if not await self.connect(vcc_id, dff_ids[0], target_port_label="Clear"):
            return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level4_johnson_counter_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 4-bit Johnson Counter IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = JohnsonCounterBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Johnson Counter IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
