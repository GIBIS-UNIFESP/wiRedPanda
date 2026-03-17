#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Up/Down Counter IC

Inputs: CLK (Clock), RST (Reset - active LOW), Direction (0=Count Down, 1=Count Up), Enable
Outputs: Q[0-3] (4-bit count value)

Architecture (built from level1_d_flip_flop ICs):
- 4 D flip-flops for counter state
- Increment logic (same carry chain as binary counter)
- Decrement logic (borrow chain using NOT(Q) signals)
- Direction mux selects between increment and decrement
- Enable mux selects between count and hold

Increment logic:
  inc[0] = NOT(Q0)
  inc[1] = Q1 XOR Q0
  inc[2] = Q2 XOR (Q0 AND Q1)
  inc[3] = Q3 XOR (Q0 AND Q1 AND Q2)

Decrement logic (borrow chain):
  dec[0] = NOT(Q0)
  dec[1] = Q1 XOR NOT(Q0)
  dec[2] = Q2 XOR (NOT(Q0) AND NOT(Q1))
  dec[3] = Q3 XOR (NOT(Q0) AND NOT(Q1) AND NOT(Q2))

Selection:
  count_val[i] = Mux(dec[i], inc[i], Direction)
  D[i] = Mux(Q[i], count_val[i], Enable)

Usage:
    python create_level5_up_down_counter_4bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class UpDownCounter4BitBuilder(ICBuilderBase):
    """Builder for 4-bit Up/Down Counter IC (built from D flip-flop ICs)"""

    async def create(self) -> bool:
        """Create the 4-bit Up/Down Counter IC"""
        await self.begin_build('4-bit Up/Down Counter')
        if not await self.create_new_circuit():
            return False

        input_x = 50.0

        # ========== Create inputs ==========
        clk_id = await self.create_element("InputSwitch", input_x, 100.0, "CLK")
        if clk_id is None:
            return False

        rst_id = await self.create_element("InputSwitch", input_x, 100.0 + VERTICAL_STAGE_SPACING, "RST")
        if rst_id is None:
            return False

        direction_id = await self.create_element("InputSwitch", input_x, 100.0 + (2 * VERTICAL_STAGE_SPACING), "Direction")
        if direction_id is None:
            return False

        enable_id = await self.create_element("InputSwitch", input_x, 100.0 + (3 * VERTICAL_STAGE_SPACING), "Enable")
        if enable_id is None:
            return False

        # Create Vcc element for Preset inactive state
        vcc_id = await self.create_element("InputVcc", input_x, 100.0 + (4 * VERTICAL_STAGE_SPACING), "Vcc")
        if vcc_id is None:
            return False

        await self.log("  Created inputs: CLK, RST, Direction, Enable, Vcc")

        # ========== Create NOT gates for Q outputs (used by both inc and dec) ==========
        not_q_ids = []
        not_x = input_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            not_id = await self.create_element("Not", not_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"not_q{i}")
            if not_id is None:
                return False
            not_q_ids.append(not_id)

        # ========== INCREMENT LOGIC (same as binary counter) ==========

        # Carry chain AND gates: carry1 = Q0&Q1, carry2 = carry1&Q2
        inc_carry_and_ids = []
        inc_carry_x = not_x + HORIZONTAL_GATE_SPACING
        for i in range(2):
            and_id = await self.create_element("And", inc_carry_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"inc_carry{i}")
            if and_id is None:
                return False
            inc_carry_and_ids.append(and_id)

        # NOT carry gates for XOR decomposition
        inc_not_carry_ids = []
        inc_not_carry_x = inc_carry_x + HORIZONTAL_GATE_SPACING
        for i in range(3):
            not_id = await self.create_element("Not", inc_not_carry_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"inc_not_carry{i}")
            if not_id is None:
                return False
            inc_not_carry_ids.append(not_id)

        # XOR AND gates (2 per bit for bits 1-3)
        inc_xor_and_ids = []
        inc_xor_and_x = inc_not_carry_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            bit_ands = []
            for j in range(2):
                and_id = await self.create_element("And", inc_xor_and_x + (j * HORIZONTAL_GATE_SPACING), 100.0 + (i * VERTICAL_STAGE_SPACING), f"inc_xor_and{i}_{j}")
                if and_id is None:
                    return False
                bit_ands.append(and_id)
            inc_xor_and_ids.append(bit_ands)

        # XOR OR gates
        inc_xor_or_ids = []
        inc_xor_or_x = inc_xor_and_x + (2 * HORIZONTAL_GATE_SPACING)
        for i in range(4):
            or_id = await self.create_element("Or", inc_xor_or_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"inc_xor_or{i}")
            if or_id is None:
                return False
            inc_xor_or_ids.append(or_id)

        await self.log("  Created increment logic gates")

        # ========== DECREMENT LOGIC (borrow chain) ==========
        # dec[0] = NOT(Q0) -- same as inc[0], reuse not_q_ids[0]
        # dec[1] = Q1 XOR NOT(Q0) = (NOT(Q1) AND NOT(Q0)) OR (Q1 AND Q0)
        # dec[2] = Q2 XOR (NOT(Q0) AND NOT(Q1))
        # dec[3] = Q3 XOR (NOT(Q0) AND NOT(Q1) AND NOT(Q2))

        # Borrow chain AND gates: borrow1 = NOT(Q0)&NOT(Q1), borrow2 = borrow1&NOT(Q2)
        dec_borrow_and_ids = []
        dec_borrow_x = inc_xor_or_x + HORIZONTAL_GATE_SPACING
        for i in range(2):
            and_id = await self.create_element("And", dec_borrow_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"dec_borrow{i}")
            if and_id is None:
                return False
            dec_borrow_and_ids.append(and_id)

        # NOT borrow gates for XOR decomposition
        dec_not_borrow_ids = []
        dec_not_borrow_x = dec_borrow_x + HORIZONTAL_GATE_SPACING
        for i in range(3):
            not_id = await self.create_element("Not", dec_not_borrow_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"dec_not_borrow{i}")
            if not_id is None:
                return False
            dec_not_borrow_ids.append(not_id)

        # XOR AND gates for decrement (2 per bit for bits 1-3)
        dec_xor_and_ids = []
        dec_xor_and_x = dec_not_borrow_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            bit_ands = []
            for j in range(2):
                and_id = await self.create_element("And", dec_xor_and_x + (j * HORIZONTAL_GATE_SPACING), 100.0 + (i * VERTICAL_STAGE_SPACING), f"dec_xor_and{i}_{j}")
                if and_id is None:
                    return False
                bit_ands.append(and_id)
            dec_xor_and_ids.append(bit_ands)

        # XOR OR gates for decrement
        dec_xor_or_ids = []
        dec_xor_or_x = dec_xor_and_x + (2 * HORIZONTAL_GATE_SPACING)
        for i in range(4):
            or_id = await self.create_element("Or", dec_xor_or_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"dec_xor_or{i}")
            if or_id is None:
                return False
            dec_xor_or_ids.append(or_id)

        await self.log("  Created decrement logic gates")

        # ========== DIRECTION MUX (selects inc vs dec) ==========
        dir_mux_ids = []
        dir_mux_x = dec_xor_or_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            mux_id = await self.create_element("Mux", dir_mux_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"dir_mux{i}")
            if mux_id is None:
                return False
            dir_mux_ids.append(mux_id)

        # ========== ENABLE MUX (selects count vs hold) ==========
        en_mux_ids = []
        en_mux_x = dir_mux_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            mux_id = await self.create_element("Mux", en_mux_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"en_mux{i}")
            if mux_id is None:
                return False
            en_mux_ids.append(mux_id)

        await self.log("  Created direction and enable muxes")

        # ========== D FLIP-FLOPS ==========
        dff_ids = []
        dff_x = en_mux_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level1_d_flip_flop")):

                return False

            ff_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level1_d_flip_flop"), dff_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"FF{i}")
            if ff_id is None:
                return False
            dff_ids.append(ff_id)

        await self.log("  Instantiated 4 D Flip-Flop ICs")

        # ========== OUTPUT LEDs ==========
        output_led_ids = []
        output_x = dff_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            led_id = await self.create_element("Led", output_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"Q[{i}]")
            if led_id is None:
                return False
            output_led_ids.append(led_id)

        await self.log("  Created output LEDs")

        # ===================================================================
        # WIRING
        # ===================================================================

        # ========== Connect CLK to all FFs ==========
        for i in range(4):
            if not await self.connect(clk_id, dff_ids[i], target_port_label="Clock"):
                return False

        # ========== Connect Preset to Vcc (inactive), Clear to RST ==========
        for i in range(4):
            if not await self.connect(vcc_id, dff_ids[i], target_port_label="Preset"):
                return False

            if not await self.connect(rst_id, dff_ids[i], target_port_label="Clear"):
                return False

        # ========== Connect FF outputs to LEDs and NOT gates ==========
        for i in range(4):
            if not await self.connect(dff_ids[i], output_led_ids[i], source_port_label="Q"):
                return False

            if not await self.connect(dff_ids[i], not_q_ids[i], source_port_label="Q"):
                return False

        # ========== Connect Enable mux outputs to FF D inputs ==========
        for i in range(4):
            if not await self.connect(en_mux_ids[i], dff_ids[i], target_port_label="D"):
                return False

        # ========== INCREMENT CARRY CHAIN ==========
        # carry1 = Q0 AND Q1
        if not await self.connect(dff_ids[0], inc_carry_and_ids[0], source_port_label="Q"):
            return False

        if not await self.connect(dff_ids[1], inc_carry_and_ids[0], source_port_label="Q", target_port=1):
            return False

        # carry2 = carry1 AND Q2
        if not await self.connect(inc_carry_and_ids[0], inc_carry_and_ids[1]):
            return False

        if not await self.connect(dff_ids[2], inc_carry_and_ids[1], source_port_label="Q", target_port=1):
            return False

        # ========== INCREMENT NOT CARRY ==========
        # NOT Q0
        if not await self.connect(dff_ids[0], inc_not_carry_ids[0], source_port_label="Q"):
            return False

        # NOT carry1
        if not await self.connect(inc_carry_and_ids[0], inc_not_carry_ids[1]):
            return False

        # NOT carry2
        if not await self.connect(inc_carry_and_ids[1], inc_not_carry_ids[2]):
            return False

        # ========== INCREMENT XOR LOGIC ==========

        # Bit 0: inc[0] = NOT(Q0) -- output from not_q_ids[0]
        # Feed NOT(Q0) to inc_xor_or[0] (just pass through via AND/OR)
        # Actually, for bit 0, inc[0] = NOT(Q0). We'll route it through the XOR OR:
        # inc_xor_and[0][0] = NOT(Q0) AND 1 = NOT(Q0)
        # inc_xor_and[0][1] = Q0 AND 0 = 0
        # inc_xor_or[0] = NOT(Q0) OR 0 = NOT(Q0)
        # Simpler: just connect not_q_ids[0] directly to inc_xor_or[0]
        # But we created 2 AND gates per bit... Let's use the simpler approach:
        # Connect NOT(Q0) to one AND input and Vcc to other
        if not await self.connect(not_q_ids[0], inc_xor_and_ids[0][0]):
            return False

        if not await self.connect(vcc_id, inc_xor_and_ids[0][0], target_port=1):
            return False

        # Second AND for bit 0 not needed, connect GND-equivalent (tie inputs to get 0)
        # Actually, leave it unconnected - default is 0 which is fine for OR
        # Let's connect Q0 AND GND = 0 to second AND
        if not await self.connect(dff_ids[0], inc_xor_and_ids[0][1], source_port_label="Q"):
            return False

        if not await self.connect(inc_not_carry_ids[0], inc_xor_and_ids[0][1], target_port=1):
            return False

        # OR for bit 0
        if not await self.connect(inc_xor_and_ids[0][0], inc_xor_or_ids[0]):
            return False

        if not await self.connect(inc_xor_and_ids[0][1], inc_xor_or_ids[0], target_port=1):
            return False

        # Bit 1: inc[1] = Q1 XOR Q0 = (NOT(Q1) AND Q0) OR (Q1 AND NOT(Q0))
        if not await self.connect(not_q_ids[1], inc_xor_and_ids[1][0]):
            return False

        if not await self.connect(dff_ids[0], inc_xor_and_ids[1][0], source_port_label="Q", target_port=1):
            return False

        if not await self.connect(dff_ids[1], inc_xor_and_ids[1][1], source_port_label="Q"):
            return False

        if not await self.connect(inc_not_carry_ids[0], inc_xor_and_ids[1][1], target_port=1):
            return False

        if not await self.connect(inc_xor_and_ids[1][0], inc_xor_or_ids[1]):
            return False

        if not await self.connect(inc_xor_and_ids[1][1], inc_xor_or_ids[1], target_port=1):
            return False

        # Bit 2: inc[2] = Q2 XOR carry1 = (NOT(Q2) AND carry1) OR (Q2 AND NOT(carry1))
        if not await self.connect(not_q_ids[2], inc_xor_and_ids[2][0]):
            return False

        if not await self.connect(inc_carry_and_ids[0], inc_xor_and_ids[2][0], target_port=1):
            return False

        if not await self.connect(dff_ids[2], inc_xor_and_ids[2][1], source_port_label="Q"):
            return False

        if not await self.connect(inc_not_carry_ids[1], inc_xor_and_ids[2][1], target_port=1):
            return False

        if not await self.connect(inc_xor_and_ids[2][0], inc_xor_or_ids[2]):
            return False

        if not await self.connect(inc_xor_and_ids[2][1], inc_xor_or_ids[2], target_port=1):
            return False

        # Bit 3: inc[3] = Q3 XOR carry2 = (NOT(Q3) AND carry2) OR (Q3 AND NOT(carry2))
        if not await self.connect(not_q_ids[3], inc_xor_and_ids[3][0]):
            return False

        if not await self.connect(inc_carry_and_ids[1], inc_xor_and_ids[3][0], target_port=1):
            return False

        if not await self.connect(dff_ids[3], inc_xor_and_ids[3][1], source_port_label="Q"):
            return False

        if not await self.connect(inc_not_carry_ids[2], inc_xor_and_ids[3][1], target_port=1):
            return False

        if not await self.connect(inc_xor_and_ids[3][0], inc_xor_or_ids[3]):
            return False

        if not await self.connect(inc_xor_and_ids[3][1], inc_xor_or_ids[3], target_port=1):
            return False

        await self.log("  Wired increment logic")

        # ========== DECREMENT BORROW CHAIN ==========
        # borrow1 = NOT(Q0) AND NOT(Q1)
        if not await self.connect(not_q_ids[0], dec_borrow_and_ids[0]):
            return False

        if not await self.connect(not_q_ids[1], dec_borrow_and_ids[0], target_port=1):
            return False

        # borrow2 = borrow1 AND NOT(Q2)
        if not await self.connect(dec_borrow_and_ids[0], dec_borrow_and_ids[1]):
            return False

        if not await self.connect(not_q_ids[2], dec_borrow_and_ids[1], target_port=1):
            return False

        # ========== DECREMENT NOT BORROW ==========
        # NOT(NOT(Q0)) = Q0 (for XOR decomposition)
        if not await self.connect(not_q_ids[0], dec_not_borrow_ids[0]):
            return False

        # NOT borrow1
        if not await self.connect(dec_borrow_and_ids[0], dec_not_borrow_ids[1]):
            return False

        # NOT borrow2
        if not await self.connect(dec_borrow_and_ids[1], dec_not_borrow_ids[2]):
            return False

        # ========== DECREMENT XOR LOGIC ==========

        # Bit 0: dec[0] = NOT(Q0) -- same as inc[0]
        # Route through AND/OR for consistency
        if not await self.connect(not_q_ids[0], dec_xor_and_ids[0][0]):
            return False

        if not await self.connect(vcc_id, dec_xor_and_ids[0][0], target_port=1):
            return False

        # dec bit 0: Q0 AND NOT(Q0) = 0 (ensures OR outputs NOT(Q0))
        if not await self.connect(dff_ids[0], dec_xor_and_ids[0][1], source_port_label="Q"):
            return False

        if not await self.connect(not_q_ids[0], dec_xor_and_ids[0][1], target_port=1):
            return False

        if not await self.connect(dec_xor_and_ids[0][0], dec_xor_or_ids[0]):
            return False

        if not await self.connect(dec_xor_and_ids[0][1], dec_xor_or_ids[0], target_port=1):
            return False

        # Bit 1: dec[1] = Q1 XOR NOT(Q0) = (NOT(Q1) AND NOT(Q0)) OR (Q1 AND Q0)
        # borrow for bit 1 is NOT(Q0), NOT(borrow) = Q0 = dec_not_borrow[0]
        if not await self.connect(not_q_ids[1], dec_xor_and_ids[1][0]):
            return False

        if not await self.connect(not_q_ids[0], dec_xor_and_ids[1][0], target_port=1):
            return False

        if not await self.connect(dff_ids[1], dec_xor_and_ids[1][1], source_port_label="Q"):
            return False

        # Q1 AND NOT(NOT(Q0)) = Q1 AND Q0
        if not await self.connect(dec_not_borrow_ids[0], dec_xor_and_ids[1][1], target_port=1):
            return False

        if not await self.connect(dec_xor_and_ids[1][0], dec_xor_or_ids[1]):
            return False

        if not await self.connect(dec_xor_and_ids[1][1], dec_xor_or_ids[1], target_port=1):
            return False

        # Bit 2: dec[2] = Q2 XOR borrow1 = (NOT(Q2) AND borrow1) OR (Q2 AND NOT(borrow1))
        if not await self.connect(not_q_ids[2], dec_xor_and_ids[2][0]):
            return False

        if not await self.connect(dec_borrow_and_ids[0], dec_xor_and_ids[2][0], target_port=1):
            return False

        if not await self.connect(dff_ids[2], dec_xor_and_ids[2][1], source_port_label="Q"):
            return False

        if not await self.connect(dec_not_borrow_ids[1], dec_xor_and_ids[2][1], target_port=1):
            return False

        if not await self.connect(dec_xor_and_ids[2][0], dec_xor_or_ids[2]):
            return False

        if not await self.connect(dec_xor_and_ids[2][1], dec_xor_or_ids[2], target_port=1):
            return False

        # Bit 3: dec[3] = Q3 XOR borrow2 = (NOT(Q3) AND borrow2) OR (Q3 AND NOT(borrow2))
        if not await self.connect(not_q_ids[3], dec_xor_and_ids[3][0]):
            return False

        if not await self.connect(dec_borrow_and_ids[1], dec_xor_and_ids[3][0], target_port=1):
            return False

        if not await self.connect(dff_ids[3], dec_xor_and_ids[3][1], source_port_label="Q"):
            return False

        if not await self.connect(dec_not_borrow_ids[2], dec_xor_and_ids[3][1], target_port=1):
            return False

        if not await self.connect(dec_xor_and_ids[3][0], dec_xor_or_ids[3]):
            return False

        if not await self.connect(dec_xor_and_ids[3][1], dec_xor_or_ids[3], target_port=1):
            return False

        await self.log("  Wired decrement logic")

        # ========== DIRECTION MUX WIRING ==========
        # Select between decrement and increment based on direction
        # Mux: In0=dec (Direction=0, count down), In1=inc (Direction=1, count up), Select=Direction
        for i in range(4):
            # Connect decrement result to In0 (selected when Direction=0)
            if not await self.connect(dec_xor_or_ids[i], dir_mux_ids[i], target_port_label="In0"):
                return False

            # Connect increment result to In1 (selected when Direction=1)
            if not await self.connect(inc_xor_or_ids[i], dir_mux_ids[i], target_port_label="In1"):
                return False

            # Connect Direction signal to Select
            if not await self.connect(direction_id, dir_mux_ids[i], target_port_label="S0"):
                return False

        await self.log("  Wired direction muxes")

        # ========== ENABLE MUX WIRING ==========
        # Select between hold and count modes based on enable signal
        # Mux: In0=Q[i] (Enable=0, hold current value), In1=count_val[i] (Enable=1, load count value), Select=Enable
        for i in range(4):
            # Connect current FF output to In0 (selected when Enable=0, holds value)
            if not await self.connect(dff_ids[i], en_mux_ids[i], source_port_label="Q", target_port_label="In0"):
                return False

            # Connect count value (from direction mux) to In1 (selected when Enable=1, counts)
            if not await self.connect(dir_mux_ids[i], en_mux_ids[i], target_port_label="In1"):
                return False

            # Connect Enable signal to Select
            if not await self.connect(enable_id, en_mux_ids[i], target_port_label="S0"):
                return False

        await self.log("  Wired enable muxes")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level5_up_down_counter_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"Successfully created 4-bit Up/Down Counter IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = UpDownCounter4BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Up/Down Counter IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
