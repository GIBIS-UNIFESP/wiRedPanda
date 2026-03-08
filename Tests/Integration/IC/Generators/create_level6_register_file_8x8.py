#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 8×8 Register File IC (Level 6)

Extended register file with 8 registers, each 8 bits wide.
Implements dual-read, single-write architecture using IC composition.

Inputs:
  - Write_Addr[0..2] (3-bit write address for 8 registers)
  - Read_Addr1[0..2] (3-bit read address port 1)
  - Read_Addr2[0..2] (3-bit read address port 2)
  - Data_In[0..7] (8-bit write data)
  - Write_Enable (write control signal)
  - Clock (clock signal for synchronous write)

Outputs:
  - Read_Data1[0..7] (8-bit read port 1 output)
  - Read_Data2[0..7] (8-bit read port 2 output)

Architecture (based on level5_register_file_4x4):
  - Write decoder: level2_decoder_3to8 IC (converts address to one-hot)
  - Write gates: AND gates combining decoder output with write enable
  - Hold muxes: Mux selecting between Q (hold) and Data_In (write) for each FF
  - Storage: 8×8 array of D flip-flops
  - Read muxes: Mux selecting from storage outputs based on read address

Usage:
    python3 create_level6_register_file_8x8.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class RegisterFile8x8Builder(ICBuilderBase):
    """Builder for 8×8 Register File IC (Level 6)"""

    async def create(self) -> bool:
        """Create the 8×8 Register File IC"""
        await self.begin_build('Register File 8×8')
        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Configuration
        num_registers = 8
        data_width = 8
        address_bits = 3

        # Position hierarchy
        input_x_start = 50.0
        write_addr_y = 100.0
        read_addr1_y = write_addr_y + VERTICAL_STAGE_SPACING
        read_addr2_y = read_addr1_y + VERTICAL_STAGE_SPACING
        data_in_y = read_addr2_y + VERTICAL_STAGE_SPACING
        control_y = data_in_y + VERTICAL_STAGE_SPACING

        # ========== Create Inputs ==========
        await self.log("  Creating input switches...")

        # Write address inputs
        write_addr = []
        for i in range(address_bits):
            elem_id = await self.create_element("InputSwitch", input_x_start + (i * HORIZONTAL_GATE_SPACING), write_addr_y, f"Write_Addr[{i}]")
            if elem_id is None:
                return False
            write_addr.append(elem_id)

        # Read address 1 inputs
        read_addr_1 = []
        for i in range(address_bits):
            elem_id = await self.create_element("InputSwitch", input_x_start + (i * HORIZONTAL_GATE_SPACING), read_addr1_y, f"Read_Addr1[{i}]")
            if elem_id is None:
                return False
            read_addr_1.append(elem_id)

        # Read address 2 inputs
        read_addr_2 = []
        for i in range(address_bits):
            elem_id = await self.create_element("InputSwitch", input_x_start + (i * HORIZONTAL_GATE_SPACING), read_addr2_y, f"Read_Addr2[{i}]")
            if elem_id is None:
                return False
            read_addr_2.append(elem_id)

        # Data input
        data_in = []
        for i in range(data_width):
            elem_id = await self.create_element("InputSwitch", input_x_start + (i * HORIZONTAL_GATE_SPACING * 0.5), data_in_y, f"Data_In[{i}]")
            if elem_id is None:
                return False
            data_in.append(elem_id)

        # Control signals
        write_enable = await self.create_element("InputSwitch", input_x_start, control_y, "Write_Enable")
        if write_enable is None:
            return False

        clock = await self.create_element("InputSwitch", input_x_start + HORIZONTAL_GATE_SPACING, control_y, "Clock")
        if clock is None:
            return False

        await self.log(f"  ✓ Created all inputs (3+3+3+8+2 = 19 inputs)")

        # ========== Instantiate Write Decoder IC ==========
        decoder_x = input_x_start + HORIZONTAL_GATE_SPACING * 4
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_decoder_3to8")):

            return False

        decoder_ic = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_decoder_3to8"), decoder_x, write_addr_y, "Write_Decoder")
        if decoder_ic is None:
            return False

        # Connect write address inputs to decoder
        for i in range(address_bits):
            if not await self.connect(write_addr[i], decoder_ic, target_port_label=f"addr[{i}]"):
                return False

        await self.log("  ✓ Instantiated write decoder IC (level2_decoder_3to8)")

        # ========== Create Write Gates (AND gates) ==========
        write_gates = []
        gates_x = decoder_x + HORIZONTAL_GATE_SPACING * 2
        for reg_idx in range(num_registers):
            gate_id = await self.create_element("And", gates_x + ((reg_idx % 4) * HORIZONTAL_GATE_SPACING * 0.5), write_addr_y + ((reg_idx // 4) * VERTICAL_STAGE_SPACING * 0.5), f"writeGate[{reg_idx}]")
            if gate_id is None:
                return False
            write_gates.append(gate_id)

            # Connect decoder output[reg_idx] to AND input 0 (register select)
            if not await self.connect(decoder_ic, gate_id, source_port_label=f"out[{reg_idx}]"):
                return False

            # Connect write enable to AND input 1 (write control)
            if not await self.connect(write_enable, gate_id, target_port=1):
                return False

        await self.log("  ✓ Created write gates (AND gates)")

        # ========== Create Storage Array with Hold Muxes ==========
        array_x = gates_x + HORIZONTAL_GATE_SPACING * 3
        storage = []
        hold_muxes = []

        for reg_idx in range(num_registers):
            storage_row = []
            mux_row = []

            for bit_idx in range(data_width):
                # Create D Flip-Flop for storage
                ff_id = await self.create_element("DFlipFlop", array_x + (bit_idx * HORIZONTAL_GATE_SPACING * 0.4), write_addr_y + (reg_idx * VERTICAL_STAGE_SPACING * 0.6), f"storage[{reg_idx}][{bit_idx}]")
                if ff_id is None:
                    return False
                storage_row.append(ff_id)

                # Connect clock to flip-flop Clock input
                if not await self.connect(clock, ff_id, target_port_label="Clock"):
                    return False

            storage.append(storage_row)

            # Create hold muxes for each bit (selects between Q=hold and Data_In=write)
            for bit_idx in range(data_width):
                mux_id = await self.create_element("Mux", array_x - HORIZONTAL_GATE_SPACING + (bit_idx * HORIZONTAL_GATE_SPACING * 0.4), write_addr_y + (reg_idx * VERTICAL_STAGE_SPACING * 0.6), f"holdMux[{reg_idx}][{bit_idx}]")
                if mux_id is None:
                    return False
                mux_row.append(mux_id)

                # Set mux input size to 2 (In0, In1) + 1 select (S0)
                set_props = await self.mcp.send_command("set_element_properties", {
                    "element_id": mux_id,
                    "input_size": 3
                })
                if not set_props.success:
                    self.log_error(f"Failed to set holdMux[{reg_idx}][{bit_idx}] input_size: {set_props.error}")
                    return False

                # In0: Q output (current value - for hold)
                if not await self.connect(storage[reg_idx][bit_idx], mux_id, target_port_label="In0"):
                    return False

                # In1: Data_In (new value - for write)
                if not await self.connect(data_in[bit_idx], mux_id, target_port_label="In1"):
                    return False

                # S0: Write gate (selects In1 when writing, In0 when holding)
                if not await self.connect(write_gates[reg_idx], mux_id, target_port_label="S0"):
                    return False

                # Mux output to flip-flop Data input
                if not await self.connect(mux_id, storage[reg_idx][bit_idx], target_port_label="Data"):
                    return False

            hold_muxes.append(mux_row)

        await self.log("  ✓ Created 8×8 storage array with hold muxes")

        # ========== Create Output LEDs and Read Muxes ==========
        output_x = array_x + HORIZONTAL_GATE_SPACING * 4
        read_data_1 = []
        read_data_2 = []

        for i in range(data_width):
            elem_id = await self.create_element("Led", output_x + (i * HORIZONTAL_GATE_SPACING * 0.4), read_addr1_y, f"Read_Data1[{i}]")
            if elem_id is None:
                return False
            read_data_1.append(elem_id)

            elem_id = await self.create_element("Led", output_x + (i * HORIZONTAL_GATE_SPACING * 0.4), read_addr2_y, f"Read_Data2[{i}]")
            if elem_id is None:
                return False
            read_data_2.append(elem_id)

        await self.log("  ✓ Created output LEDs")

        # ========== Create Read Multiplexers ==========
        read_mux_x = array_x + HORIZONTAL_GATE_SPACING * 2

        for bit_idx in range(data_width):
            # Read mux 1
            read_mux1_id = await self.create_element("Mux", read_mux_x, read_addr1_y + (bit_idx * VERTICAL_STAGE_SPACING * 0.3), f"readMux1[{bit_idx}]")
            if read_mux1_id is None:
                return False

            # Set input size: 8 data inputs + 3 address select bits = 11
            set_props = await self.mcp.send_command("set_element_properties", {
                "element_id": read_mux1_id,
                "input_size": num_registers + address_bits
            })
            if not set_props.success:
                self.log_error(f"Failed to set readMux1[{bit_idx}] input_size: {set_props.error}")
                return False

            # Connect storage outputs to mux data inputs
            for reg_idx in range(num_registers):
                if not await self.connect(storage[reg_idx][bit_idx], read_mux1_id, target_port_label=f"In{reg_idx}"):
                    return False

            # Connect read address to mux select inputs
            for addr_idx in range(address_bits):
                if not await self.connect(read_addr_1[addr_idx], read_mux1_id, target_port_label=f"S{addr_idx}"):
                    return False

            # Connect mux output to LED
            if not await self.connect(read_mux1_id, read_data_1[bit_idx]):
                return False

            # Read mux 2
            read_mux2_id = await self.create_element("Mux", read_mux_x + HORIZONTAL_GATE_SPACING, read_addr2_y + (bit_idx * VERTICAL_STAGE_SPACING * 0.3), f"readMux2[{bit_idx}]")
            if read_mux2_id is None:
                return False

            # Set input size: 8 data inputs + 3 address select bits = 11
            set_props = await self.mcp.send_command("set_element_properties", {
                "element_id": read_mux2_id,
                "input_size": num_registers + address_bits
            })
            if not set_props.success:
                self.log_error(f"Failed to set readMux2[{bit_idx}] input_size: {set_props.error}")
                return False

            # Connect storage outputs to mux data inputs
            for reg_idx in range(num_registers):
                if not await self.connect(storage[reg_idx][bit_idx], read_mux2_id, target_port_label=f"In{reg_idx}"):
                    return False

            # Connect read address to mux select inputs
            for addr_idx in range(address_bits):
                if not await self.connect(read_addr_2[addr_idx], read_mux2_id, target_port_label=f"S{addr_idx}"):
                    return False

            # Connect mux output to LED
            if not await self.connect(read_mux2_id, read_data_2[bit_idx]):
                return False

        await self.log("  ✓ Created read multiplexers")

        # Connect Preset/Clear pins to keep them inactive
        vcc_id = await self.create_element("InputVcc", array_x, control_y, "Vcc_PresetClear")
        if vcc_id is None:
            return False

        # Connect Vcc to all flip-flops' Preset and Clear (keep inactive)
        for reg_idx in range(num_registers):
            for bit_idx in range(data_width):
                ff_id = storage[reg_idx][bit_idx]

                # Connect Vcc to Preset (~Preset, active-LOW)
                if not await self.connect(vcc_id, ff_id, target_port_label="~Preset"):
                    return False

                # Connect Vcc to Clear (~Clear, active-LOW)
                if not await self.connect(vcc_id, ff_id, target_port_label="~Clear"):
                    return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level6_register_file_8x8.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Successfully created 8×8 Register File IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Architecture: 8 registers × 8 bits, dual-read single-write with hold muxes")
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
        exit_code = asyncio.run(run_ic_builder(build, "Register File 8×8 IC (Level 6)"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
