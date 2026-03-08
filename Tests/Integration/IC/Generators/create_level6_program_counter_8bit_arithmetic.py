#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 8-bit Program Counter IC

Inputs:
  - loadValue[0-7]: 8-bit load value
  - load: Load enable signal
  - increment: Increment enable signal
  - clock: Clock signal

Outputs:
  - pc[0-7]: 8-bit program counter value
  - pc_plus_1[0-7]: Next PC (current PC + 1)

Architecture:
  - Instantiate level6_register_8bit: 8-bit register with reset
  - Instantiate level6_adder_nbit: 8-bit PC + 1 logic
  - Add mux layer for load/increment control

Usage:
    python create_level6_program_counter_8bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class ProgramCounter8bitBuilder(ICBuilderBase):
    """Builder for 8-bit Program Counter IC (composed from register + adder)"""

    async def create(self) -> bool:
        """Create the 8-bit Program Counter IC"""
        await self.begin_build('Program Counter 8-bit')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_x = 50.0

        # Create loadValue[0-7] inputs
        load_value_inputs = []
        for i in range(8):
            lv_id = await self.create_element("InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), 100.0, f"loadValue[{i}]")
            if lv_id is None:
                return False
            load_value_inputs.append(lv_id)
        await self.log("  ✓ Created 8 loadValue inputs")

        # Control inputs: load, inc, reset, clock (all lowercase)
        ctrl_x = input_x + (8 * HORIZONTAL_GATE_SPACING)

        load_id = await self.create_element("InputSwitch", ctrl_x, 100.0, "load")
        if load_id is None:
            return False

        inc_id = await self.create_element("InputSwitch", ctrl_x, 100.0 + VERTICAL_STAGE_SPACING, "inc")
        if inc_id is None:
            return False

        reset_id = await self.create_element("InputSwitch", ctrl_x, 100.0 + (2 * VERTICAL_STAGE_SPACING), "reset")
        if reset_id is None:
            return False

        clk_id = await self.create_element("InputSwitch", ctrl_x, 100.0 + (3 * VERTICAL_STAGE_SPACING), "clock")
        if clk_id is None:
            return False

        await self.log("  ✓ Created load, inc, reset, clock control inputs")

        # Instantiate 8-bit Register IC
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level6_register_8bit")):

            return False

        register_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level6_register_8bit"), ctrl_x + HORIZONTAL_GATE_SPACING, 100.0, "Register8bit")
        if register_id is None:
            return False
        await self.log("  ✓ Instantiated 8-bit Register IC")

        # Instantiate 8-bit Adder IC (for PC + 1)
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level6_ripple_adder_8bit")):

            return False

        adder_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level6_ripple_adder_8bit"), ctrl_x + (2 * HORIZONTAL_GATE_SPACING), 100.0, "Adder8bit")
        if adder_id is None:
            return False
        await self.log("  ✓ Instantiated 8-bit Adder IC")

        # Connect adder A inputs to register outputs (PC current value)
        for i in range(8):
            if not await self.connect(register_id, adder_id, source_port_label=f"Q[{i}]", target_port_label=f"A[{i}]"):
                return False

        # Connect adder B inputs to constant 1 (for PC+1)
        # B[0] should be 1, B[1-7] should be 0
        # Create using InputVcc (constant high/1) element

        vcc_id = await self.create_element("InputVcc", ctrl_x + (3 * HORIZONTAL_GATE_SPACING), 50.0 + (2 * VERTICAL_STAGE_SPACING), "VCC_Const1")
        if vcc_id is None:
            return False

        # Connect InputVcc to adder B[0] (constant 1)
        if not await self.connect(vcc_id, adder_id, target_port_label="B[0]"):
            return False

        # B[1-7] are left unconnected (default to 0, which is correct)
        await self.log("  ✓ Connected adder B[0] = InputVcc (constant 1)")
        await self.log("  ✓ Setting up cascaded mux logic for load/inc/hold priority")

        # Mux1: Select between hold (PC) and increment (PC+1) based on inc AND NOT(load)
        # Mux2: Select between Mux1 output and loadValue based on load

        # Create NOT(load) gate for control logic
        not_load_id = await self.create_element("Not", ctrl_x + (3.5 * HORIZONTAL_GATE_SPACING), 350.0, "NotLoad")
        if not_load_id is None:
            return False

        # Create AND(inc, NOT(load)) gate
        and_inc_id = await self.create_element("And", ctrl_x + (3.5 * HORIZONTAL_GATE_SPACING), 350.0 + VERTICAL_STAGE_SPACING, "AndIncNotLoad")
        if and_inc_id is None:
            return False

        # Connect control logic
        if not await self.connect(load_id, not_load_id):
            return False

        # Connect inc to AND port 0
        if not await self.connect(inc_id, and_inc_id, target_port=0):
            return False

        # Connect NOT(load) to AND port 1
        if not await self.connect(not_load_id, and_inc_id, target_port=1):
            return False

        # Create 8-bit Mux1 cascade: Select between hold (PC) and increment (PC+1)
        # Controlled by AND(inc, NOT load)
        mux1_gates = []
        for i in range(8):
            mux1_id = await self.create_element("Mux", ctrl_x + (3.7 * HORIZONTAL_GATE_SPACING), 200.0 + i * VERTICAL_STAGE_SPACING, f"Mux1[{i}]")
            if mux1_id is None:
                return False
            mux1_gates.append(mux1_id)

            # Port 0: Hold (PC) from register Q
            if not await self.connect(register_id, mux1_id, source_port_label=f"Q[{i}]", target_port_label="In0"):
                return False

            # Port 1: Increment (PC+1) from adder Sum
            if not await self.connect(adder_id, mux1_id, source_port_label=f"Sum[{i}]", target_port_label="In1"):
                return False

            # Port 2: Select - AND(inc, NOT load)
            if not await self.connect(and_inc_id, mux1_id, target_port_label="S0"):
                return False

        await self.log("  ✓ Created Mux1 layer (hold vs increment)")

        # Create 8-bit Mux2 cascade: Select between Mux1 output and loadValue
        # Controlled by load
        mux2_gates = []
        for i in range(8):
            mux2_id = await self.create_element("Mux", ctrl_x + (3.9 * HORIZONTAL_GATE_SPACING), 200.0 + i * VERTICAL_STAGE_SPACING, f"Mux2[{i}]")
            if mux2_id is None:
                return False
            mux2_gates.append(mux2_id)

            # Port 0: Mux1 output (hold or increment result)
            if not await self.connect(mux1_gates[i], mux2_id, target_port_label="In0"):
                return False

            # Port 1: loadValue
            if not await self.connect(load_value_inputs[i], mux2_id, target_port_label="In1"):
                return False

            # Port 2: Select - load
            if not await self.connect(load_id, mux2_id, target_port_label="S0"):
                return False

        await self.log("  ✓ Created Mux2 layer (load selection with priority)")

        # Connect Mux2 outputs to register Data inputs
        for i in range(8):
            if not await self.connect(mux2_gates[i], register_id, target_port_label=f"Data[{i}]"):
                return False

        await self.log("  ✓ Connected mux2 outputs to register inputs")

        # Connect clock to register
        if not await self.connect(clk_id, register_id, target_port_label="Clock"):
            return False

        # Connect reset to register Reset
        if not await self.connect(reset_id, register_id, target_port_label="Reset"):
            return False

        # WriteEnable: OR(load, inc) - write on clock edge when either load or inc is asserted
        we_or_id = await self.create_element("Or", ctrl_x + (3.2 * HORIZONTAL_GATE_SPACING), 50.0 + (3 * VERTICAL_STAGE_SPACING), "WriteEnable_OR")
        if we_or_id is None:
            return False

        # Connect inputs to WriteEnable OR gate
        # load to OR port 0, inc to OR port 1
        if not await self.connect(load_id, we_or_id, target_port=0):
            return False

        if not await self.connect(inc_id, we_or_id, target_port=1):
            return False

        # Connect WriteEnable OR output to register
        if not await self.connect(we_or_id, register_id, target_port_label="WriteEnable"):
            return False

        await self.log("  ✓ Connected clock, reset, and WriteEnable (OR(load, inc)) to register")

        # Create output LEDs for pc[0-7] (note: lowercase for test compatibility)
        output_x = ctrl_x + (4 * HORIZONTAL_GATE_SPACING)
        for i in range(8):
            led_id = await self.create_element("Led", output_x, 100.0 + i * VERTICAL_STAGE_SPACING, f"pc[{i}]")
            if led_id is None:
                return False

            # Connect register output to pc output LED
            if not await self.connect(register_id, led_id, source_port_label=f"Q[{i}]"):
                return False

        await self.log("  ✓ Created pc[7:0] outputs")

        # Create output LEDs for pc_plus_1 (adder Sum output = PC + 1)
        pc_plus_1_x = output_x + HORIZONTAL_GATE_SPACING
        for i in range(8):
            pc_plus_1_led = await self.create_element("Led", pc_plus_1_x, 100.0 + i * VERTICAL_STAGE_SPACING, f"pc_plus_1[{i}]")
            if pc_plus_1_led is None:
                return False

            # Connect adder Sum output to pc_plus_1 LED
            if not await self.connect(adder_id, pc_plus_1_led, source_port_label=f"Sum[{i}]"):
                return False

        await self.log("  ✓ Created 8 pc_plus_1 outputs (PC + 1)")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level6_program_counter_8bit_arithmetic.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 8-bit Program Counter IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        await self.log(f"   Outputs: pc[0-7], pc_plus_1[0-7]")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = ProgramCounter8bitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "8-bit Program Counter IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
