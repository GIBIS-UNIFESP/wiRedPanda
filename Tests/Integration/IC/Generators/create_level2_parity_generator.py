#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Parity Generator IC (4-bit)

Given 4 data bits, generates the parity bit (XOR of all inputs).
Output is 1 when the number of 1-bits is odd (odd parity).

Inputs:
  - Data[0] to Data[3]: 4 data input bits

Outputs:
  - Parity: 1-bit parity output

Architecture:
  - Binary tree of XOR gates (2 stages)
  - Stage 1: 2 XOR gates (pairs)
  - Stage 2: 1 XOR gate (final)
  - Output LED

Usage:
    python create_level2_parity_generator.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class ParityGeneratorBuilder(ICBuilderBase):
    """Builder for 4-bit Parity Generator IC"""

    async def create(self) -> bool:
        """Create the 4-bit Parity Generator IC"""
        width = 4

        await self.begin_build("Parity Generator")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position calculation
        input_x = 50.0
        output_x = input_x + HORIZONTAL_GATE_SPACING * 5

        # Create input switches for Data[0 to 3]
        data_inputs = []
        for i in range(width):
            element_id = await self.create_element("InputSwitch", input_x, 100.0 + i * VERTICAL_STAGE_SPACING, f"Data[{i}]")
            if element_id is None:
                return False
            data_inputs.append(element_id)

        await self.log(f"  ✓ Created {width} input switches")

        # Build binary XOR tree
        current_stage = data_inputs
        stage_num = 1
        stage_x = input_x + HORIZONTAL_GATE_SPACING

        while len(current_stage) > 1:
            next_stage = []

            # Create pairs of XOR gates for current stage
            for pair_idx in range(0, len(current_stage), 2):
                xor_id = await self.create_element("Xor", stage_x + (pair_idx // 2 % 2) * HORIZONTAL_GATE_SPACING, 100.0 + (pair_idx // 4) * VERTICAL_STAGE_SPACING, f"xor_s{stage_num}[{pair_idx // 2}]")
                if xor_id is None:
                    return False
                next_stage.append(xor_id)

                # Connect inputs to this XOR gate
                for port_idx in range(2):
                    input_idx = pair_idx + port_idx
                    if input_idx < len(current_stage):
                        if not await self.connect(current_stage[input_idx], xor_id, target_port=port_idx):
                            return False

            await self.log(f"  ✓ Created stage {stage_num}: {len(next_stage)} XOR gates")

            current_stage = next_stage
            stage_num += 1
            stage_x += HORIZONTAL_GATE_SPACING

        # Final parity output (should be single XOR gate)
        parity_xor_id = current_stage[0]

        # Create output LED
        parity_led = await self.create_element("Led", output_x, 100.0 + (width // 2) * VERTICAL_STAGE_SPACING, "Parity")
        if parity_led is None:
            return False

        # Connect final XOR to output LED
        if not await self.connect(parity_xor_id, parity_led):
            return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level2_parity_generator.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created Parity Generator IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = ParityGeneratorBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Parity Generator IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
