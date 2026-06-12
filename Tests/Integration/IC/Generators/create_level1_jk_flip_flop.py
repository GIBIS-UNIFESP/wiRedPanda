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
  FALLING edge (end of the clock pulse).
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
import sys

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import (
    HORIZONTAL_GATE_SPACING,
    VERTICAL_STAGE_SPACING,
)


class JKFlipFlopBuilder(ICBuilderBase):
    """Builder for JK Flip-Flop IC"""

    async def create(self) -> bool:
        """Create the JK Flip-Flop IC"""
        self.error_context.set_context("JK Flip-Flop")
        await self.log("🔧 Creating JKFlipFlop IC...")
        self.element_count = 0
        self.connection_count = 0

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
        input_j_response = await self.mcp.send_command("create_element", {
            "type": "InputSwitch",
            "x": input_x,
            "y": row1_y,
            "label": "J"
        })
        if not input_j_response or not input_j_response.success:
            print("❌ Failed to create input J")
            return False
        input_j_id = input_j_response.result.get('element_id') if input_j_response.result else None
        self.element_count += 1
        await self.log(f"  ✓ Created input J (id={input_j_id})")

        input_k_response = await self.mcp.send_command("create_element", {
            "type": "InputSwitch",
            "x": input_x,
            "y": row2_y,
            "label": "K"
        })
        if not input_k_response or not input_k_response.success:
            print("❌ Failed to create input K")
            return False
        input_k_id = input_k_response.result.get('element_id') if input_k_response.result else None
        self.element_count += 1
        await self.log(f"  ✓ Created input K (id={input_k_id})")

        input_clk_response = await self.mcp.send_command("create_element", {
            "type": "InputSwitch",
            "x": input_x,
            "y": clock_y,
            "label": "Clock"
        })
        if not input_clk_response or not input_clk_response.success:
            print("❌ Failed to create input Clock")
            return False
        input_clk_id = input_clk_response.result.get('element_id') if input_clk_response.result else None
        self.element_count += 1
        await self.log(f"  ✓ Created input Clock (id={input_clk_id})")

        # Create PRESET input (active LOW)
        input_preset_response = await self.mcp.send_command("create_element", {
            "type": "InputSwitch",
            "x": input_x,
            "y": preset_y,
            "label": "Preset"
        })
        if not input_preset_response or not input_preset_response.success:
            print("❌ Failed to create input Preset")
            return False
        input_preset_id = input_preset_response.result.get('element_id') if input_preset_response.result else None
        self.element_count += 1
        await self.log(f"  ✓ Created input Preset (id={input_preset_id})")

        # Create CLEAR input (active LOW)
        input_clear_response = await self.mcp.send_command("create_element", {
            "type": "InputSwitch",
            "x": input_x,
            "y": clear_y,
            "label": "Clear"
        })
        if not input_clear_response or not input_clear_response.success:
            print("❌ Failed to create input Clear")
            return False
        input_clear_id = input_clear_response.result.get('element_id') if input_clear_response.result else None
        self.element_count += 1
        await self.log(f"  ✓ Created input Clear (id={input_clear_id})")

        # Create NOT gate for clock inversion
        not_clk_response = await self.mcp.send_command("create_element", {
            "type": "Not",
            "x": not_clk_x,
            "y": clock_y,
            "label": "not_clk"
        })
        if not not_clk_response or not not_clk_response.success:
            print("❌ Failed to create NOT Clock gate")
            return False
        not_clk_id = not_clk_response.result.get('element_id') if not_clk_response.result else None
        self.element_count += 1
        await self.log(f"  ✓ Created NOT Clock gate (id={not_clk_id})")

        # ========== Preset/Clear Control Logic ==========

        # NOT gate to invert Preset
        not_preset_response = await self.mcp.send_command("create_element", {
            "type": "Not",
            "x": not_clk_x,
            "y": preset_y,
            "label": "not_preset"
        })
        if not not_preset_response or not not_preset_response.success:
            print("❌ Failed to create NOT Preset gate")
            return False
        not_preset_id = not_preset_response.result.get('element_id') if not_preset_response.result else None
        self.element_count += 1

        # NOT gate to invert Clear
        not_clear_response = await self.mcp.send_command("create_element", {
            "type": "Not",
            "x": not_clk_x,
            "y": clear_y,
            "label": "not_clear"
        })
        if not not_clear_response or not not_clear_response.success:
            print("❌ Failed to create NOT Clear gate")
            return False
        not_clear_id = not_clear_response.result.get('element_id') if not_clear_response.result else None
        self.element_count += 1

        # OR gate for S (slave_gate_r OR NOT(clear))
        # When Clear=0: NOT(0)=1 -> OR forces slave NOR Q input high -> Q=0
        or_s_response = await self.mcp.send_command("create_element", {
            "type": "Or",
            "x": slave_gate_x + (HORIZONTAL_GATE_SPACING / 2),
            "y": row1_y + (VERTICAL_STAGE_SPACING * 2),
            "label": "or_s"
        })
        if not or_s_response or not or_s_response.success:
            print("❌ Failed to create OR S gate")
            return False
        or_s_id = or_s_response.result.get('element_id') if or_s_response.result else None
        self.element_count += 1

        # OR gate for R (slave_gate_s OR NOT(preset))
        # When Preset=0: NOT(0)=1 -> OR forces slave NOR Q_bar input high -> Q_bar=0 -> Q=1
        or_r_response = await self.mcp.send_command("create_element", {
            "type": "Or",
            "x": slave_gate_x + (HORIZONTAL_GATE_SPACING / 2),
            "y": row1_y + (VERTICAL_STAGE_SPACING * 3),
            "label": "or_r"
        })
        if not or_r_response or not or_r_response.success:
            print("❌ Failed to create OR R gate")
            return False
        or_r_id = or_r_response.result.get('element_id') if or_r_response.result else None
        self.element_count += 1

        # Master-side injection (F56-consistent): force BOTH latches on
        # Preset/Clear so async controls work under any clock level.
        master_or_s_response = await self.mcp.send_command("create_element", {
            "type": "Or",
            "x": master_gate_x + (HORIZONTAL_GATE_SPACING / 2),
            "y": row1_y + (VERTICAL_STAGE_SPACING * 2),
            "label": "master_or_s"
        })
        if not master_or_s_response or not master_or_s_response.success:
            print("❌ Failed to create master OR S gate")
            return False
        master_or_s_id = master_or_s_response.result.get('element_id') if master_or_s_response.result else None
        self.element_count += 1

        master_or_r_response = await self.mcp.send_command("create_element", {
            "type": "Or",
            "x": master_gate_x + (HORIZONTAL_GATE_SPACING / 2),
            "y": row1_y + (VERTICAL_STAGE_SPACING * 3),
            "label": "master_or_r"
        })
        if not master_or_r_response or not master_or_r_response.success:
            print("❌ Failed to create master OR R gate")
            return False
        master_or_r_id = master_or_r_response.result.get('element_id') if master_or_r_response.result else None
        self.element_count += 1

        # Connect Preset to NOT gate
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": input_preset_id,
            "source_port": 0,
            "target_id": not_preset_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect Preset to NOT gate")
            return False
        self.connection_count += 1

        # Connect Clear to NOT gate
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": input_clear_id,
            "source_port": 0,
            "target_id": not_clear_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect Clear to NOT gate")
            return False
        self.connection_count += 1

        # ========== Master Stage - Feedback Logic ==========

        # AND gate: J AND Q_bar (for setting via feedback) - will be connected later
        master_and_s_response = await self.mcp.send_command("create_element", {
            "type": "And",
            "x": master_and_x,
            "y": row1_y,
            "label": "master_and_s"
        })
        if not master_and_s_response or not master_and_s_response.success:
            print("❌ Failed to create master AND S gate")
            return False
        master_and_s_id = master_and_s_response.result.get('element_id') if master_and_s_response.result else None
        self.element_count += 1

        # AND gate: K AND Q (for resetting via feedback) - will be connected later
        master_and_r_response = await self.mcp.send_command("create_element", {
            "type": "And",
            "x": master_and_x,
            "y": row2_y,
            "label": "master_and_r"
        })
        if not master_and_r_response or not master_and_r_response.success:
            print("❌ Failed to create master AND R gate")
            return False
        master_and_r_id = master_and_r_response.result.get('element_id') if master_and_r_response.result else None
        self.element_count += 1

        # AND gates for clock gating
        master_gate_s_response = await self.mcp.send_command("create_element", {
            "type": "And",
            "x": master_gate_x,
            "y": row1_y,
            "label": "master_gate_s"
        })
        if not master_gate_s_response or not master_gate_s_response.success:
            print("❌ Failed to create master gate S")
            return False
        master_gate_s_id = master_gate_s_response.result.get('element_id') if master_gate_s_response.result else None
        self.element_count += 1

        master_gate_r_response = await self.mcp.send_command("create_element", {
            "type": "And",
            "x": master_gate_x,
            "y": row2_y,
            "label": "master_gate_r"
        })
        if not master_gate_r_response or not master_gate_r_response.success:
            print("❌ Failed to create master gate R")
            return False
        master_gate_r_id = master_gate_r_response.result.get('element_id') if master_gate_r_response.result else None
        self.element_count += 1

        # Master SR Latch NOR gates
        master_nor_q_response = await self.mcp.send_command("create_element", {
            "type": "Nor",
            "x": master_nor_x,
            "y": row1_y,
            "label": "master_nor_q"
        })
        if not master_nor_q_response or not master_nor_q_response.success:
            print("❌ Failed to create master NOR Q")
            return False
        master_nor_q_id = master_nor_q_response.result.get('element_id') if master_nor_q_response.result else None
        self.element_count += 1

        master_nor_qbar_response = await self.mcp.send_command("create_element", {
            "type": "Nor",
            "x": master_nor_x,
            "y": row2_y,
            "label": "master_nor_qbar"
        })
        if not master_nor_qbar_response or not master_nor_qbar_response.success:
            print("❌ Failed to create master NOR Q_bar")
            return False
        master_nor_qbar_id = master_nor_qbar_response.result.get('element_id') if master_nor_qbar_response.result else None
        self.element_count += 1

        # ========== Slave Stage ==========
        # (F55: the old slave_and_s/slave_and_r J/K-gated transfer gates are
        # gone — a textbook slave just transfers the master state.)

        # Slave gate AND gates
        slave_gate_s_response = await self.mcp.send_command("create_element", {
            "type": "And",
            "x": slave_gate_x,
            "y": row1_y,
            "label": "slave_gate_s"
        })
        if not slave_gate_s_response or not slave_gate_s_response.success:
            print("❌ Failed to create slave gate S")
            return False
        slave_gate_s_id = slave_gate_s_response.result.get('element_id') if slave_gate_s_response.result else None
        self.element_count += 1

        slave_gate_r_response = await self.mcp.send_command("create_element", {
            "type": "And",
            "x": slave_gate_x,
            "y": row2_y,
            "label": "slave_gate_r"
        })
        if not slave_gate_r_response or not slave_gate_r_response.success:
            print("❌ Failed to create slave gate R")
            return False
        slave_gate_r_id = slave_gate_r_response.result.get('element_id') if slave_gate_r_response.result else None
        self.element_count += 1

        # Slave SR Latch NOR gates
        slave_nor_q_response = await self.mcp.send_command("create_element", {
            "type": "Nor",
            "x": slave_nor_x,
            "y": row1_y,
            "label": "slave_nor_q"
        })
        if not slave_nor_q_response or not slave_nor_q_response.success:
            print("❌ Failed to create slave NOR Q")
            return False
        slave_nor_q_id = slave_nor_q_response.result.get('element_id') if slave_nor_q_response.result else None
        self.element_count += 1

        slave_nor_qbar_response = await self.mcp.send_command("create_element", {
            "type": "Nor",
            "x": slave_nor_x,
            "y": row2_y,
            "label": "slave_nor_qbar"
        })
        if not slave_nor_qbar_response or not slave_nor_qbar_response.success:
            print("❌ Failed to create slave NOR Q_bar")
            return False
        slave_nor_qbar_id = slave_nor_qbar_response.result.get('element_id') if slave_nor_qbar_response.result else None
        self.element_count += 1

        # Create output LEDs
        q_response = await self.mcp.send_command("create_element", {
            "type": "Led",
            "x": output_x,
            "y": row1_y,
            "label": "Q"
        })
        if not q_response or not q_response.success:
            print("❌ Failed to create Q LED")
            return False
        q_id = q_response.result.get('element_id') if q_response.result else None
        self.element_count += 1

        qbar_response = await self.mcp.send_command("create_element", {
            "type": "Led",
            "x": output_x,
            "y": row2_y,
            "label": "Q_bar"
        })
        if not qbar_response or not qbar_response.success:
            print("❌ Failed to create Q_bar LED")
            return False
        qbar_id = qbar_response.result.get('element_id') if qbar_response.result else None
        self.element_count += 1

        # ========== Connect Clock Inversion ==========

        conn = await self.mcp.send_command("connect_elements", {
            "source_id": input_clk_id,
            "source_port": 0,
            "target_id": not_clk_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect Clock to NOT")
            return False
        self.connection_count += 1

        # ========== Connect Master Stage ==========

        # Connect J to master AND S
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": input_j_id,
            "source_port": 0,
            "target_id": master_and_s_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect J to master AND S")
            return False
        self.connection_count += 1

        # Connect K to master AND R
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": input_k_id,
            "source_port": 0,
            "target_id": master_and_r_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect K to master AND R")
            return False
        self.connection_count += 1

        # Connect master AND S to master gate S port 0
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": master_and_s_id,
            "source_port": 0,
            "target_id": master_gate_s_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect master AND S to master gate S")
            return False
        self.connection_count += 1

        # Connect Clock to master gate S port 1 (master transparent when
        # CLK=1 — classic pulse-triggered MS-JK; F55)
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": input_clk_id,
            "source_port": 0,
            "target_id": master_gate_s_id,
            "target_port": 1
        })
        if not conn or not conn.success:
            print("❌ Failed to connect Clock to master gate S")
            return False
        self.connection_count += 1

        # Connect master AND R to master gate R port 0
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": master_and_r_id,
            "source_port": 0,
            "target_id": master_gate_r_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect master AND R to master gate R")
            return False
        self.connection_count += 1

        # Connect Clock to master gate R port 1
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": input_clk_id,
            "source_port": 0,
            "target_id": master_gate_r_id,
            "target_port": 1
        })
        if not conn or not conn.success:
            print("❌ Failed to connect Clock to master gate R")
            return False
        self.connection_count += 1

        # Master R path: OR(gated reset path, NOT(Clear)) into the Q-producing
        # NOR — Clear forces the master low too (F56-consistent)
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": master_gate_r_id,
            "source_port": 0,
            "target_id": master_or_r_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect master gate R to master OR R")
            return False
        self.connection_count += 1

        conn = await self.mcp.send_command("connect_elements", {
            "source_id": not_clear_id,
            "source_port": 0,
            "target_id": master_or_r_id,
            "target_port": 1
        })
        if not conn or not conn.success:
            print("❌ Failed to connect NOT Clear to master OR R")
            return False
        self.connection_count += 1

        conn = await self.mcp.send_command("connect_elements", {
            "source_id": master_or_r_id,
            "source_port": 0,
            "target_id": master_nor_q_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect master OR R to master NOR Q")
            return False
        self.connection_count += 1

        # Connect master NOR Q_bar to master NOR Q port 1 (feedback)
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": master_nor_qbar_id,
            "source_port": 0,
            "target_id": master_nor_q_id,
            "target_port": 1
        })
        if not conn or not conn.success:
            print("❌ Failed to connect master NOR Q_bar to master NOR Q")
            return False
        self.connection_count += 1

        # Master S path: OR(gated set path, NOT(Preset)) into the
        # Q_bar-producing NOR — Preset forces the master high too (F56)
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": master_gate_s_id,
            "source_port": 0,
            "target_id": master_or_s_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect master gate S to master OR S")
            return False
        self.connection_count += 1

        conn = await self.mcp.send_command("connect_elements", {
            "source_id": not_preset_id,
            "source_port": 0,
            "target_id": master_or_s_id,
            "target_port": 1
        })
        if not conn or not conn.success:
            print("❌ Failed to connect NOT Preset to master OR S")
            return False
        self.connection_count += 1

        conn = await self.mcp.send_command("connect_elements", {
            "source_id": master_or_s_id,
            "source_port": 0,
            "target_id": master_nor_qbar_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect master OR S to master NOR Q_bar")
            return False
        self.connection_count += 1

        # Connect master NOR Q to master NOR Q_bar port 1 (feedback)
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": master_nor_q_id,
            "source_port": 0,
            "target_id": master_nor_qbar_id,
            "target_port": 1
        })
        if not conn or not conn.success:
            print("❌ Failed to connect master NOR Q to master NOR Q_bar")
            return False
        self.connection_count += 1

        # ========== Connect Slave Stage via Master Outputs ==========
        # The slave just transfers the master state while CLK=0 (F55):
        #   S_slave = Qm AND NOT(CLK), R_slave = Q_bar_m AND NOT(CLK)

        # Connect master NOR Q to slave gate S port 0 (transfer Qm)
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": master_nor_q_id,
            "source_port": 0,
            "target_id": slave_gate_s_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect master NOR Q to slave gate S")
            return False
        self.connection_count += 1

        # Connect NOT Clock to slave gate S port 1 (slave open when CLK=0)
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": not_clk_id,
            "source_port": 0,
            "target_id": slave_gate_s_id,
            "target_port": 1
        })
        if not conn or not conn.success:
            print("❌ Failed to connect NOT Clock to slave gate S")
            return False
        self.connection_count += 1

        # Connect master NOR Q_bar to slave gate R port 0 (transfer Q_bar_m)
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": master_nor_qbar_id,
            "source_port": 0,
            "target_id": slave_gate_r_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect master NOR Q_bar to slave gate R")
            return False
        self.connection_count += 1

        # Connect NOT Clock to slave gate R port 1
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": not_clk_id,
            "source_port": 0,
            "target_id": slave_gate_r_id,
            "target_port": 1
        })
        if not conn or not conn.success:
            print("❌ Failed to connect NOT Clock to slave gate R")
            return False
        self.connection_count += 1

        # ========== Preset/Clear OR Gate Injection into Slave Latch ==========

        # Connect slave gate R to OR S gate (input 0)
        # Normal path: slave gate R drives slave NOR Q via OR S
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": slave_gate_r_id,
            "source_port": 0,
            "target_id": or_s_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect slave gate R to OR S")
            return False
        self.connection_count += 1

        # Connect NOT Clear to OR S gate (input 1)
        # When Clear=0: NOT(0)=1 -> OR=1 -> slave NOR Q forced low -> Q=0
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": not_clear_id,
            "source_port": 0,
            "target_id": or_s_id,
            "target_port": 1
        })
        if not conn or not conn.success:
            print("❌ Failed to connect NOT Clear to OR S")
            return False
        self.connection_count += 1

        # Connect OR S to slave NOR Q port 0 (replaces direct slave gate R connection)
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": or_s_id,
            "source_port": 0,
            "target_id": slave_nor_q_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect OR S to slave NOR Q")
            return False
        self.connection_count += 1

        # Connect slave NOR Q_bar to slave NOR Q port 1 (feedback)
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": slave_nor_qbar_id,
            "source_port": 0,
            "target_id": slave_nor_q_id,
            "target_port": 1
        })
        if not conn or not conn.success:
            print("❌ Failed to connect slave NOR Q_bar to slave NOR Q")
            return False
        self.connection_count += 1

        # Connect slave gate S to OR R gate (input 0)
        # Normal path: slave gate S drives slave NOR Q_bar via OR R
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": slave_gate_s_id,
            "source_port": 0,
            "target_id": or_r_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect slave gate S to OR R")
            return False
        self.connection_count += 1

        # Connect NOT Preset to OR R gate (input 1)
        # When Preset=0: NOT(0)=1 -> OR=1 -> slave NOR Q_bar forced low -> Q_bar=0 -> Q=1
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": not_preset_id,
            "source_port": 0,
            "target_id": or_r_id,
            "target_port": 1
        })
        if not conn or not conn.success:
            print("❌ Failed to connect NOT Preset to OR R")
            return False
        self.connection_count += 1

        # Connect OR R to slave NOR Q_bar port 0 (replaces direct slave gate S connection)
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": or_r_id,
            "source_port": 0,
            "target_id": slave_nor_qbar_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect OR R to slave NOR Q_bar")
            return False
        self.connection_count += 1

        # Connect slave NOR Q to slave NOR Q_bar port 1 (feedback)
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": slave_nor_q_id,
            "source_port": 0,
            "target_id": slave_nor_qbar_id,
            "target_port": 1
        })
        if not conn or not conn.success:
            print("❌ Failed to connect slave NOR Q to slave NOR Q_bar")
            return False
        self.connection_count += 1

        # ========== Connect Final Feedback ==========
        # Textbook JK feedback comes from the SLAVE outputs (locked while the
        # master is transparent), so J=K=1 toggles without a race (F55).

        # Connect slave NOR Q_bar to master AND S port 1
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": slave_nor_qbar_id,
            "source_port": 0,
            "target_id": master_and_s_id,
            "target_port": 1
        })
        if not conn or not conn.success:
            print("❌ Failed to connect slave NOR Q_bar to master AND S")
            return False
        self.connection_count += 1

        # Connect slave NOR Q to master AND R port 1
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": slave_nor_q_id,
            "source_port": 0,
            "target_id": master_and_r_id,
            "target_port": 1
        })
        if not conn or not conn.success:
            print("❌ Failed to connect slave NOR Q to master AND R")
            return False
        self.connection_count += 1

        # ========== Connect Outputs ==========

        # Connect slave NOR Q to Q LED
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": slave_nor_q_id,
            "source_port": 0,
            "target_id": q_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect slave NOR Q to Q LED")
            return False
        self.connection_count += 1

        # Connect slave NOR Q_bar to Q_bar LED
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": slave_nor_qbar_id,
            "source_port": 0,
            "target_id": qbar_id,
            "target_port": 0
        })
        if not conn or not conn.success:
            print("❌ Failed to connect slave NOR Q_bar to Q_bar LED")
            return False
        self.connection_count += 1

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
    exit_code = asyncio.run(run_ic_builder(build, "JK Flip-Flop IC"))
    sys.exit(exit_code)
