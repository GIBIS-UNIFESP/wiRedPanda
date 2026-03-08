#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create CPU Level 6: Stack Memory Interface IC

Composes Stack Pointer with 256-byte RAM and address multiplexing.

This IC handles all stack memory operations:
- Stack Pointer management (PUSH/POP)
- Memory address selection (Program memory vs Stack memory)
- Data path from register to memory
- Data path from memory to register

Inputs:
  - SP_LoadValue[0..7] - Value to load into Stack Pointer
  - SP_Load - Load Stack Pointer
  - SP_Push - Decrement SP (PUSH operation)
  - SP_Pop - Increment SP (POP operation)
  - SP_Reset - Reset SP to 0xFF
  - Address[0..7] - Memory address (from PC or other source)
  - AddressSelect - 0=Use Address input, 1=Use SP
  - DataIn[0..7] - Data to write to memory
  - MemWrite - Memory write enable
  - MemRead - Memory read enable
  - Clock - Clock signal
  - Enable - Enable updates

Outputs:
  - SP[0..7] - Current Stack Pointer value
  - DataOut[0..7] - Data read from memory
  - FinalAddress[0..7] - Selected memory address (Address or SP)

Architecture:
- level6_stack_pointer_8bit.panda - 8-bit SP with PUSH/POP
- level6_ram_256x8.panda - 256-byte RAM
- 8x 2-to-1 Multiplexer - Select between Address[i] and SP[i]
- Data paths for read/write operations
- Control signal distribution

Usage:
    python3 create_level6_stack_memory_interface.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING


class StackMemoryInterfaceBuilder(ICBuilderBase):
    """Builder for Stack Memory Interface IC"""

    async def create(self) -> bool:
        """Create the Stack Memory Interface IC"""
        await self.begin_build("Stack Memory Interface")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # ========== INPUT ELEMENTS ==========

        # Create input switches for all control and data signals
        inputs = {}
        input_labels = [
            # Stack Pointer controls
            "SP_LoadValue[0]", "SP_LoadValue[1]", "SP_LoadValue[2]", "SP_LoadValue[3]",
            "SP_LoadValue[4]", "SP_LoadValue[5]", "SP_LoadValue[6]", "SP_LoadValue[7]",
            "SP_Load", "SP_Push", "SP_Pop", "SP_Reset",
            # Address inputs
            "Address[0]", "Address[1]", "Address[2]", "Address[3]",
            "Address[4]", "Address[5]", "Address[6]", "Address[7]",
            "AddressSelect",
            # Data inputs
            "DataIn[0]", "DataIn[1]", "DataIn[2]", "DataIn[3]",
            "DataIn[4]", "DataIn[5]", "DataIn[6]", "DataIn[7]",
            # Control signals
            "MemWrite", "MemRead", "Clock", "Enable"
        ]

        input_x = 50.0
        for idx, label in enumerate(input_labels):
            element_id = await self.create_element("InputSwitch", input_x, 100.0 + (idx * 20.0), label)
            if element_id is None:
                return False
            inputs[label] = element_id

        # ========== MAJOR COMPONENTS ==========

        # Instantiate Stack Pointer (just created)
        await self.log("📦 Instantiating Stack Pointer 8-bit...")
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level6_stack_pointer_8bit")):

            return False

        sp_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level6_stack_pointer_8bit"), 300.0, 200.0, "StackPointer")
        if sp_id is None:
            return False

        # Instantiate RAM (256x8)
        await self.log("📦 Instantiating 256-byte RAM...")
        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level6_ram_256x8")):

            return False

        ram_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level6_ram_256x8"), 750.0, 200.0, "StackRAM")
        if ram_id is None:
            return False

        # Create 8x 2-to-1 Multiplexers for address selection (Address vs SP)
        await self.log("📦 Creating address multiplexers...")
        address_muxes = []
        mux_x = 600.0
        for i in range(8):
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level2_mux_2to1")):

                return False

            element_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level2_mux_2to1"), mux_x + (i * HORIZONTAL_GATE_SPACING), 200.0, f"AddrMux[{i}]")
            if element_id is None:
                return False
            address_muxes.append(element_id)

        # Create output LEDs for outputs
        await self.log("📦 Creating output LEDs...")
        outputs = {}
        out_x = 1300.0
        out_labels = [
            "SP[0]", "SP[1]", "SP[2]", "SP[3]", "SP[4]", "SP[5]", "SP[6]", "SP[7]",
            "DataOut[0]", "DataOut[1]", "DataOut[2]", "DataOut[3]",
            "DataOut[4]", "DataOut[5]", "DataOut[6]", "DataOut[7]",
            "FinalAddress[0]", "FinalAddress[1]", "FinalAddress[2]", "FinalAddress[3]",
            "FinalAddress[4]", "FinalAddress[5]", "FinalAddress[6]", "FinalAddress[7]"
        ]

        for idx, label in enumerate(out_labels):
            element_id = await self.create_element("Led", out_x, 100.0 + (idx * 20.0), label)
            if element_id is None:
                return False
            outputs[label] = element_id

        # ========== CONNECTIONS ==========

        await self.log(f"✓ Created {self.element_count} elements")
        await self.log("🔌 Making connections...")

        # Stack Pointer inputs
        for i in range(8):
            # SP_LoadValue[i] → Stack Pointer LoadValue[i]
            if not await self.connect(inputs[f"SP_LoadValue[{i}]"], sp_id, target_port_label=f"LoadValue[{i}]"):
                return False

        # Stack Pointer control signals (using semantic port labels)
        sp_control_mappings = {
            "SP_Load": "Load",      # Load port on Stack Pointer
            "SP_Push": "Push",      # Push port
            "SP_Pop": "Pop",        # Pop port
            "SP_Reset": "Reset",    # Reset port
            "Clock": "Clock"        # Clock port
        }

        for ctrl_name, port_label in sp_control_mappings.items():
            if not await self.connect(inputs[ctrl_name], sp_id, target_port_label=port_label):
                return False

        # Stack Pointer outputs (SP bits)
        for i in range(8):
            # Stack Pointer output[i] → Output LED SP[i]
            if not await self.connect(sp_id, outputs[f"SP[{i}]"], source_port_label=f"SP[{i}]"):
                return False

            # Stack Pointer output[i] → Address Mux[i] input 1 (SP selection)
            # Mux level2_mux_2to1: Data[0]=Address, Data[1]=SP, Sel[0]=AddressSelect
            if not await self.connect(sp_id, address_muxes[i], source_port_label=f"SP[{i}]", target_port_label="Data[1]"):
                return False

        # Address inputs
        for i in range(8):
            # Address[i] → Address Mux[i] input 0 (PC selection)
            if not await self.connect(inputs[f"Address[{i}]"], address_muxes[i]):
                return False

            # Address Mux control: AddressSelect → all muxes select input (Sel[0] port)
            if not await self.connect(inputs["AddressSelect"], address_muxes[i], target_port_label="Sel[0]"):
                return False

            # Address Mux output → RAM address[i]
            if not await self.connect(address_muxes[i], ram_id, target_port_label=f"Address[{i}]"):
                return False

            # Address Mux output → Output FinalAddress[i]
            if not await self.connect(address_muxes[i], outputs[f"FinalAddress[{i}]"]):
                return False

        # Data write path
        for i in range(8):
            # DataIn[i] → RAM write data[i]
            if not await self.connect(inputs[f"DataIn[{i}]"], ram_id, target_port_label=f"DataIn[{i}]"):
                return False

        # Data read path
        for i in range(8):
            # RAM read data[i] → DataOut[i]
            if not await self.connect(ram_id, outputs[f"DataOut[{i}]"], source_port_label=f"DataOut[{i}]"):
                return False

        # Control signals to RAM
        # RAM uses: WriteEnable (port label) and Clock (port label)
        ram_control_map = {
            "MemWrite": "WriteEnable",
            "Clock": "Clock"
        }

        for ctrl_name, ram_label in ram_control_map.items():
            if not await self.connect(inputs[ctrl_name], ram_id, target_port_label=ram_label):
                return False

        await self.log(f"✓ Made {self.connection_count} connections")

        # Save the IC
        output_file = IC_COMPONENTS_DIR / "level6_stack_memory_interface.panda"
        if not await self.save_circuit(str(output_file)):
            return False

        await self.log(f"✅ Successfully created Stack Memory Interface IC: {output_file}")
        return True


async def build(mcp) -> bool:
    """Build the Stack Memory Interface IC"""
    builder = StackMemoryInterfaceBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "Stack Memory Interface"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
