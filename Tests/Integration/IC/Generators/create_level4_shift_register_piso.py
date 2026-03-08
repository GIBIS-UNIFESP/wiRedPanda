#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Parallel-In Serial-Out (PISO) Shift Register IC

Inputs: CLK (Clock), LOAD (Load Control), D[0:3] (Parallel Load Data)
Outputs: SOUT (Serial Output)

Circuit:
- 4 D flip-flops forming a shift register
- Multiplexer logic to select between parallel load and shift modes
- AND/OR gates to implement load/shift path selection
- NOT gate to invert load control signal

PISO Shift Register behavior:
- When LOAD=1: Parallel data loaded into all FFs on clock rising edge
- When LOAD=0: Register shifts right, MSB feeds in from external source (not connected),
  LSB (FF[0]) shifts out as serial output
- Shift chain: FF[3] → FF[2] → FF[1] → FF[0] → serialOut

Implementation:
- 4 D Flip-Flops (DFlipFlop)
- 4 AND gates (load path: LOAD AND D[i])
- 4 AND gates (shift path: NOT LOAD AND FF[i+1])
- 4 OR gates (multiplex: loadGate OR shiftGate)
- 1 NOT gate (invert LOAD signal)

Usage:
    python create_shift_register_piso.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING


class ShiftRegisterPISOBuilder(ICBuilderBase):
    """Builder for Parallel-In Serial-Out Shift Register IC"""

    async def create(self) -> bool:
        """Create the PISO Shift Register IC"""
        await self.begin_build("PISO Shift Register")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position hierarchy
        input_x = 50.0
        input_y_clk = 50.0
        input_y_load = 100.0
        input_y_data_base = 150.0
        not_load_x = input_x + HORIZONTAL_GATE_SPACING
        not_load_y = input_y_load
        load_gate_x = input_x + (2 * HORIZONTAL_GATE_SPACING)
        load_gate_y_base = input_y_load
        shift_gate_x = input_x + (2 * HORIZONTAL_GATE_SPACING)
        shift_gate_y_base = input_y_load + 30.0
        select_gate_x = input_x + (3 * HORIZONTAL_GATE_SPACING)
        select_gate_y_base = input_y_load + 15.0
        dff_x = input_x + (4 * HORIZONTAL_GATE_SPACING)
        dff_y_base = input_y_load
        sout_x = input_x + (5 * HORIZONTAL_GATE_SPACING)
        sout_y = input_y_load
        bit_spacing = 100.0
        data_input_spacing = 40.0

        # Create input controls
        clk_id = await self.create_element("InputSwitch", input_x, input_y_clk, "CLK")
        if clk_id is None:
            return False
        await self.log(f"  ✓ Created input CLK (id={clk_id})")

        load_id = await self.create_element("InputSwitch", input_x, input_y_load, "LOAD")
        if load_id is None:
            return False
        await self.log(f"  ✓ Created input LOAD (id={load_id})")

        # Create data inputs (4-bit)
        data_in_ids = []
        for i in range(4):
            d_id = await self.create_element("InputSwitch", input_x, input_y_data_base + (i * data_input_spacing), f"D{i}")
            if d_id is None:
                return False
            data_in_ids.append(d_id)
            await self.log(f"  ✓ Created input D{i} (id={d_id})")

        # Create NOT gate for load inversion
        not_load_id = await self.create_element("Not", not_load_x, not_load_y, "not_load")
        if not_load_id is None:
            return False

        # Create D flip-flops (4-bit) using level1_d_flip_flop IC
        dff_ids = []
        for i in range(4):
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level1_d_flip_flop")):

                return False

            ff_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level1_d_flip_flop"), dff_x, dff_y_base + (i * bit_spacing), f"FF{i}")
            if ff_id is None:
                return False
            dff_ids.append(ff_id)
            await self.log(f"  ✓ Instantiated level1_d_flip_flop {i} (id={ff_id})")

        # Instantiate load path Bus Mux 4-bit IC
        # BusMux(GND, D[0-3], LOAD) = LOAD AND D[0-3]
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit")):

            return False

        load_mux_ic_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit"), load_gate_x, load_gate_y_base, "BusMux_Load")
        if load_mux_ic_id is None:
            return False
        await self.log(f"  ✓ Instantiated load path Bus Multiplexer IC (id={load_mux_ic_id})")

        # Instantiate shift path Bus Mux 4-bit IC
        # BusMux(GND, shiftIn[0-3], NOT_LOAD) = NOT_LOAD AND shiftIn[0-3]
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit")):

            return False

        shift_mux_ic_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_bus_mux_4bit"), shift_gate_x, shift_gate_y_base, "BusMux_Shift")
        if shift_mux_ic_id is None:
            return False
        await self.log(f"  ✓ Instantiated shift path Bus Multiplexer IC (id={shift_mux_ic_id})")

        # Instantiate select/mux Mux2to1 IC (4 times for 4 bits)
        # Mux(load_gate[i], shift_gate[i], select_bit) - but we'll use OR instead for clarity
        # Actually, for load/shift selection, use a Mux2to1: Mux(shift, load, LOAD) = (NOT LOAD AND shift) OR (LOAD AND load)
        select_gate_ids = []
        for i in range(4):
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_mux_2to1")):

                return False

            mux_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_mux_2to1"), select_gate_x, select_gate_y_base + (i * bit_spacing), f"mux_sel{i}")
            if mux_id is None:
                return False
            select_gate_ids.append(mux_id)

        # Create serial output LED
        sout_id = await self.create_element("Led", sout_x, sout_y, "SOUT")
        if sout_id is None:
            return False
        await self.log(f"  ✓ Created serial output SOUT (id={sout_id})")

        # Create InputGnd for constant 0 (needed for Mux port 0)
        gnd_id = await self.create_element("InputGnd", load_gate_x - HORIZONTAL_GATE_SPACING, 250.0, "GND")
        if gnd_id is None:
            return False
        await self.log(f"  ✓ Created InputGnd constant (id={gnd_id})")

        # ========== Connect NOT gate for load inversion ==========
        if not await self.connect(load_id, not_load_id):
            return False

        # ========== Connect clock to all FFs ==========
        for i in range(4):
            if not await self.connect(clk_id, dff_ids[i], target_port_label="Clock"):
                return False

        # ========== Connect load path Bus Mux ==========
        # BusMux(GND, D[0-3], LOAD) = LOAD AND D[0-3]
        for i in range(4):
            # Connect In0[i] to GND (constant 0)
            if not await self.connect(gnd_id, load_mux_ic_id, target_port_label=f"In0[{i}]"):
                return False

            # Connect In1[i] to D[i]
            if not await self.connect(data_in_ids[i], load_mux_ic_id, target_port_label=f"In1[{i}]"):
                return False

        # Connect Sel to LOAD (select signal)
        if not await self.connect(load_id, load_mux_ic_id, target_port_label="Sel"):
            return False

        # ========== Connect shift path Bus Mux ==========
        # BusMux(GND, shiftIn[0-3], NOT_LOAD) = NOT_LOAD AND shiftIn[0-3]
        for i in range(4):
            # Connect In0[i] to GND (constant 0)
            if not await self.connect(gnd_id, shift_mux_ic_id, target_port_label=f"In0[{i}]"):
                return False

            # Connect In1[i] to shiftIn[i] (Q[i+1] for i<3, otherwise unconnected/0)
            if i < 3:
                if not await self.connect(dff_ids[i + 1], shift_mux_ic_id, source_port_label="Q", target_port_label=f"In1[{i}]"):
                    return False

        # Connect Sel to NOT_LOAD (select signal)
        if not await self.connect(not_load_id, shift_mux_ic_id, target_port_label="Sel"):
            return False

        # ========== Connect select Mux gates ==========
        for i in range(4):
            # Mux(shift_result, load_result, LOAD) = (NOT LOAD AND shift) OR (LOAD AND load)
            # Port 0: shift_result (NOT_LOAD AND shiftIn[i]) from shift BusMux Out[i]
            if not await self.connect(shift_mux_ic_id, select_gate_ids[i], source_port_label=f"Out[{i}]"):
                return False

            # Port 1: load_result (LOAD AND D[i]) from load BusMux Out[i]
            if not await self.connect(load_mux_ic_id, select_gate_ids[i], source_port_label=f"Out[{i}]", target_port_label="Data[1]"):
                return False

            # Port 2: LOAD (select signal) - selects between shift (port 0) and load (port 1)
            if not await self.connect(load_id, select_gate_ids[i], target_port_label="Sel[0]"):
                return False

            # Connect select gate output to FF D input
            if not await self.connect(select_gate_ids[i], dff_ids[i], target_port_label="D"):
                return False

        # Create Vcc element for inactive Preset/Clear pins
        vcc_id = await self.create_element("InputVcc", dff_x - HORIZONTAL_GATE_SPACING, dff_y_base + 4 * bit_spacing, "Vcc")
        if vcc_id is None:
            return False
        await self.log(f"  ✓ Created Vcc element")

        # ========== Connect Vcc to all Preset/Clear pins ==========
        for i in range(4):
            # Connect Vcc to Preset (active-LOW, so HIGH keeps it inactive)
            if not await self.connect(vcc_id, dff_ids[i], target_port_label="Preset"):
                return False

            # Connect Vcc to Clear (active-LOW, so HIGH keeps it inactive)
            if not await self.connect(vcc_id, dff_ids[i], target_port_label="Clear"):
                return False

        # ========== Connect serial output ==========
        if not await self.connect(dff_ids[0], sout_id, source_port_label="Q"):
            return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level4_shift_register_piso.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created PISO Shift Register IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = ShiftRegisterPISOBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "PISO Shift Register IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
