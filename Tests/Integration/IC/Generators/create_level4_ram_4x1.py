#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create RAM 4x1 IC (4 cells, 1-bit wide)

Inputs:
  - Address[0], Address[1]: 2-bit address
  - DataIn: 1-bit data input
  - WriteEnable: Write control signal
  - Clock: Clock signal for synchronous write

Outputs:
  - DataOut: 1-bit data output

Architecture:
  - level2_decoder_2to4 IC: Converts address to one-hot cell select
  - 4 AND gates: Write control for each cell
  - 4× (MUX + DFlipFlop): Data path and storage
  - level2_mux_4to1 IC: Read multiplexer

Usage:
    python create_level4_ram_4x1.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING


class RAM4x1Builder(ICBuilderBase):
    """Builder for RAM 4x1 IC"""

    async def create(self) -> bool:
        """Create RAM 4x1 IC"""
        address_bits = 2
        num_cells = 4

        await self.begin_build("RAM 4x1")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position hierarchy
        input_y = 50.0
        gate_spacing = 40.0
        write_control_x = 200.0
        data_mux_x = write_control_x + gate_spacing
        storage_ff_x = data_mux_x + gate_spacing
        output_x = 600.0
        output_y = 250.0

        # ========== Create Input Elements ==========
        # Address inputs
        address_inputs = []
        for i in range(address_bits):
            addr_id = await self.create_element("InputSwitch", 50.0 + i * HORIZONTAL_GATE_SPACING, input_y, f"Address[{i}]")
            if addr_id is None:
                return False
            address_inputs.append(addr_id)
            await self.log(f"  ✓ Created Address[{i}]")

        # DataIn input
        data_in_id = await self.create_element("InputSwitch", 50.0 + address_bits * HORIZONTAL_GATE_SPACING, input_y, "DataIn")
        if data_in_id is None:
            return False
        await self.log("  ✓ Created DataIn")

        # WriteEnable input
        write_en_id = await self.create_element("InputSwitch", 50.0 + (address_bits + 1) * HORIZONTAL_GATE_SPACING, input_y, "WriteEnable")
        if write_en_id is None:
            return False
        await self.log("  ✓ Created WriteEnable")

        # Clock input
        clock_id = await self.create_element("InputSwitch", 50.0 + (address_bits + 2) * HORIZONTAL_GATE_SPACING, input_y, "Clock")
        if clock_id is None:
            return False
        await self.log("  ✓ Created Clock")

        # ========== Create Write Control AND Gates ==========
        write_control_ands = []
        for i in range(num_cells):
            gate_id = await self.create_element("And", write_control_x, 100.0 + i * gate_spacing, f"write_ctrl[{i}]")
            if gate_id is None:
                return False
            write_control_ands.append(gate_id)

        await self.log(f"  ✓ Created {num_cells} write control AND gates")

        # ========== Create Storage DFlipFlops and Data MUX Gates ==========
        storage_ffs = []
        data_mux_gates = []

        for i in range(num_cells):
            # Create Mux for data path
            mux_id = await self.create_element("Mux", data_mux_x, 100.0 + i * gate_spacing, f"data_mux[{i}]")
            if mux_id is None:
                return False
            data_mux_gates.append(mux_id)

            # Create storage DFlipFlop
            ff_id = await self.create_element("DFlipFlop", storage_ff_x, 100.0 + i * gate_spacing, f"storage[{i}]")
            if ff_id is None:
                return False
            storage_ffs.append(ff_id)

        await self.log(f"  ✓ Created {num_cells} storage DFlipFlops and data MUX gates")

        # ========== Create Output LED ==========
        output_led = await self.create_element("Led", output_x, output_y, "DataOut")
        if output_led is None:
            return False
        await self.log("  ✓ Created output LED")

        # ========== Create Decoder IC Instance ==========
        decoder_ic_name = str(IC_COMPONENTS_DIR / "level2_decoder_2to4")
        if not self.check_dependency(decoder_ic_name):

            return False

        decoder_ic = await self.instantiate_ic(decoder_ic_name, 150.0, 150.0, "AddrDecoder")
        if decoder_ic is None:
            return False
        await self.log("  ✓ Loaded decoder IC")

        # Connect address inputs to decoder
        for i in range(address_bits):
            if not await self.connect(address_inputs[i], decoder_ic, target_port_label=f"addr[{i}]"):
                return False

        # ========== Create Multiplexer IC Instance ==========
        mux_ic_name = str(IC_COMPONENTS_DIR / "level2_mux_4to1")
        if not self.check_dependency(mux_ic_name):

            return False

        read_mux_ic = await self.instantiate_ic(mux_ic_name, 500.0, 250.0, "ReadMux")
        if read_mux_ic is None:
            return False
        await self.log("  ✓ Loaded multiplexer IC")

        # ========== Connect Write Control ==========
        for i in range(num_cells):
            # Decoder output for cell i enables write to cell i
            if not await self.connect(decoder_ic, write_control_ands[i], source_port_label=f"out[{i}]"):
                return False

            # WriteEnable signal AND with address decoder (write only if both asserted)
            if not await self.connect(write_en_id, write_control_ands[i], target_port=1):
                return False

        # ========== Connect Data Path ==========
        for i in range(num_cells):
            # Q (feedback from FF) -> MUX In0 (hold path: keep stored value)
            if not await self.connect(storage_ffs[i], data_mux_gates[i], target_port_label="In0"):
                return False

            # DataIn -> MUX In1 (write path: load new value)
            if not await self.connect(data_in_id, data_mux_gates[i], target_port_label="In1"):
                return False

            # Write control signal -> MUX select (0=hold, 1=write)
            if not await self.connect(write_control_ands[i], data_mux_gates[i], target_port_label="S0"):
                return False

            # MUX output -> FF Data input
            if not await self.connect(data_mux_gates[i], storage_ffs[i], target_port_label="Data"):
                return False

            # Connect Clock to DFlipFlop
            if not await self.connect(clock_id, storage_ffs[i], target_port_label="Clock"):
                return False

            # Connect DFlipFlop Q output to read multiplexer input
            if not await self.connect(storage_ffs[i], read_mux_ic, target_port_label=f"Data[{i}]", source_port_label="Q"):
                return False

        # ========== Connect Read Multiplexer ==========
        for i in range(address_bits):
            if not await self.connect(address_inputs[i], read_mux_ic, target_port_label=f"Sel[{i}]"):
                return False

        # Connect read multiplexer output to LED
        if not await self.connect(read_mux_ic, output_led, source_port_label="Output"):
            return False

        # ========== Save Circuit ==========
        output_file = str(IC_COMPONENTS_DIR / "level4_ram_4x1.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created RAM 4x1 IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = RAM4x1Builder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "RAM 4x1 IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
