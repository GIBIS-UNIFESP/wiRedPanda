#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Data Forwarding Unit IC

Implements a 4-to-1 data forwarding multiplexer for bypassing data
between pipeline stages in a pipelined CPU architecture.

Inputs:
  DataA[0..7] (8-bit data from pipeline stage 1)
  DataB[0..7] (8-bit data from pipeline stage 2)
  DataC[0..7] (8-bit data from pipeline stage 3)
  DataD[0..7] (8-bit data from pipeline stage 4)
  Select[0..1] (2-bit select signal: 00=A, 01=B, 10=C, 11=D)

Outputs:
  ForwardedData[0..7] (8-bit selected/forwarded data)

Architecture:
  - 4-to-1 multiplexer for each data bit (8 multiplexers total)
  - Select signal determines which stage's data is forwarded
  - All data bits use the same select signal

Purpose:
  Resolves Read-After-Write (RAW) data hazards in pipelined processors
  by forwarding results from later pipeline stages to earlier stages,
  avoiding stalls and improving pipeline throughput.

Usage:
    python create_level7_data_forwarding_unit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class DataForwardingUnitBuilder(ICBuilderBase):
    """Builder for Data Forwarding Unit IC"""

    async def create(self) -> bool:
        """Create the Data Forwarding Unit IC"""
        await self.begin_build('Data Forwarding Unit')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0

        # Create DataA inputs (8-bit)
        data_a_inputs = []
        for i in range(8):
            data_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), 100.0, f"DataA[{i}]")
            if data_id is None:
                return False
            data_a_inputs.append(data_id)
        await self.log("  ✓ Created 8 DataA inputs")

        # Create DataB inputs (8-bit)
        data_b_inputs = []
        for i in range(8):
            data_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), 100.0 + VERTICAL_STAGE_SPACING, f"DataB[{i}]")
            if data_id is None:
                return False
            data_b_inputs.append(data_id)
        await self.log("  ✓ Created 8 DataB inputs")

        # Create DataC inputs (8-bit)
        data_c_inputs = []
        for i in range(8):
            data_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), 100.0 + (2 * VERTICAL_STAGE_SPACING), f"DataC[{i}]")
            if data_id is None:
                return False
            data_c_inputs.append(data_id)
        await self.log("  ✓ Created 8 DataC inputs")

        # Create DataD inputs (8-bit)
        data_d_inputs = []
        for i in range(8):
            data_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), 100.0 + (3 * VERTICAL_STAGE_SPACING), f"DataD[{i}]")
            if data_id is None:
                return False
            data_d_inputs.append(data_id)
        await self.log("  ✓ Created 8 DataD inputs")

        # Create Select inputs (2-bit)
        select_inputs = []
        for i in range(2):
            sel_id = await self.create_element("InputSwitch", input_x + (8 * HORIZONTAL_GATE_SPACING) + (i * HORIZONTAL_GATE_SPACING), 100.0 + VERTICAL_STAGE_SPACING, f"Select[{i}]")
            if sel_id is None:
                return False
            select_inputs.append(sel_id)
        await self.log("  ✓ Created 2 Select inputs")

        # Create 4-to-1 multiplexers for each bit using level2_mux_4to1 IC (8 total)
        output_x = input_x + (12 * HORIZONTAL_GATE_SPACING)
        mux_ids = []
        for bit_idx in range(8):
            # Instantiate level2_mux_4to1 IC for this bit
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_mux_4to1")):

                return False

            mux_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_mux_4to1"), output_x + (bit_idx * HORIZONTAL_GATE_SPACING), 250.0, f"Mux4to1_{bit_idx}")
            if mux_id is None:
                return False
            mux_ids.append(mux_id)

            # Connect DataA[bit] to Mux Data[0] input
            if not await self.connect(data_a_inputs[bit_idx], mux_id, target_port_label="Data[0]"):
                return False

            # Connect DataB[bit] to Mux Data[1] input
            if not await self.connect(data_b_inputs[bit_idx], mux_id, target_port_label="Data[1]"):
                return False

            # Connect DataC[bit] to Mux Data[2] input
            if not await self.connect(data_c_inputs[bit_idx], mux_id, target_port_label="Data[2]"):
                return False

            # Connect DataD[bit] to Mux Data[3] input
            if not await self.connect(data_d_inputs[bit_idx], mux_id, target_port_label="Data[3]"):
                return False

            # Connect Select[0] to Mux Sel[0] input
            if not await self.connect(select_inputs[0], mux_id, target_port_label="Sel[0]"):
                return False

            # Connect Select[1] to Mux Sel[1] input
            if not await self.connect(select_inputs[1], mux_id, target_port_label="Sel[1]"):
                return False

            # Create output LED
            led_id = await self.create_element("Led", output_x + (10 * HORIZONTAL_GATE_SPACING) + (bit_idx * HORIZONTAL_GATE_SPACING), 250.0, f"ForwardedData[{bit_idx}]")
            if led_id is None:
                return False

            # Connect Mux output to LED
            if not await self.connect(mux_id, led_id, source_port_label="Output"):
                return False

        await self.log("  ✓ Created 8-bit 4-to-1 multiplexer array using level2_mux_4to1 ICs")

        output_file = str(IC_COMPONENTS_DIR / "level7_data_forwarding_unit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created Data Forwarding Unit IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = DataForwardingUnitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Data Forwarding Unit IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
