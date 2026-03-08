#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create SR Latch IC

Inputs: S (Set), R (Reset)
Outputs: Q, Q_bar (complement)

Circuit:
- 2 cross-coupled NOR gates
- SR Latch truth table:
  - S=1, R=0 -> Q=1, Q_bar=0 (Set)
  - S=0, R=1 -> Q=0, Q_bar=1 (Reset)
  - S=0, R=0 -> Q=previous, Q_bar=previous (Hold)
  - S=1, R=1 -> Invalid (both outputs go low)

Usage:
    python create_sr_latch.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import (
    HORIZONTAL_GATE_SPACING,
    VERTICAL_STAGE_SPACING,
)


class SRLatchBuilder(ICBuilderBase):
    """Builder for SR Latch IC"""

    async def create(self) -> bool:
        """Create the SR Latch IC"""
        await self.begin_build("SR Latch")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position calculation with proper spacing
        input_x = 50.0
        gate_x = input_x + HORIZONTAL_GATE_SPACING
        output_x = gate_x + HORIZONTAL_GATE_SPACING

        top_y = 100.0
        bottom_y = top_y + VERTICAL_STAGE_SPACING

        # Create input switches for S and R
        input_s_id = await self.create_element("InputSwitch", input_x, top_y, "S")
        if input_s_id is None:
            return False
        await self.log(f"  ✓ Created input S (id={input_s_id})")

        input_r_id = await self.create_element("InputSwitch", input_x, bottom_y, "R")
        if input_r_id is None:
            return False
        await self.log(f"  ✓ Created input R (id={input_r_id})")

        # Create cross-coupled NOR gates (feedback loops)
        # NOR1: (R, Q_bar_out) -> Q
        nor1_id = await self.create_element("Nor", gate_x, top_y, "nor_q")
        if nor1_id is None:
            return False
        await self.log(f"  ✓ Created NOR1 gate (id={nor1_id})")

        # NOR2: (S, Q_out) -> Q_bar
        nor2_id = await self.create_element("Nor", gate_x, bottom_y, "nor_qbar")
        if nor2_id is None:
            return False
        await self.log(f"  ✓ Created NOR2 gate (id={nor2_id})")

        # Create output LEDs
        q_id = await self.create_element("Led", output_x, top_y, "Q")
        if q_id is None:
            return False
        await self.log(f"  ✓ Created Q output (id={q_id})")

        qbar_id = await self.create_element("Led", output_x, bottom_y, "Q_bar")
        if qbar_id is None:
            return False
        await self.log(f"  ✓ Created Q_bar output (id={qbar_id})")

        # ========== Connect cross-coupled latch ==========
        # Standard NOR-based SR latch:
        #   Q = NOR(R, Q_bar)  -> R goes to NOR1
        #   Q_bar = NOR(S, Q)  -> S goes to NOR2

        # Connect R to NOR1 port 0 (R input to Q-producing NOR)
        if not await self.connect(input_r_id, nor1_id):
            return False

        # Connect NOR2 output to NOR1 port 1 (Q_bar feedback)
        if not await self.connect(nor2_id, nor1_id, target_port=1):
            return False

        # Connect S to NOR2 port 0 (S input to Q_bar-producing NOR)
        if not await self.connect(input_s_id, nor2_id):
            return False

        # Connect NOR1 output to NOR2 port 1 (Q feedback)
        if not await self.connect(nor1_id, nor2_id, target_port=1):
            return False

        # ========== Connect outputs to LEDs ==========

        # Connect NOR1 output to Q LED
        if not await self.connect(nor1_id, q_id):
            return False

        # Connect NOR2 output to Q_bar LED
        if not await self.connect(nor2_id, qbar_id):
            return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level1_sr_latch.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created SRLatch IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = SRLatchBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "SR Latch IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
