#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create CPU Level 2: 8-bit Register IC

Composes 8 instances of 1-bit Register with shared clock/reset/enable.

Inputs: Data[0..7], Clock, WriteEnable, Reset (11 inputs)
Outputs: Q[0..7] (8 outputs)

Architecture:
- Bit 0: 1-bit Register(Data[0], Clock, WriteEnable, Reset) → Q[0]
- Bit 1: 1-bit Register(Data[1], Clock, WriteEnable, Reset) → Q[1]
- ...
- Bit 7: 1-bit Register(Data[7], Clock, WriteEnable, Reset) → Q[7]

Usage:
    python create_level6_register_8bit.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder


class Register8BitBuilder(ICBuilderBase):
    """Builder for 8-bit Register IC"""

    async def create(self) -> bool:
        """Create the 8-bit Register IC"""
        await self.begin_build("8-bit Register")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Create 8-bit Data input switches. Data[0-2] stay vertically stacked
        # alongside Clock/WriteEnable/Reset (same column, same rows) -- Y order
        # among all boundary inputs must stay exactly as-is, since IC port order
        # is derived from a Y-sort and this IC's exported port order is a golden
        # (level6_register_8bit.sv). Data[3-7] have no shared row to preserve, so
        # instead of continuing straight down input_x (a separate block far below
        # the register row that forced long diagonal wires up to Reg[3..7]), each
        # sits in its own X column directly under the Reg[bit]/Q[bit] it feeds --
        # same Y (so port order is untouched), much shorter/near-vertical wire.
        input_x = 50.0
        reg_x = input_x + 2 * HORIZONTAL_GATE_SPACING
        data_inputs = []
        for i in range(8):
            data_x = input_x if i < 3 else reg_x + i * HORIZONTAL_GATE_SPACING
            element_id = await self.create_element(
                "InputSwitch", data_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"Data[{i}]"
            )
            if element_id is None:
                return False
            data_inputs.append(element_id)

        # Create control signals (shared by all bits) - vertically stacked
        ctrl_x = input_x + HORIZONTAL_GATE_SPACING
        clock = await self.create_element("InputSwitch", ctrl_x, 100.0, "Clock")
        if clock is None:
            return False

        write_enable = await self.create_element("InputSwitch", ctrl_x, 100.0 + VERTICAL_STAGE_SPACING, "WriteEnable")
        if write_enable is None:
            return False

        reset = await self.create_element("InputSwitch", ctrl_x, 100.0 + 2 * VERTICAL_STAGE_SPACING, "Reset")
        if reset is None:
            return False

        await self.log("  ✓ Created 8 data inputs + 3 control signals")

        # Instantiate 1-bit Register IC instances
        registers = []
        reg_y = 100.0
        for bit in range(8):
            reg_id = await self.instantiate_ic(
                "level3_register_1bit",
                reg_x + bit * HORIZONTAL_GATE_SPACING,
                reg_y,
                f"Reg[{bit}]",
            )
            if reg_id is None:
                return False
            registers.append(reg_id)
            await self.log(f"  ✓ Instantiated Register IC for bit {bit}")

            # Connect Data[bit] to register Data input
            if not await self.connect(data_inputs[bit], reg_id, target_port_label="Data"):
                return False

            # Connect Clock to register Clock input
            if not await self.connect(clock, reg_id, target_port_label="Clock"):
                return False

            # Connect WriteEnable to register WriteEnable input
            if not await self.connect(write_enable, reg_id, target_port_label="WriteEnable"):
                return False

            # Connect Reset to register Reset input
            if not await self.connect(reset, reg_id, target_port_label="Reset"):
                return False

        await self.log("  ✓ Created and connected 8 1-bit Registers in parallel")

        # Create output LEDs for Q bits. Reg[i]'s label is drawn IC-style (rotated
        # alongside the chip body) and its rendered height grows with the embedded
        # 1-bit Register's own port count, so a flat single-step gap isn't enough
        # clearance -- the label text reached down into Q[i]. An extra half-step
        # clears it (output Y order among Q[0..7] is unaffected: they all move
        # down by the same amount, so their relative order -- and X tiebreak -- is
        # unchanged).
        output_y = reg_y + 1.5 * VERTICAL_STAGE_SPACING
        for bit in range(8):
            q_led = await self.create_element("Led", reg_x + bit * HORIZONTAL_GATE_SPACING, output_y, f"Q[{bit}]")
            if q_led is None:
                return False

            if not await self.connect(registers[bit], q_led, source_port_label="Q"):
                return False

        await self.log("  ✓ Created 8 Q outputs")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level6_register_8bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 8-bit Register IC"
            f"({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = Register8BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "8-bit Register IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
