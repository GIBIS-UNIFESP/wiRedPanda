#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create Register File 8x8 IC (8 registers, 8 bits wide)

Inputs:
  - Write_Addr[0..2]: Write address (3-bit)
  - Read_Addr1[0..2]: Read port 1 address (3-bit)
  - Read_Addr2[0..2]: Read port 2 address (3-bit)
  - Data_In[0..7]: Write data (8-bit)
  - Write_Enable: Write control signal
  - Clock: Clock signal for synchronous write

Outputs:
  - Read_Data1[0..7]: Read port 1 data (8-bit)
  - Read_Data2[0..7]: Read port 2 data (8-bit)

Architecture:
  - Write decoder: level2_decoder_3to8 IC
  - Write gates: 8 AND gates (decoder output × write enable)
  - Storage: 8×8 array of D flip-flops with hold muxes
  - Read muxes: 2×8 multiplexers (one per bit per read port)

Usage:
    python create_level5_register_file_8x8.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class RegisterFile8x8Builder(ICBuilderBase):
    """Builder for Register File 8x8 IC"""

    async def create(self) -> bool:
        """Create Register File 8x8 IC"""
        num_registers = 8
        data_width = 8
        address_bits = 3

        await self.begin_build("Register File 8×8")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Position hierarchy for logical stages
        input_x_start = 50.0
        write_addr_y = 100.0
        read_addr1_y = write_addr_y + VERTICAL_STAGE_SPACING
        read_addr2_y = read_addr1_y + VERTICAL_STAGE_SPACING
        data_in_y = read_addr2_y + VERTICAL_STAGE_SPACING
        control_y = data_in_y + VERTICAL_STAGE_SPACING

        decoder_x = input_x_start + HORIZONTAL_GATE_SPACING
        write_gates_x = decoder_x + HORIZONTAL_GATE_SPACING
        storage_x = write_gates_x + HORIZONTAL_GATE_SPACING
        read_mux_x = storage_x + HORIZONTAL_GATE_SPACING
        output_x = read_mux_x + HORIZONTAL_GATE_SPACING

        dense_spacing = 60.0

        # ========== Create Input Switches ==========
        # Write address
        write_addr = []
        for i in range(address_bits):
            elem_id = await self.create_element("InputSwitch", input_x_start + i * dense_spacing, write_addr_y, f"Write_Addr[{i}]")
            if elem_id is None:
                return False
            write_addr.append(elem_id)

        # Read address 1
        read_addr_1 = []
        for i in range(address_bits):
            elem_id = await self.create_element("InputSwitch", input_x_start + i * dense_spacing, read_addr1_y, f"Read_Addr1[{i}]")
            if elem_id is None:
                return False
            read_addr_1.append(elem_id)

        # Read address 2
        read_addr_2 = []
        for i in range(address_bits):
            elem_id = await self.create_element("InputSwitch", input_x_start + i * dense_spacing, read_addr2_y, f"Read_Addr2[{i}]")
            if elem_id is None:
                return False
            read_addr_2.append(elem_id)

        # Data input
        data_in = []
        for i in range(data_width):
            elem_id = await self.create_element("InputSwitch", input_x_start + i * dense_spacing, data_in_y, f"Data_In[{i}]")
            if elem_id is None:
                return False
            data_in.append(elem_id)

        # Write enable
        write_enable = await self.create_element("InputSwitch", input_x_start, control_y, "Write_Enable")
        if write_enable is None:
            return False

        # Clock
        clock = await self.create_element("InputSwitch", input_x_start + dense_spacing, control_y, "Clock")
        if clock is None:
            return False

        await self.log("  ✓ Created input switches")

        # ========== Create Output LEDs ==========
        read_data_1 = []
        for i in range(data_width):
            elem_id = await self.create_element("Led", output_x + i * dense_spacing, read_addr1_y, f"Read_Data1[{i}]")
            if elem_id is None:
                return False
            read_data_1.append(elem_id)

        read_data_2 = []
        for i in range(data_width):
            elem_id = await self.create_element("Led", output_x + i * dense_spacing, read_addr2_y, f"Read_Data2[{i}]")
            if elem_id is None:
                return False
            read_data_2.append(elem_id)

        await self.log("  ✓ Created output LEDs")

        # ========== Instantiate Write Decoder IC ==========
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_decoder_3to8")):

            return False

        decoder_ic = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_decoder_3to8"), decoder_x, write_addr_y, "Write_Decoder")
        if decoder_ic is None:
            return False

        for i in range(address_bits):
            if not await self.connect(write_addr[i], decoder_ic, target_port_label=f"addr[{i}]"):
                return False

        await self.log("  ✓ Instantiated write decoder IC")

        # ========== Create Write Gates ==========
        write_gates = []
        for reg_idx in range(num_registers):
            gate_id = await self.create_element("And", write_gates_x + (reg_idx % 2) * HORIZONTAL_GATE_SPACING * 0.7, write_addr_y + (reg_idx // 2) * VERTICAL_STAGE_SPACING * 0.6, f"writeGate[{reg_idx}]")
            if gate_id is None:
                return False
            write_gates.append(gate_id)

            # Connect decoder output[reg_idx] to AND input 0
            if not await self.connect(decoder_ic, gate_id, source_port_label=f"out[{reg_idx}]"):
                return False

            # Connect write_enable to AND input 1
            if not await self.connect(write_enable, gate_id, target_port=1):
                return False

        await self.log("  ✓ Created write gates")

        # ========== Create Storage Array ==========
        array_spacing = 50.0
        storage = []
        hold_muxes = []

        for reg_idx in range(num_registers):
            storage_row = []
            mux_row = []

            for bit_idx in range(data_width):
                ff_id = await self.create_element("DFlipFlop", storage_x + bit_idx * array_spacing, write_addr_y + reg_idx * VERTICAL_STAGE_SPACING * 0.7, f"storage[{reg_idx}][{bit_idx}]")
                if ff_id is None:
                    return False
                storage_row.append(ff_id)

                # Connect clock to flip-flop Clock input
                if not await self.connect(clock, ff_id, target_port_label="Clock"):
                    return False

            storage.append(storage_row)

            for bit_idx in range(data_width):
                mux_id = await self.create_element("Mux", storage_x - HORIZONTAL_GATE_SPACING + bit_idx * array_spacing, write_addr_y + reg_idx * VERTICAL_STAGE_SPACING * 0.7, f"holdMux[{reg_idx}][{bit_idx}]")
                if mux_id is None:
                    return False
                mux_row.append(mux_id)

                set_props = await self.mcp.send_command("change_input_size", {
                    "element_id": mux_id,
                    "size": 3
                })
                if not set_props.success:
                    self.log_error(f"Failed to set holdMux[{reg_idx}][{bit_idx}] input_size")
                    return False

                # Hold path: storage[reg] Q output to Mux In0
                if not await self.connect(storage[reg_idx][bit_idx], mux_id, target_port_label="In0"):
                    return False

                # Load path: data_in[bit] to Mux In1
                if not await self.connect(data_in[bit_idx], mux_id, target_port_label="In1"):
                    return False

                # Write control: write_gate to Mux select (0=hold, 1=load)
                if not await self.connect(write_gates[reg_idx], mux_id, target_port_label="S0"):
                    return False

                # Mux output to flip-flop Data input
                if not await self.connect(mux_id, storage[reg_idx][bit_idx], target_port_label="Data"):
                    return False

            hold_muxes.append(mux_row)

        await self.log("  ✓ Created storage array and hold muxes")

        # ========== Create Read Multiplexers ==========
        read_mux_spacing = 50.0

        for bit_idx in range(data_width):
            # Read multiplexer 1
            read_mux1_id = await self.create_element("Mux", read_mux_x, read_addr1_y + bit_idx * read_mux_spacing, f"readMux1[{bit_idx}]")
            if read_mux1_id is None:
                return False

            set_props = await self.mcp.send_command("change_input_size", {
                "element_id": read_mux1_id,
                "size": num_registers + address_bits
            })
            if not set_props.success:
                self.log_error(f"Failed to set readMux1[{bit_idx}] input_size")
                return False

            for reg_idx in range(num_registers):
                if not await self.connect(storage[reg_idx][bit_idx], read_mux1_id, target_port_label=f"In{reg_idx}"):
                    return False

            for addr_idx in range(address_bits):
                if not await self.connect(read_addr_1[addr_idx], read_mux1_id, target_port_label=f"S{addr_idx}"):
                    return False

            if not await self.connect(read_mux1_id, read_data_1[bit_idx]):
                return False

            # Read multiplexer 2
            read_mux2_id = await self.create_element("Mux", read_mux_x + HORIZONTAL_GATE_SPACING, read_addr1_y + bit_idx * read_mux_spacing, f"readMux2[{bit_idx}]")
            if read_mux2_id is None:
                return False

            set_props = await self.mcp.send_command("change_input_size", {
                "element_id": read_mux2_id,
                "size": num_registers + address_bits
            })
            if not set_props.success:
                self.log_error(f"Failed to set readMux2[{bit_idx}] input_size")
                return False

            for reg_idx in range(num_registers):
                if not await self.connect(storage[reg_idx][bit_idx], read_mux2_id, target_port_label=f"In{reg_idx}"):
                    return False

            for addr_idx in range(address_bits):
                if not await self.connect(read_addr_2[addr_idx], read_mux2_id, target_port_label=f"S{addr_idx}"):
                    return False

            if not await self.connect(read_mux2_id, read_data_2[bit_idx]):
                return False

        await self.log("  ✓ Created read multiplexers")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level5_register_file_8x8.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created RegisterFile 8×8 IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = RegisterFile8x8Builder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Register File 8×8 IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
