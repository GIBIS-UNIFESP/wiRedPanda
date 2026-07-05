#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create RAM 4x1 IC (4 cells, 1-bit wide)

Inputs:
  - Address[0], Address[1]: 2-bit address
  - DataIn: 1-bit data input
  - WriteEnable: Write control signal
  - Reset: async clear of all cells (active-high; matches level4_ram_8x1 F54)
  - Clock: Clock signal for synchronous write

Outputs:
  - DataOut: 1-bit data output

Architecture:
  - level2_decoder_2to4 IC: Converts address to one-hot cell select
  - 4 AND gates: Write control for each cell
  - 4× (MUX + DFlipFlop): Data path and storage
  - level2_mux_4to1 IC: Read multiplexer
  - Reset NOT gate: drives every cell's active-low async ~Clear

Usage:
    python create_level4_ram_4x1.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder

# The 4x1 and 8x1 RAMs are the same cell-array architecture at two sizes; their
# builders share the address/data/write/reset wiring scaffold, differing only in
# address_bits/num_cells. The duplication is intentional (each generator stays a
# self-contained, readable script) rather than hoisted into a shared base method.
# pylint: disable=duplicate-code


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

        # WriteEnable, Clock and Reset are created further below, once the storage
        # array's real vertical extent is known (see "Create Control Inputs") -- they
        # fan out to every cell, so they're placed nearer the array's centre instead of
        # this top row, and that placement needs the array's post-clearance geometry.

        # ========== Create Decoder IC Instance ==========
        # Instantiated early (with its real measured size) so the write-control
        # grid below can clear the decoder's actual footprint -- an embedded IC's
        # width isn't a flat stock-element size (see instantiate_ic_with_size).
        decoder_ic_name = "level2_decoder_2to4"
        decoder_x = 150.0
        grid_start_y = input_y + VERTICAL_STAGE_SPACING
        decoder_handle = await self.instantiate_ic_with_size(decoder_ic_name, decoder_x, grid_start_y, "AddrDecoder")
        if decoder_handle is None:
            return False
        decoder_ic = decoder_handle.element_id
        await self.log("  ✓ Loaded decoder IC")

        # A decoder fans its output ports out symmetrically around its own vertical
        # centre (GraphicElement::updatePortsProperties); with enough outputs (one per
        # storage cell), the real bounding box measured above reaches above the decoder's
        # own placement position far enough to clip the Address[] input labels sitting
        # right above it. React to the *measured* height (not a guessed port-layout
        # formula) by nudging the decoder down to clear it, and carrying the row grid
        # down with it so everything anchored to grid_start_y stays aligned to the decoder.
        decoder_overshoot = max(0.0, (decoder_handle.height - 64.0) / 2.0)
        if decoder_overshoot > 0.0:
            corrected_grid_start_y = grid_start_y + decoder_overshoot
            move_response = await self.mcp.send_command(
                "move_element", {"element_id": decoder_ic, "x": decoder_x, "y": corrected_grid_start_y}
            )
            if not move_response.success:
                self.log_error("reposition AddrDecoder for label clearance")
                return False
            new_position = move_response.result.get("new_position", {}) if move_response.result else {}
            grid_start_y = new_position.get("y", corrected_grid_start_y)
            await self.log(f"  ✓ Repositioned AddrDecoder down {decoder_overshoot:.1f}px for label clearance")

        # Connect address inputs to decoder
        for i in range(address_bits):
            if not await self.connect(address_inputs[i], decoder_ic, target_port_label=f"addr[{i}]"):
                return False

        # Write-control/data-mux/storage grid must clear the decoder's real
        # width instead of a flat spacing constant.
        write_control_x = decoder_x + max(HORIZONTAL_GATE_SPACING, decoder_handle.width)
        data_mux_x = write_control_x + HORIZONTAL_GATE_SPACING
        # "data_mux[N]" reaches into "storage[N]" at a standard 1x step on
        # platforms that render the label a bit wider than the default Linux
        # font (observed on Windows CI), so this column gets extra clearance.
        storage_ff_x = data_mux_x + HORIZONTAL_GATE_SPACING + 32

        # ========== Create Control Inputs (WriteEnable, Clock, Reset) ==========
        # WriteEnable/Clock/Reset each fan out to every one of the num_cells rows below
        # (write_ctrl[]/storage[] all the way down the column), so parking them on the
        # top input row -- as far as possible from most of the rows they drive -- forces
        # long diagonal wires to every cell but the first. Stack them instead in a
        # dedicated column left of the decoder (below the decoder/Address[] row, so this
        # doesn't reopen the label-clearance issue above), centred on the array's own
        # vertical midpoint so the average wire to every row is shorter.
        #
        # This column must clear the decoder's own footprint (which starts at
        # decoder_x and can extend a few px left of it -- see decoder_handle above), so
        # it stays flush with the input row's leftmost column (Address[0]/DataIn's x=50)
        # rather than decoder_x.
        control_x = 50.0
        array_span = (num_cells - 1) * VERTICAL_STAGE_SPACING
        array_center_y = grid_start_y + (array_span / 2.0) + 32.0
        control_block_top = array_center_y - 1.5 * VERTICAL_STAGE_SPACING

        # WriteEnable input
        write_en_id = await self.create_element("InputSwitch", control_x, control_block_top, "WriteEnable")
        if write_en_id is None:
            return False
        await self.log("  ✓ Created WriteEnable")

        # Clock input
        clock_id = await self.create_element(
            "InputSwitch", control_x, control_block_top + VERTICAL_STAGE_SPACING, "Clock"
        )
        if clock_id is None:
            return False
        await self.log("  ✓ Created Clock")

        # Reset input (async clear of all cells — matches level4_ram_8x1 F54).
        # Active-HIGH at the boundary; inverted to drive each cell's active-low ~Clear.
        reset_id = await self.create_element(
            "InputSwitch", control_x, control_block_top + 2 * VERTICAL_STAGE_SPACING, "Reset"
        )
        if reset_id is None:
            return False
        await self.log("  ✓ Created Reset")

        not_reset_id = await self.create_element(
            "Not", control_x, control_block_top + 3 * VERTICAL_STAGE_SPACING, "NOT_Reset"
        )
        if not_reset_id is None:
            return False
        if not await self.connect(reset_id, not_reset_id):
            return False

        # ========== Create Write Control AND Gates ==========
        write_control_ands = []
        for i in range(num_cells):
            gate_id = await self.create_element(
                "And", write_control_x, grid_start_y + i * VERTICAL_STAGE_SPACING, f"write_ctrl[{i}]"
            )
            if gate_id is None:
                return False
            write_control_ands.append(gate_id)

        await self.log(f"  ✓ Created {num_cells} write control AND gates")

        # ========== Create Storage DFlipFlops and Data MUX Gates ==========
        storage_ffs = []
        data_mux_gates = []

        for i in range(num_cells):
            # Create Mux for data path
            mux_id = await self.create_element(
                "Mux", data_mux_x, grid_start_y + i * VERTICAL_STAGE_SPACING, f"data_mux[{i}]"
            )
            if mux_id is None:
                return False
            data_mux_gates.append(mux_id)

            # Create storage DFlipFlop
            ff_id = await self.create_element(
                "DFlipFlop", storage_ff_x, grid_start_y + i * VERTICAL_STAGE_SPACING, f"storage[{i}]"
            )
            if ff_id is None:
                return False
            storage_ffs.append(ff_id)

        await self.log(f"  ✓ Created {num_cells} storage DFlipFlops and data MUX gates")

        # ========== Create Multiplexer IC Instance ==========
        # Placed (and sized) relative to storage_ff_x -- the storage column's
        # real width grew with the decoder above, so a flat 500.0 constant is no
        # longer guaranteed to clear it.
        mux_ic_name = "level2_mux_4to1"
        mux_x = storage_ff_x + HORIZONTAL_GATE_SPACING
        mux_handle = await self.instantiate_ic_with_size(mux_ic_name, mux_x, 250.0, "ReadMux")
        if mux_handle is None:
            return False
        read_mux_ic = mux_handle.element_id
        await self.log("  ✓ Loaded multiplexer IC")

        # ========== Create Output LED ==========
        output_x = mux_x + max(HORIZONTAL_GATE_SPACING, mux_handle.width)
        output_led = await self.create_element("Led", output_x, output_y, "DataOut")
        if output_led is None:
            return False
        await self.log("  ✓ Created output LED")

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

            # Connect inverted Reset to the cell's async ~Clear (Reset=1 -> ~Clear=0 -> cell cleared)
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
        output_file = str(IC_COMPONENTS_DIR / "level4_ram_4x1.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created RAM 4x1 IC ({self.element_count} elements, {self.connection_count} connections)"
        )
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
