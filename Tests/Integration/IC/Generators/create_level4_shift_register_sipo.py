#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Serial-In Parallel-Out (SIPO) Shift Register IC

Inputs: CLK (Clock), SIN (Serial Input)
Outputs: Q[0:3] (Parallel Output)

Circuit:
- 4 D flip-flops cascaded in a chain
- Serial input feeds to first flip-flop
- Each flip-flop output feeds to next flip-flop's input
- All flip-flops share common clock

SIPO Shift Register behavior:
- Bits shift in serially (one per clock cycle)
- All bits available in parallel at any time
- Shift direction: left (serial input → FF0 → FF1 → FF2 → FF3)
- No load/clear controls for simplicity

Implementation:
- 4 D Flip-Flops (DFlipFlop) cascaded
- Serial input directly feeds FF0 D input
- FF[i] Q output → FF[i+1] D input

Usage:
    python create_shift_register_sipo.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class ShiftRegisterSIPOBuilder(ICBuilderBase):
    """Builder for Serial-In Parallel-Out Shift Register IC"""

    async def create(self) -> bool:
        """Create the SIPO Shift Register IC"""
        await self.begin_build("SIPO Shift Register")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Create input controls
        input_x = 50.0
        clk_id = await self.create_element("InputSwitch", input_x, 100.0, "CLK")
        if clk_id is None:
            return False
        await self.log(f"  ✓ Created input CLK")

        sin_id = await self.create_element("InputSwitch", input_x, 100.0 + VERTICAL_STAGE_SPACING, "SIN")
        if sin_id is None:
            return False
        await self.log(f"  ✓ Created input SIN")

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

        # Create output LEDs (4-bit)
        output_led_ids = []
        output_y = 100.0 + VERTICAL_STAGE_SPACING
        for i in range(4):
            led_id = await self.create_element("Led", dff_x + (i * HORIZONTAL_GATE_SPACING), output_y, f"Q{i}")
            if led_id is None:
                return False
            output_led_ids.append(led_id)
            await self.log(f"  ✓ Created output Q{i}")

        # ========== Connect shift register chain ==========

        # Connect serial input to FF0 D input
        if not await self.connect(sin_id, dff_ids[0], target_port_label="D"):
            return False

        # Connect FF chain: FF[i] Q → FF[i+1] D
        for i in range(3):
            if not await self.connect(dff_ids[i], dff_ids[i + 1], source_port_label="Q", target_port_label="D"):
                return False

        # Connect clock to all FFs
        for i in range(4):
            if not await self.connect(clk_id, dff_ids[i], target_port_label="Clock"):
                return False

        # ========== Connect FF outputs to LEDs ==========
        for i in range(4):
            if not await self.connect(dff_ids[i], output_led_ids[i], source_port_label="Q"):
                return False

        # Create Vcc element for inactive Preset/Clear pins
        vcc_id = await self.create_element("InputVcc", dff_x - HORIZONTAL_GATE_SPACING, 100.0 + 2 * VERTICAL_STAGE_SPACING, "Vcc")
        if vcc_id is None:
            return False
        await self.log(f"  ✓ Created Vcc element")

        # ========== Connect Vcc to all Preset/Clear pins ==========
        for i in range(4):
            # Connect Vcc to Preset (active-LOW, so HIGH keeps it inactive)
            if not await self.connect(vcc_id, dff_ids[i], target_port_label="Preset"):
                return False

            # Connect Vcc to Clear (active-LOW, so HIGH keeps it inactive)
            if not await self.connect(vcc_id, dff_ids[i], target_port_label="Clear"):
                return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level4_shift_register_sipo.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created SIPO Shift Register IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = ShiftRegisterSIPOBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "SIPO Shift Register IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
