#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Modulo-N Counter IC

Inputs: CLK (Clock), RST (Reset - active LOW), Modulo[0-3] (4-bit modulo value)
Outputs: Q[0-3] (4-bit count value), Overflow (pulse when count wraps)

Architecture (built from level1_d_flip_flop ICs):
- 4 D flip-flops for counter state
- Increment logic (same carry chain as binary counter)
- level3_comparator_4bit_equality IC compares NEXT value with modulo
- When next_count == modulo, force D inputs to 0 (reset on next clock)
- D[i] = inc[i] AND NOT(equal)

Modulo Counter behavior (counts 0 to N-1, then wraps):
- Counts 0, 1, 2, ..., N-1, 0, 1, ...
- Overflow signals when counter wraps

Usage:
    python create_level5_modulo_counter_4bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class ModuloCounter4BitBuilder(ICBuilderBase):
    """Builder for 4-bit Modulo-N Counter IC (built from D flip-flop ICs)"""

    async def create(self) -> bool:
        """Create the 4-bit Modulo-N Counter IC"""
        await self.begin_build('4-bit Modulo-N Counter')
        if not await self.create_new_circuit():
            return False

        input_x = 50.0

        # ========== Create inputs ==========
        clk_id = await self.create_element("InputSwitch", input_x, 100.0, "CLK")
        if clk_id is None:
            return False

        rst_id = await self.create_element("InputSwitch", input_x, 100.0 + (0.5 * VERTICAL_STAGE_SPACING), "RST")
        if rst_id is None:
            return False

        # Create Modulo inputs (4-bit)
        modulo_ids = []
        for i in range(4):
            modulo_id = await self.create_element("InputSwitch", input_x, 100.0 + ((i + 1) * VERTICAL_STAGE_SPACING), f"Modulo[{i}]")
            if modulo_id is None:
                return False
            modulo_ids.append(modulo_id)

        # Create Vcc element for Preset/Clear inactive state
        vcc_id = await self.create_element("InputVcc", input_x, 100.0 + (5 * VERTICAL_STAGE_SPACING), "Vcc")
        if vcc_id is None:
            return False

        await self.log("  Created inputs: CLK, RST, Modulo[0-3], Vcc")

        # ========== Create NOT gates for Q outputs ==========
        not_q_ids = []
        not_x = input_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            not_id = await self.create_element("Not", not_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"not_q{i}")
            if not_id is None:
                return False
            not_q_ids.append(not_id)

        # ========== INCREMENT LOGIC (same as binary counter) ==========

        # Carry chain AND gates
        carry_and_ids = []
        carry_x = not_x + HORIZONTAL_GATE_SPACING
        for i in range(2):
            and_id = await self.create_element("And", carry_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"and_carry{i}")
            if and_id is None:
                return False
            carry_and_ids.append(and_id)

        # NOT carry gates for XOR decomposition
        not_carry_ids = []
        not_carry_x = carry_x + HORIZONTAL_GATE_SPACING
        for i in range(3):
            not_id = await self.create_element("Not", not_carry_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"not_carry{i}")
            if not_id is None:
                return False
            not_carry_ids.append(not_id)

        # XOR AND gates (2 per bit)
        xor_and_ids = []
        xor_and_x = not_carry_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            bit_ands = []
            for j in range(2):
                and_id = await self.create_element("And", xor_and_x + (j * HORIZONTAL_GATE_SPACING), 100.0 + (i * VERTICAL_STAGE_SPACING), f"and_xor{i}_{j}")
                if and_id is None:
                    return False
                bit_ands.append(and_id)
            xor_and_ids.append(bit_ands)

        # XOR OR gates (increment output before gating)
        xor_or_ids = []
        xor_or_x = xor_and_x + (2 * HORIZONTAL_GATE_SPACING)
        for i in range(4):
            or_id = await self.create_element("Or", xor_or_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"or_xor{i}")
            if or_id is None:
                return False
            xor_or_ids.append(or_id)

        await self.log("  Created increment logic gates")

        # ========== COMPARATOR IC ==========
        comparator_x = xor_or_x + HORIZONTAL_GATE_SPACING
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level3_comparator_4bit_equality")):

            return False

        comparator_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level3_comparator_4bit_equality"), comparator_x, 100.0, "Comparator")
        if comparator_id is None:
            return False
        await self.log("  Instantiated 4-bit Equality Comparator IC")

        # ========== NOT(equal) gate ==========
        not_equal_id = await self.create_element("Not", comparator_x + HORIZONTAL_GATE_SPACING, 100.0, "not_equal")
        if not_equal_id is None:
            return False

        # ========== GATE AND gates: D[i] = inc[i] AND NOT(equal) ==========
        gate_and_ids = []
        gate_and_x = comparator_x + (2 * HORIZONTAL_GATE_SPACING)
        for i in range(4):
            and_id = await self.create_element("And", gate_and_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"gate_and{i}")
            if and_id is None:
                return False
            gate_and_ids.append(and_id)

        await self.log("  Created comparator gating logic")

        # ========== D FLIP-FLOPS ==========
        dff_ids = []
        dff_x = gate_and_x + HORIZONTAL_GATE_SPACING
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

        # Overflow output LED
        overflow_id = await self.create_element("Led", output_x, 100.0 + (4 * VERTICAL_STAGE_SPACING), "Overflow")
        if overflow_id is None:
            return False

        await self.log("  Created output LEDs (Q[0-3] + Overflow)")

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

        # ========== Connect gate AND outputs to FF D inputs ==========
        for i in range(4):
            if not await self.connect(gate_and_ids[i], dff_ids[i], target_port_label="D"):
                return False

        # ========== CARRY CHAIN ==========
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

        # ========== NOT CARRY ==========
        if not await self.connect(dff_ids[0], not_carry_ids[0], source_port_label="Q"):
            return False

        if not await self.connect(carry_and_ids[0], not_carry_ids[1]):
            return False

        if not await self.connect(carry_and_ids[1], not_carry_ids[2]):
            return False

        # ========== XOR LOGIC ==========

        # Bit 0: inc[0] = NOT(Q0)
        if not await self.connect(not_q_ids[0], xor_and_ids[0][0]):
            return False

        if not await self.connect(vcc_id, xor_and_ids[0][0], target_port=1):
            return False

        if not await self.connect(dff_ids[0], xor_and_ids[0][1], source_port_label="Q"):
            return False

        if not await self.connect(not_carry_ids[0], xor_and_ids[0][1], target_port=1):
            return False

        if not await self.connect(xor_and_ids[0][0], xor_or_ids[0]):
            return False

        if not await self.connect(xor_and_ids[0][1], xor_or_ids[0], target_port=1):
            return False

        # Bit 1: inc[1] = Q1 XOR Q0
        if not await self.connect(not_q_ids[1], xor_and_ids[1][0]):
            return False

        if not await self.connect(dff_ids[0], xor_and_ids[1][0], source_port_label="Q", target_port=1):
            return False

        if not await self.connect(dff_ids[1], xor_and_ids[1][1], source_port_label="Q"):
            return False

        if not await self.connect(not_carry_ids[0], xor_and_ids[1][1], target_port=1):
            return False

        if not await self.connect(xor_and_ids[1][0], xor_or_ids[1]):
            return False

        if not await self.connect(xor_and_ids[1][1], xor_or_ids[1], target_port=1):
            return False

        # Bit 2: inc[2] = Q2 XOR carry1
        if not await self.connect(not_q_ids[2], xor_and_ids[2][0]):
            return False

        if not await self.connect(carry_and_ids[0], xor_and_ids[2][0], target_port=1):
            return False

        if not await self.connect(dff_ids[2], xor_and_ids[2][1], source_port_label="Q"):
            return False

        if not await self.connect(not_carry_ids[1], xor_and_ids[2][1], target_port=1):
            return False

        if not await self.connect(xor_and_ids[2][0], xor_or_ids[2]):
            return False

        if not await self.connect(xor_and_ids[2][1], xor_or_ids[2], target_port=1):
            return False

        # Bit 3: inc[3] = Q3 XOR carry2
        if not await self.connect(not_q_ids[3], xor_and_ids[3][0]):
            return False

        if not await self.connect(carry_and_ids[1], xor_and_ids[3][0], target_port=1):
            return False

        if not await self.connect(dff_ids[3], xor_and_ids[3][1], source_port_label="Q"):
            return False

        if not await self.connect(not_carry_ids[2], xor_and_ids[3][1], target_port=1):
            return False

        if not await self.connect(xor_and_ids[3][0], xor_or_ids[3]):
            return False

        if not await self.connect(xor_and_ids[3][1], xor_or_ids[3], target_port=1):
            return False

        await self.log("  Wired increment logic")

        # ========== COMPARATOR WIRING ==========
        # Compare next increment value against modulo limit
        # Connect inc[i] (next_count value) to comparator A inputs
        for i in range(4):
            if not await self.connect(xor_or_ids[i], comparator_id, target_port_label=f"A[{i}]"):
                return False

        # Connect Modulo[i] limit to comparator B inputs
        for i in range(4):
            if not await self.connect(modulo_ids[i], comparator_id, target_port_label=f"B[{i}]"):
                return False

        # Connect comparator Equal output to NOT gate (convert Equal to NotEqual)
        if not await self.connect(comparator_id, not_equal_id, source_port_label="Equal"):
            return False

        # Connect Equal signal to Overflow LED (asserts when counter would overflow)
        if not await self.connect(comparator_id, overflow_id, source_port_label="Equal"):
            return False

        await self.log("  Wired comparator")

        # ========== GATE AND WIRING ==========
        # Gating logic: D[i] = inc[i] AND NOT(equal) prevents loading when overflow detected
        for i in range(4):
            # Connect increment result to first AND input
            if not await self.connect(xor_or_ids[i], gate_and_ids[i]):
                return False

            # Connect NOT(equal) to second AND input (gate prevents update on overflow)
            if not await self.connect(not_equal_id, gate_and_ids[i], target_port=1):
                return False

        await self.log("  Wired gate AND logic")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level5_modulo_counter_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"Successfully created 4-bit Modulo Counter IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = ModuloCounter4BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Modulo Counter IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
