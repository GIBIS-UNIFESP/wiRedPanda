#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create D Latch IC

Inputs: D (Data), Enable
Outputs: Q, Q_bar

Circuit:
- Input data D is inverted with NOT gate to get D_bar
- When Enable=1: Pass D and D_bar to SR latch (S=D, R=D_bar)
- When Enable=0: Both S and R become 0 (hold state)
- Implementation uses: 1 NOT gate, 2 AND gates, 2 NOR gates (latch)

D Latch behavior:
- When Enable=1: Q follows D input
- When Enable=0: Q holds last value (latch holds state)

Usage:
    python create_d_latch.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import (
    HORIZONTAL_GATE_SPACING,
    VERTICAL_STAGE_SPACING,
)


class DLatchBuilder(ICBuilderBase):
    """Builder for D Latch IC"""

    async def create(self) -> bool:
        """Create the D Latch IC"""
        await self.begin_build("D Latch")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position calculation with proper spacing
        # 5 horizontal stages: Input -> NOT -> AND -> NOR -> LED
        input_x = 50.0
        not_x = input_x + HORIZONTAL_GATE_SPACING
        and_x = not_x + HORIZONTAL_GATE_SPACING
        nor_x = and_x + HORIZONTAL_GATE_SPACING
        output_x = nor_x + HORIZONTAL_GATE_SPACING

        top_y = 100.0
        bottom_y = top_y + VERTICAL_STAGE_SPACING

        # Create input switches for D and Enable
        input_d_id = await self.create_element("InputSwitch", input_x, top_y, "D")
        if input_d_id is None:
            return False
        await self.log(f"  ✓ Created input D (id={input_d_id})")

        input_en_id = await self.create_element("InputSwitch", input_x, bottom_y, "Enable")
        if input_en_id is None:
            return False
        await self.log(f"  ✓ Created input Enable (id={input_en_id})")

        # Create NOT gate to invert D
        not_id = await self.create_element("Not", not_x, top_y, "not_d")
        if not_id is None:
            return False
        await self.log(f"  ✓ Created NOT gate (id={not_id})")

        # Create AND gates to gate D and D_bar with Enable
        # AND1: D AND Enable -> S
        and1_id = await self.create_element("And", and_x, top_y, "and_s")
        if and1_id is None:
            return False
        await self.log(f"  ✓ Created AND1 gate (id={and1_id})")

        # AND2: D_bar AND Enable -> R
        and2_id = await self.create_element("And", and_x, bottom_y, "and_r")
        if and2_id is None:
            return False
        await self.log(f"  ✓ Created AND2 gate (id={and2_id})")

        # Create cross-coupled NOR gates (SR latch)
        # NOR1: (R, Q_bar_out) -> Q
        nor1_id = await self.create_element("Nor", nor_x, top_y, "nor_q")
        if nor1_id is None:
            return False
        await self.log(f"  ✓ Created NOR1 gate (id={nor1_id})")

        # NOR2: (R, Q_out) -> Q_bar
        nor2_id = await self.create_element("Nor", nor_x, bottom_y, "nor_qbar")
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

        # ========== Connect control logic ==========

        # Connect D to NOT gate
        if not await self.connect(input_d_id, not_id):
            return False

        # Connect D to AND1 port 0
        if not await self.connect(input_d_id, and1_id):
            return False

        # Connect Enable to AND1 port 1
        if not await self.connect(input_en_id, and1_id, target_port=1):
            return False

        # Connect NOT output to AND2 port 0
        if not await self.connect(not_id, and2_id):
            return False

        # Connect Enable to AND2 port 1
        if not await self.connect(input_en_id, and2_id, target_port=1):
            return False

        # ========== Connect SR latch ==========
        # Standard NOR-based SR latch:
        #   Q = NOR(R, Q_bar)  -> R (AND2) goes to NOR1
        #   Q_bar = NOR(S, Q)  -> S (AND1) goes to NOR2

        # Connect AND2 output to NOR1 port 0 (R input to Q-producing NOR)
        if not await self.connect(and2_id, nor1_id):
            return False

        # Connect NOR2 output to NOR1 port 1 (Q_bar feedback)
        if not await self.connect(nor2_id, nor1_id, target_port=1):
            return False

        # Connect AND1 output to NOR2 port 0 (S input to Q_bar-producing NOR)
        if not await self.connect(and1_id, nor2_id):
            return False

        # Connect NOR1 output to NOR2 port 1 (feedback)
        if not await self.connect(nor1_id, nor2_id, target_port=1):
            return False

        # ========== Connect outputs ==========

        # Connect NOR1 output to Q LED
        if not await self.connect(nor1_id, q_id):
            return False

        # Connect NOR2 output to Q_bar LED
        if not await self.connect(nor2_id, qbar_id):
            return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level1_d_latch.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created DLatch IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = DLatchBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "D Latch IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
