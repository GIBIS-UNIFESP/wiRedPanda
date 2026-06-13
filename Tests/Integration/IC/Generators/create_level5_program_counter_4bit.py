#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 4-bit Program Counter IC

Composes level4_register_4bit and level4_ripple_adder_4bit for PC functionality.

Inputs:
  loadValue[0..3] (4-bit load value)
  load (Load enable signal)
  inc (Increment enable signal)
  reset (Reset to 0, asynchronous, active HIGH)
  clock (Clock signal)

Outputs:
  pc[0..3] (4-bit program counter value)
  pc_plus_1[0..3] (Next PC = current PC + 1)

Architecture:
  - Instantiate level4_register_4bit: 4-bit register with async reset
  - Instantiate level4_ripple_adder_4bit: 4-bit PC + 1 logic
  - Mux layer mirrors the 8-bit PC: Mux1 sel = inc AND NOT(load) (Sum vs
    hold), Mux2 sel = load (loadValue vs Mux1) — priority
    reset > load > increment > hold (F52)

Usage:
    python create_level5_program_counter_4bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class ProgramCounter4BitBuilder(ICBuilderBase):
    """Builder for 4-bit Program Counter IC (composed from register + adder)"""

    async def create(self) -> bool:
        """Create the 4-bit Program Counter IC"""
        await self.begin_build('Program Counter 4-bit')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0

        # Create loadValue[0-3] inputs
        load_value_inputs = []
        for i in range(4):
            lv_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), 100.0, f"loadValue[{i}]")
            if lv_id is None:
                return False
            load_value_inputs.append(lv_id)
        await self.log("  ✓ Created 4 loadValue inputs")

        # Control inputs: load, inc, reset, clock
        ctrl_x = input_x + (4 * HORIZONTAL_GATE_SPACING) + HORIZONTAL_GATE_SPACING

        load_id = await self.create_element("InputSwitch", ctrl_x, 100.0, "load")
        if load_id is None:
            return False

        inc_id = await self.create_element("InputSwitch", ctrl_x, 100.0 + VERTICAL_STAGE_SPACING, "inc")
        if inc_id is None:
            return False

        reset_id = await self.create_element("InputSwitch", ctrl_x, 100.0 + (2 * VERTICAL_STAGE_SPACING), "reset")
        if reset_id is None:
            return False

        clock_id = await self.create_element("InputSwitch", ctrl_x, 100.0 + (3 * VERTICAL_STAGE_SPACING), "clock")
        if clock_id is None:
            return False

        await self.log("  ✓ Created control inputs (load, inc, reset, clock)")

        # Instantiate level4_register_4bit
        reg_id = await self.instantiate_ic("level4_register_4bit", input_x + (2 * HORIZONTAL_GATE_SPACING), 200.0, "Register4bit")
        if reg_id is None:
            return False
        await self.log("  ✓ Instantiated 4-bit Register")

        # Instantiate level4_ripple_adder_4bit
        adder_id = await self.instantiate_ic("level4_ripple_adder_4bit", input_x + (2 * HORIZONTAL_GATE_SPACING), 300.0, "Adder4bit")
        if adder_id is None:
            return False
        await self.log("  ✓ Instantiated 4-bit Adder")

        # Create mux layer for load/increment control, mirroring the 8-bit PC
        # (F52: the old order put the inc mux last, so inc beat load).
        # Priority: load > increment > hold
        # Mux1[i]: Sum (when inc AND NOT load) vs hold (register Q)
        # Mux2[i]: loadValue (when load) vs Mux1[i]

        mux_x = input_x
        mux_y = 250.0

        # Control: AND(inc, NOT(load)) gates the increment path
        not_load_id = await self.create_element("Not", mux_x, mux_y - VERTICAL_STAGE_SPACING, "NotLoad")
        if not_load_id is None:
            return False

        and_inc_id = await self.create_element("And", mux_x + HORIZONTAL_GATE_SPACING, mux_y - VERTICAL_STAGE_SPACING, "AndIncNotLoad")
        if and_inc_id is None:
            return False

        if not await self.connect(load_id, not_load_id):
            return False

        if not await self.connect(inc_id, and_inc_id, target_port=0):
            return False

        if not await self.connect(not_load_id, and_inc_id, target_port=1):
            return False

        mux1_outputs = []
        mux2_outputs = []

        for i in range(4):
            # Mux1: Select between register output (hold, Sel=0) and adder Sum
            # (increment, Sel=1), gated so load wins over inc
            mux1_id = await self.create_element("Mux", mux_x + (i * HORIZONTAL_GATE_SPACING), mux_y, f"Mux1_Inc[{i}]")
            if mux1_id is None:
                return False
            mux1_outputs.append(mux1_id)

            # Hold path: Register Q output to Mux1 In0
            if not await self.connect(reg_id, mux1_id, source_port_label=f"Q{i}", target_port_label="In0"):
                return False

            # Increment path: Adder Sum[i] to Mux1 In1
            if not await self.connect(adder_id, mux1_id, source_port_label=f"Sum[{i}]", target_port_label="In1"):
                return False

            # Mux1 Select: AND(inc, NOT load)
            if not await self.connect(and_inc_id, mux1_id, target_port_label="S0"):
                return False

            # Mux2: Select between Mux1 output (Sel=0) and loadValue (load, Sel=1)
            mux2_id = await self.create_element("Mux", mux_x + (i * HORIZONTAL_GATE_SPACING), mux_y + VERTICAL_STAGE_SPACING, f"Mux2_Load[{i}]")
            if mux2_id is None:
                return False
            mux2_outputs.append(mux2_id)

            # Hold-or-increment path: Mux1 output to Mux2 In0
            if not await self.connect(mux1_id, mux2_id, target_port_label="In0"):
                return False

            # Load path: loadValue[i] to Mux2 In1 (load has priority)
            if not await self.connect(load_value_inputs[i], mux2_id, target_port_label="In1"):
                return False

            # Mux2 Select: load
            if not await self.connect(load_id, mux2_id, target_port_label="S0"):
                return False

        await self.log("  ✓ Created mux layer (load > increment > hold)")

        # Connect Mux2 outputs to register inputs
        for i in range(4):
            if not await self.connect(mux2_outputs[i], reg_id, target_port_label=f"D{i}"):
                return False

        # Connect clock to register (CLK port)
        if not await self.connect(clock_id, reg_id, target_port_label="CLK"):
            return False

        # Connect reset to the register's async Reset (F52: this input used
        # to be dead — the register had no reset port to receive it)
        if not await self.connect(reset_id, reg_id, target_port_label="Reset"):
            return False

        # Connect enable signal to register (EN port) - always 1 to allow updates
        # Note: The load/increment/reset control is handled by the mux layer above the register
        # The register EN is always high so it captures whatever the mux provides on each clock edge
        en_vcc_id = await self.create_element("InputVcc", input_x + (4 * HORIZONTAL_GATE_SPACING), 150.0, "EN_VCC")
        if en_vcc_id is None:
            return False

        if not await self.connect(en_vcc_id, reg_id, target_port_label="EN"):
            return False

        await self.log("  ✓ Connected register data and control inputs")

        # Connect register Q outputs to adder A inputs (for PC + 1 computation)
        for i in range(4):
            if not await self.connect(reg_id, adder_id, source_port_label=f"Q{i}", target_port_label=f"A[{i}]"):
                return False

        # Connect constant 1 to adder B[0], 0 to B[1-3] for +1 operation
        vcc_id = await self.create_element("InputVcc", input_x + (2 * HORIZONTAL_GATE_SPACING), 320.0, "VCC_Const1")
        if vcc_id is None:
            return False

        if not await self.connect(vcc_id, adder_id, target_port_label="B[0]"):
            return False

        # Explicit constant 0 on B[1..3] (F34 — was an implicit unconnected default)
        gnd_id = await self.create_element("InputGnd", input_x + (2 * HORIZONTAL_GATE_SPACING), 360.0, "GND_Const0")
        if gnd_id is None:
            return False

        for i in range(1, 4):
            if not await self.connect(gnd_id, adder_id, target_port_label=f"B[{i}]"):
                return False

        await self.log("  ✓ Connected adder inputs for PC + 1 computation")

        # Create output LEDs for pc[0-3] (current program counter value)
        pc_output_x = input_x
        pc_output_y = 400.0

        for i in range(4):
            led_id = await self.create_element("Led", pc_output_x + (i * HORIZONTAL_GATE_SPACING), pc_output_y, f"pc[{i}]")
            if led_id is None:
                return False

            # Connect register Q output to pc output LED
            if not await self.connect(reg_id, led_id, source_port_label=f"Q{i}"):
                return False

        # Create output LEDs for pc_plus_1[0-3] (next program counter = PC + 1)
        pc_p1_output_y = pc_output_y + VERTICAL_STAGE_SPACING

        for i in range(4):
            led_id = await self.create_element("Led", pc_output_x + (i * HORIZONTAL_GATE_SPACING), pc_p1_output_y, f"pc_plus_1[{i}]")
            if led_id is None:
                return False

            # Connect adder Sum output to pc_plus_1 output LED
            if not await self.connect(adder_id, led_id, source_port_label=f"Sum[{i}]"):
                return False

        await self.log("  ✓ Created pc[0-3] and pc_plus_1[0-3] outputs")

        output_file = str(IC_COMPONENTS_DIR / "level5_program_counter_4bit.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 4-bit Program Counter IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = ProgramCounter4BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "4-bit Program Counter IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
