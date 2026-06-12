#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create JK Flip-Flop IC with Preset and Clear

Inputs: J (Set), K (Reset), Clock, Preset (active LOW), Clear (active LOW)
Outputs: Q, Q_bar

Circuit (textbook pulse-triggered master-slave JK, 7476-style — F55):
- Master latch is transparent while Clock=1 and takes its J/K feedback from
  the SLAVE outputs (S_m = J AND Q_bar_slave, R_m = K AND Q_slave), which are
  locked during the high phase — so J=K=1 is a clean toggle with no
  combinational race (the old circuit fed the master back its own outputs,
  leaving the J=K=1 master with no stable state).
- Slave latch transfers the master state while Clock=0: Q changes on the
  FALLING edge (end of the clock pulse). NOTE: this is the opposite polarity
  from the RISING-edge D flip-flop in this library — mind the edge when
  composing the two into counters/registers.
- When J=1, K=0: Set Q=1
- When J=0, K=1: Reset Q=0
- When J=1, K=1: Toggle Q
- When J=0, K=0: Hold Q
- Preset/Clear override normal operation (asynchronous), injected into BOTH
  latches (matches the F56 D flip-flop fix) so they work under any clock
  level and the forced value survives release.
- Classic master-slave caveat (textbook "1's catching"): J/K pulses during
  the clock-high phase are caught by the master; Q itself only changes on
  the falling edge.

Implementation:
- 2 AND gates for J/K with slave feedback
- 4 AND gates for clock gating (master pair on CLK, slave pair on NOT CLK)
- Clock control via NOT gate
- Master-slave SR latch cores
- Preset/Clear via OR gate injection into both latches

JK Flip-Flop truth table:
J K | Action
----|-------
0 0 | Hold
0 1 | Reset (Q=0)
1 0 | Set (Q=1)
1 1 | Toggle

Preset (active LOW): Forces Q=1, Q_bar=0
Clear (active LOW): Forces Q=0, Q_bar=1

Usage:
    python create_jk_flip_flop.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import (
    HORIZONTAL_GATE_SPACING,
    VERTICAL_STAGE_SPACING,
)


class JKFlipFlopBuilder(ICBuilderBase):
    """Builder for JK Flip-Flop IC"""

    async def create(self) -> bool:
        """Create the JK Flip-Flop IC"""
        await self.begin_build("JK Flip-Flop")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position calculation with proper spacing (7 stages horizontally + clock control)
        input_x = 50.0
        not_clk_x = input_x + HORIZONTAL_GATE_SPACING
        master_and_x = not_clk_x + HORIZONTAL_GATE_SPACING
        master_gate_x = master_and_x + HORIZONTAL_GATE_SPACING
        master_nor_x = master_gate_x + HORIZONTAL_GATE_SPACING
        slave_and_x = master_nor_x + HORIZONTAL_GATE_SPACING
        slave_gate_x = slave_and_x + HORIZONTAL_GATE_SPACING
        slave_nor_x = slave_gate_x + HORIZONTAL_GATE_SPACING
        output_x = slave_nor_x + HORIZONTAL_GATE_SPACING

        row1_y = 100.0  # Top row: J, master S, gate S, NOR Q, slave S, gate S, NOR Q, Q output
        row2_y = row1_y + VERTICAL_STAGE_SPACING  # Bottom row: K, master R, gate R, NOR Q_bar, slave R, gate R, NOR Q_bar, Q_bar output
        clock_y = row2_y + VERTICAL_STAGE_SPACING  # Third row for Clock input
        preset_y = clock_y + VERTICAL_STAGE_SPACING  # Fourth row for Preset input
        clear_y = preset_y + VERTICAL_STAGE_SPACING  # Fifth row for Clear input

        # Create input switches for J, K, Clock, Preset, and Clear
        input_j_id = await self.create_element("InputSwitch", input_x, row1_y, "J")
        if input_j_id is None:
            return False
        await self.log(f"  ✓ Created input J (id={input_j_id})")

        input_k_id = await self.create_element("InputSwitch", input_x, row2_y, "K")
        if input_k_id is None:
            return False
        await self.log(f"  ✓ Created input K (id={input_k_id})")

        input_clk_id = await self.create_element("InputSwitch", input_x, clock_y, "Clock")
        if input_clk_id is None:
            return False
        await self.log(f"  ✓ Created input Clock (id={input_clk_id})")

        # Create PRESET input (active LOW)
        input_preset_id = await self.create_element("InputSwitch", input_x, preset_y, "Preset")
        if input_preset_id is None:
            return False
        await self.log(f"  ✓ Created input Preset (id={input_preset_id})")

        # Create CLEAR input (active LOW)
        input_clear_id = await self.create_element("InputSwitch", input_x, clear_y, "Clear")
        if input_clear_id is None:
            return False
        await self.log(f"  ✓ Created input Clear (id={input_clear_id})")

        # Create NOT gate for clock inversion
        not_clk_id = await self.create_element("Not", not_clk_x, clock_y, "not_clk")
        if not_clk_id is None:
            return False
        await self.log(f"  ✓ Created NOT Clock gate (id={not_clk_id})")

        # ========== Preset/Clear Control Logic ==========

        # NOT gate to invert Preset
        not_preset_id = await self.create_element("Not", not_clk_x, preset_y, "not_preset")
        if not_preset_id is None:
            return False

        # NOT gate to invert Clear
        not_clear_id = await self.create_element("Not", not_clk_x, clear_y, "not_clear")
        if not_clear_id is None:
            return False

        # Slave reset-side OR (slave_gate_r OR NOT(clear)) -> drives the Q NOR.
        # When Clear=0: NOT(0)=1 -> OR forces slave NOR Q input high -> Q=0.
        # (F63: this gate performs the Reset function, so it is labelled "or_r"
        # to match the master pair and the D-FF convention — was mislabelled "or_s".)
        or_r_id = await self.create_element(
            "Or", slave_gate_x + (HORIZONTAL_GATE_SPACING / 2), row1_y + (VERTICAL_STAGE_SPACING * 2), "or_r")
        if or_r_id is None:
            return False

        # Slave set-side OR (slave_gate_s OR NOT(preset)) -> drives the Q_bar NOR.
        # When Preset=0: NOT(0)=1 -> OR forces slave NOR Q_bar input high -> Q_bar=0 -> Q=1.
        # (F63: this gate performs the Set function, so it is labelled "or_s" —
        # was mislabelled "or_r".)
        or_s_id = await self.create_element(
            "Or", slave_gate_x + (HORIZONTAL_GATE_SPACING / 2), row1_y + (VERTICAL_STAGE_SPACING * 3), "or_s")
        if or_s_id is None:
            return False

        # Master-side injection (F56-consistent): force BOTH latches on
        # Preset/Clear so async controls work under any clock level.
        master_or_s_id = await self.create_element(
            "Or", master_gate_x + (HORIZONTAL_GATE_SPACING / 2), row1_y + (VERTICAL_STAGE_SPACING * 2), "master_or_s")
        if master_or_s_id is None:
            return False

        master_or_r_id = await self.create_element(
            "Or", master_gate_x + (HORIZONTAL_GATE_SPACING / 2), row1_y + (VERTICAL_STAGE_SPACING * 3), "master_or_r")
        if master_or_r_id is None:
            return False

        # Connect Preset to NOT gate
        if not await self.connect(input_preset_id, not_preset_id):
            return False

        # Connect Clear to NOT gate
        if not await self.connect(input_clear_id, not_clear_id):
            return False

        # ========== Master Stage - Feedback Logic ==========

        # AND gate: J AND Q_bar (for setting via feedback) - will be connected later
        master_and_s_id = await self.create_element("And", master_and_x, row1_y, "master_and_s")
        if master_and_s_id is None:
            return False

        # AND gate: K AND Q (for resetting via feedback) - will be connected later
        master_and_r_id = await self.create_element("And", master_and_x, row2_y, "master_and_r")
        if master_and_r_id is None:
            return False

        # AND gates for clock gating
        master_gate_s_id = await self.create_element("And", master_gate_x, row1_y, "master_gate_s")
        if master_gate_s_id is None:
            return False

        master_gate_r_id = await self.create_element("And", master_gate_x, row2_y, "master_gate_r")
        if master_gate_r_id is None:
            return False

        # Master SR Latch NOR gates
        master_nor_q_id = await self.create_element("Nor", master_nor_x, row1_y, "master_nor_q")
        if master_nor_q_id is None:
            return False

        master_nor_qbar_id = await self.create_element("Nor", master_nor_x, row2_y, "master_nor_qbar")
        if master_nor_qbar_id is None:
            return False

        # ========== Slave Stage ==========
        # (F55: the old slave_and_s/slave_and_r J/K-gated transfer gates are
        # gone — a textbook slave just transfers the master state.)

        # Slave gate AND gates
        slave_gate_s_id = await self.create_element("And", slave_gate_x, row1_y, "slave_gate_s")
        if slave_gate_s_id is None:
            return False

        slave_gate_r_id = await self.create_element("And", slave_gate_x, row2_y, "slave_gate_r")
        if slave_gate_r_id is None:
            return False

        # Slave SR Latch NOR gates
        slave_nor_q_id = await self.create_element("Nor", slave_nor_x, row1_y, "slave_nor_q")
        if slave_nor_q_id is None:
            return False

        slave_nor_qbar_id = await self.create_element("Nor", slave_nor_x, row2_y, "slave_nor_qbar")
        if slave_nor_qbar_id is None:
            return False

        # Create output LEDs
        q_id = await self.create_element("Led", output_x, row1_y, "Q")
        if q_id is None:
            return False

        qbar_id = await self.create_element("Led", output_x, row2_y, "Q_bar")
        if qbar_id is None:
            return False

        # ========== Connect Clock Inversion ==========

        if not await self.connect(input_clk_id, not_clk_id):
            return False

        # ========== Connect Master Stage ==========

        # Connect J to master AND S
        if not await self.connect(input_j_id, master_and_s_id):
            return False

        # Connect K to master AND R
        if not await self.connect(input_k_id, master_and_r_id):
            return False

        # Connect master AND S to master gate S port 0
        if not await self.connect(master_and_s_id, master_gate_s_id):
            return False

        # Connect Clock to master gate S port 1 (master transparent when
        # CLK=1 — classic pulse-triggered MS-JK; F55)
        if not await self.connect(input_clk_id, master_gate_s_id, target_port=1):
            return False

        # Connect master AND R to master gate R port 0
        if not await self.connect(master_and_r_id, master_gate_r_id):
            return False

        # Connect Clock to master gate R port 1
        if not await self.connect(input_clk_id, master_gate_r_id, target_port=1):
            return False

        # Master R path: OR(gated reset path, NOT(Clear)) into the Q-producing
        # NOR — Clear forces the master low too (F56-consistent)
        if not await self.connect(master_gate_r_id, master_or_r_id):
            return False

        if not await self.connect(not_clear_id, master_or_r_id, target_port=1):
            return False

        if not await self.connect(master_or_r_id, master_nor_q_id):
            return False

        # Connect master NOR Q_bar to master NOR Q port 1 (feedback)
        if not await self.connect(master_nor_qbar_id, master_nor_q_id, target_port=1):
            return False

        # Master S path: OR(gated set path, NOT(Preset)) into the
        # Q_bar-producing NOR — Preset forces the master high too (F56)
        if not await self.connect(master_gate_s_id, master_or_s_id):
            return False

        if not await self.connect(not_preset_id, master_or_s_id, target_port=1):
            return False

        if not await self.connect(master_or_s_id, master_nor_qbar_id):
            return False

        # Connect master NOR Q to master NOR Q_bar port 1 (feedback)
        if not await self.connect(master_nor_q_id, master_nor_qbar_id, target_port=1):
            return False

        # ========== Connect Slave Stage via Master Outputs ==========
        # The slave just transfers the master state while CLK=0 (F55):
        #   S_slave = Qm AND NOT(CLK), R_slave = Q_bar_m AND NOT(CLK)

        # Connect master NOR Q to slave gate S port 0 (transfer Qm)
        if not await self.connect(master_nor_q_id, slave_gate_s_id):
            return False

        # Connect NOT Clock to slave gate S port 1 (slave open when CLK=0)
        if not await self.connect(not_clk_id, slave_gate_s_id, target_port=1):
            return False

        # Connect master NOR Q_bar to slave gate R port 0 (transfer Q_bar_m)
        if not await self.connect(master_nor_qbar_id, slave_gate_r_id):
            return False

        # Connect NOT Clock to slave gate R port 1
        if not await self.connect(not_clk_id, slave_gate_r_id, target_port=1):
            return False

        # ========== Preset/Clear OR Gate Injection into Slave Latch ==========

        # Connect slave gate R to OR R gate (input 0)
        # Normal path: slave gate R drives slave NOR Q via the reset-side OR
        if not await self.connect(slave_gate_r_id, or_r_id):
            return False

        # Connect NOT Clear to OR R gate (input 1)
        # When Clear=0: NOT(0)=1 -> OR=1 -> slave NOR Q forced low -> Q=0
        if not await self.connect(not_clear_id, or_r_id, target_port=1):
            return False

        # Connect OR R to slave NOR Q port 0 (replaces direct slave gate R connection)
        if not await self.connect(or_r_id, slave_nor_q_id):
            return False

        # Connect slave NOR Q_bar to slave NOR Q port 1 (feedback)
        if not await self.connect(slave_nor_qbar_id, slave_nor_q_id, target_port=1):
            return False

        # Connect slave gate S to OR S gate (input 0)
        # Normal path: slave gate S drives slave NOR Q_bar via the set-side OR
        if not await self.connect(slave_gate_s_id, or_s_id):
            return False

        # Connect NOT Preset to OR S gate (input 1)
        # When Preset=0: NOT(0)=1 -> OR=1 -> slave NOR Q_bar forced low -> Q_bar=0 -> Q=1
        if not await self.connect(not_preset_id, or_s_id, target_port=1):
            return False

        # Connect OR S to slave NOR Q_bar port 0 (replaces direct slave gate S connection)
        if not await self.connect(or_s_id, slave_nor_qbar_id):
            return False

        # Connect slave NOR Q to slave NOR Q_bar port 1 (feedback)
        if not await self.connect(slave_nor_q_id, slave_nor_qbar_id, target_port=1):
            return False

        # ========== Connect Final Feedback ==========
        # Textbook JK feedback comes from the SLAVE outputs (locked while the
        # master is transparent), so J=K=1 toggles without a race (F55).

        # Connect slave NOR Q_bar to master AND S port 1
        if not await self.connect(slave_nor_qbar_id, master_and_s_id, target_port=1):
            return False

        # Connect slave NOR Q to master AND R port 1
        if not await self.connect(slave_nor_q_id, master_and_r_id, target_port=1):
            return False

        # ========== Connect Outputs ==========

        # Connect slave NOR Q to Q LED
        if not await self.connect(slave_nor_q_id, q_id):
            return False

        # Connect slave NOR Q_bar to Q_bar LED
        if not await self.connect(slave_nor_qbar_id, qbar_id):
            return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level1_jk_flip_flop.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created JKFlipFlop IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = JKFlipFlopBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    exit_code = asyncio.run(run_ic_builder(build, "JK Flip-Flop IC"))
    sys.exit(exit_code)
