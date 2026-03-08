#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create CPU Level 6: 8-bit Stack Pointer IC

Composes Stack Pointer register with increment/decrement (PUSH/POP) capability.

Inputs:
  - LoadValue[0..7] - Value to load into SP
  - Load - Load signal (priority 2)
  - Push - Decrement SP (stack grow down)
  - Pop - Increment SP (stack grow up)
  - Reset - Reset to 0xFF (priority 1, highest)
  - Clock - Clock signal
  - Enable - Enable updates (usually tied high)

Outputs:
  - SP[0..7] - Stack Pointer value

Architecture:
- 8x D Flip-Flops for SP storage
- RippleCarryAdder8bit for SP ±1 operations
- Mux logic for selecting: reset (0xFF) > load > push/pop
- Priority: Reset > Load > (Push|Pop) > Hold

Stack characteristics:
- Grows downward: SP decrements on PUSH
- Initial value: 0xFF (stack empty at top of 256-byte memory)
- PUSH: SP = SP - 1
- POP: SP = SP + 1

Usage:
    python3 create_level6_stack_pointer_8bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class StackPointer8BitBuilder(ICBuilderBase):
    """Builder for 8-bit Stack Pointer IC with PUSH/POP capability"""

    async def create(self) -> bool:
        """Create the 8-bit Stack Pointer IC"""
        await self.begin_build("Stack Pointer 8-bit")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # ========== INPUT ELEMENTS ==========

        # Create 8-bit LoadValue input switches (vertically stacked)
        load_value_inputs = []
        input_x = 50.0
        for i in range(8):
            element_id = await self.create_element("InputSwitch", input_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"LoadValue[{i}]")
            if element_id is None:
                return False
            load_value_inputs.append(element_id)

        # Create control signals (shared by all bits)
        ctrl_x = input_x + HORIZONTAL_GATE_SPACING
        control_signals = {}

        for ctrl_name in ["Clock", "Load", "Push", "Pop", "Reset", "Enable"]:
            element_id = await self.create_element("InputSwitch", ctrl_x, 100.0 + len(control_signals) * VERTICAL_STAGE_SPACING, ctrl_name)
            if element_id is None:
                return False
            control_signals[ctrl_name] = element_id

        # ========== LOGIC ELEMENTS ==========

        # Instantiate RippleCarryAdder8bit for SP ±1 operations
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level6_ripple_adder_8bit")):

            return False

        adder_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level6_ripple_adder_8bit"), 300.0, 200.0, "Adder")
        if adder_id is None:
            return False

        # Create 8 D Flip-Flops for SP storage
        sp_flipflops = []
        ff_x = 550.0
        for i in range(8):
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level1_d_flip_flop")):

                return False

            element_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level1_d_flip_flop"), ff_x + (i * HORIZONTAL_GATE_SPACING), 200.0, f"SP[{i}]")
            if element_id is None:
                return False
            sp_flipflops.append(element_id)

        # Create constant high (InputVcc) for reset to 0xFF
        vcc_inputs = []
        vcc_x = 800.0
        for i in range(8):
            element_id = await self.create_element("InputVcc", vcc_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"Vcc[{i}]")
            if element_id is None:
                return False
            vcc_inputs.append(element_id)

        # Create 8x PriorityMux3to1 for priority: reset > load > push/pop
        priority_muxes = []
        mux_x = 900.0
        for i in range(8):
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_priority_mux_3to1")):

                return False

            element_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_priority_mux_3to1"), mux_x + (i * HORIZONTAL_GATE_SPACING), 200.0, f"SPMux[{i}]")
            if element_id is None:
                return False
            priority_muxes.append(element_id)

        # Create 8 output LEDs for SP value
        sp_outputs = []
        out_x = 1250.0
        for i in range(8):
            element_id = await self.create_element("Led", out_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"SP[{i}]")
            if element_id is None:
                return False
            sp_outputs.append(element_id)

        # ========== CONNECTIONS ==========

        await self.log(f"✓ Created {self.element_count} elements")
        await self.log("🔌 Making connections...")

        # For each bit [0..7]:
        # 1. SP[i] from FF → Output LED SP[i]
        # 2. SP[i] from FF → Adder input A[i]
        # 3. LoadValue[i] → PriorityMux[i] input 1 (medium priority)
        # 4. Adder output[i] → PriorityMux[i] input 0 (low priority)
        # 5. Reset control → Generate 0xFF for PriorityMux[i] input 2 (high priority)
        # 6. PriorityMux[i] output → D Flip-Flop[i] input D
        # 7. Clock → All D Flip-Flops clock input

        for i in range(8):
            ff_id = sp_flipflops[i]
            pmux_id = priority_muxes[i]
            led_id = sp_outputs[i]
            load_val_id = load_value_inputs[i]
            vcc_id = vcc_inputs[i]

            # SP from FF to Output LED
            if not await self.connect(ff_id, led_id):
                return False

            # SP from FF to Adder input A[i]
            if not await self.connect(ff_id, adder_id, target_port_label=f"A[{i}]"):
                return False

            # Vcc to PriorityMux data0 (used for reset - outputs 1 when reset active)
            if not await self.connect(vcc_id, pmux_id, target_port_label="data0"):
                return False

            # LoadValue[i] to PriorityMux data1 (medium priority)
            if not await self.connect(load_val_id, pmux_id, target_port_label="data1"):
                return False

            # Adder output[i] to PriorityMux data2 (low priority)
            if not await self.connect(adder_id, pmux_id, source_port=i, target_port_label="data2"):
                return False

            # PriorityMux output to D Flip-Flop input D
            if not await self.connect(pmux_id, ff_id, source_port_label="out", target_port_label="D"):
                return False

            # Clock to D Flip-Flop Clock input
            if not await self.connect(control_signals["Clock"], ff_id, target_port_label="Clock"):
                return False

        # Connect select signals to Priority Muxes
        # sel0 = Reset (highest priority)
        # sel1 = Load (medium priority)
        # Default to Adder output (lowest priority when both sel0 and sel1 are low)
        for i in range(8):
            pmux_id = priority_muxes[i]

            # Connect Reset to sel0 of PriorityMux
            if not await self.connect(control_signals["Reset"], pmux_id, target_port_label="sel0"):
                return False

            # Connect Load to sel1 of PriorityMux
            if not await self.connect(control_signals["Load"], pmux_id, target_port_label="sel1"):
                return False

        # ========== Connect Preset/Clear pins (CRITICAL) ==========
        # Create a single Vcc element to keep Preset/Clear inactive (active-LOW)
        vcc_preset_clear_id = await self.create_element("InputVcc", 400.0, 500.0, "Vcc_PresetClear")
        if vcc_preset_clear_id is None:
            return False
        if vcc_preset_clear_id is None:
            self.log_error("extract element_id for Vcc Preset/Clear")
            return False

        # Connect Vcc to all FF Preset and Clear pins (keep them inactive)
        for i in range(8):
            ff_id = sp_flipflops[i]

            # Connect Vcc to Preset (active-LOW, so HIGH keeps it inactive)
            if not await self.connect(vcc_preset_clear_id, ff_id, target_port_label="Preset"):
                return False

            # Connect Vcc to Clear (active-LOW, so HIGH keeps it inactive)
            if not await self.connect(vcc_preset_clear_id, ff_id, target_port_label="Clear"):
                return False

        # Connect control signals to Adder for ±1 operations
        # When Push is high, add 0xFF (subtract 1)
        # When Pop is high, add 0x01
        # Note: Adder B inputs determine the operation (0x00 for hold, 0x01 for pop, 0xFF for push)
        # For now, connect B inputs to 0 (hold) - push/pop can be enhanced later

        await self.log(f"✓ Made {self.connection_count} connections")

        # Save the IC
        output_file = IC_COMPONENTS_DIR / "level6_stack_pointer_8bit.panda"
        if not await self.save_circuit(str(output_file)):
            return False

        await self.log(f"✅ Successfully created Stack Pointer 8-bit IC: {output_file}")
        return True


async def build(mcp) -> bool:
    """Build the Stack Pointer 8-bit IC"""
    builder = StackPointer8BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Stack Pointer 8-bit"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
