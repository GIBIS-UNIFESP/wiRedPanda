#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Priority Encoder 8-to-3 IC (with priority: 7 > 6 > ... > 0)

Inputs: data[0..7] (8 data input bits)
Outputs: addr[0..2] (3-bit binary output of highest priority active input)

Circuit Logic:
- Uses cascaded inhibit signals to ensure only the highest priority input is encoded
- inhibit[i] = NOT(OR of all higher priority inputs)
- selected[i] = data[i] AND inhibit[i] (only true if i is the highest priority active input)
- addr bits encode the selected input using combinational logic

Usage:
    python create_priority_encoder_8to3.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class PriorityEncoder8to3Builder(ICBuilderBase):
    """Builder for Priority Encoder 8-to-3 IC"""

    async def create(self) -> bool:
        """Create the Priority Encoder 8-to-3 IC"""
        await self.begin_build('Priority Encoder 8-to-3')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position hierarchy
        input_x = 50.0
        input_y = 100.0
        not_inhibit6_x = input_x + HORIZONTAL_GATE_SPACING
        not_inhibit6_y = input_y + VERTICAL_STAGE_SPACING
        or_chain_x = input_x + (2 * HORIZONTAL_GATE_SPACING)
        or_chain_y = input_y + (1.5 * VERTICAL_STAGE_SPACING)
        inhibit_x = input_x + (3 * HORIZONTAL_GATE_SPACING)
        inhibit_base_y = input_y + VERTICAL_STAGE_SPACING
        selected_x = input_x + (4 * HORIZONTAL_GATE_SPACING)
        selected_base_y = input_y + VERTICAL_STAGE_SPACING
        addr_or_x = input_x + (5 * HORIZONTAL_GATE_SPACING)
        addr_or_final_x = input_x + (6 * HORIZONTAL_GATE_SPACING)
        output_x = input_x + (7 * HORIZONTAL_GATE_SPACING)
        output_base_y = input_y + (1.5 * VERTICAL_STAGE_SPACING)
        inhibit_spacing = 30.0
        selected_spacing = 30.0
        addr_output_spacing = 50.0

        # Create input switches for data bits (8 inputs)
        data_inputs = []
        for i in range(8):
            data_id = await self.create_element("InputSwitch", input_x, 100.0 + i * VERTICAL_STAGE_SPACING, f"data[{i}]")
            if data_id is None:
                return False
            data_inputs.append(data_id)
            await self.log(f"  Created data[{i}] (id={data_id})")

        # Build inhibit cascade signals
        # inhibit[i] = NOT(OR of all inputs with priority > i)
        # inhibit7: always 1 (data7 has no higher priority inputs)
        # inhibit6: NOT(data[7])
        # inhibit5: NOT(data[7] OR data[6])
        # etc.

        await self.log("  Building inhibit cascade signals:")

        # NOT(data[7]) for inhibit6
        inhibit6 = await self.create_element("Not", not_inhibit6_x, not_inhibit6_y, "inhibit6")
        if inhibit6 is None:
            return False
        if not await self.connect(data_inputs[7], inhibit6):
            return False

        # Build OR gates for inhibit signals (OPTIMIZED - multi-input OR gates)
        # inhibit[i] = NOT(OR of all data with priority > i)
        # This replaces the cascading chain with direct multi-input OR gates
        or_chain = []  # or_chain[i] = OR(data[7..i])

        # or_chain[7] = data[7]
        or_chain.append(data_inputs[7])

        # or_chain[6] = 2-input OR(data[7], data[6])
        or76 = await self.create_element("Or", or_chain_x, or_chain_y, "or_76")
        if or76 is None:
            return False

        if not await self.connect(data_inputs[7], or76):
            return False

        if not await self.connect(data_inputs[6], or76, target_port=1):
            return False
        or_chain.append(or76)

        # Create remaining OR gates with increasing input counts
        # or_chain[5] = 3-input OR(data[7], data[6], data[5])
        # or_chain[4] = 4-input OR(data[7], data[6], data[5], data[4])
        # ... etc
        for i in range(5, -1, -1):
            num_inputs = 8 - i  # Number of data inputs to OR together

            or_gate = await self.create_element("Or", or_chain_x + (7-i)*15.0, or_chain_y, f"or_{7}_to_{i}")
            if or_gate is None:
                return False

            # Set OR gate to have correct number of inputs
            if num_inputs > 2:
                # Set input size for gates that need >2 inputs
                set_props = await self.mcp.send_command("change_input_size", {
                    "element_id": or_gate,
                    "size": num_inputs
                })
                if not set_props.success:
                    print(f"Warning: Could not set input_size={num_inputs} for OR gate")

            # Connect all required data inputs
            for port_idx in range(num_inputs):
                data_bit_idx = 7 - port_idx  # data[7], then data[6], etc.
                if not await self.connect(data_inputs[data_bit_idx], or_gate, target_port=port_idx):
                    return False

            or_chain.append(or_gate)

        # Build inhibit gates from OR chain
        inhibits: list[int | None] = [None] * 8  # inhibits[i] = NOT(or_chain[7-i])
        inhibits[7] = None  # data[7] is never inhibited

        # inhibit[6] already created above
        inhibits[6] = inhibit6

        # Create inhibit gates for 0-5
        for i in range(5, -1, -1):
            inhibit_gate = await self.create_element("Not", inhibit_x, inhibit_base_y + (5-i)*inhibit_spacing, f"inhibit{i}")
            if inhibit_gate is None:
                return False
            # Connect OR chain output
            or_chain_idx = 6 - i  # or_chain[6-i] = OR(data[7..i+1])
            if not await self.connect(or_chain[or_chain_idx], inhibit_gate):
                return False
            inhibits[i] = inhibit_gate

        await self.log("  Created inhibit cascade")

        # Create selected signals: selected[i] = data[i] AND inhibit[i]
        selected: list[int | None] = [None] * 8
        selected[7] = data_inputs[7]  # data[7] doesn't need AND gate (no inhibition)

        for i in range(6, -1, -1):
            sel_gate = await self.create_element("And", selected_x, selected_base_y + (6-i)*selected_spacing, f"sel{i}")
            if sel_gate is None:
                return False
            # Connect data[i]
            if not await self.connect(data_inputs[i], sel_gate):
                return False
            # Connect inhibit[i]
            inhibit_i = inhibits[i]
            assert inhibit_i is not None
            if not await self.connect(inhibit_i, sel_gate, target_port=1):
                return False
            selected[i] = sel_gate

        await self.log("  Created selected signals")

        # Narrow list[int | None] to int: all entries 0-7 are set above
        # (the loop returned False early if any create_element failed)
        sel1, sel2, sel3, sel4, sel5, sel6, sel7 = (
            selected[1], selected[2], selected[3], selected[4],
            selected[5], selected[6], selected[7],
        )
        assert sel1 is not None and sel2 is not None and sel3 is not None
        assert sel4 is not None and sel5 is not None and sel6 is not None and sel7 is not None

        # Encode the selected inputs
        # addr[2]: 1 if selected[4..7] (indices with bit2=1: 4,5,6,7)
        addr2_gate = await self.create_element("Or", addr_or_x, output_base_y, "addr2")
        if addr2_gate is None:
            return False
        for idx, sel_src in enumerate([sel4, sel5]):
            if idx >= 2:
                break  # OR gate has only 2 inputs
            if not await self.connect(sel_src, addr2_gate, target_port=idx):
                return False

        # Need more OR gates for addr[2] = selected[4] OR selected[5] OR selected[6] OR selected[7]
        or_addr2_56 = await self.create_element("Or", addr_or_x, output_base_y + (1.5 * VERTICAL_STAGE_SPACING), "or_addr2_56")
        if or_addr2_56 is None:
            return False
        if not await self.connect(sel6, or_addr2_56):
            return False
        if not await self.connect(sel7, or_addr2_56, target_port=1):
            return False

        addr2_final = await self.create_element("Or", addr_or_final_x, output_base_y + (0.75 * VERTICAL_STAGE_SPACING), "addr2_final")
        if addr2_final is None:
            return False
        if not await self.connect(addr2_gate, addr2_final):
            return False
        if not await self.connect(or_addr2_56, addr2_final, target_port=1):
            return False

        # addr[1]: 1 if selected[2..3] OR selected[6..7] (indices with bit1=1: 2,3,6,7)
        or_addr1_23 = await self.create_element("Or", addr_or_x, output_base_y + (3 * VERTICAL_STAGE_SPACING), "or_addr1_23")
        if or_addr1_23 is None:
            return False
        if not await self.connect(sel2, or_addr1_23):
            return False
        if not await self.connect(sel3, or_addr1_23, target_port=1):
            return False

        or_addr1_67 = await self.create_element("Or", addr_or_x, output_base_y + (4 * VERTICAL_STAGE_SPACING), "or_addr1_67")
        if or_addr1_67 is None:
            return False
        if not await self.connect(sel6, or_addr1_67):
            return False
        if not await self.connect(sel7, or_addr1_67, target_port=1):
            return False

        addr1_final = await self.create_element("Or", addr_or_final_x, output_base_y + (3.05 * VERTICAL_STAGE_SPACING), "addr1_final")
        if addr1_final is None:
            return False
        if not await self.connect(or_addr1_23, addr1_final):
            return False
        if not await self.connect(or_addr1_67, addr1_final, target_port=1):
            return False

        # addr[0]: 1 if selected[1] OR selected[3] OR selected[5] OR selected[7] (odd indices)
        or_addr0_13 = await self.create_element("Or", addr_or_x, output_base_y + (5 * VERTICAL_STAGE_SPACING), "or_addr0_13")
        if or_addr0_13 is None:
            return False
        if not await self.connect(sel1, or_addr0_13):
            return False
        if not await self.connect(sel3, or_addr0_13, target_port=1):
            return False

        or_addr0_57 = await self.create_element("Or", addr_or_x, output_base_y + (6 * VERTICAL_STAGE_SPACING), "or_addr0_57")
        if or_addr0_57 is None:
            return False
        if not await self.connect(sel5, or_addr0_57):
            return False
        if not await self.connect(sel7, or_addr0_57, target_port=1):
            return False

        addr0_final = await self.create_element("Or", addr_or_final_x, output_base_y + (5.05 * VERTICAL_STAGE_SPACING), "addr0_final")
        if addr0_final is None:
            return False
        if not await self.connect(or_addr0_13, addr0_final):
            return False
        if not await self.connect(or_addr0_57, addr0_final, target_port=1):
            return False

        await self.log("  Created output encoding logic")

        # Create output LEDs for the three address bits
        output_leds = []
        for i in range(3):
            led_id = await self.create_element("Led", output_x, output_base_y + i * addr_output_spacing, f"addr[{i}]")
            if led_id is None:
                return False
            output_leds.append(led_id)

        # Connect address outputs to LEDs
        addr_outputs = [addr0_final, addr1_final, addr2_final]
        for i, led_id in enumerate(output_leds):
            if not await self.connect(addr_outputs[i], led_id):
                return False

        await self.log("  Created and connected output LEDs")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level2_priority_encoder_8to3.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"Successfully created PriorityEncoder8to3 IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = PriorityEncoder8to3Builder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Priority Encoder 8-to-3 IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
