#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Ring Counter IC

Inputs:
  - CLK (Clock), Init (initialise to the valid seed state 0001 - active HIGH)
  - CountEnable (active-HIGH; low = hold), Load (active-HIGH; synchronous parallel
    load), Data[0:3] (value captured when Load=1)
Outputs: Q[0:3] (Counter Output)

Circuit:
- 4 D flip-flops cascaded in a ring (circular shift)
- Preset input to initialize FF[0] to 1
- Output of each FF feeds back to next FF's D input
- FF[3] output feeds back to FF[0] input (circular)

Ring Counter behavior (one-hot sequence):
- Initialize with Init=1: Sets Q[0]=1, all others=0
- Rotates single '1' around the ring: 0001→0010→0100→1000→0001
- Period: 4 clock cycles
- Used for sequencing and state machines

Implementation:
- 4 D Flip-Flops (DFlipFlop)
- Simple circular interconnections
- PRESET signal connected to FF[0] PRESET input (port 2)

Usage:
    python create_ring_counter.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder


class RingCounterBuilder(ICBuilderBase):
    """Builder for 4-bit Ring Counter IC"""

    async def create(self) -> bool:
        """Create the 4-bit Ring Counter IC"""
        await self.begin_build("4-bit Ring Counter")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Create input controls
        input_x = 50.0
        clk_id = await self.create_element("InputSwitch", input_x, 100.0, "CLK")
        if clk_id is None:
            return False
        await self.log("  ✓ Created input CLK")

        init_id = await self.create_element("InputSwitch", input_x, 100.0 + VERTICAL_STAGE_SPACING, "Init")
        if init_id is None:
            return False
        await self.log("  ✓ Created input Init")

        # Control inputs (active-HIGH): CountEnable holds the value when low;
        # Load synchronously captures Data[0-3] on the next edge. Tie CountEnable
        # high and Load low for the normal one-hot rotation.
        ce_id = await self.create_element("InputSwitch", input_x, 40.0, "CountEnable")
        if ce_id is None:
            return False
        load_id = await self.create_element("InputSwitch", input_x, 0.0, "Load")
        if load_id is None:
            return False
        data_in_ids = []
        for i in range(4):
            d_id = await self.create_element("InputSwitch", input_x, 160.0 + (i * 30.0), f"Data[{i}]")
            if d_id is None:
                return False
            data_in_ids.append(d_id)

        # Create D flip-flops (4-bit) using level1_d_flip_flop IC
        dff_ids = []
        dff_x = input_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            ff_id = await self.instantiate_ic(
                "level1_d_flip_flop", dff_x + (i * HORIZONTAL_GATE_SPACING), 100.0, f"FF{i}"
            )
            if ff_id is None:
                return False
            dff_ids.append(ff_id)
            await self.log(f"  ✓ Instantiated level1_d_flip_flop {i}")

        # Vcc to hold the mux Enables active (StaticInput -> no IC boundary port)
        mux_vcc_id = await self.create_element("InputVcc", input_x, 220.0, "MuxVcc")
        if mux_vcc_id is None:
            return False

        # D-input mux chain per bit (same pattern as the binary counter):
        #   hold_mux_i = mux(CE, Q_i,            shift_in_i)   CE=0 hold, =1 rotate
        #   load_mux_i = mux(LD, hold_mux_i.Out, Data_i)       LD=1 parallel load
        #   FF_i.D = load_mux_i.Out
        # The ring wiring below feeds each hold_mux's Data[1].
        hold_mux_ids = []
        mux_x = input_x + 0.5 * HORIZONTAL_GATE_SPACING
        for i in range(4):
            hm = await self.instantiate_ic("level2_mux_2to1", mux_x, 220.0 + (i * 30.0), f"hold_mux{i}")
            if hm is None:
                return False
            hold_mux_ids.append(hm)
            if not await self.connect(dff_ids[i], hm, source_port_label="Q", target_port_label="Data[0]"):
                return False
            if not await self.connect(ce_id, hm, target_port_label="Sel[0]"):
                return False
            if not await self.connect(mux_vcc_id, hm, target_port_label="Enable"):
                return False

            lm = await self.instantiate_ic(
                "level2_mux_2to1", mux_x + HORIZONTAL_GATE_SPACING, 225.0 + (i * 30.0), f"load_mux{i}"
            )
            if lm is None:
                return False
            if not await self.connect(hm, lm, source_port_label="Output", target_port_label="Data[0]"):
                return False
            if not await self.connect(data_in_ids[i], lm, target_port_label="Data[1]"):
                return False
            if not await self.connect(load_id, lm, target_port_label="Sel[0]"):
                return False
            if not await self.connect(mux_vcc_id, lm, target_port_label="Enable"):
                return False
            if not await self.connect(lm, dff_ids[i], source_port_label="Output", target_port_label="D"):
                return False

        # Create output LEDs (4-bit)
        output_led_ids = []
        output_y = 100.0 + VERTICAL_STAGE_SPACING
        for i in range(4):
            led_id = await self.create_element("Led", dff_x + (i * HORIZONTAL_GATE_SPACING), output_y, f"Q{i}")
            if led_id is None:
                return False
            output_led_ids.append(led_id)
            await self.log(f"  ✓ Created output Q{i}")

        # ========== Connect clock to all FFs ==========
        for i in range(4):
            if not await self.connect(clk_id, dff_ids[i], target_port_label="Clock"):
                return False

        # ========== Connect ring path (into the hold-mux shift input) ==========
        for i in range(4):
            next_idx = (i + 1) % 4
            # Q[i] → hold_mux[i+1].Data[1] (the "rotate" input)
            if not await self.connect(
                dff_ids[i], hold_mux_ids[next_idx], source_port_label="Q", target_port_label="Data[1]"
            ):
                return False

        # ========== Connect FF outputs to LEDs ==========
        for i in range(4):
            if not await self.connect(dff_ids[i], output_led_ids[i], source_port_label="Q"):
                return False

        # ========== Connect Init directly to FF[0] Preset (active-HIGH) ==========
        # Init=1 sets FF0 (Preset) and clears FF1-3 (Clear) -> seed 0001.
        if not await self.connect(init_id, dff_ids[0], target_port_label="Preset"):
            return False

        # Gnd holds the otherwise-unused Preset/Clear pins inactive (active-HIGH)
        gnd_id = await self.create_element(
            "InputGnd", dff_x - HORIZONTAL_GATE_SPACING, 100.0 + 2 * VERTICAL_STAGE_SPACING, "Gnd"
        )
        if gnd_id is None:
            return False
        await self.log("  ✓ Created Gnd element")

        # ========== Connect Gnd/Init to Preset/Clear pins ==========
        for i in range(1, 4):
            # Connect Gnd to Preset (active-HIGH, so LOW keeps it inactive)
            if not await self.connect(gnd_id, dff_ids[i], target_port_label="Preset"):
                return False

            # Connect Init to Clear (Init=1 -> clear FF[1-3] to Q=0)
            if not await self.connect(init_id, dff_ids[i], target_port_label="Clear"):
                return False

        # Also connect Gnd to FF[0]'s Clear (keep it inactive)
        if not await self.connect(gnd_id, dff_ids[0], target_port_label="Clear"):
            return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level4_ring_counter_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 4-bit Ring Counter IC ({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = RingCounterBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Ring Counter IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
