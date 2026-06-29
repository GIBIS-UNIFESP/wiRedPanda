#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Johnson Counter IC (Twisted Ring Counter)

Inputs:
  - CLK (Clock), Init (initialise to the valid seed state 0001 - active HIGH)
  - CountEnable (active-HIGH; low = hold), Load (active-HIGH; synchronous parallel
    load), Data[0:3] (value captured when Load=1)
Outputs: Q[0:3] (Counter Output)

Circuit:
- 4 D flip-flops cascaded with inverted feedback
- Preset input to initialize FF[0] to 1
- Output of each FF feeds to next FF's D input
- FF[3] output feeds through NOT gate back to FF[0] input (twisted ring)

Johnson Counter behavior (fill/drain sequence):
- Initialize with Init=1: Sets Q[0]=1, all others=0
- Fills with 1's: 0001→0011→0111→1111
- Then drains: 1110→1100→1000→0000
- Then repeats: 0001
- Period: 8 clock cycles (vs 4 for regular ring counter)
- Used for sequencing and state machines

Implementation:
- 4 D Flip-Flops (DFlipFlop)
- 1 NOT gate (invert Q[3] for twisted feedback)
- Simple cascade interconnections

Usage:
    python create_johnson_counter.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder

# Johnson and ring counters are the same shift-register-with-feedback topology,
# differing only in the feedback tap (inverted vs straight). Their builders share
# the FF chain and count-enable/load wiring scaffold; the duplication is kept
# inline so each generator remains a self-contained, readable script.
# pylint: disable=duplicate-code


class JohnsonCounterBuilder(ICBuilderBase):
    """Builder for 4-bit Johnson Counter IC"""

    async def create(self) -> bool:
        """Create the 4-bit Johnson Counter IC"""
        await self.begin_build("4-bit Johnson Counter")

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
        # high and Load low for the normal twisted-ring sequence.
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
                str(IC_COMPONENTS_DIR / "level1_d_flip_flop"), dff_x + (i * HORIZONTAL_GATE_SPACING), 100.0, f"FF{i}"
            )
            if ff_id is None:
                return False
            dff_ids.append(ff_id)
            await self.log(f"  ✓ Instantiated level1_d_flip_flop {i}")

        # Vcc to hold each FF's unused Preset/Clear pin inactive (active-LOW -> HIGH)
        vcc_id = await self.create_element("InputVcc", input_x, 220.0, "Vcc")
        if vcc_id is None:
            return False

        # D-input mux chain per bit (same pattern as the binary counter):
        #   hold_mux_i = mux(CE, Q_i,            shift_in_i)   CE=0 hold, =1 shift
        #   load_mux_i = mux(LD, hold_mux_i.Out, Data_i)       LD=1 parallel load
        #   FF_i.D = load_mux_i.Out
        # The cascade/feedback wiring below feeds each hold_mux's Data[1].
        hold_mux_ids = []
        mux_x = input_x + 0.5 * HORIZONTAL_GATE_SPACING
        for i in range(4):
            hm = await self.instantiate_ic(
                str(IC_COMPONENTS_DIR / "level2_mux_2to1"), mux_x, 220.0 + (i * 30.0), f"hold_mux{i}"
            )
            if hm is None:
                return False
            hold_mux_ids.append(hm)
            if not await self.connect(dff_ids[i], hm, source_port_label="Q", target_port_label="Data[0]"):
                return False
            if not await self.connect(ce_id, hm, target_port_label="Sel[0]"):
                return False

            lm = await self.instantiate_ic(
                str(IC_COMPONENTS_DIR / "level2_mux_2to1"),
                mux_x + HORIZONTAL_GATE_SPACING,
                225.0 + (i * 30.0),
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

        # Create NOT gate for twisted feedback
        not_id = await self.create_element(
            "Not", dff_x + (3 * HORIZONTAL_GATE_SPACING), 100.0 + VERTICAL_STAGE_SPACING, "not_q3"
        )
        if not_id is None:
            return False
        await self.log("  ✓ Created NOT gate")

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

        # ========== Connect cascade path (into the hold-mux shift input) ==========
        for i in range(3):
            # Q[i] → hold_mux[i+1].Data[1] (the "shift" input)
            if not await self.connect(
                dff_ids[i], hold_mux_ids[i + 1], source_port_label="Q", target_port_label="Data[1]"
            ):
                return False

        # ========== Connect twisted feedback path ==========
        # Q[3] → NOT → hold_mux[0].Data[1]
        if not await self.connect(dff_ids[3], not_id, source_port_label="Q"):
            return False

        if not await self.connect(not_id, hold_mux_ids[0], target_port_label="Data[1]"):
            return False

        # ========== Connect FF outputs to LEDs ==========
        for i in range(4):
            if not await self.connect(dff_ids[i], output_led_ids[i], source_port_label="Q"):
                return False

        # ========== Seed the valid state 0001 from Init (boundary active-HIGH) ==========
        # The level1 D-FF has active-LOW Preset/Clear, so invert Init: Init=1 ->
        # init_not=0 drives FF0.Preset (Q0=1) and FF1-3.Clear (Q1-3=0) -> 0001.
        # Each FF's other async pin is tied to Vcc (active-LOW inactive = HIGH).
        init_not = await self.create_element(
            "Not", dff_x - HORIZONTAL_GATE_SPACING, 100.0 + 2 * VERTICAL_STAGE_SPACING, "init_not"
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
        output_file = str(IC_COMPONENTS_DIR / "level4_johnson_counter_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 4-bit Johnson Counter IC "
            f"({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = JohnsonCounterBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Johnson Counter IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
