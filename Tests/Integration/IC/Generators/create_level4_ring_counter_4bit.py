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
    python create_level4_ring_counter_4bit.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder

# Johnson and ring counters are the same shift-register-with-feedback topology,
# differing only in the feedback tap (inverted vs straight). Their builders share
# the FF chain and count-enable/load wiring scaffold; the duplication is kept
# inline so each generator remains a self-contained, readable script.
# pylint: disable=duplicate-code


class RingCounterBuilder(ICBuilderBase):
    """Builder for 4-bit Ring Counter IC"""

    async def create(self) -> bool:
        """Create the 4-bit Ring Counter IC"""
        await self.begin_build("4-bit Ring Counter")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Create input controls
        # Single vertical column for all control + data switches (all at
        # input_x). Order matches this IC's current port order -- derived from
        # a Y-position sort of the old (broken, 0-40px-apart) coordinates:
        # Load, CountEnable, Clock, Data[0], Data[1], Init, Data[2], Data[3] --
        # preserved here (just with real VERTICAL_STAGE_SPACING clearance) so
        # any embedder referencing ports by index doesn't silently break.
        input_x = 50.0
        load_id = await self.create_element("InputSwitch", input_x, 0.0, "Load")
        if load_id is None:
            return False

        # Control inputs (active-HIGH): CountEnable holds the value when low;
        # Load synchronously captures Data[0-3] on the next edge. Tie CountEnable
        # high and Load low for the normal one-hot rotation.
        ce_id = await self.create_element("InputSwitch", input_x, VERTICAL_STAGE_SPACING, "CountEnable")
        if ce_id is None:
            return False

        clk_id = await self.create_element("InputSwitch", input_x, 2 * VERTICAL_STAGE_SPACING, "Clock")
        if clk_id is None:
            return False
        await self.log("  ✓ Created input CLK")

        data_in_ids = []
        for i in range(2):
            # Data[0-1] sit between Clock and Init, matching the pre-fix Y-sort
            # order (Data[2-3] follow Init below).
            d_id = await self.create_element("InputSwitch", input_x, (3 + i) * VERTICAL_STAGE_SPACING, f"Data[{i}]")
            if d_id is None:
                return False
            data_in_ids.append(d_id)

        init_id = await self.create_element("InputSwitch", input_x, 5 * VERTICAL_STAGE_SPACING, "Init")
        if init_id is None:
            return False
        await self.log("  ✓ Created input Init")

        for i in range(2, 4):
            d_id = await self.create_element("InputSwitch", input_x, (4 + i) * VERTICAL_STAGE_SPACING, f"Data[{i}]")
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

        # Vcc to hold each FF's unused Preset/Clear pin inactive (active-LOW ->
        # HIGH). InputVcc is not a named IC port (routed to the IC's internal
        # elements rather than sorted into the boundary port list), so it only
        # needs clearance from its neighbors, not a preserved port rank.
        vcc_id = await self.create_element("InputVcc", input_x, 8 * VERTICAL_STAGE_SPACING, "Vcc")
        if vcc_id is None:
            return False

        # D-input mux chain per bit (same pattern as the binary counter):
        #   hold_mux_i = mux(CE, Q_i,            shift_in_i)   CE=0 hold, =1 rotate
        #   load_mux_i = mux(LD, hold_mux_i.Out, Data_i)       LD=1 parallel load
        #   FF_i.D = load_mux_i.Out
        # The ring wiring below feeds each hold_mux's Data[1].
        # Both columns sit a full gate-spacing past the last DFF column (not a
        # fraction of one) -- level2_mux_2to1's real rendered width doesn't fit
        # in the fractional 0.5x gap this used to use, which put hold_mux0/1
        # right on top of the control column.
        hold_mux_ids = []
        mux_x = dff_x + (4 * HORIZONTAL_GATE_SPACING)
        for i in range(4):
            hm = await self.instantiate_ic(
                "level2_mux_2to1", mux_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"hold_mux{i}"
            )
            if hm is None:
                return False
            hold_mux_ids.append(hm)
            if not await self.connect(dff_ids[i], hm, source_port_label="Q", target_port_label="Data[0]"):
                return False
            if not await self.connect(ce_id, hm, target_port_label="Sel[0]"):
                return False

            lm = await self.instantiate_ic(
                "level2_mux_2to1",
                mux_x + HORIZONTAL_GATE_SPACING,
                100.0 + (i * VERTICAL_STAGE_SPACING),
                f"load_mux{i}",
            )
            if lm is None:
                return False
            if not await self.connect(hm, lm, source_port_label="Output", target_port_label="Data[0]"):
                return False
            if not await self.connect(data_in_ids[i], lm, target_port_label="Data[1]"):
                return False
            if not await self.connect(load_id, lm, target_port_label="Sel[0]"):
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

        # ========== Seed the valid state 0001 from Init (boundary active-HIGH) ==========
        # The level1 D-FF has active-LOW Preset/Clear, so invert Init: Init=1 ->
        # init_not=0 drives FF0.Preset (Q0=1) and FF1-3.Clear (Q1-3=0) -> 0001.
        # Each FF's other async pin is tied to Vcc (active-LOW inactive = HIGH).
        # dff_x - HORIZONTAL_GATE_SPACING is input_x, i.e. the control column --
        # placing this gate there landed it inside the control switches' own
        # occupied band, so it gets a dedicated column to the left of them
        # instead (at Init's row, since that's what it inverts).
        init_not = await self.create_element(
            "Not", input_x - HORIZONTAL_GATE_SPACING, 5 * VERTICAL_STAGE_SPACING, "init_not"
        )
        if init_not is None:
            return False
        if not await self.connect(init_id, init_not):
            return False

        # FF[0]: Init presets Q0 to 1; Clear held inactive.
        if not await self.connect(init_not, dff_ids[0], target_port_label="Preset"):
            return False
        if not await self.connect(vcc_id, dff_ids[0], target_port_label="Clear"):
            return False

        # FF[1-3]: Init clears Qi to 0; Preset held inactive.
        for i in range(1, 4):
            if not await self.connect(vcc_id, dff_ids[i], target_port_label="Preset"):
                return False
            if not await self.connect(init_not, dff_ids[i], target_port_label="Clear"):
                return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level4_ring_counter_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 4-bit Ring Counter IC "
            f"({self.element_count} elements, {self.connection_count} connections)"
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
