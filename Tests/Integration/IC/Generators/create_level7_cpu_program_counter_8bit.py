#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create CPU Level 7: 8-bit Program Counter IC

Composes the Level 6 Program Counter with additional CPU-level integration.

Inputs:
  Data[0..7] (load value)
  Clock, Load, Inc, Reset, WriteEnable

Outputs:
  Address[0..7] (current PC value)

Architecture:
  - Instantiate level6_program_counter_8bit (has register, adder, mux control logic)
  - Pass through all inputs/outputs to CPU interface

Usage:
    python create_level7_program_counter_8bit_cpu.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class ProgramCounter7BitBuilder(ICBuilderBase):
    """Builder for Level 7 8-bit Program Counter IC (composes Level 6 PC)"""

    async def create(self) -> bool:
        """Create the Level 7 8-bit Program Counter IC"""
        await self.begin_build("Program Counter 8-bit (Level 7)")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        data_input_x = 50.0
        data_input_y = 100.0
        control_input_x = data_input_x + (8 * HORIZONTAL_GATE_SPACING) + HORIZONTAL_GATE_SPACING
        control_input_y = data_input_y

        # Create all input ports
        data_inputs = []
        for i in range(8):
            element_id = await self.create_element("InputSwitch", data_input_x + (i * HORIZONTAL_GATE_SPACING), data_input_y, f"Data[{i}]")
            if element_id is None:
                return False
            data_inputs.append(element_id)

        # Control signals
        clock = await self.create_element("InputSwitch", control_input_x, control_input_y, "Clock")
        if clock is None:
            return False

        load = await self.create_element("InputSwitch", control_input_x, control_input_y + VERTICAL_STAGE_SPACING, "Load")
        if load is None:
            return False

        inc = await self.create_element("InputSwitch", control_input_x, control_input_y + (2 * VERTICAL_STAGE_SPACING), "Inc")
        if inc is None:
            return False

        reset = await self.create_element("InputSwitch", control_input_x, control_input_y + (3 * VERTICAL_STAGE_SPACING), "Reset")
        if reset is None:
            return False

        write_enable = await self.create_element("InputSwitch", control_input_x, control_input_y + (4 * VERTICAL_STAGE_SPACING), "WriteEnable")
        if write_enable is None:
            return False

        await self.log("  ✓ Created 8 data inputs + 5 control signals")

        # Instantiate Level 6 Program Counter IC
        pc_x = data_input_x + (4 * HORIZONTAL_GATE_SPACING)
        pc_y = control_input_y + (6 * VERTICAL_STAGE_SPACING)

        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level6_program_counter_8bit_arithmetic")):


            return False


        pc_ic = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level6_program_counter_8bit_arithmetic"), pc_x, pc_y, "PC")
        if pc_ic is None:
            return False
        await self.log("  ✓ Instantiated Level 6 Program Counter IC")

        # Connect all inputs to Level 6 PC
        if not await self.connect(clock, pc_ic, target_port_label="clock"):
            return False

        if not await self.connect(load, pc_ic, target_port_label="load"):
            return False

        if not await self.connect(inc, pc_ic, target_port_label="inc"):
            return False

        if not await self.connect(reset, pc_ic, target_port_label="reset"):
            return False

        # Connect Data inputs to PC loadValue ports
        for i in range(8):
            if not await self.connect(data_inputs[i], pc_ic, target_port_label=f"loadValue[{i}]"):
                return False

        await self.log("  ✓ Connected all control and data inputs to PC IC")

        # Note: WriteEnable is handled internally by Level 6 PC (load OR inc)
        # It's exposed here for CPU interface compatibility but not used

        # Create output LEDs and connect to PC outputs
        output_x = data_input_x
        output_y = pc_y + (3 * VERTICAL_STAGE_SPACING)

        for i in range(8):
            address_led = await self.create_element("Led", output_x + (i * HORIZONTAL_GATE_SPACING), output_y, f"Address[{i}]")
            if address_led is None:
                return False

            if not await self.connect(pc_ic, address_led, source_port_label=f"pc[{i}]"):
                return False

        await self.log("  ✓ Created 8 Address outputs")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level7_cpu_program_counter_8bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created Level 7 Program Counter IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = ProgramCounter7BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Level 7 8-bit Program Counter IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
