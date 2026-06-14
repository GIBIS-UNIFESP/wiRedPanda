#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create RAM 8x1 IC (8 cells, 1-bit wide)

Inputs:
  - Address[0], Address[1], Address[2]: 3-bit address
  - DataIn: 1-bit data input
  - WriteEnable: Write control signal
  - Clock: Clock signal for synchronous write
  - Reset: async clear of all cells (active HIGH; F54)

Outputs:
  - DataOut: 1-bit data output

Architecture:
  - level2_decoder_3to8 IC: Converts address to one-hot cell select
  - 8 AND gates: Write control for each cell
  - 8× (MUX + DFlipFlop): Data path and storage
  - Reset -> NOT -> all cells' ~Clear (the register_1bit pattern)
  - level2_mux_8to1 IC: Read multiplexer

Usage:
    python create_level4_ram_8x1.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder


class RAM8x1Builder(ICBuilderBase):
    """Builder for RAM 8x1 IC"""

    async def create(self) -> bool:
        """Create RAM 8x1 IC"""
        address_bits = 3
        num_cells = 8

        await self.begin_build("RAM 8x1")

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
            addr_id = await self.create_element(
                "InputSwitch", 50.0 + i * HORIZONTAL_GATE_SPACING, input_y, f"Address[{i}]"
            )
            if addr_id is None:
                return False
            address_inputs.append(addr_id)
            await self.log(f"  ✓ Created Address[{i}]")

        # DataIn input
        data_in_id = await self.create_element(
            "InputSwitch", 50.0 + address_bits * HORIZONTAL_GATE_SPACING, input_y, "DataIn"
        )
        if data_in_id is None:
            return False
        await self.log("  ✓ Created DataIn")

        # WriteEnable input
        write_en_id = await self.create_element(
            "InputSwitch", 50.0 + (address_bits + 1) * HORIZONTAL_GATE_SPACING, input_y, "WriteEnable"
        )
        if write_en_id is None:
            return False
        await self.log("  ✓ Created WriteEnable")

        # Clock input
        clock_id = await self.create_element(
            "InputSwitch", 50.0 + (address_bits + 2) * HORIZONTAL_GATE_SPACING, input_y, "Clock"
        )
        if clock_id is None:
            return False
        await self.log("  ✓ Created Clock")

        # Reset input (async clear of all cells — F54)
        reset_id = await self.create_element(
            "InputSwitch", 50.0 + (address_bits + 3) * HORIZONTAL_GATE_SPACING, input_y, "Reset"
        )
        if reset_id is None:
            return False
        await self.log("  ✓ Created Reset")

        not_reset_id = await self.create_element(
            "Not", 50.0 + (address_bits + 4) * HORIZONTAL_GATE_SPACING, input_y, "NOT_Reset"
        )
        if not_reset_id is None:
            return False

        conn_ok = await self.connect(reset_id, not_reset_id)
        if not conn_ok:
            return False

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
        decoder_ic = await self.instantiate_ic("level2_decoder_3to8", 150.0, 150.0, "AddrDecoder")
        if decoder_ic is None:
            return False
        await self.log("  ✓ Loaded decoder IC")

        # Connect address inputs to decoder
        for i in range(address_bits):
            if not await self.connect(address_inputs[i], decoder_ic, target_port_label=f"addr[{i}]"):
                return False

        # Gate the write decoder with WriteEnable: a cell is selected for write
        # only while WriteEnable is high. Folds away the per-cell write_ctrl ANDs;
        # decoder.out[i] now already carries (addr==i AND WriteEnable).
        if not await self.connect(write_en_id, decoder_ic, target_port_label="Enable"):
            return False

        # ========== Create Multiplexer IC Instance ==========
        read_mux_ic = await self.instantiate_ic("level2_mux_8to1", 500.0, 250.0, "ReadMux")
        if read_mux_ic is None:
            return False
        await self.log("  ✓ Loaded multiplexer IC")

        # Reads are asynchronous: the read mux is always enabled (tie enable high).
        read_vcc_id = await self.create_element("InputVcc", 450.0, 300.0, "ReadEnable_Vcc")
        if read_vcc_id is None:
            return False
        if not await self.connect(read_vcc_id, read_mux_ic, target_port_label="Enable"):
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
            if not await self.connect(
                decoder_ic, data_mux_gates[i], source_port_label=f"out[{i}]", target_port_label="S0"
            ):
                return False

            # MUX output -> FF Data input
            if not await self.connect(data_mux_gates[i], storage_ffs[i], target_port_label="Data"):
                return False

            # Connect Clock to DFlipFlop
            if not await self.connect(clock_id, storage_ffs[i], target_port_label="Clock"):
                return False

            # Connect inverted Reset to the cell's async ~Clear (F54)
            if not await self.connect(not_reset_id, storage_ffs[i], target_port_label="~Clear"):
                return False

            # Connect DFlipFlop Q output to read multiplexer input
            if not await self.connect(
                storage_ffs[i], read_mux_ic, target_port_label=f"Data[{i}]", source_port_label="Q"
            ):
                return False

        # ========== Connect Read Multiplexer ==========
        for i in range(address_bits):
            if not await self.connect(address_inputs[i], read_mux_ic, target_port_label=f"Sel[{i}]"):
                return False

        # Connect read multiplexer output to LED
        if not await self.connect(read_mux_ic, output_led, source_port_label="Output"):
            return False

        # ========== Save Circuit ==========
        output_file = str(IC_COMPONENTS_DIR / "level4_ram_8x1.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created RAM 8x1 IC ({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = RAM8x1Builder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "RAM 8x1 IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
