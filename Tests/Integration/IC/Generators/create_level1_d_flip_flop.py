#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create D Flip-Flop IC with Preset and Clear

Inputs: D (Data), Clock, Preset (active HIGH), Clear (active HIGH)
Outputs: Q, Q_bar

Circuit (gate-level master-slave, built from primitives — not "D Latch" parts):
- Master is a gated SR latch transparent while Clock=LOW; slave is a gated SR
  latch transparent while Clock=HIGH. The pair captures D on the RISING edge
  (slave opens and copies what the master grabbed during the low phase).
- D is steered into S/R via AND gates gated by the clock level; the slave's R
  path reuses the master's own Qm_bar output (a true complement), so no slave
  NOT(Qm) gate is needed.
- Preset/Clear override normal operation (asynchronous), injected into BOTH
  latches so they work under any clock level (F56).

Implementation (gate inventory):
- 1 NOT gate: invert Clock.
- 1 NOT gate: master D_bar (master_not_d).
- Master: 2 AND (D*clk', D_bar*clk' -> S/R), 2 NOR (cross-coupled core).
- Slave:  2 AND (Qm*clk, Qm_bar*clk -> S/R), 2 NOR (cross-coupled core).
- 4 OR gates: inject Preset/Clear DIRECTLY (active-high) into the S/R of BOTH
  latches (master_or_s/master_or_r + or_s/or_r) — forcing master and slave, so
  async controls survive any clock level and the forced value holds on release
  (F56).

D Flip-Flop behavior:
- RISING-edge triggered: Q captures D on the clock's rising edge.
  (Note: the JK flip-flop in this library is FALLING-edge — mind the polarity
  when composing the two.)
- Output stable during the rest of the clock cycle.
- Preset (active HIGH): Forces Q=1, Q_bar=0
- Clear (active HIGH): Forces Q=0, Q_bar=1

Usage:
    python3 create_level1_d_flip_flop.py
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
        input_clear_id = await self.create_element(
            "InputSwitch", input_x, bottom_y + (2 * VERTICAL_STAGE_SPACING), "Clear"
        )
        if input_clear_id is None:
            return False
        await self.log(f"  ✓ Created input Clear (id={input_clear_id})")

        # Create NOT gate to invert clock
        not_id = await self.create_element("Not", not_clk_x, bottom_y, "not_clk")
        if not_id is None:
            return False
        await self.log(f"  ✓ Created NOT gate (id={not_id})")

        # ========== Preset/Clear Control Logic (active-HIGH) ==========
        # Preset/Clear are injected DIRECTLY into the latch S/R OR-gates (no input
        # inverters): Preset=1 forces Q=1, Clear=1 forces Q=0. Injected into BOTH
        # latches (F56) so the forced value holds under any clock level.

        # OR gate for R: injects the data reset path and Clear into the
        # Q NOR — driving it high forces Q=0. (F31: this gate was labelled
        # "or_s" although it performs the Reset function.)
        or_r_id = await self.create_element("Or", master_not_x, bottom_y + (2 * VERTICAL_STAGE_SPACING), "or_r")
        if or_r_id is None:
            return False

        # OR gate for S: injects the data set path and NOT(Preset) into the
        # Q_bar NOR — driving it high forces Q_bar=0, i.e. Q=1.
        or_s_id = await self.create_element("Or", master_not_x, bottom_y + (3 * VERTICAL_STAGE_SPACING), "or_s")
        if or_s_id is None:
            return False

        # Master-side injection (F56): a 7474 forces BOTH latches. Without
        # these, asserting Preset/Clear while Clock=1 contends with the open
        # slave's data path (master holding the opposite value drives the
        # invalid Q=Q_bar=0 state, and the forced level is lost on release).
        master_or_r_id = await self.create_element(
            "Or", master_not_x, bottom_y + (4 * VERTICAL_STAGE_SPACING), "master_or_r"
        )
        if master_or_r_id is None:
            return False

        master_or_s_id = await self.create_element(
            "Or", master_not_x, bottom_y + (5 * VERTICAL_STAGE_SPACING), "master_or_s"
        )
        if master_or_s_id is None:
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

        # Master NOR1: (R, Qm_bar) -> Qm (R forces Qm=0; F62: was mislabelled "S")
        master_nor1_id = await self.create_element("Nor", master_nor_x, top_y, "master_nor_q")
        if master_nor1_id is None:
            return False

        # Master NOR2: (S, Qm) -> Qm_bar (S forces Qm_bar=0; F62: was mislabelled "R")
        master_nor2_id = await self.create_element("Nor", master_nor_x, bottom_y, "master_nor_qbar")
        if master_nor2_id is None:
            return False

        # ========== Slave Latch Components ==========
        # The slave's R path uses the master's own Qm_bar output
        # (master_nor2) directly — no separate NOT(Qm) gate, since the master
        # latch already produces the complement and there is no slave→master
        # feedback to disturb its settle.

        # Slave AND1: Qm AND Clock -> S
        slave_and1_id = await self.create_element("And", slave_and_x, top_y, "slave_and_s")
        if slave_and1_id is None:
            return False

        # Slave AND2: Qm_bar AND Clock -> R
        slave_and2_id = await self.create_element("And", slave_and_x, bottom_y, "slave_and_r")
        if slave_and2_id is None:
            return False

        # Slave NOR1: (R, Q_bar) -> Q (R forces Q=0; F62: was mislabelled "S")
        slave_nor1_id = await self.create_element("Nor", slave_nor_x, top_y, "slave_nor_q")
        if slave_nor1_id is None:
            return False

        # Slave NOR2: (S, Q) -> Q_bar (S forces Q_bar=0; F62: was mislabelled "R")
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

        # Master R path: OR(data reset path, Clear) into the Q NOR (F56)
        if not await self.connect(master_and2_id, master_or_r_id):
            return False

        if not await self.connect(input_clear_id, master_or_r_id, target_port=1):
            return False

        if not await self.connect(master_or_r_id, master_nor1_id):
            return False

        # Connect master NOR2 to master NOR1 (feedback)
        if not await self.connect(master_nor2_id, master_nor1_id, target_port=1):
            return False

        # Master S path: OR(data set path, Preset) into the Q_bar NOR (F56)
        if not await self.connect(master_and1_id, master_or_s_id):
            return False

        if not await self.connect(input_preset_id, master_or_s_id, target_port=1):
            return False

        if not await self.connect(master_or_s_id, master_nor2_id):
            return False

        # Connect master NOR1 to master NOR2 (feedback)
        if not await self.connect(master_nor1_id, master_nor2_id, target_port=1):
            return False

        # ========== Connect Slave Latch ==========

        # Connect master NOR1 to slave AND1
        if not await self.connect(master_nor1_id, slave_and1_id):
            return False

        # Connect Clock to slave AND1
        if not await self.connect(input_clk_id, slave_and1_id, target_port=1):
            return False

        # Connect master NOR2 (Qm_bar) directly to slave AND2
        if not await self.connect(master_nor2_id, slave_and2_id):
            return False

        # Connect Clock to slave AND2
        if not await self.connect(input_clk_id, slave_and2_id, target_port=1):
            return False

        # Connect slave AND2 to OR R gate (input 0)
        if not await self.connect(slave_and2_id, or_r_id):
            return False

        # Connect Preset to OR S gate (input 1) — active-high
        if not await self.connect(input_preset_id, or_s_id, target_port=1):
            return False

        # Connect OR S to slave NOR2 (S forces Q_bar=0 to make Q=1)
        if not await self.connect(or_s_id, slave_nor2_id):
            return False

        # Connect slave NOR2 to slave NOR1 (feedback)
        if not await self.connect(slave_nor2_id, slave_nor1_id, target_port=1):
            return False

        # Connect slave AND1 to OR S gate (input 0)
        if not await self.connect(slave_and1_id, or_s_id):
            return False

        # Connect Clear to OR R gate (input 1) — active-high
        if not await self.connect(input_clear_id, or_r_id, target_port=1):
            return False

        # Connect OR R to slave NOR1 (R forces Q=0 to make Q_bar=1)
        if not await self.connect(or_r_id, slave_nor1_id):
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

        await self.log(
            f"✅ Successfully created DFlipFlop IC ({self.element_count} elements, {self.connection_count} connections)"
        )
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
