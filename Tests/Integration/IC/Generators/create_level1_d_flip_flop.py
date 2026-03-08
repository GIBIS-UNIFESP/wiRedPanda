#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create D Flip-Flop IC with Preset and Clear

Inputs: D (Data), Clock, Preset (active LOW), Clear (active LOW)
Outputs: Q, Q_bar

Circuit:
- Master-Slave configuration using two D latches
- Master latch active on clock low
- Slave latch active on clock high
- Ensures output changes only on clock edge
- Preset/Clear override normal operation (asynchronous)

Implementation:
- 1 NOT gate (invert clock for master/slave control)
- 2 NOT gates (invert Preset and Clear for OR logic)
- 2 OR gates (combine Preset/Clear with S/R signals)
- 2 D Latches (master and slave)

D Flip-Flop behavior:
- Output changes on rising edge of clock
- Input D sampled and captured on clock edge
- Output stable during clock cycle
- Preset (active LOW): Forces Q=1, Q_bar=0
- Clear (active LOW): Forces Q=0, Q_bar=1

Usage:
    python create_d_flip_flop.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import (
    HORIZONTAL_GATE_SPACING,
    VERTICAL_STAGE_SPACING,
)


class DFlipFlopBuilder(ICBuilderBase):
    """Builder for D Flip-Flop IC using master-slave latches"""

    async def create(self) -> bool:
        """Create the D Flip-Flop IC"""
        await self.begin_build("D Flip-Flop")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position calculation with proper spacing (8 stages horizontally)
        input_x = 50.0
        not_clk_x = input_x + HORIZONTAL_GATE_SPACING
        master_not_x = not_clk_x + HORIZONTAL_GATE_SPACING
        master_and_x = master_not_x + HORIZONTAL_GATE_SPACING
        master_nor_x = master_and_x + HORIZONTAL_GATE_SPACING
        slave_not_x = master_nor_x + HORIZONTAL_GATE_SPACING
        slave_and_x = slave_not_x + HORIZONTAL_GATE_SPACING
        slave_nor_x = slave_and_x + HORIZONTAL_GATE_SPACING
        output_x = slave_nor_x + HORIZONTAL_GATE_SPACING

        top_y = 100.0
        bottom_y = top_y + VERTICAL_STAGE_SPACING

        # Create input switches for D and Clock
        input_d_id = await self.create_element("InputSwitch", input_x, top_y, "D")
        if input_d_id is None:
            return False
        await self.log(f"  ✓ Created input D (id={input_d_id})")

        input_clk_id = await self.create_element("InputSwitch", input_x, bottom_y, "Clock")
        if input_clk_id is None:
            return False
        await self.log(f"  ✓ Created input Clock (id={input_clk_id})")

        # Create PRESET input (active LOW)
        input_preset_id = await self.create_element("InputSwitch", input_x, bottom_y + VERTICAL_STAGE_SPACING, "Preset")
        if input_preset_id is None:
            return False
        await self.log(f"  ✓ Created input Preset (id={input_preset_id})")

        # Create CLEAR input (active LOW)
        input_clear_id = await self.create_element("InputSwitch", input_x, bottom_y + (2 * VERTICAL_STAGE_SPACING), "Clear")
        if input_clear_id is None:
            return False
        await self.log(f"  ✓ Created input Clear (id={input_clear_id})")

        # Create NOT gate to invert clock
        not_id = await self.create_element("Not", not_clk_x, bottom_y, "not_clk")
        if not_id is None:
            return False
        await self.log(f"  ✓ Created NOT gate (id={not_id})")

        # ========== Preset/Clear Control Logic ==========

        # NOT gate to invert Preset
        not_preset_id = await self.create_element("Not", not_clk_x, bottom_y + VERTICAL_STAGE_SPACING, "not_preset")
        if not_preset_id is None:
            return False

        # NOT gate to invert Clear
        not_clear_id = await self.create_element("Not", not_clk_x, bottom_y + (2 * VERTICAL_STAGE_SPACING), "not_clear")
        if not_clear_id is None:
            return False

        # OR gate for S (slave_and1 OR NOT(clear))
        or_s_id = await self.create_element("Or", master_not_x, bottom_y + (2 * VERTICAL_STAGE_SPACING), "or_s")
        if or_s_id is None:
            return False

        # OR gate for R (slave_and2 OR NOT(preset))
        or_r_id = await self.create_element("Or", master_not_x, bottom_y + (3 * VERTICAL_STAGE_SPACING), "or_r")
        if or_r_id is None:
            return False

        # Connect Preset to NOT gate
        if not await self.connect(input_preset_id, not_preset_id):
            return False

        # Connect Clear to NOT gate
        if not await self.connect(input_clear_id, not_clear_id):
            return False

        # ========== Master Latch Components ==========

        # Master NOT gate to invert D
        master_not_id = await self.create_element("Not", master_not_x, top_y, "master_not_d")
        if master_not_id is None:
            return False

        # Master AND1: D AND NOT Clock -> S
        master_and1_id = await self.create_element("And", master_and_x, top_y, "master_and_s")
        if master_and1_id is None:
            return False

        # Master AND2: NOT D AND NOT Clock -> R
        master_and2_id = await self.create_element("And", master_and_x, bottom_y, "master_and_r")
        if master_and2_id is None:
            return False

        # Master NOR1: (S, Q_bar) -> Qm
        master_nor1_id = await self.create_element("Nor", master_nor_x, top_y, "master_nor_q")
        if master_nor1_id is None:
            return False

        # Master NOR2: (R, Q) -> Qm_bar
        master_nor2_id = await self.create_element("Nor", master_nor_x, bottom_y, "master_nor_qbar")
        if master_nor2_id is None:
            return False

        # ========== Slave Latch Components ==========

        # Slave NOT gate to invert Qm
        slave_not_id = await self.create_element("Not", slave_not_x, top_y, "slave_not_qm")
        if slave_not_id is None:
            return False

        # Slave AND1: Qm AND Clock -> S
        slave_and1_id = await self.create_element("And", slave_and_x, top_y, "slave_and_s")
        if slave_and1_id is None:
            return False

        # Slave AND2: Qm_bar AND Clock -> R
        slave_and2_id = await self.create_element("And", slave_and_x, bottom_y, "slave_and_r")
        if slave_and2_id is None:
            return False

        # Slave NOR1: (S, Q_bar) -> Q
        slave_nor1_id = await self.create_element("Nor", slave_nor_x, top_y, "slave_nor_q")
        if slave_nor1_id is None:
            return False

        # Slave NOR2: (R, Q) -> Q_bar
        slave_nor2_id = await self.create_element("Nor", slave_nor_x, bottom_y, "slave_nor_qbar")
        if slave_nor2_id is None:
            return False

        # Create output LEDs
        q_id = await self.create_element("Led", output_x, top_y, "Q")
        if q_id is None:
            return False

        qbar_id = await self.create_element("Led", output_x, bottom_y, "Q_bar")
        if qbar_id is None:
            return False

        # ========== Connect clock inversion ==========

        if not await self.connect(input_clk_id, not_id):
            return False

        # ========== Connect Master Latch ==========

        # Connect D to master NOT
        if not await self.connect(input_d_id, master_not_id):
            return False

        # Connect D to master AND1
        if not await self.connect(input_d_id, master_and1_id):
            return False

        # Connect NOT Clock to master AND1
        if not await self.connect(not_id, master_and1_id, target_port=1):
            return False

        # Connect master NOT to master AND2
        if not await self.connect(master_not_id, master_and2_id):
            return False

        # Connect NOT Clock to master AND2
        if not await self.connect(not_id, master_and2_id, target_port=1):
            return False

        # Connect master AND2 to master NOR1 (R to NOR1)
        if not await self.connect(master_and2_id, master_nor1_id):
            return False

        # Connect master NOR2 to master NOR1 (feedback)
        if not await self.connect(master_nor2_id, master_nor1_id, target_port=1):
            return False

        # Connect master AND1 to master NOR2 (S to NOR2)
        if not await self.connect(master_and1_id, master_nor2_id):
            return False

        # Connect master NOR1 to master NOR2 (feedback)
        if not await self.connect(master_nor1_id, master_nor2_id, target_port=1):
            return False

        # ========== Connect Slave Latch ==========

        # Connect master NOR1 to slave NOT
        if not await self.connect(master_nor1_id, slave_not_id):
            return False

        # Connect master NOR1 to slave AND1
        if not await self.connect(master_nor1_id, slave_and1_id):
            return False

        # Connect Clock to slave AND1
        if not await self.connect(input_clk_id, slave_and1_id, target_port=1):
            return False

        # Connect slave NOT to slave AND2
        if not await self.connect(slave_not_id, slave_and2_id):
            return False

        # Connect Clock to slave AND2
        if not await self.connect(input_clk_id, slave_and2_id, target_port=1):
            return False

        # Connect slave AND2 to OR S gate (input 0)
        if not await self.connect(slave_and2_id, or_s_id):
            return False

        # Connect NOT Preset to OR R gate (input 1)
        if not await self.connect(not_preset_id, or_r_id, target_port=1):
            return False

        # Connect OR R to slave NOR2 (R forces Q_bar=0 to make Q=1)
        if not await self.connect(or_r_id, slave_nor2_id):
            return False

        # Connect slave NOR2 to slave NOR1 (feedback)
        if not await self.connect(slave_nor2_id, slave_nor1_id, target_port=1):
            return False

        # Connect slave AND1 to OR R gate (input 0)
        if not await self.connect(slave_and1_id, or_r_id):
            return False

        # Connect NOT Clear to OR S gate (input 1)
        if not await self.connect(not_clear_id, or_s_id, target_port=1):
            return False

        # Connect OR S to slave NOR1 (S forces Q=0 to make Q_bar=1)
        if not await self.connect(or_s_id, slave_nor1_id):
            return False

        # Connect slave NOR1 to slave NOR2 (feedback)
        if not await self.connect(slave_nor1_id, slave_nor2_id, target_port=1):
            return False

        # ========== Connect final outputs ==========

        # Connect slave NOR1 to Q LED
        if not await self.connect(slave_nor1_id, q_id):
            return False

        # Connect slave NOR2 to Q_bar LED
        if not await self.connect(slave_nor2_id, qbar_id):
            return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level1_d_flip_flop.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created DFlipFlop IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = DFlipFlopBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "D Flip-Flop IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
