#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Flag Register IC

Implements a 4-bit status flag register that stores CPU status flags.

Inputs:
  FlagIn[0..3] (4-bit flag inputs)
    FlagIn[0]: Zero flag (Z) - set when result = 0
    FlagIn[1]: Sign flag (S) - set when result is negative
    FlagIn[2]: Carry flag (C) - set when carry-out from arithmetic
    FlagIn[3]: Overflow flag (O) - set when signed overflow
  Load (Load enable signal - when 1, flags are updated on clock)
  Clock (Clock signal for synchronous update)

Outputs:
  FlagOut[0..3] (4-bit flag outputs - current flag state)
    FlagOut[0]: Zero flag
    FlagOut[1]: Sign flag
    FlagOut[2]: Carry flag
    FlagOut[3]: Overflow flag

Architecture:
  - 4× D flip-flops for flag storage (one per flag)
  - 4× Mux for load control (select between hold and load)
  - Common clock and load enable signal

Usage:
    python create_level7_flag_register.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class FlagRegisterBuilder(ICBuilderBase):
    """Builder for Flag Register IC"""

    async def create(self) -> bool:
        """Create the Flag Register IC"""
        await self.begin_build('Flag Register')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0

        # Create flag input switches (4-bit)
        flag_names = ["Zero", "Sign", "Carry", "Overflow"]
        flag_inputs = []
        for i in range(4):
            flag_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), 100.0, f"FlagIn[{i}]_{flag_names[i]}")
            if flag_id is None:
                return False
            flag_inputs.append(flag_id)
        await self.log("  ✓ Created 4 flag inputs (Z, S, C, O)")

        # Create control inputs
        load_id = await self.create_element("InputSwitch", input_x + (4 * HORIZONTAL_GATE_SPACING), 100.0, "Load")
        if load_id is None:
            return False

        clk_id = await self.create_element("InputSwitch", input_x + (4 * HORIZONTAL_GATE_SPACING), 100.0 + VERTICAL_STAGE_SPACING, "Clock")
        if clk_id is None:
            return False
        await self.log("  ✓ Created Load and Clock inputs")

        # Create Vcc for inactive Preset/Clear signals
        vcc_id = await self.create_element("InputVcc", input_x + (5 * HORIZONTAL_GATE_SPACING), 100.0, "Vcc")
        if vcc_id is None:
            return False
        await self.log("  ✓ Created Vcc for FF control signals")

        # Create muxes for load control (one per flag)
        mux_ids = []
        for i in range(4):
            mux_id = await self.create_element("Mux", input_x + (i * HORIZONTAL_GATE_SPACING), 200.0, f"Mux{i}")
            if mux_id is None:
                return False
            mux_ids.append(mux_id)

            # Connect flag input to Mux port 1 (load path)
            if not await self.connect(flag_inputs[i], mux_id, target_port_label="In1"):
                return False

            # Connect Load to Mux select
            if not await self.connect(load_id, mux_id, target_port_label="S0"):
                return False
        await self.log("  ✓ Created 4 muxes for load control")

        # Create D flip-flops for flag storage (one per flag) using level1_d_flip_flop IC
        ff_ids = []
        for i in range(4):
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level1_d_flip_flop")):

                return False

            ff_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level1_d_flip_flop"), input_x + (i * HORIZONTAL_GATE_SPACING), 300.0, f"FlagFF{i}")
            if ff_id is None:
                return False
            ff_ids.append(ff_id)

            # Connect mux output to FF D input
            if not await self.connect(mux_ids[i], ff_id, target_port_label="D"):
                return False

            # Connect Clock to FF clock
            if not await self.connect(clk_id, ff_id, target_port_label="Clock"):
                return False

            # Connect Vcc to Preset (inactive HIGH)
            if not await self.connect(vcc_id, ff_id, target_port_label="Preset"):
                return False

            # Connect Vcc to Clear (inactive HIGH)
            if not await self.connect(vcc_id, ff_id, target_port_label="Clear"):
                return False

            # Connect FF Q output back to Mux port 0 (hold path)
            if not await self.connect(ff_id, mux_ids[i], source_port_label="Q", target_port_label="In0"):
                return False
        await self.log("  ✓ Created 4 flag flip-flops with feedback")

        # Create output LEDs for flags
        output_x = input_x + (6 * HORIZONTAL_GATE_SPACING)
        for i in range(4):
            led_id = await self.create_element("Led", output_x, 300.0 + (i * VERTICAL_STAGE_SPACING), f"FlagOut[{i}]_{flag_names[i]}")
            if led_id is None:
                return False

            # Connect FF Q output to LED
            if not await self.connect(ff_ids[i], led_id, source_port_label="Q"):
                return False
        await self.log("  ✓ Created 4 flag outputs")

        output_file = str(IC_COMPONENTS_DIR / "level7_flag_register.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created Flag Register IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = FlagRegisterBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Flag Register IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
