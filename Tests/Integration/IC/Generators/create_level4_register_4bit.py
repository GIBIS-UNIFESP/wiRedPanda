#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Register IC

Implements a parallel load register with enable control using proper IC composition.

Inputs: CLK (Clock), EN (Enable), D[0:3] (Data Input)
Outputs: Q[0:3] (Data Output)

Circuit Architecture:
- 1× level4_bus_mux_4bit IC: Selects between load (D) and hold (Q_feedback) paths
- 4× DFlipFlop elements: State storage
- Clock input: Shared by all flip-flops

Register Behavior:
- When EN=1: D inputs loaded into register on clock rising edge
- When EN=0: Current register value held (data inputs ignored)
- No reset/clear control for simplicity

Internal Structure:
- Load path: D[0-3] connected to Mux In0[0-3]
- Hold path: Q_feedback[0-3] connected to Mux In1[0-3]
- Select: EN controls which path (0=hold, 1=load)
- Output: Mux Out[0-3] → FF D inputs

This refactoring eliminates manual gate logic (NOT, AND, OR) by using the
composite bus_mux_4bit IC, demonstrating proper IC composition hierarchy.

Usage:
    python create_level4_register_4bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class RegisterBuilder(ICBuilderBase):
    """Builder for 4-bit Register IC with enable control"""

    async def create(self) -> bool:
        """Create the 4-bit Register IC"""
        await self.begin_build("4-bit Register")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Create input controls
        input_x = 50.0
        clk_id = await self.create_element("InputSwitch", input_x, 100.0, "CLK")
        if clk_id is None:
            return False
        await self.log(f"  ✓ Created input CLK (id={clk_id})")

        en_id = await self.create_element("InputSwitch", input_x, 100.0 + VERTICAL_STAGE_SPACING, "EN")
        if en_id is None:
            return False
        await self.log(f"  ✓ Created input EN (id={en_id})")

        # Create data inputs (4-bit)
        data_in_ids = []
        for i in range(4):
            d_id = await self.create_element("InputSwitch", input_x, 100.0 + (2 + i) * VERTICAL_STAGE_SPACING, f"D{i}")
            if d_id is None:
                return False
            data_in_ids.append(d_id)
            await self.log(f"  ✓ Created input D{i} (id={d_id})")

        # Instantiate 4-bit Bus Multiplexer IC (replaces manual AND/OR gates)
        mux_x = input_x + HORIZONTAL_GATE_SPACING
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit")):

            return False

        mux_ic_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit"), mux_x, 100.0 + VERTICAL_STAGE_SPACING, "BusMux_LoadHold")
        if mux_ic_id is None:
            return False
        await self.log(f"  ✓ Instantiated BusMux_LoadHold (id={mux_ic_id})")

        # Create D flip-flops (4-bit)
        dff_ids = []
        dff_x = mux_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            ff_id = await self.create_element("DFlipFlop", dff_x, 100.0 + (2 + i) * VERTICAL_STAGE_SPACING, f"FF{i}")
            if ff_id is None:
                return False
            dff_ids.append(ff_id)
            await self.log(f"  ✓ Created DFlipFlop {i} (id={ff_id})")

        # Create output LEDs (4-bit)
        output_led_ids = []
        output_x = dff_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            led_id = await self.create_element("Led", output_x, 100.0 + (2 + i) * VERTICAL_STAGE_SPACING, f"Q{i}")
            if led_id is None:
                return False
            output_led_ids.append(led_id)
            await self.log(f"  ✓ Created output Q{i} (id={led_id})")

        # ========== Connect data inputs to mux load path (In0) ==========
        for i in range(4):
            if not await self.connect(data_in_ids[i], mux_ic_id, target_port_label=f"In0[{i}]"):
                return False
            await self.log(f"  ✓ Connected D{i} → Mux In0[{i}]")

        # ========== Connect clock to all flip-flops ==========
        for i in range(4):
            if not await self.connect(clk_id, dff_ids[i], target_port_label="Clock"):
                return False

        # ========== Connect mux output to flip-flop D inputs ==========
        for i in range(4):
            if not await self.connect(mux_ic_id, dff_ids[i], source_port_label=f"Out[{i}]"):
                return False
            await self.log(f"  ✓ Connected Mux Out[{i}] → FF{i} D")

        # ========== Connect flip-flop outputs to LEDs and feedback ==========
        for i in range(4):
            # Q output to LED
            if not await self.connect(dff_ids[i], output_led_ids[i]):
                return False

            # Q feedback to mux hold path (In1)
            if not await self.connect(dff_ids[i], mux_ic_id, target_port_label=f"In1[{i}]"):
                return False
            await self.log(f"  ✓ Connected FF{i} Q → Mux In1[{i}] (feedback)")

        # ========== Invert EN and connect to mux select ==========
        # Bus mux logic: Sel=0 → In0 (load), Sel=1 → In1 (hold)
        # We need: EN=1 → load, EN=0 → hold
        # So we invert EN: Sel = NOT(EN)
        not_gate_id = await self.create_element("Not", input_x + HORIZONTAL_GATE_SPACING, 100.0 + VERTICAL_STAGE_SPACING, "NOT_EN")
        if not_gate_id is None:
            return False
        await self.log(f"  ✓ Created NOT gate for EN inversion (id={not_gate_id})")

        # Connect EN to NOT gate input
        if not await self.connect(en_id, not_gate_id):
            return False

        # Connect NOT gate output to mux Sel
        if not await self.connect(not_gate_id, mux_ic_id, target_port_label="Sel"):
            return False
        await self.log(f"  ✓ Connected EN → NOT → Mux Sel")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level4_register_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 4-bit Register IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = RegisterBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Register IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
