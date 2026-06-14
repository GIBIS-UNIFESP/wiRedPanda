#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Binary Counter IC

Inputs:
  - CLK (Clock)
  - CountEnable, Load, Reset (74161-style controls, all active-HIGH)
  - Data[0:3] (parallel-load value, captured on the next edge when Load=1)
Outputs: Q[0:3] (Counter Output, 0-15)

74161-style controls (all active-HIGH; tie CountEnable high, Load/Reset low for a
free-running counter):
  - Reset=1: asynchronously zeroes the counter
  - Load=1: synchronously loads Data[0:3] on the next rising edge
  - CountEnable=0: holds the current value (pause); =1 counts

Circuit:
- 4 D flip-flops for counter state (4-bit value)
- Ripple-carry increment logic using AND/OR gates
- NOT gates for signal inversion

Binary Counter behavior (0→1→2→...→15→0):
- Increments by 1 on each clock rising edge
- Wraps around from 15 to 0
- No load/reset controls for simplicity

Implementation:
- 4 D Flip-Flops (DFlipFlop)
- 4 NOT gates (invert each Q output)
- 2 AND gates (carry chain: carry1=Q0&Q1, carry2=carry1&Q2)
- 3 NOT gates (invert carries for XOR)
- 6 AND gates (bits 1-3 × 2 for XOR: each bit = (NOT Qi AND carryIn) OR (Qi AND NOT carryIn);
  bit 0 is just NOT Q0, no XOR gates)
- 3 OR gates (XOR results, bits 1-3)

Increment Logic:
- Bit 0: D0 = NOT Q0 (always toggle)
- Bit 1: D1 = Q1 XOR Q0
- Bit 2: D2 = Q2 XOR (Q0 AND Q1)
- Bit 3: D3 = Q3 XOR (Q0 AND Q1 AND Q2)

Usage:
    python create_binary_counter_4bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class BinaryCounter4BitBuilder(ICBuilderBase):
    """Builder for 4-bit Binary Counter IC"""

    async def create(self) -> bool:
        """Create the 4-bit Binary Counter IC"""
        await self.begin_build("4-bit Binary Counter")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Create clock input
        input_x = 50.0
        clk_id = await self.create_element("InputSwitch", input_x, 100.0, "CLK")
        if clk_id is None:
            return False
        await self.log(f"  ✓ Created input CLK")

        # 74161-style control inputs (all active-HIGH):
        #   CountEnable : when low the counter holds its value
        #   Load        : synchronous parallel load of Data[0-3] on the next edge
        #   Clear       : asynchronous clear to 0
        # Tie CountEnable high (and Load/Clear low) for a free-running counter.
        ce_id = await self.create_element("InputSwitch", input_x, 60.0, "CountEnable")
        if ce_id is None:
            return False
        load_id = await self.create_element("InputSwitch", input_x, 20.0, "Load")
        if load_id is None:
            return False
        reset_id = await self.create_element("InputSwitch", input_x, -20.0, "Reset")
        if reset_id is None:
            return False
        data_in_ids = []
        for i in range(4):
            d_id = await self.create_element(
                "InputSwitch", input_x - 40.0, 100.0 + (i * VERTICAL_STAGE_SPACING), f"Data[{i}]"
            )
            if d_id is None:
                return False
            data_in_ids.append(d_id)

        # Vcc drives the mux enables; Gnd holds the FF Preset pins inactive
        vcc_id = await self.create_element("InputVcc", input_x, 140.0, "Vcc")
        if vcc_id is None:
            return False
        gnd_id = await self.create_element("InputGnd", input_x, 170.0, "Gnd")
        if gnd_id is None:
            return False
        await self.log(f"  ✓ Created Vcc (mux enables) and Gnd (inactive Preset)")

        # Create NOT gates for each Q output (stage 1)
        not_q_ids = []
        not_x = input_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            not_id = await self.create_element("Not", not_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"not_q{i}")
            if not_id is None:
                return False
            not_q_ids.append(not_id)

        # Create AND gates for carry chain (stage 2)
        carry_and_ids = []
        carry_x = not_x + HORIZONTAL_GATE_SPACING
        for i in range(2):
            and_id = await self.create_element("And", carry_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"and_carry{i}")
            if and_id is None:
                return False
            carry_and_ids.append(and_id)

        # Create NOT gates for carry inversion (stage 3)
        not_carry_ids = []
        not_carry_x = carry_x + HORIZONTAL_GATE_SPACING
        for i in range(3):
            not_id = await self.create_element(
                "Not", not_carry_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"not_carry{i}"
            )
            if not_id is None:
                return False
            not_carry_ids.append(not_id)

        # Create XOR AND gates (stage 4) — bits 1-3 only: bit 0 is a plain
        # NOT(Q0) with no XOR decomposition (F59: the bit-0 gates used to be
        # created and left dead). Index 0 stays as a placeholder so the
        # wiring below can keep addressing gates by bit number.
        xor_and_ids: list = [None]
        xor_and_x = not_carry_x + HORIZONTAL_GATE_SPACING
        for i in range(1, 4):
            bit_ands = []
            for j in range(2):
                and_id = await self.create_element(
                    "And",
                    xor_and_x + (j * HORIZONTAL_GATE_SPACING),
                    100.0 + (i * VERTICAL_STAGE_SPACING),
                    f"and_xor{i}_{j}",
                )
                if and_id is None:
                    return False
                bit_ands.append(and_id)
            xor_and_ids.append(bit_ands)

        # Create XOR OR gates (stage 5) — bits 1-3 only, same reason
        xor_or_ids: list = [None]
        xor_or_x = xor_and_x + (2 * HORIZONTAL_GATE_SPACING)
        for i in range(1, 4):
            or_id = await self.create_element("Or", xor_or_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"or_xor{i}")
            if or_id is None:
                return False
            xor_or_ids.append(or_id)

        # Instantiate level1_d_flip_flop ICs (stage 6)
        dff_ids = []
        dff_x = xor_or_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            ff_id = await self.instantiate_ic(
                "level1_d_flip_flop", dff_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"FF{i}"
            )
            if ff_id is None:
                return False
            dff_ids.append(ff_id)
            await self.log(f"  ✓ Instantiated D Flip-Flop IC {i}")

        # D-input mux chain per bit:
        #   hold_mux_i = mux(CE,  Q_i,            next_i)   CE=0 -> hold, CE=1 -> count
        #   load_mux_i = mux(LD,  hold_mux_i.Out, Data_i)   LD=1 -> parallel load
        #   FF_i.D = load_mux_i.Out
        # The bit logic below feeds each hold_mux's Data[1] (the "count" input).
        hold_mux_ids = []
        mux_x = xor_or_x + 0.5 * HORIZONTAL_GATE_SPACING
        load_mux_x = xor_or_x + 0.75 * HORIZONTAL_GATE_SPACING
        for i in range(4):
            m = await self.instantiate_ic(
                "level2_mux_2to1", mux_x, 110.0 + (i * VERTICAL_STAGE_SPACING), f"hold_mux{i}"
            )
            if m is None:
                return False
            hold_mux_ids.append(m)
            if not await self.connect(dff_ids[i], m, source_port_label="Q", target_port_label="Data[0]"):
                return False
            if not await self.connect(ce_id, m, target_port_label="Sel[0]"):
                return False
            if not await self.connect(vcc_id, m, target_port_label="Enable"):
                return False

            # Synchronous load mux: select external Data when Load is high
            lm = await self.instantiate_ic(
                "level2_mux_2to1", load_mux_x, 115.0 + (i * VERTICAL_STAGE_SPACING), f"load_mux{i}"
            )
            if lm is None:
                return False
            if not await self.connect(m, lm, source_port_label="Output", target_port_label="Data[0]"):
                return False
            if not await self.connect(data_in_ids[i], lm, target_port_label="Data[1]"):
                return False
            if not await self.connect(load_id, lm, target_port_label="Sel[0]"):
                return False
            if not await self.connect(vcc_id, lm, target_port_label="Enable"):
                return False
            if not await self.connect(lm, dff_ids[i], source_port_label="Output", target_port_label="D"):
                return False

        # Create output LEDs (stage 7)
        output_led_ids = []
        output_x = dff_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            led_id = await self.create_element("Led", output_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"Q{i}")
            if led_id is None:
                return False
            output_led_ids.append(led_id)
            await self.log(f"  ✓ Created output Q{i}")

        # ========== Connect clock to all FFs ==========
        for i in range(4):
            if not await self.connect(clk_id, dff_ids[i], target_port_label="Clock"):
                return False

        # ========== Connect Preset (inactive) and asynchronous Reset ==========
        # Preset unused -> tied low (active-HIGH inactive). The active-HIGH Reset
        # input drives the FFs' active-HIGH Clear directly (no inverter): Reset=1
        # asynchronously zeroes the counter.
        for i in range(4):
            # Preset to Gnd (active-HIGH, so LOW keeps it inactive)
            if not await self.connect(gnd_id, dff_ids[i], target_port_label="Preset"):
                return False

            # Reset drives FF Clear directly: Reset=1 -> FF Clear=1 -> zero
            if not await self.connect(reset_id, dff_ids[i], target_port_label="Clear"):
                return False

        # ========== Connect FF outputs to LEDs and NOT gates ==========
        for i in range(4):
            # FF Q to output LED
            if not await self.connect(dff_ids[i], output_led_ids[i], source_port_label="Q"):
                return False

            # FF Q to NOT Q
            if not await self.connect(dff_ids[i], not_q_ids[i], source_port_label="Q"):
                return False

        # ========== Connect carry chain ==========
        # carry1 = Q0 AND Q1
        if not await self.connect(dff_ids[0], carry_and_ids[0], source_port_label="Q"):
            return False

        if not await self.connect(dff_ids[1], carry_and_ids[0], source_port_label="Q", target_port=1):
            return False

        # carry2 = carry1 AND Q2
        if not await self.connect(carry_and_ids[0], carry_and_ids[1]):
            return False

        if not await self.connect(dff_ids[2], carry_and_ids[1], source_port_label="Q", target_port=1):
            return False

        # ========== Connect NOT carry gates ==========
        # NOT Q0
        if not await self.connect(dff_ids[0], not_carry_ids[0], source_port_label="Q"):
            return False

        # NOT carry1
        if not await self.connect(carry_and_ids[0], not_carry_ids[1]):
            return False

        # NOT carry2
        if not await self.connect(carry_and_ids[1], not_carry_ids[2]):
            return False

        # ========== Build XOR logic for each bit ==========

        # Bit 0: count input = NOT Q0 (toggle)
        if not await self.connect(not_q_ids[0], hold_mux_ids[0], target_port_label="Data[1]"):
            return False

        # Bit 1: D1 = Q1 XOR Q0 = (NOT Q1 AND Q0) OR (Q1 AND NOT Q0)
        # NOT Q1 AND Q0
        if not await self.connect(not_q_ids[1], xor_and_ids[1][0]):
            return False

        if not await self.connect(dff_ids[0], xor_and_ids[1][0], source_port_label="Q", target_port=1):
            return False

        # Q1 AND NOT Q0
        if not await self.connect(dff_ids[1], xor_and_ids[1][1], source_port_label="Q"):
            return False

        if not await self.connect(not_carry_ids[0], xor_and_ids[1][1], target_port=1):
            return False

        # XOR OR
        if not await self.connect(xor_and_ids[1][0], xor_or_ids[1]):
            return False

        if not await self.connect(xor_and_ids[1][1], xor_or_ids[1], target_port=1):
            return False

        if not await self.connect(xor_or_ids[1], hold_mux_ids[1], target_port_label="Data[1]"):
            return False

        # Bit 2: D2 = Q2 XOR (Q0 AND Q1) = (NOT Q2 AND carry1) OR (Q2 AND NOT carry1)
        # NOT Q2 AND carry1
        if not await self.connect(not_q_ids[2], xor_and_ids[2][0]):
            return False

        if not await self.connect(carry_and_ids[0], xor_and_ids[2][0], target_port=1):
            return False

        # Q2 AND NOT carry1
        if not await self.connect(dff_ids[2], xor_and_ids[2][1], source_port_label="Q"):
            return False

        if not await self.connect(not_carry_ids[1], xor_and_ids[2][1], target_port=1):
            return False

        # XOR OR
        if not await self.connect(xor_and_ids[2][0], xor_or_ids[2]):
            return False

        if not await self.connect(xor_and_ids[2][1], xor_or_ids[2], target_port=1):
            return False

        if not await self.connect(xor_or_ids[2], hold_mux_ids[2], target_port_label="Data[1]"):
            return False

        # Bit 3: D3 = Q3 XOR (Q0 AND Q1 AND Q2) = (NOT Q3 AND carry2) OR (Q3 AND NOT carry2)
        # NOT Q3 AND carry2
        if not await self.connect(not_q_ids[3], xor_and_ids[3][0]):
            return False

        if not await self.connect(carry_and_ids[1], xor_and_ids[3][0], target_port=1):
            return False

        # Q3 AND NOT carry2
        if not await self.connect(dff_ids[3], xor_and_ids[3][1], source_port_label="Q"):
            return False

        if not await self.connect(not_carry_ids[2], xor_and_ids[3][1], target_port=1):
            return False

        # XOR OR
        if not await self.connect(xor_and_ids[3][0], xor_or_ids[3]):
            return False

        if not await self.connect(xor_and_ids[3][1], xor_or_ids[3], target_port=1):
            return False

        if not await self.connect(xor_or_ids[3], hold_mux_ids[3], target_port_label="Data[1]"):
            return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level4_binary_counter_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created 4-bit Binary Counter IC ({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = BinaryCounter4BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Binary Counter IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
