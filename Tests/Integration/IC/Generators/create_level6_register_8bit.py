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
    python create_cpu_8bit_register.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class Register8BitBuilder(ICBuilderBase):
    """Builder for 8-bit Register IC"""

    async def create(self) -> bool:
        """Create the 8-bit Register IC"""
        await self.begin_build("8-bit Register")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Create 8-bit Data input switches (vertically stacked)
        data_inputs = []
        input_x = 50.0
        for i in range(8):
            element_id = await self.create_element("InputSwitch", input_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"Data[{i}]")
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
        reg_x = ctrl_x + HORIZONTAL_GATE_SPACING
        reg_y = 100.0
        for bit in range(8):
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level3_register_1bit")):

                return False

            reg_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level3_register_1bit"), reg_x + bit * HORIZONTAL_GATE_SPACING, reg_y, f"Reg[{bit}]")
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

        # Create output LEDs for Q bits
        output_y = reg_y + VERTICAL_STAGE_SPACING
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

        await self.log(f"✅ Successfully created 8-bit Register IC ({self.element_count} elements, {self.connection_count} connections)")
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
