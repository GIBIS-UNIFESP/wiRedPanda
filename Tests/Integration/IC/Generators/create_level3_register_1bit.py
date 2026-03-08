#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create CPU Level 1: 1-bit Register IC

A 1-bit register stores a single bit using a D Flip-Flop.
Clock-controlled write via WriteEnable signal.

Inputs: Data, Clock, WriteEnable, Reset (4 inputs)
Outputs: Q, NotQ (2 outputs)

Usage:
    python create_cpu_1bit_register.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class Register1BitBuilder(ICBuilderBase):
    """Builder for 1-bit Register IC"""

    async def create(self) -> bool:
        """Create the 1-bit Register IC"""
        self.error_context.set_context("1-bit Register")
        await self.log("🔧 Creating 1-bit Register IC...")
        self.element_count = 0
        self.connection_count = 0

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # ---- Create inputs ----
        input_x = 50.0
        input_y = 100.0

        # Position hierarchy for logic stages
        not_reset_x = input_x + HORIZONTAL_GATE_SPACING
        not_reset_y = input_y + (3 * VERTICAL_STAGE_SPACING)
        not_we_x = input_x + HORIZONTAL_GATE_SPACING
        not_we_y = input_y + VERTICAL_STAGE_SPACING
        mux_x = input_x + (2 * HORIZONTAL_GATE_SPACING)
        mux_y = input_y + VERTICAL_STAGE_SPACING
        dff_x = input_x + (3 * HORIZONTAL_GATE_SPACING)
        dff_y = input_y + VERTICAL_STAGE_SPACING
        vcc_x = input_x + (2 * HORIZONTAL_GATE_SPACING)
        vcc_y = input_y + (2 * VERTICAL_STAGE_SPACING)
        output_x = input_x + (4 * HORIZONTAL_GATE_SPACING)
        output_q_y = input_y
        output_notq_y = input_y + (2 * VERTICAL_STAGE_SPACING)

        response = await self.mcp.send_command("create_element", {
            "type": "InputSwitch", "x": input_x, "y": input_y, "label": "Data"
        })
        if not response or not response.success:
            print(self.error_context.format_error("Create input Data"))
            return False
        data_input = response.result.get('element_id') if response.result else None
        self.element_count += 1
        await self.log("  ✓ Created input Data")

        response = await self.mcp.send_command("create_element", {
            "type": "InputSwitch", "x": input_x, "y": input_y + VERTICAL_STAGE_SPACING, "label": "Clock"
        })
        if not response or not response.success:
            print(self.error_context.format_error("Create input Clock"))
            return False
        clock_input = response.result.get('element_id') if response.result else None
        self.element_count += 1
        await self.log("  ✓ Created input Clock")

        response = await self.mcp.send_command("create_element", {
            "type": "InputSwitch", "x": input_x, "y": input_y + (2 * VERTICAL_STAGE_SPACING), "label": "WriteEnable"
        })
        if not response or not response.success:
            print(self.error_context.format_error("Create input WriteEnable"))
            return False
        write_enable_input = response.result.get('element_id') if response.result else None
        self.element_count += 1
        await self.log("  ✓ Created input WriteEnable")

        response = await self.mcp.send_command("create_element", {
            "type": "InputSwitch", "x": input_x, "y": input_y + (3 * VERTICAL_STAGE_SPACING), "label": "Reset"
        })
        if not response or not response.success:
            print(self.error_context.format_error("Create input Reset"))
            return False
        reset_input = response.result.get('element_id') if response.result else None
        self.element_count += 1
        await self.log("  ✓ Created input Reset")

        # ---- Create NOT gate to invert Reset signal (active-HIGH to active-LOW) ----
        response = await self.mcp.send_command("create_element", {
            "type": "Not", "x": not_reset_x, "y": not_reset_y, "label": "not_reset"
        })
        if not response or not response.success:
            print(self.error_context.format_error("create NOT gate for reset inversion"))
            return False
        not_reset = response.result.get('element_id') if response.result else None
        self.element_count += 1
        await self.log("  ✓ Created NOT gate to invert reset signal")

        conn = await self.mcp.send_command("connect_elements", {
            "source_id": reset_input, "source_port": 0,
            "target_id": not_reset, "target_port": 0
        })
        if not conn or not conn.success:
            print(self.error_context.format_error("connect Reset to NOT gate"))
            return False
        self.connection_count += 1

        # ---- Create NOT gate to invert WriteEnable for Mux control ----
        # Mux: select=0 -> data[0], select=1 -> data[1]
        # We want: WriteEnable=1 -> data[0] (Data), WriteEnable=0 -> data[1] (Q)
        # So we need to invert WriteEnable for the Mux select line
        response = await self.mcp.send_command("create_element", {
            "type": "Not", "x": not_we_x, "y": not_we_y, "label": "not_writenable"
        })
        if not response or not response.success:
            print(self.error_context.format_error("create NOT gate to invert WriteEnable"))
            return False
        not_writenable = response.result.get('element_id') if response.result else None
        self.element_count += 1

        conn = await self.mcp.send_command("connect_elements", {
            "source_id": write_enable_input, "source_port": 0,
            "target_id": not_writenable, "target_port": 0
        })
        if not conn or not conn.success:
            print(self.error_context.format_error("connect WriteEnable to NOT gate"))
            return False
        self.connection_count += 1

        # ---- Create Mux2x1 to implement WriteEnable control ----
        # When WriteEnable=1: select Data input
        # When WriteEnable=0: select Q (feedback) to hold value
        response = await self.mcp.send_command("create_element", {
            "type": "Mux", "x": mux_x, "y": mux_y, "label": "write_mux"
        })
        if not response or not response.success:
            print(self.error_context.format_error("create Mux2x1 for WriteEnable control"))
            return False
        write_mux = response.result.get('element_id') if response.result else None
        self.element_count += 1
        await self.log("  ✓ Created Mux2x1 for WriteEnable control")

        # Connect Data to Mux input 0 using semantic label (selected when WriteEnable=1, i.e., when NOT(WriteEnable)=0)
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": data_input, "source_port": 0,
            "target_id": write_mux, "target_port_label": "In0"
        })
        if not conn or not conn.success:
            print(self.error_context.format_error("connect Data to Mux input 0"))
            return False
        self.connection_count += 1

        # Connect inverted WriteEnable to Mux select line using semantic label
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": not_writenable, "source_port": 0,
            "target_id": write_mux, "target_port_label": "S0"
        })
        if not conn or not conn.success:
            print(self.error_context.format_error("connect NOT(WriteEnable) to Mux select"))
            return False
        self.connection_count += 1

        # ---- Create D Flip-Flop ----
        response = await self.mcp.send_command("create_element", {
            "type": "DFlipFlop", "x": dff_x, "y": dff_y, "label": "dflipflop"
        })
        if not response or not response.success:
            print(self.error_context.format_error("create D Flip-Flop"))
            return False
        dflipflop = response.result.get('element_id') if response.result else None
        self.element_count += 1
        await self.log("  ✓ Created D Flip-Flop")

        # Connect Mux output to D flip-flop D input
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": write_mux, "source_port": 0,
            "target_id": dflipflop, "target_port_label": "Data"
        })
        if not conn or not conn.success:
            print(self.error_context.format_error("connect Mux output to D"))
            return False
        self.connection_count += 1

        # Connect Clock input
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": clock_input, "source_port": 0,
            "target_id": dflipflop, "target_port_label": "Clock"
        })
        if not conn or not conn.success:
            print(self.error_context.format_error("connect Clock"))
            return False
        self.connection_count += 1

        # Create InputVcc (constant HIGH) to keep Preset inactive
        response = await self.mcp.send_command("create_element", {
            "type": "InputVcc", "x": vcc_x, "y": vcc_y, "label": "Vcc"
        })
        if not response or not response.success:
            print(self.error_context.format_error("create InputVcc for preset"))
            return False
        vcc_id = response.result.get('element_id') if response.result else None
        self.element_count += 1

        conn = await self.mcp.send_command("connect_elements", {
            "source_id": vcc_id, "source_port": 0,
            "target_id": dflipflop, "target_port_label": "~Preset"
        })
        if not conn or not conn.success:
            print(self.error_context.format_error("connect Vcc to Preset"))
            return False
        self.connection_count += 1

        conn = await self.mcp.send_command("connect_elements", {
            "source_id": not_reset, "source_port": 0,
            "target_id": dflipflop, "target_port_label": "~Clear"
        })
        if not conn or not conn.success:
            print(self.error_context.format_error("connect inverted Reset to Clear"))
            return False
        self.connection_count += 1
        await self.log("  ✓ Connected Data, Clock, Reset (inverted) to D Flip-Flop")

        # ---- Create outputs ----
        response = await self.mcp.send_command("create_element", {
            "type": "Led", "x": output_x, "y": output_q_y, "label": "Q"
        })
        if not response or not response.success:
            print(self.error_context.format_error("create output Q LED"))
            return False
        q_led = response.result.get('element_id') if response.result else None
        self.element_count += 1

        conn = await self.mcp.send_command("connect_elements", {
            "source_id": dflipflop, "source_port_label": "Q",
            "target_id": q_led, "target_port": 0
        })
        if not conn or not conn.success:
            print(self.error_context.format_error("connect Q to LED"))
            return False
        self.connection_count += 1

        # Connect Q feedback to Mux input 1 using semantic label (selected when WriteEnable=0)
        conn = await self.mcp.send_command("connect_elements", {
            "source_id": dflipflop, "source_port_label": "Q",
            "target_id": write_mux, "target_port_label": "In1"
        })
        if not conn or not conn.success:
            print(self.error_context.format_error("connect Q feedback to Mux input 1"))
            return False
        self.connection_count += 1
        await self.log("  ✓ Connected Q feedback to Mux for hold functionality")

        response = await self.mcp.send_command("create_element", {
            "type": "Led", "x": output_x, "y": output_notq_y, "label": "NotQ"
        })
        if not response or not response.success:
            print(self.error_context.format_error("create output NotQ LED"))
            return False
        notq_led = response.result.get('element_id') if response.result else None
        self.element_count += 1

        conn = await self.mcp.send_command("connect_elements", {
            "source_id": dflipflop, "source_port_label": "~Q",
            "target_id": notq_led, "target_port": 0
        })
        if not conn or not conn.success:
            print(self.error_context.format_error("connect NotQ to LED"))
            return False
        self.connection_count += 1
        await self.log("  ✓ Created outputs: Q, NotQ")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level3_register_1bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 1-bit Register IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = Register1BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    exit_code = asyncio.run(run_ic_builder(build, "1-bit Register IC"))
    exit(exit_code)
