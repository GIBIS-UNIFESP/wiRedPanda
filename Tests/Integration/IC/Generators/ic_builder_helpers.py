#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Shared parametrised IC builder classes.

Each class replaces a set of near-identical generator files; the entry-point
files become thin wrappers that pass different constructor arguments.
"""

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase


class DecoderBuilder(ICBuilderBase):
    """N-to-2^N binary decoder. Covers decoder_2to4, decoder_3to8, decoder_4to16."""

    def __init__(self, mcp, address_bits: int, verbose: bool = True) -> None:
        super().__init__(mcp, verbose)
        self.address_bits = address_bits
        self.num_outputs = 2**address_bits

    async def create(self) -> bool:
        """Create a binary decoder IC."""
        width = self.address_bits
        num_outputs = self.num_outputs
        display = f"Decoder {width}-to-{num_outputs}"
        output_name = f"level2_decoder_{width}to{num_outputs}"

        await self.begin_build(display)
        if not await self.create_new_circuit():
            return False

        input_x = 50.0
        not_x = input_x + HORIZONTAL_GATE_SPACING
        and_x = not_x + HORIZONTAL_GATE_SPACING
        output_x = and_x + HORIZONTAL_GATE_SPACING

        addr_inputs = []
        for i in range(width):
            addr_id = await self.create_element(
                "InputSwitch", input_x, 100.0 + i * VERTICAL_STAGE_SPACING, f"addr[{i}]"
            )
            if addr_id is None:
                return False
            addr_inputs.append(addr_id)
            await self.log(f"  ✓ Created addr[{i}]")

        not_gates = []
        for i in range(width):
            not_id = await self.create_element("Not", not_x, 100.0 + i * VERTICAL_STAGE_SPACING, f"not_addr[{i}]")
            if not_id is None:
                return False
            not_gates.append(not_id)
            if not await self.connect(addr_inputs[i], not_id):
                return False

        await self.log(f"  ✓ Created {width} NOT gates")

        output_leds = []
        and_gates = []
        for i in range(num_outputs):
            gate_id = await self.create_element(
                "And",
                and_x + (i % 4) * HORIZONTAL_GATE_SPACING,
                100.0 + (i // 4) * VERTICAL_STAGE_SPACING,
                f"and[{i}]",
            )
            if gate_id is None:
                return False
            and_gates.append(gate_id)

            if width > 2:
                set_props = await self.mcp.send_command("change_input_size", {"element_id": gate_id, "size": width})
                if not set_props.success:
                    self.log_error(f"Failed to set input_size={width} for AND gate {i}")
                    return False

            for bit_pos in range(width):
                source_id = addr_inputs[bit_pos] if i & 1 << bit_pos != 0 else not_gates[bit_pos]
                if not await self.connect(source_id, gate_id, target_port=bit_pos):
                    return False

            led_id = await self.create_element(
                "Led",
                output_x + (i % 4) * HORIZONTAL_GATE_SPACING,
                100.0 + (i // 4) * VERTICAL_STAGE_SPACING,
                f"out[{i}]",
            )
            if led_id is None:
                return False
            output_leds.append(led_id)
            if not await self.connect(gate_id, led_id):
                return False

        await self.log(f"  ✓ Created {num_outputs} AND gates with {num_outputs} output LEDs")

        output_file = str(IC_COMPONENTS_DIR / f"{output_name}.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created {display} IC({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


class RamBuilder(ICBuilderBase):
    """1-bit-wide RAM with N cells. Covers ram_4x1 (address_bits=2) and ram_8x1 (3)."""

    def __init__(self, mcp, address_bits: int, verbose: bool = True) -> None:
        super().__init__(mcp, verbose)
        self.address_bits = address_bits
        self.num_cells = 2**address_bits

    async def create(self) -> bool:
        """Create a RAM IC."""
        address_bits = self.address_bits
        num_cells = self.num_cells
        display = f"RAM {num_cells}x1"
        output_name = f"level4_ram_{num_cells}x1"
        decoder_name = f"level2_decoder_{address_bits}to{num_cells}"
        mux_name = f"level2_mux_{num_cells}to1"

        await self.begin_build(display)
        if not await self.create_new_circuit():
            return False

        input_y = 50.0
        gate_spacing = 40.0
        write_control_x = 200.0
        data_mux_x = write_control_x + gate_spacing
        storage_ff_x = data_mux_x + gate_spacing
        output_x = 600.0
        output_y = 250.0

        address_inputs = []
        for i in range(address_bits):
            addr_id = await self.create_element(
                "InputSwitch", 50.0 + i * HORIZONTAL_GATE_SPACING, input_y, f"Address[{i}]"
            )
            if addr_id is None:
                return False
            address_inputs.append(addr_id)
            await self.log(f"  ✓ Created Address[{i}]")

        data_in_id = await self.create_element(
            "InputSwitch", 50.0 + address_bits * HORIZONTAL_GATE_SPACING, input_y, "DataIn"
        )
        if data_in_id is None:
            return False
        await self.log("  ✓ Created DataIn")

        write_en_id = await self.create_element(
            "InputSwitch", 50.0 + (address_bits + 1) * HORIZONTAL_GATE_SPACING, input_y, "WriteEnable"
        )
        if write_en_id is None:
            return False
        await self.log("  ✓ Created WriteEnable")

        clock_id = await self.create_element(
            "InputSwitch", 50.0 + (address_bits + 2) * HORIZONTAL_GATE_SPACING, input_y, "Clock"
        )
        if clock_id is None:
            return False
        await self.log("  ✓ Created Clock")

        write_control_ands = []
        for i in range(num_cells):
            gate_id = await self.create_element("And", write_control_x, 100.0 + i * gate_spacing, f"write_ctrl[{i}]")
            if gate_id is None:
                return False
            write_control_ands.append(gate_id)
        await self.log(f"  ✓ Created {num_cells} write control AND gates")

        storage_ffs = []
        data_mux_gates = []
        for i in range(num_cells):
            mux_id = await self.create_element("Mux", data_mux_x, 100.0 + i * gate_spacing, f"data_mux[{i}]")
            if mux_id is None:
                return False
            data_mux_gates.append(mux_id)

            ff_id = await self.create_element("DFlipFlop", storage_ff_x, 100.0 + i * gate_spacing, f"storage[{i}]")
            if ff_id is None:
                return False
            storage_ffs.append(ff_id)
        await self.log(f"  ✓ Created {num_cells} storage DFlipFlops and data MUX gates")

        output_led = await self.create_element("Led", output_x, output_y, "DataOut")
        if output_led is None:
            return False
        await self.log("  ✓ Created output LED")

        decoder_ic = await self.instantiate_ic(decoder_name, 150.0, 150.0, "AddrDecoder")
        if decoder_ic is None:
            return False
        await self.log("  ✓ Loaded decoder IC")

        for i in range(address_bits):
            if not await self.connect(address_inputs[i], decoder_ic, target_port_label=f"addr[{i}]"):
                return False

        read_mux_ic = await self.instantiate_ic(mux_name, 500.0, 250.0, "ReadMux")
        if read_mux_ic is None:
            return False
        await self.log("  ✓ Loaded multiplexer IC")

        for i in range(num_cells):
            if not await self.connect(decoder_ic, write_control_ands[i], source_port_label=f"out[{i}]"):
                return False
            if not await self.connect(write_en_id, write_control_ands[i], target_port=1):
                return False

        for i in range(num_cells):
            if not await self.connect(storage_ffs[i], data_mux_gates[i], target_port_label="In0"):
                return False
            if not await self.connect(data_in_id, data_mux_gates[i], target_port_label="In1"):
                return False
            if not await self.connect(write_control_ands[i], data_mux_gates[i], target_port_label="S0"):
                return False
            if not await self.connect(data_mux_gates[i], storage_ffs[i], target_port_label="Data"):
                return False
            if not await self.connect(clock_id, storage_ffs[i], target_port_label="Clock"):
                return False
            if not await self.connect(
                storage_ffs[i], read_mux_ic, target_port_label=f"Data[{i}]", source_port_label="Q"
            ):
                return False

        for i in range(address_bits):
            if not await self.connect(address_inputs[i], read_mux_ic, target_port_label=f"Sel[{i}]"):
                return False
        if not await self.connect(read_mux_ic, output_led, source_port_label="Output"):
            return False

        output_file = str(IC_COMPONENTS_DIR / f"{output_name}.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created {display} IC({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


class RegisterFileBuilder(ICBuilderBase):
    """Register file with N registers of W bits. Covers register_file_4x4 and 8x8."""

    def __init__(
        self, mcp, num_registers: int, data_width: int, address_bits: int, level: int = 5, verbose: bool = True
    ) -> None:
        super().__init__(mcp, verbose)
        self.num_registers = num_registers
        self.data_width = data_width
        self.address_bits = address_bits
        self.level = level

    async def create(self) -> bool:
        """Create a register file IC."""
        num_registers = self.num_registers
        data_width = self.data_width
        address_bits = self.address_bits
        display = f"Register File {num_registers}×{data_width}"
        # F32: the level-5 and level-6 8×8 register-file fixtures are the same
        # circuit — only the output filename's level prefix differs.
        output_name = f"level{self.level}_register_file_{num_registers}x{data_width}"
        decoder_name = f"level2_decoder_{address_bits}to{num_registers}"

        await self.begin_build(display)
        if not await self.create_new_circuit():
            return False

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

        write_addr = []
        for i in range(address_bits):
            elem_id = await self.create_element(
                "InputSwitch", input_x_start + i * dense_spacing, write_addr_y, f"Write_Addr[{i}]"
            )
            if elem_id is None:
                return False
            write_addr.append(elem_id)

        read_addr_1 = []
        for i in range(address_bits):
            elem_id = await self.create_element(
                "InputSwitch", input_x_start + i * dense_spacing, read_addr1_y, f"Read_Addr1[{i}]"
            )
            if elem_id is None:
                return False
            read_addr_1.append(elem_id)

        read_addr_2 = []
        for i in range(address_bits):
            elem_id = await self.create_element(
                "InputSwitch", input_x_start + i * dense_spacing, read_addr2_y, f"Read_Addr2[{i}]"
            )
            if elem_id is None:
                return False
            read_addr_2.append(elem_id)

        data_in = []
        for i in range(data_width):
            elem_id = await self.create_element(
                "InputSwitch", input_x_start + i * dense_spacing, data_in_y, f"Data_In[{i}]"
            )
            if elem_id is None:
                return False
            data_in.append(elem_id)

        write_enable = await self.create_element("InputSwitch", input_x_start, control_y, "Write_Enable")
        if write_enable is None:
            return False

        clock = await self.create_element("InputSwitch", input_x_start + dense_spacing, control_y, "Clock")
        if clock is None:
            return False

        await self.log("  ✓ Created input switches")

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

        decoder_ic = await self.instantiate_ic(decoder_name, decoder_x, write_addr_y, "Write_Decoder")
        if decoder_ic is None:
            return False

        for i in range(address_bits):
            if not await self.connect(write_addr[i], decoder_ic, target_port_label=f"addr[{i}]"):
                return False

        await self.log("  ✓ Instantiated write decoder IC")

        write_gates = []
        for reg_idx in range(num_registers):
            gate_id = await self.create_element(
                "And",
                write_gates_x + (reg_idx % 2) * HORIZONTAL_GATE_SPACING * 0.7,
                write_addr_y + (reg_idx // 2) * VERTICAL_STAGE_SPACING * 0.6,
                f"writeGate[{reg_idx}]",
            )
            if gate_id is None:
                return False
            write_gates.append(gate_id)
            if not await self.connect(decoder_ic, gate_id, source_port_label=f"out[{reg_idx}]"):
                return False
            if not await self.connect(write_enable, gate_id, target_port=1):
                return False

        await self.log("  ✓ Created write gates")

        array_spacing = 50.0
        storage = []
        hold_muxes = []

        for reg_idx in range(num_registers):
            storage_row = []
            mux_row = []

            for bit_idx in range(data_width):
                ff_id = await self.create_element(
                    "DFlipFlop",
                    storage_x + bit_idx * array_spacing,
                    write_addr_y + reg_idx * VERTICAL_STAGE_SPACING * 0.7,
                    f"storage[{reg_idx}][{bit_idx}]",
                )
                if ff_id is None:
                    return False
                storage_row.append(ff_id)
                if not await self.connect(clock, ff_id, target_port_label="Clock"):
                    return False

            storage.append(storage_row)

            for bit_idx in range(data_width):
                mux_id = await self.create_element(
                    "Mux",
                    storage_x - HORIZONTAL_GATE_SPACING + bit_idx * array_spacing,
                    write_addr_y + reg_idx * VERTICAL_STAGE_SPACING * 0.7,
                    f"holdMux[{reg_idx}][{bit_idx}]",
                )
                if mux_id is None:
                    return False
                mux_row.append(mux_id)

                set_props = await self.mcp.send_command("change_input_size", {"element_id": mux_id, "size": 3})
                if not set_props.success:
                    self.log_error(f"Failed to set holdMux[{reg_idx}][{bit_idx}] input_size")
                    return False

                if not await self.connect(storage[reg_idx][bit_idx], mux_id, target_port_label="In0"):
                    return False
                if not await self.connect(data_in[bit_idx], mux_id, target_port_label="In1"):
                    return False
                if not await self.connect(write_gates[reg_idx], mux_id, target_port_label="S0"):
                    return False
                if not await self.connect(mux_id, storage[reg_idx][bit_idx], target_port_label="Data"):
                    return False

            hold_muxes.append(mux_row)

        await self.log("  ✓ Created storage array and hold muxes")

        read_mux_spacing = 50.0
        for bit_idx in range(data_width):
            read_mux1_id = await self.create_element(
                "Mux", read_mux_x, read_addr1_y + bit_idx * read_mux_spacing, f"readMux1[{bit_idx}]"
            )
            if read_mux1_id is None:
                return False
            set_props = await self.mcp.send_command(
                "change_input_size",
                {"element_id": read_mux1_id, "size": num_registers + address_bits},
            )
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

            read_mux2_id = await self.create_element(
                "Mux",
                read_mux_x + HORIZONTAL_GATE_SPACING,
                read_addr1_y + bit_idx * read_mux_spacing,
                f"readMux2[{bit_idx}]",
            )
            if read_mux2_id is None:
                return False
            set_props = await self.mcp.send_command(
                "change_input_size",
                {"element_id": read_mux2_id, "size": num_registers + address_bits},
            )
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

        output_file = str(IC_COMPONENTS_DIR / f"{output_name}.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created {display} IC({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


class BarrelShiftBuilder(ICBuilderBase):
    """4-bit barrel shifter/rotator. is_rotator=True → wrap-around; False → zero-fill."""

    def __init__(self, mcp, is_rotator: bool, verbose: bool = True) -> None:
        super().__init__(mcp, verbose)
        self.is_rotator = is_rotator

    async def create(self) -> bool:
        """Create the barrel shifter or rotator IC."""
        is_rotator = self.is_rotator
        if is_rotator:
            display = "Barrel Rotator"
            output_name = "level5_barrel_rotator"
            amt0_label, amt1_label, dir_label = "Rotate_Amount[0]", "Rotate_Amount[1]", "Direction"
            out_label = "Result"
        else:
            display = "Barrel Shifter 4-bit"
            output_name = "level5_barrel_shifter_4bit"
            amt0_label, amt1_label, dir_label = "ShiftAmount[0]", "ShiftAmount[1]", "ShiftDirection"
            out_label = "ShiftedData"

        await self.begin_build(display)
        if not await self.create_new_circuit():
            return False

        input_x_start = 50.0
        data_input_y = 100.0
        ctrl_x = input_x_start + HORIZONTAL_GATE_SPACING * 2.5

        left_stage1_y = 200.0
        left_stage2_y = left_stage1_y + VERTICAL_STAGE_SPACING
        right_stage1_y = left_stage2_y + VERTICAL_STAGE_SPACING
        right_stage2_y = right_stage1_y + VERTICAL_STAGE_SPACING
        direction_y = 300.0
        output_x = input_x_start + HORIZONTAL_GATE_SPACING * 3
        dense_spacing = 40.0

        data_inputs = []
        for i in range(4):
            data_id = await self.create_element(
                "InputSwitch", input_x_start + i * dense_spacing, data_input_y, f"Data[{i}]"
            )
            if data_id is None:
                return False
            data_inputs.append(data_id)

        amt0_id = await self.create_element("InputSwitch", ctrl_x, data_input_y, amt0_label)
        if amt0_id is None:
            return False

        amt1_id = await self.create_element("InputSwitch", ctrl_x, data_input_y + VERTICAL_STAGE_SPACING, amt1_label)
        if amt1_id is None:
            return False

        if is_rotator:
            dir_id = await self.create_element(
                "InputSwitch", ctrl_x, data_input_y + VERTICAL_STAGE_SPACING * 2, dir_label
            )
            if dir_id is None:
                return False
            gnd: int = 0  # ground not used in rotator mode
        else:
            dir_id = await self.create_element(
                "InputSwitch", ctrl_x, data_input_y + VERTICAL_STAGE_SPACING * 1.5, dir_label
            )
            if dir_id is None:
                return False
            ground_elem = await self.create_element(
                "InputGnd", input_x_start, data_input_y + VERTICAL_STAGE_SPACING * 1.5, "Ground"
            )
            if ground_elem is None:
                return False
            gnd = ground_elem

        bus_mux = "level4_bus_mux_4bit"
        left_s1 = await self.instantiate_ic(
            bus_mux, input_x_start + HORIZONTAL_GATE_SPACING, left_stage1_y, "BusMux_Left_S1"
        )
        if left_s1 is None:
            return False

        left_s2 = await self.instantiate_ic(
            bus_mux, input_x_start + HORIZONTAL_GATE_SPACING, left_stage2_y, "BusMux_Left_S2"
        )
        if left_s2 is None:
            return False

        right_s1 = await self.instantiate_ic(
            bus_mux, input_x_start + HORIZONTAL_GATE_SPACING, right_stage1_y, "BusMux_Right_S1"
        )
        if right_s1 is None:
            return False

        right_s2 = await self.instantiate_ic(
            bus_mux, input_x_start + HORIZONTAL_GATE_SPACING, right_stage2_y, "BusMux_Right_S2"
        )
        if right_s2 is None:
            return False

        dir_mux = await self.instantiate_ic(bus_mux, output_x, direction_y, "BusMux_Direction")
        if dir_mux is None:
            return False

        output_leds = []
        for i in range(4):
            led_id = await self.create_element(
                "Led",
                output_x + HORIZONTAL_GATE_SPACING + i * dense_spacing,
                direction_y,
                f"{out_label}[{i}]",
            )
            if led_id is None:
                return False
            output_leds.append(led_id)

        # Left Stage 1: In0 = pass-through; In1 = rotated/shifted by 1
        for i in range(4):
            if not await self.connect(data_inputs[i], left_s1, target_port_label=f"In0[{i}]"):
                return False
            if is_rotator:
                src = (i - 1 + 4) % 4
                if not await self.connect(data_inputs[src], left_s1, target_port_label=f"In1[{i}]"):
                    return False
            else:
                src_node = data_inputs[i - 1] if i > 0 else gnd
                if not await self.connect(src_node, left_s1, target_port_label=f"In1[{i}]"):
                    return False
        if not await self.connect(amt0_id, left_s1, target_port_label="Sel"):
            return False

        # Left Stage 2: In0 = stage1 pass-through; In1 = rotated/shifted by 2
        for i in range(4):
            if not await self.connect(left_s1, left_s2, source_port_label=f"Out[{i}]", target_port_label=f"In0[{i}]"):
                return False
            if is_rotator:
                src = (i - 2 + 4) % 4
                if not await self.connect(
                    left_s1, left_s2, source_port_label=f"Out[{src}]", target_port_label=f"In1[{i}]"
                ):
                    return False
            else:
                if i >= 2:
                    if not await self.connect(
                        left_s1, left_s2, source_port_label=f"Out[{i - 2}]", target_port_label=f"In1[{i}]"
                    ):
                        return False
                else:
                    if not await self.connect(gnd, left_s2, target_port_label=f"In1[{i}]"):
                        return False
        if not await self.connect(amt1_id, left_s2, target_port_label="Sel"):
            return False

        # Right Stage 1: In0 = pass-through; In1 = rotated/shifted by 1 right
        for i in range(4):
            if not await self.connect(data_inputs[i], right_s1, target_port_label=f"In0[{i}]"):
                return False
            if is_rotator:
                src = (i + 1) % 4
                if not await self.connect(data_inputs[src], right_s1, target_port_label=f"In1[{i}]"):
                    return False
            else:
                src_node = data_inputs[i + 1] if i < 3 else gnd
                if not await self.connect(src_node, right_s1, target_port_label=f"In1[{i}]"):
                    return False
        if not await self.connect(amt0_id, right_s1, target_port_label="Sel"):
            return False

        # Right Stage 2: In0 = stage1 pass-through; In1 = rotated/shifted by 2 right
        for i in range(4):
            if not await self.connect(right_s1, right_s2, source_port_label=f"Out[{i}]", target_port_label=f"In0[{i}]"):
                return False
            if is_rotator:
                src = (i + 2) % 4
                if not await self.connect(
                    right_s1, right_s2, source_port_label=f"Out[{src}]", target_port_label=f"In1[{i}]"
                ):
                    return False
            else:
                if i < 2:
                    if not await self.connect(
                        right_s1, right_s2, source_port_label=f"Out[{i + 2}]", target_port_label=f"In1[{i}]"
                    ):
                        return False
                else:
                    if not await self.connect(gnd, right_s2, target_port_label=f"In1[{i}]"):
                        return False
        if not await self.connect(amt1_id, right_s2, target_port_label="Sel"):
            return False

        # Direction selector: In0=left, In1=right, Sel=Direction/ShiftDirection
        for i in range(4):
            if not await self.connect(left_s2, dir_mux, source_port_label=f"Out[{i}]", target_port_label=f"In0[{i}]"):
                return False
            if not await self.connect(right_s2, dir_mux, source_port_label=f"Out[{i}]", target_port_label=f"In1[{i}]"):
                return False
        if not await self.connect(dir_id, dir_mux, target_port_label="Sel"):
            return False
        for i in range(4):
            if not await self.connect(dir_mux, output_leds[i], source_port_label=f"Out[{i}]"):
                return False

        output_file = str(IC_COMPONENTS_DIR / f"{output_name}.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created {display} IC({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


class CounterBuilder(ICBuilderBase):
    """4-bit counter. mode: 'loadable' | 'modulo'.

    The plain binary counter is no longer built here: the 74161-style binary
    counter (CountEnable/Load/Clear + parallel Data) diverged far enough from
    the loadable/modulo pattern that it lives in its own standalone
    BinaryCounter4BitBuilder (create_level4_binary_counter_4bit.py).
    """

    def __init__(self, mcp, mode: str, verbose: bool = True) -> None:
        super().__init__(mcp, verbose)
        self.mode = mode

    async def create(self) -> bool:
        """Create a 4-bit counter IC."""
        mode = self.mode
        if mode == "loadable":
            display = "4-bit Loadable Counter"
            output_name = "level5_loadable_counter_4bit"
            out_fmt = "Q[{i}]"
        elif mode == "modulo":
            display = "4-bit Modulo-N Counter"
            output_name = "level5_modulo_counter_4bit"
            out_fmt = "Q[{i}]"
        else:
            raise ValueError(f"Unknown counter mode: {mode!r}")

        await self.begin_build(display)
        if not await self.create_new_circuit():
            return False

        input_x = 50.0

        clk_id = await self.create_element("InputSwitch", input_x, 100.0, "CLK")
        if clk_id is None:
            return False

        load_id = None
        data_ids = []
        modulo_ids = []

        if mode == "loadable":
            load_id = await self.create_element("InputSwitch", input_x, 100.0 + VERTICAL_STAGE_SPACING, "Load")
            if load_id is None:
                return False
            for i in range(4):
                d_id = await self.create_element(
                    "InputSwitch", input_x, 100.0 + ((i + 2) * VERTICAL_STAGE_SPACING), f"D[{i}]"
                )
                if d_id is None:
                    return False
                data_ids.append(d_id)
            vcc_id = await self.create_element("InputVcc", input_x, 100.0 + (6 * VERTICAL_STAGE_SPACING), "Vcc")
        elif mode == "modulo":
            for i in range(4):
                m_id = await self.create_element(
                    "InputSwitch", input_x, 100.0 + ((i + 1) * VERTICAL_STAGE_SPACING), f"Modulo[{i}]"
                )
                if m_id is None:
                    return False
                modulo_ids.append(m_id)
            vcc_id = await self.create_element("InputVcc", input_x, 100.0 + (5 * VERTICAL_STAGE_SPACING), "Vcc")
        else:
            vcc_id = await self.create_element("InputVcc", input_x, 140.0, "Vcc")

        if vcc_id is None:
            return False

        not_x = input_x + HORIZONTAL_GATE_SPACING
        not_q_ids = []
        for i in range(4):
            not_id = await self.create_element("Not", not_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"not_q{i}")
            if not_id is None:
                return False
            not_q_ids.append(not_id)

        carry_x = not_x + HORIZONTAL_GATE_SPACING
        carry_and_ids = []
        for i in range(2):
            and_id = await self.create_element("And", carry_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"and_carry{i}")
            if and_id is None:
                return False
            carry_and_ids.append(and_id)

        not_carry_x = carry_x + HORIZONTAL_GATE_SPACING
        not_carry_ids = []
        for i in range(3):
            not_id = await self.create_element(
                "Not", not_carry_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"not_carry{i}"
            )
            if not_id is None:
                return False
            not_carry_ids.append(not_id)

        xor_and_x = not_carry_x + HORIZONTAL_GATE_SPACING
        xor_and_ids = []
        for i in range(4):
            bit_ands = []
            for j in range(2):
                and_id = await self.create_element(
                    "And",
                    xor_and_x + (j * HORIZONTAL_GATE_SPACING),
                    100.0 + (i * VERTICAL_STAGE_SPACING),
                    f"and_xor{i}_{j}",
                )
                if and_id is None:
                    return False
                bit_ands.append(and_id)
            xor_and_ids.append(bit_ands)

        xor_or_x = xor_and_x + (2 * HORIZONTAL_GATE_SPACING)
        xor_or_ids = []
        for i in range(4):
            or_id = await self.create_element("Or", xor_or_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"or_xor{i}")
            if or_id is None:
                return False
            xor_or_ids.append(or_id)

        mux_ids = []
        comparator_id = None
        not_equal_id = None
        gate_and_ids = []

        if mode == "loadable":
            mux_x = xor_or_x + HORIZONTAL_GATE_SPACING
            for i in range(4):
                mux_id = await self.create_element("Mux", mux_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"load_mux{i}")
                if mux_id is None:
                    return False
                mux_ids.append(mux_id)
            dff_x = mux_x + HORIZONTAL_GATE_SPACING
        elif mode == "modulo":
            comparator_x = xor_or_x + HORIZONTAL_GATE_SPACING
            comp_name = "level3_comparator_4bit_equality"
            comparator_id = await self.instantiate_ic(comp_name, comparator_x, 100.0, "Comparator")
            if comparator_id is None:
                return False
            not_equal_id = await self.create_element("Not", comparator_x + HORIZONTAL_GATE_SPACING, 100.0, "not_equal")
            if not_equal_id is None:
                return False
            gate_and_x = comparator_x + (2 * HORIZONTAL_GATE_SPACING)
            for i in range(4):
                and_id = await self.create_element(
                    "And", gate_and_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"gate_and{i}"
                )
                if and_id is None:
                    return False
                gate_and_ids.append(and_id)
            dff_x = gate_and_x + HORIZONTAL_GATE_SPACING
        else:
            dff_x = xor_or_x + HORIZONTAL_GATE_SPACING

        dff_ids = []
        for i in range(4):
            dff_dep = "level1_d_flip_flop"
            ff_id = await self.instantiate_ic(dff_dep, dff_x, 100.0 + (i * VERTICAL_STAGE_SPACING), f"FF{i}")
            if ff_id is None:
                return False
            dff_ids.append(ff_id)

        output_x = dff_x + HORIZONTAL_GATE_SPACING
        output_led_ids = []
        for i in range(4):
            label = out_fmt.format(i=i) if "{i}" in out_fmt else f"{out_fmt}{i}"
            led_id = await self.create_element("Led", output_x, 100.0 + (i * VERTICAL_STAGE_SPACING), label)
            if led_id is None:
                return False
            output_led_ids.append(led_id)

        overflow_id = None
        if mode == "modulo":
            overflow_id = await self.create_element("Led", output_x, 100.0 + (4 * VERTICAL_STAGE_SPACING), "Overflow")
            if overflow_id is None:
                return False

        # === WIRING ===

        for i in range(4):
            if not await self.connect(clk_id, dff_ids[i], target_port_label="Clock"):
                return False

        for i in range(4):
            if not await self.connect(vcc_id, dff_ids[i], target_port_label="Preset"):
                return False
            if not await self.connect(vcc_id, dff_ids[i], target_port_label="Clear"):
                return False

        for i in range(4):
            if not await self.connect(dff_ids[i], output_led_ids[i], source_port_label="Q"):
                return False
            if not await self.connect(dff_ids[i], not_q_ids[i], source_port_label="Q"):
                return False

        if mode == "binary":
            # Bit 0: NOT(Q0) → FF.D directly
            if not await self.connect(not_q_ids[0], dff_ids[0], target_port_label="D"):
                return False
        else:
            # Bit 0: route through XOR AND/OR for consistency with multi-stage path
            if not await self.connect(not_q_ids[0], xor_and_ids[0][0]):
                return False
            if not await self.connect(vcc_id, xor_and_ids[0][0], target_port=1):
                return False
            if not await self.connect(dff_ids[0], xor_and_ids[0][1], source_port_label="Q"):
                return False
            if not await self.connect(not_carry_ids[0], xor_and_ids[0][1], target_port=1):
                return False
            if not await self.connect(xor_and_ids[0][0], xor_or_ids[0]):
                return False
            if not await self.connect(xor_and_ids[0][1], xor_or_ids[0], target_port=1):
                return False

        # Carry chain
        if not await self.connect(dff_ids[0], carry_and_ids[0], source_port_label="Q"):
            return False
        if not await self.connect(dff_ids[1], carry_and_ids[0], source_port_label="Q", target_port=1):
            return False
        if not await self.connect(carry_and_ids[0], carry_and_ids[1]):
            return False
        if not await self.connect(dff_ids[2], carry_and_ids[1], source_port_label="Q", target_port=1):
            return False

        # NOT carry
        if not await self.connect(dff_ids[0], not_carry_ids[0], source_port_label="Q"):
            return False
        if not await self.connect(carry_and_ids[0], not_carry_ids[1]):
            return False
        if not await self.connect(carry_and_ids[1], not_carry_ids[2]):
            return False

        # XOR bits 1, 2, 3 (same for all modes)
        xor_sources = [
            (not_q_ids[1], dff_ids[0], dff_ids[1], not_carry_ids[0]),
            (not_q_ids[2], carry_and_ids[0], dff_ids[2], not_carry_ids[1]),
            (not_q_ids[3], carry_and_ids[1], dff_ids[3], not_carry_ids[2]),
        ]
        for k, (not_q, carry_src, q_src, not_carry) in enumerate(xor_sources):
            bit = k + 1
            if not await self.connect(not_q, xor_and_ids[bit][0]):
                return False
            # bit 1: carry_src is dff_ids[0], needs source_port_label="Q"
            if bit == 1:
                if not await self.connect(dff_ids[0], xor_and_ids[bit][0], source_port_label="Q", target_port=1):
                    return False
            else:
                if not await self.connect(carry_src, xor_and_ids[bit][0], target_port=1):
                    return False
            if not await self.connect(q_src, xor_and_ids[bit][1], source_port_label="Q"):
                return False
            if not await self.connect(not_carry, xor_and_ids[bit][1], target_port=1):
                return False
            if not await self.connect(xor_and_ids[bit][0], xor_or_ids[bit]):
                return False
            if not await self.connect(xor_and_ids[bit][1], xor_or_ids[bit], target_port=1):
                return False

        # Final path from xor_or to FF.D (mode-specific)
        if mode == "binary":
            for i in range(1, 4):
                if not await self.connect(xor_or_ids[i], dff_ids[i], target_port_label="D"):
                    return False
        elif mode == "loadable":
            assert load_id is not None
            for i in range(4):
                if not await self.connect(xor_or_ids[i], mux_ids[i], target_port_label="In0"):
                    return False
                if not await self.connect(data_ids[i], mux_ids[i], target_port_label="In1"):
                    return False
                if not await self.connect(load_id, mux_ids[i], target_port_label="S0"):
                    return False
                if not await self.connect(mux_ids[i], dff_ids[i], target_port_label="D"):
                    return False
        elif mode == "modulo":
            assert comparator_id is not None
            assert not_equal_id is not None
            assert overflow_id is not None
            for i in range(4):
                if not await self.connect(xor_or_ids[i], comparator_id, target_port_label=f"A[{i}]"):
                    return False
                if not await self.connect(modulo_ids[i], comparator_id, target_port_label=f"B[{i}]"):
                    return False
            if not await self.connect(comparator_id, not_equal_id, source_port_label="Equal"):
                return False
            if not await self.connect(comparator_id, overflow_id, source_port_label="Equal"):
                return False
            for i in range(4):
                if not await self.connect(xor_or_ids[i], gate_and_ids[i]):
                    return False
                if not await self.connect(not_equal_id, gate_and_ids[i], target_port=1):
                    return False
                if not await self.connect(gate_and_ids[i], dff_ids[i], target_port_label="D"):
                    return False

        output_file = str(IC_COMPONENTS_DIR / f"{output_name}.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created {display} IC({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


class ComparatorBuilder(ICBuilderBase):
    """4-bit magnitude comparator. level=3 or 4 (output filename only differs)."""

    def __init__(self, mcp, level: int, verbose: bool = True) -> None:
        super().__init__(mcp, verbose)
        self.level = level

    async def create(self) -> bool:
        """Create the 4-bit Comparator IC."""
        output_name = f"level{self.level}_comparator_4bit"

        await self.begin_build("Comparator 4-bit")
        if not await self.create_new_circuit():
            return False

        input_x = 50.0
        not_x = input_x + HORIZONTAL_GATE_SPACING
        and_x = not_x + HORIZONTAL_GATE_SPACING
        cascade_x = and_x + HORIZONTAL_GATE_SPACING
        or_x = cascade_x + HORIZONTAL_GATE_SPACING
        final_x = or_x + HORIZONTAL_GATE_SPACING
        output_x = final_x + HORIZONTAL_GATE_SPACING

        a_inputs = []
        for i in range(4):
            a_id = await self.create_element("InputSwitch", input_x, 100.0 + i * VERTICAL_STAGE_SPACING, f"A[{i}]")
            if a_id is None:
                return False
            a_inputs.append(a_id)

        b_inputs = []
        for i in range(4):
            b_id = await self.create_element(
                "InputSwitch", input_x, 100.0 + (4 + i) * VERTICAL_STAGE_SPACING, f"B[{i}]"
            )
            if b_id is None:
                return False
            b_inputs.append(b_id)

        await self.log("  ✓ Created inputs (A[0-3], B[0-3])")

        # 74LS85 cascade inputs: tie EqualIn high and GreaterIn/LessIn low for
        # standalone use; for a wider comparison feed the less-significant
        # comparator's outputs here.
        gt_in = await self.create_element("InputSwitch", input_x, 100.0 + 8 * VERTICAL_STAGE_SPACING, "GreaterIn")
        if gt_in is None:
            return False
        eq_in = await self.create_element("InputSwitch", input_x, 100.0 + 9 * VERTICAL_STAGE_SPACING, "EqualIn")
        if eq_in is None:
            return False
        lt_in = await self.create_element("InputSwitch", input_x, 100.0 + 10 * VERTICAL_STAGE_SPACING, "LessIn")
        if lt_in is None:
            return False
        await self.log("  ✓ Created cascade inputs (GreaterIn, EqualIn, LessIn)")

        not_a = []
        not_b = []
        for i in range(4):
            not_a_id = await self.create_element("Not", not_x, 100.0 + i * VERTICAL_STAGE_SPACING, f"notA[{i}]")
            if not_a_id is None:
                return False
            not_a.append(not_a_id)
            if not await self.connect(a_inputs[i], not_a_id):
                return False

            not_b_id = await self.create_element("Not", not_x, 100.0 + (4 + i) * VERTICAL_STAGE_SPACING, f"notB[{i}]")
            if not_b_id is None:
                return False
            not_b.append(not_b_id)
            if not await self.connect(b_inputs[i], not_b_id):
                return False

        await self.log("  ✓ Created NOT gates")

        xnor_gates = []
        and_greater = []
        and_less = []
        for i in range(4):
            xnor_id = await self.create_element(
                "Xnor",
                and_x + i * HORIZONTAL_GATE_SPACING,
                100.0 + i * VERTICAL_STAGE_SPACING,
                f"xnor[{i}]",
            )
            if xnor_id is None:
                return False
            xnor_gates.append(xnor_id)
            if not await self.connect(a_inputs[i], xnor_id):
                return False
            if not await self.connect(b_inputs[i], xnor_id, target_port=1):
                return False

            and_g_id = await self.create_element(
                "And",
                and_x + i * HORIZONTAL_GATE_SPACING,
                100.0 + (4 + i) * VERTICAL_STAGE_SPACING,
                f"andGreater[{i}]",
            )
            if and_g_id is None:
                return False
            and_greater.append(and_g_id)
            if not await self.connect(a_inputs[i], and_g_id):
                return False
            if not await self.connect(not_b[i], and_g_id, target_port=1):
                return False

            and_l_id = await self.create_element(
                "And",
                and_x + i * HORIZONTAL_GATE_SPACING,
                100.0 + (8 + i) * VERTICAL_STAGE_SPACING,
                f"andLess[{i}]",
            )
            if and_l_id is None:
                return False
            and_less.append(and_l_id)
            if not await self.connect(not_a[i], and_l_id):
                return False
            if not await self.connect(b_inputs[i], and_l_id, target_port=1):
                return False

        await self.log("  ✓ Created comparison gates (XNOR, AND Greater, AND Less)")

        and_equal = await self.create_element(
            "And", cascade_x + HORIZONTAL_GATE_SPACING, 100.0 + VERTICAL_STAGE_SPACING, "andEqual"
        )
        if and_equal is None:
            return False
        set_props = await self.mcp.send_command("change_input_size", {"element_id": and_equal, "size": 4})
        if not set_props.success:
            self.log_error("Failed to set input_size=4 for equality AND gate")
            return False
        for i in range(4):
            if not await self.connect(xnor_gates[i], and_equal, target_port=i):
                return False
        await self.log("  ✓ Created equality cascade (4-input AND)")

        cascade_greater = [and_greater[0]]
        cascade_less = [and_less[0]]
        for i in range(1, 4):
            and_casc_g_id = await self.create_element(
                "And",
                cascade_x + (i % 2) * HORIZONTAL_GATE_SPACING,
                100.0 + (4 + (i // 2)) * VERTICAL_STAGE_SPACING,
                f"andCascadeGreater[{i}]",
            )
            if and_casc_g_id is None:
                return False
            if not await self.connect(xnor_gates[i], and_casc_g_id):
                return False
            if not await self.connect(cascade_greater[i - 1], and_casc_g_id, target_port=1):
                return False

            or_casc_g_id = await self.create_element(
                "Or",
                or_x + (i % 2) * HORIZONTAL_GATE_SPACING,
                100.0 + (4 + (i // 2)) * VERTICAL_STAGE_SPACING,
                f"orCascadeGreater[{i}]",
            )
            if or_casc_g_id is None:
                return False
            cascade_greater.append(or_casc_g_id)
            if not await self.connect(and_greater[i], or_casc_g_id):
                return False
            if not await self.connect(and_casc_g_id, or_casc_g_id, target_port=1):
                return False

            and_casc_l_id = await self.create_element(
                "And",
                cascade_x + (i % 2) * HORIZONTAL_GATE_SPACING,
                100.0 + (6 + (i // 2)) * VERTICAL_STAGE_SPACING,
                f"andCascadeLess[{i}]",
            )
            if and_casc_l_id is None:
                return False
            if not await self.connect(xnor_gates[i], and_casc_l_id):
                return False
            if not await self.connect(cascade_less[i - 1], and_casc_l_id, target_port=1):
                return False

            or_casc_l_id = await self.create_element(
                "Or",
                or_x + (i % 2) * HORIZONTAL_GATE_SPACING,
                100.0 + (6 + (i // 2)) * VERTICAL_STAGE_SPACING,
                f"orCascadeLess[{i}]",
            )
            if or_casc_l_id is None:
                return False
            cascade_less.append(or_casc_l_id)
            if not await self.connect(and_less[i], or_casc_l_id):
                return False
            if not await self.connect(and_casc_l_id, or_casc_l_id, target_port=1):
                return False

        await self.log("  ✓ Created cascade OR gates with proper comparison logic")

        # ========== 74LS85 cascade final stage ==========
        # eq_all = and_equal (all four bit-pairs equal):
        #   Greater = local-greater OR (eq_all AND GreaterIn)
        #   Less    = local-less    OR (eq_all AND LessIn)
        #   Equal   = eq_all AND EqualIn
        eq_gt_and = await self.create_element("And", final_x, 100.0 + 2 * VERTICAL_STAGE_SPACING, "eqAndGtIn")
        if eq_gt_and is None:
            return False
        if not await self.connect(and_equal, eq_gt_and):
            return False
        if not await self.connect(gt_in, eq_gt_and, target_port=1):
            return False

        greater_final = await self.create_element("Or", final_x, 100.0 + 3 * VERTICAL_STAGE_SPACING, "greaterFinal")
        if greater_final is None:
            return False
        if not await self.connect(cascade_greater[3], greater_final):
            return False
        if not await self.connect(eq_gt_and, greater_final, target_port=1):
            return False

        equal_final = await self.create_element("And", final_x, 100.0 + 5 * VERTICAL_STAGE_SPACING, "equalFinal")
        if equal_final is None:
            return False
        if not await self.connect(and_equal, equal_final):
            return False
        if not await self.connect(eq_in, equal_final, target_port=1):
            return False

        eq_lt_and = await self.create_element("And", final_x, 100.0 + 6 * VERTICAL_STAGE_SPACING, "eqAndLtIn")
        if eq_lt_and is None:
            return False
        if not await self.connect(and_equal, eq_lt_and):
            return False
        if not await self.connect(lt_in, eq_lt_and, target_port=1):
            return False

        less_final = await self.create_element("Or", final_x, 100.0 + 7 * VERTICAL_STAGE_SPACING, "lessFinal")
        if less_final is None:
            return False
        if not await self.connect(cascade_less[3], less_final):
            return False
        if not await self.connect(eq_lt_and, less_final, target_port=1):
            return False

        greater_led = await self.create_element("Led", output_x, 100.0 + 3 * VERTICAL_STAGE_SPACING, "Greater")
        if greater_led is None:
            return False
        if not await self.connect(greater_final, greater_led):
            return False

        equal_led = await self.create_element("Led", output_x, 100.0 + 5 * VERTICAL_STAGE_SPACING, "Equal")
        if equal_led is None:
            return False
        if not await self.connect(equal_final, equal_led):
            return False

        less_led = await self.create_element("Led", output_x, 100.0 + 7 * VERTICAL_STAGE_SPACING, "Less")
        if less_led is None:
            return False
        if not await self.connect(less_final, less_led):
            return False

        await self.log("  ✓ Created output LEDs")

        output_file = str(IC_COMPONENTS_DIR / f"{output_name}.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created Comparator4Bit IC"
            f"({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


class MuxNto1Builder(ICBuilderBase):
    """N-to-1 multiplexer with log2(N) select bits. Covers mux_4to1 and mux_8to1."""

    def __init__(self, mcp, data_inputs: int, verbose: bool = True) -> None:
        super().__init__(mcp, verbose)
        self.data_inputs = data_inputs

    async def create(self) -> bool:
        """Create an N-to-1 mux IC."""
        data_inputs = self.data_inputs
        select_bits = data_inputs.bit_length() - 1
        input_size = data_inputs + select_bits
        output_name = f"level2_mux_{data_inputs}to1"
        display = f"Multiplexer {data_inputs}-to-1"

        await self.begin_build(display)
        if not await self.create_new_circuit():
            return False

        input_x = 50.0
        input_y = 100.0

        data_ids = []
        for i in range(data_inputs):
            data_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), input_y, f"Data[{i}]"
            )
            if data_id is None:
                return False
            data_ids.append(data_id)
            await self.log(f"  ✓ Created Data[{i}] (id={data_id})")

        select_ids = []
        select_x = input_x + (data_inputs * HORIZONTAL_GATE_SPACING) + HORIZONTAL_GATE_SPACING
        for i in range(select_bits):
            sel_id = await self.create_element(
                "InputSwitch", select_x + (i * HORIZONTAL_GATE_SPACING), input_y, f"Sel[{i}]"
            )
            if sel_id is None:
                return False
            select_ids.append(sel_id)
            await self.log(f"  ✓ Created Sel[{i}] (id={sel_id})")

        mux_x = select_x + (2 * HORIZONTAL_GATE_SPACING)
        mux_y = input_y + VERTICAL_STAGE_SPACING
        mux_id = await self.create_element("Mux", mux_x, mux_y, f"Mux_{data_inputs}to1")
        if mux_id is None:
            return False
        await self.log(f"  ✓ Created Mux (id={mux_id})")

        resp = await self.mcp.send_command("change_input_size", {"element_id": mux_id, "size": input_size})
        if not resp.success:
            self.log_error(f"Failed to set Mux inputSize: {resp.error}")
            return False
        await self.log(f"  ✓ Set Mux inputSize to {input_size}")

        for i in range(data_inputs):
            if not await self.connect(data_ids[i], mux_id, target_port_label=f"In{i}"):
                return False
        for i in range(select_bits):
            if not await self.connect(select_ids[i], mux_id, target_port_label=f"S{i}"):
                return False

        output_x = mux_x + HORIZONTAL_GATE_SPACING
        output_id = await self.create_element("Led", output_x, mux_y, "Output")
        if output_id is None:
            return False
        if not await self.connect(mux_id, output_id, source_port_label="Out"):
            return False
        await self.log("  ✓ Connected Mux output to LED")

        output_file = str(IC_COMPONENTS_DIR / f"{output_name}.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created {data_inputs}:1 Multiplexer IC"
            f"({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


class ParityBuilder(ICBuilderBase):
    """Binary XOR-tree parity IC. is_checker=True → 8-bit checker; False → 4-bit generator."""

    def __init__(self, mcp, is_checker: bool, verbose: bool = True) -> None:
        super().__init__(mcp, verbose)
        self.is_checker = is_checker

    async def create(self) -> bool:
        """Create parity checker or generator IC."""
        is_checker = self.is_checker
        if is_checker:
            width = 8
            in_label = "data"
            out_label = "parity"
            display = "Parity Checker"
            output_name = "level2_parity_checker"
        else:
            width = 4
            in_label = "Data"
            out_label = "Parity"
            display = "Parity Generator"
            output_name = "level2_parity_generator"

        await self.begin_build(display)
        if not await self.create_new_circuit():
            return False

        input_x = 50.0
        output_x = input_x + HORIZONTAL_GATE_SPACING * 5

        data_inputs = []
        for i in range(width):
            elem_id = await self.create_element(
                "InputSwitch", input_x, 100.0 + i * VERTICAL_STAGE_SPACING, f"{in_label}[{i}]"
            )
            if elem_id is None:
                return False
            data_inputs.append(elem_id)
        await self.log(f"  ✓ Created {width} input switches")

        current_stage = data_inputs
        stage_num = 1
        stage_x = input_x + HORIZONTAL_GATE_SPACING
        while len(current_stage) > 1:
            next_stage = []
            for pair_idx in range(0, len(current_stage), 2):
                xor_id = await self.create_element(
                    "Xor",
                    stage_x + (pair_idx // 2 % 2) * HORIZONTAL_GATE_SPACING,
                    100.0 + (pair_idx // 4) * VERTICAL_STAGE_SPACING,
                    f"xor_s{stage_num}[{pair_idx // 2}]",
                )
                if xor_id is None:
                    return False
                next_stage.append(xor_id)
                for port_idx in range(2):
                    input_idx = pair_idx + port_idx
                    if input_idx < len(current_stage) and not await self.connect(
                        current_stage[input_idx], xor_id, target_port=port_idx
                    ):
                        return False
            await self.log(f"  ✓ Created stage {stage_num}: {len(next_stage)} XOR gates")
            current_stage = next_stage
            stage_num += 1
            stage_x += HORIZONTAL_GATE_SPACING

        # Cascade input (74180-style): XOR a chained parity bit from a less-
        # significant block into this block's parity. Tied low (the default) it
        # is a no-op, so a single block behaves exactly as before; to build a
        # wider parity tree, feed another block's parity output in here.
        cascade_in = await self.create_element(
            "InputSwitch", input_x, 100.0 + width * VERTICAL_STAGE_SPACING, "CascadeIn"
        )
        if cascade_in is None:
            return False
        cascade_xor = await self.create_element(
            "Xor", stage_x, 100.0 + (width // 2) * VERTICAL_STAGE_SPACING, "cascade_xor"
        )
        if cascade_xor is None:
            return False
        if not await self.connect(current_stage[0], cascade_xor):
            return False
        if not await self.connect(cascade_in, cascade_xor, target_port=1):
            return False
        current_stage = [cascade_xor]

        parity_led = await self.create_element(
            "Led", output_x, 100.0 + (width // 2) * VERTICAL_STAGE_SPACING, out_label
        )
        if parity_led is None:
            return False
        if not await self.connect(current_stage[0], parity_led):
            return False

        output_file = str(IC_COMPONENTS_DIR / f"{output_name}.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created {display} IC({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


class RingJohnsonCounterBuilder(ICBuilderBase):
    """4-bit ring/Johnson counter. is_johnson=True → twisted feedback; False → circular."""

    def __init__(self, mcp, is_johnson: bool, verbose: bool = True) -> None:
        super().__init__(mcp, verbose)
        self.is_johnson = is_johnson

    async def create(self) -> bool:
        """Create the ring or Johnson counter IC."""
        is_johnson = self.is_johnson
        if is_johnson:
            display = "4-bit Johnson Counter"
            output_name = "level4_johnson_counter_4bit"
        else:
            display = "4-bit Ring Counter"
            output_name = "level4_ring_counter_4bit"

        await self.begin_build(display)
        if not await self.create_new_circuit():
            return False

        input_x = 50.0
        clk_id = await self.create_element("InputSwitch", input_x, 100.0, "CLK")
        if clk_id is None:
            return False
        await self.log("  ✓ Created input CLK")

        preset_id = await self.create_element("InputSwitch", input_x, 100.0 + VERTICAL_STAGE_SPACING, "PRESET")
        if preset_id is None:
            return False
        await self.log("  ✓ Created input PRESET")

        dff_dep = "level1_d_flip_flop"
        dff_ids = []
        dff_x = input_x + HORIZONTAL_GATE_SPACING
        for i in range(4):
            ff_id = await self.instantiate_ic(dff_dep, dff_x + (i * HORIZONTAL_GATE_SPACING), 100.0, f"FF{i}")
            if ff_id is None:
                return False
            dff_ids.append(ff_id)
            await self.log(f"  ✓ Instantiated level1_d_flip_flop {i}")

        not_id = None
        if is_johnson:
            not_id = await self.create_element(
                "Not",
                dff_x + (3 * HORIZONTAL_GATE_SPACING),
                100.0 + VERTICAL_STAGE_SPACING,
                "not_q3",
            )
            if not_id is None:
                return False
            await self.log("  ✓ Created NOT gate (twisted feedback)")

        output_led_ids = []
        output_y = 100.0 + VERTICAL_STAGE_SPACING
        for i in range(4):
            led_id = await self.create_element("Led", dff_x + (i * HORIZONTAL_GATE_SPACING), output_y, f"Q{i}")
            if led_id is None:
                return False
            output_led_ids.append(led_id)
            await self.log(f"  ✓ Created output Q{i}")

        for i in range(4):
            if not await self.connect(clk_id, dff_ids[i], target_port_label="Clock"):
                return False

        if is_johnson:
            for i in range(3):
                if not await self.connect(dff_ids[i], dff_ids[i + 1], source_port_label="Q", target_port_label="D"):
                    return False
            assert not_id is not None
            if not await self.connect(dff_ids[3], not_id, source_port_label="Q"):
                return False
            if not await self.connect(not_id, dff_ids[0], target_port_label="D"):
                return False
        else:
            for i in range(4):
                next_idx = (i + 1) % 4
                if not await self.connect(dff_ids[i], dff_ids[next_idx], source_port_label="Q", target_port_label="D"):
                    return False

        for i in range(4):
            if not await self.connect(dff_ids[i], output_led_ids[i], source_port_label="Q"):
                return False

        if not await self.connect(preset_id, dff_ids[0], target_port_label="Preset"):
            return False

        vcc_id = await self.create_element(
            "InputVcc", dff_x - HORIZONTAL_GATE_SPACING, 100.0 + 2 * VERTICAL_STAGE_SPACING, "Vcc"
        )
        if vcc_id is None:
            return False
        await self.log("  ✓ Created Vcc element")

        for i in range(1, 4):
            if not await self.connect(vcc_id, dff_ids[i], target_port_label="Preset"):
                return False
            if not await self.connect(preset_id, dff_ids[i], target_port_label="Clear"):
                return False
        if not await self.connect(vcc_id, dff_ids[0], target_port_label="Clear"):
            return False

        output_file = str(IC_COMPONENTS_DIR / f"{output_name}.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created {display} IC({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True
