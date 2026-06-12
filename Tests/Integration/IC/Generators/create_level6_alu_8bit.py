#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create 8-bit ALU IC (Arithmetic Logic Unit) - Properly Composed

Implements 8 operations (ADD, SUB, AND, OR, XOR, NOT, SHL, SHR) using composed ICs.

Inputs:
  A[0..7] (8 bits, operand A)
  B[0..7] (8 bits, operand B)
  OpCode[0..2] (3 bits, operation selector)
  CarryIn (ADD chain carry-in; defaults 0 when unconnected)
  SubCarryIn (SUB chain carry-in; defaults 1 — the two's-complement +1 —
              when unconnected, so standalone SUB works; chain it for 16-bit)
  ShrIn (SHR bit-7 fill; defaults 0 — chain the neighbor's A[0] for 16-bit; F61)
  ShlIn (SHL bit-0 fill; defaults 0 — chain the neighbor's A[7] for 16-bit; F61)

Operations (OpCode mapping):
  000: ADD (A + B)
  001: SUB (A - B via 2's complement)
  010: AND (A & B)
  011: OR (A | B)
  100: XOR (A ^ B)
  101: NOT (~A)
  110: SHL (A << 1, shift left)
  111: SHR (A >> 1, shift right)

Outputs:
  Result[0..7] (8 bits, operation result)
  Zero (1 if the final result == 0 — valid for every operation)
  Negative (final result[7])
  Carry (ADD chain carry-out)
  SubCarryOut (SUB chain carry-out, for 16-bit cascading)

Architecture (per bit):
  - Instantiate level4_ripple_alu_4bit twice to compute operations 0-3 (ADD/SUB/AND/OR)
  - Create XOR gate for operation 4 (XOR)
  - Create NOT gate for operation 5 (NOT)
  - Wire A for operation 6 (SHL = left shift)
  - Wire B for operation 7 (SHR = right shift)
  - Instantiate level3_alu_selector_5way per bit to select from 5 main operations
  - Use additional logic to handle operations 6-7

Usage:
    python create_level6_alu_8bit.py
"""

import asyncio

from ic_builder_base import ICBuilderBase, IC_COMPONENTS_DIR, run_ic_builder
from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING


class ALU8BitBuilder(ICBuilderBase):
    """Builder for 8-bit ALU IC with 8 operations using proper composition"""

    async def create(self) -> bool:
        """Create the 8-bit ALU IC"""
        await self.begin_build("ALU 8-bit")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input positions
        input_a_x_start = 50.0
        input_b_x_start = 50.0
        input_op_x = input_a_x_start + (8 * HORIZONTAL_GATE_SPACING) + HORIZONTAL_GATE_SPACING
        input_a_y = 100.0
        input_b_y = 120.0
        op_y = 100.0

        # ---- Create A operand inputs (8 bits) ----
        a_inputs = []
        for i in range(8):
            element_id = await self.create_element("InputSwitch", input_a_x_start + i * HORIZONTAL_GATE_SPACING, input_a_y, f"A[{i}]")
            if element_id is None:
                return False
            a_inputs.append(element_id)

        # ---- Create B operand inputs (8 bits) ----
        b_inputs = []
        for i in range(8):
            element_id = await self.create_element("InputSwitch", input_b_x_start + i * HORIZONTAL_GATE_SPACING, input_b_y, f"B[{i}]")
            if element_id is None:
                return False
            b_inputs.append(element_id)

        # ---- Create OpCode inputs (3 bits) ----
        opcode_inputs = []
        for i in range(3):
            element_id = await self.create_element("InputSwitch", input_op_x + i * HORIZONTAL_GATE_SPACING, op_y, f"OpCode[{i}]")
            if element_id is None:
                return False
            opcode_inputs.append(element_id)

        await self.log("  ✓ Created 16 operand inputs + 3 opcode inputs")

        # Instantiate two 4-bit ALU ICs for operations 0-3
        alu_low_x = input_op_x + HORIZONTAL_GATE_SPACING
        alu_high_x = alu_low_x + HORIZONTAL_GATE_SPACING
        alu_y = 250.0

        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_ripple_alu_4bit")):


            return False


        alu_low = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_ripple_alu_4bit"), alu_low_x, alu_y, "ALU_Low")
        if alu_low is None:
            return False

        if not self.check_dependency(str(IC_COMPONENTS_DIR / "level4_ripple_alu_4bit")):


            return False


        alu_high = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level4_ripple_alu_4bit"), alu_high_x, alu_y, "ALU_High")
        if alu_high is None:
            return False
        await self.log("  ✓ Instantiated 4-bit ALUs for operations 0-3")

        # Connect inputs to ALUs
        for i in range(4):
            for src, tgt_alu in [(a_inputs[i], alu_low), (a_inputs[i+4], alu_high)]:
                if not await self.connect(src, tgt_alu, target_port_label=f"A[{i}]"):
                    return False

            for src, tgt_alu in [(b_inputs[i], alu_low), (b_inputs[i+4], alu_high)]:
                if not await self.connect(src, tgt_alu, target_port_label=f"B[{i}]"):
                    return False

        # Carry-in ports (F26): exposed as boundary inputs so two 8-bit ALUs
        # can be chained into a 16-bit one. The saved switch state is the
        # default for unconnected IC inputs, so standalone behavior is
        # unchanged: CarryIn defaults off (0) and SubCarryIn defaults ON (the
        # +1 for two's complement SUB).
        carryin_id = await self.create_element("InputSwitch", input_a_x_start + (8 * HORIZONTAL_GATE_SPACING), 250.0, "CarryIn")
        if carryin_id is None:
            return False

        subcarryin_id = await self.create_element("InputSwitch", input_a_x_start + (9 * HORIZONTAL_GATE_SPACING), 250.0, "SubCarryIn")
        if subcarryin_id is None:
            return False
        set_sub = await self.mcp.send_command("set_input_value", {"element_id": subcarryin_id, "value": True})
        if not set_sub.success:
            self.log_error("Failed to default SubCarryIn high")
            return False

        # Connect CarryIn to low ALU CarryIn (ADD carry chain starts here)
        if not await self.connect(carryin_id, alu_low, target_port_label="CarryIn"):
            return False

        # Connect SubCarryIn to low ALU SubCarryIn (SUB carry chain starts here)
        if not await self.connect(subcarryin_id, alu_low, target_port_label="SubCarryIn"):
            return False

        # Connect low ALU CarryOut to high ALU CarryIn for ADD carry propagation
        if not await self.connect(alu_low, alu_high, source_port_label="CarryOut", target_port_label="CarryIn"):
            return False

        # Connect low ALU SubCarryOut to high ALU SubCarryIn for SUB carry propagation
        if not await self.connect(alu_low, alu_high, source_port_label="SubCarryOut", target_port_label="SubCarryIn"):
            return False
        await self.log("  ✓ Connected ADD and SUB carry chains between 4-bit ALUs")

        # Note: level4_ripple_alu_4bit no longer takes operation inputs - it outputs all 4 operations simultaneously
        # Operations are selected by level3_alu_selector_5way based on OpCode

        # Create XOR gates for operation 4 (XOR = A ^ B)
        xor_results = []
        xor_y = alu_y + VERTICAL_STAGE_SPACING
        for i in range(8):
            xor_id = await self.create_element("Xor", input_a_x_start + i * HORIZONTAL_GATE_SPACING, xor_y, f"xor_{i}")
            if xor_id is None:
                return False
            xor_results.append(xor_id)

            if not await self.connect(a_inputs[i], xor_id):
                return False

            if not await self.connect(b_inputs[i], xor_id, target_port=1):
                return False

        await self.log("  ✓ Created XOR gates for operation 4")

        # Create NOT gates for operation 5 (NOT = ~A)
        not_results = []
        not_y = xor_y + VERTICAL_STAGE_SPACING
        for i in range(8):
            not_id = await self.create_element("Not", input_a_x_start + i * HORIZONTAL_GATE_SPACING, not_y, f"not_{i}")
            if not_id is None:
                return False
            not_results.append(not_id)

            if not await self.connect(a_inputs[i], not_id):
                return False

        await self.log("  ✓ Created NOT gates for operation 5")

        # Note: AND and OR operations are already provided by level4_ripple_alu_4bit

        # Shift taps (F31: these two lists were name-swapped; the op-decode
        # select was swapped the same way, so behavior was already correct —
        # the names now state what each list holds).
        # Boundary fill ports (F61): the fills used to be internal GND
        # constants, so 16-bit SHL/SHR lost the bit crossing the byte
        # boundary. Exposed as saved-off input switches (the F26 carry-port
        # mechanism): standalone behavior is unchanged (fill = 0), and a
        # 16-bit ALU chains them from its own operand bits.

        # SHR (A >> 1): Result[i] = A[i+1], top bit fills from ShrIn.
        shrin_id = await self.create_element("InputSwitch", input_a_x_start + (10 * HORIZONTAL_GATE_SPACING), 250.0, "ShrIn")
        if shrin_id is None:
            return False

        shr_results = []
        shl_y = not_y + VERTICAL_STAGE_SPACING
        for i in range(8):
            if i < 7:
                shr_results.append(a_inputs[i + 1])
            else:
                shr_results.append(shrin_id)

        await self.log("  ✓ Created shift right taps (A >> 1, fill = ShrIn)")

        # SHL (A << 1): Result[i] = A[i-1], bottom bit fills from ShlIn.
        shlin_id = await self.create_element("InputSwitch", input_a_x_start + (11 * HORIZONTAL_GATE_SPACING), 250.0, "ShlIn")
        if shlin_id is None:
            return False

        shl_results = []
        for i in range(8):
            if i > 0:
                shl_results.append(a_inputs[i - 1])
            else:
                shl_results.append(shlin_id)

        await self.log("  ✓ Created shift left taps (A << 1, fill = ShlIn)")

        # Need to extend selector to 8-way instead of 5-way
        # For now, create additional mux layer to handle operations 5-7
        # Create selectors to choose between 5-way output and additional operations (NOT, SHL, SHR)

        # Instantiate level3_alu_selector_5way per bit to select from 5 operations (0-4)
        selector_x = input_a_x_start
        selector_y = shl_y + (2 * VERTICAL_STAGE_SPACING)
        selector_ids = []  # Store selector IDs for NOR connections
        selector_5way_outputs = []

        for i in range(8):
            # Instantiate selector for this bit (handles operations 0-4)
            if not self.check_dependency(str(IC_COMPONENTS_DIR / "level3_alu_selector_5way")):

                return False

            selector_id = await self.instantiate_ic(str(IC_COMPONENTS_DIR / "level3_alu_selector_5way"), selector_x + i * HORIZONTAL_GATE_SPACING, selector_y, f"Selector5way[{i}]")
            if selector_id is None:
                return False
            selector_ids.append(selector_id)

            # Connect operation results to selector
            alu_src = alu_low if i < 4 else alu_high
            alu_idx = i % 4

            # result0: ADD operation
            if not await self.connect(alu_src, selector_id, source_port_label=f"Result_ADD[{alu_idx}]", target_port_label="result0"):
                return False

            # result1: SUB operation
            if not await self.connect(alu_src, selector_id, source_port_label=f"Result_SUB[{alu_idx}]", target_port_label="result1"):
                return False

            # result2: AND operation
            if not await self.connect(alu_src, selector_id, source_port_label=f"Result_AND[{alu_idx}]", target_port_label="result2"):
                return False

            # result3: OR operation
            if not await self.connect(alu_src, selector_id, source_port_label=f"Result_OR[{alu_idx}]", target_port_label="result3"):
                return False

            # result4: XOR operation
            if not await self.connect(xor_results[i], selector_id, target_port_label="result4"):
                return False

            # Connect OpCode[0:2] to selector (for operations 0-4 selection)
            for op_idx in range(3):
                if not await self.connect(opcode_inputs[op_idx], selector_id, target_port_label=f"op{op_idx}"):
                    return False

            # Store selector output for further processing
            selector_5way_outputs.append(selector_id)

        await self.log("  ✓ Instantiated 8 5-way selectors for operations 0-4")

        # Final selection happens in the Mux1/Mux2/Mux3 cascade below.
        # (F26: a dead "Mux8way" layer used to sit here — 8 muxes with data
        # wired but no select and no consumer.)
        mux_8way_y = selector_y + VERTICAL_STAGE_SPACING

        # For operations 0-4: Use the 5way selector
        # For operation 5 (NOT): Use NOT gates
        # For operation 6 (SHL): Use shift left taps
        # For operation 7 (SHR): Use shift right taps

        # Create OpCode decoders for operations 5, 6, 7
        # Op5: OpCode[2] AND NOT(OpCode[1]) AND OpCode[0] (detects 101)
        # Op6: OpCode[2] AND OpCode[1] AND NOT(OpCode[0]) (detects 110)
        # Op7: OpCode[2] AND OpCode[1] AND OpCode[0] (detects 111)

        # Create NOT gates
        not_op1_id = await self.create_element("Not", selector_x - HORIZONTAL_GATE_SPACING, selector_y - VERTICAL_STAGE_SPACING, "NOT_OpCode1")
        if not_op1_id is None:
            return False

        if not await self.connect(opcode_inputs[1], not_op1_id):
            return False

        not_op0_id = await self.create_element("Not", selector_x - HORIZONTAL_GATE_SPACING, selector_y, "NOT_OpCode0")
        if not_op0_id is None:
            return False

        if not await self.connect(opcode_inputs[0], not_op0_id):
            return False

        # Op5 detector: OpCode[2] AND NOT(OpCode[1]) AND OpCode[0]
        op5_and1_id = await self.create_element("And", selector_x, selector_y - VERTICAL_STAGE_SPACING, "Op5_AND1")
        if op5_and1_id is None:
            return False

        if not await self.connect(opcode_inputs[2], op5_and1_id):
            return False

        if not await self.connect(not_op1_id, op5_and1_id, target_port=1):
            return False

        op5_and2_id = await self.create_element("And", selector_x + HORIZONTAL_GATE_SPACING, selector_y - VERTICAL_STAGE_SPACING, "Op5_AND2")
        if op5_and2_id is None:
            return False

        if not await self.connect(op5_and1_id, op5_and2_id):
            return False

        if not await self.connect(opcode_inputs[0], op5_and2_id, target_port=1):
            return False

        # Op6 detector: OpCode[2] AND OpCode[1] AND NOT(OpCode[0])
        op6_and1_id = await self.create_element("And", selector_x, selector_y, "Op6_AND1")
        if op6_and1_id is None:
            return False

        if not await self.connect(opcode_inputs[2], op6_and1_id):
            return False

        if not await self.connect(opcode_inputs[1], op6_and1_id, target_port=1):
            return False

        op6_and2_id = await self.create_element("And", selector_x + HORIZONTAL_GATE_SPACING, selector_y, "Op6_AND2")
        if op6_and2_id is None:
            return False

        if not await self.connect(op6_and1_id, op6_and2_id):
            return False

        if not await self.connect(not_op0_id, op6_and2_id, target_port=1):
            return False

        # Op7 detector: OpCode[2] AND OpCode[1] AND OpCode[0]
        op7_and1_id = await self.create_element("And", selector_x, selector_y + VERTICAL_STAGE_SPACING, "Op7_AND1")
        if op7_and1_id is None:
            return False

        if not await self.connect(opcode_inputs[2], op7_and1_id):
            return False

        if not await self.connect(opcode_inputs[1], op7_and1_id, target_port=1):
            return False

        op7_and2_id = await self.create_element("And", selector_x + HORIZONTAL_GATE_SPACING, selector_y + VERTICAL_STAGE_SPACING, "Op7_AND2")
        if op7_and2_id is None:
            return False

        if not await self.connect(op7_and1_id, op7_and2_id):
            return False

        if not await self.connect(opcode_inputs[0], op7_and2_id, target_port=1):
            return False

        await self.log("  ✓ Created OpCode decoders for operations 5, 6, 7")

        # Create cascade of muxes to select from 8 operations
        result_outputs = []
        final_result_muxes = []  # Mux3 outputs — the true per-bit results, used by the flags

        for i in range(8):
            # ========== Mux1: Select between 5-way output and NOT result ==========
            # Implements: Out = (OpCode==101) ? NOT(A) : (ops 0-4 result)
            mux1_id = await self.create_element("Mux", selector_x + i * HORIZONTAL_GATE_SPACING - (2 * HORIZONTAL_GATE_SPACING), mux_8way_y, f"Mux1_5way_NOT[{i}]")
            if mux1_id is None:
                return False

            # Mux1 In0: 5-way selector output (selected when OpCode != 101)
            if not await self.connect(selector_5way_outputs[i], mux1_id, source_port_label="out", target_port_label="In0"):
                return False

            # Mux1 In1: NOT result (selected when OpCode == 101)
            if not await self.connect(not_results[i], mux1_id, target_port_label="In1"):
                return False

            # Mux1 Select: Op5 detector (1 if OpCode==101, selects NOT; 0 if not, selects 5way)
            if not await self.connect(op5_and2_id, mux1_id, target_port_label="S0"):
                return False

            # ========== Mux2: Select between SHL and SHR results ==========
            # Implements: Out = (OpCode[1]) ? SHR : SHL
            mux2_id = await self.create_element("Mux", selector_x + i * HORIZONTAL_GATE_SPACING, mux_8way_y, f"Mux2_SHL_SHR[{i}]")
            if mux2_id is None:
                return False

            # Mux2 In0: SHR result — reached for op 7 (op6 detector low, mux3
            # still routes here via the op6-or-op7 OR).
            if not await self.connect(shr_results[i], mux2_id, target_port_label="In0"):
                return False

            # Mux2 In1: SHL result — selected when the op6 detector is high.
            if not await self.connect(shl_results[i], mux2_id, target_port_label="In1"):
                return False

            # Mux2 Select: Op6 detector (1 for SHL/Op6, 0 for SHR/Op7)
            if not await self.connect(op6_and2_id, mux2_id, target_port_label="S0"):
                return False

            # ========== Mux3: Final selector between (ops 0-5) and (ops 6-7) ==========
            # Implements: Out = ((OpCode & 110)==110) ? ShiftResult : ArithLogicResult
            mux3_id = await self.create_element("Mux", selector_x + i * HORIZONTAL_GATE_SPACING + HORIZONTAL_GATE_SPACING, mux_8way_y, f"Mux3_Final[{i}]")
            if mux3_id is None:
                return False

            # Mux3 In0: Mux1 output (ops 0-5 arithmetic/logic/NOT results)
            if not await self.connect(mux1_id, mux3_id, target_port_label="In0"):
                return False

            # Mux3 In1: Mux2 output (ops 6-7 shift results)
            if not await self.connect(mux2_id, mux3_id, target_port_label="In1"):
                return False

            # ========== Op6_OR_Op7 gate for Mux3 Select ==========
            # Determines if we're in shift operation mode (OpCode[2:1]==11x)
            or_id = await self.create_element("Or", selector_x + i * HORIZONTAL_GATE_SPACING + (2 * HORIZONTAL_GATE_SPACING), mux_8way_y, f"Op6_OR_Op7[{i}]")
            if or_id is None:
                return False

            # Connect Op6 detector to first OR input
            if not await self.connect(op6_and2_id, or_id):
                return False

            # Connect Op7 detector to second OR input
            if not await self.connect(op7_and2_id, or_id, target_port=1):
                return False

            # Mux3 Select: (Op6 OR Op7) - 1 for shift operations, 0 for others
            if not await self.connect(or_id, mux3_id, target_port_label="S0"):
                return False

            final_result_muxes.append(mux3_id)

            # ========== Create result output LED ==========
            led_id = await self.create_element("Led", selector_x + i * HORIZONTAL_GATE_SPACING, mux_8way_y + VERTICAL_STAGE_SPACING, f"Result[{i}]")
            if led_id is None:
                return False
            result_outputs.append(led_id)

            # Connect final mux output to result LED
            if not await self.connect(mux3_id, led_id, source_port_label="Out"):
                return False

        await self.log("  ✓ Instantiated operation selectors for all 8 operations")

        # Create Zero flag (1 if all Result bits are 0)
        # NOR all result outputs together
        zero_nor_id = await self.create_element("Nor", selector_x, selector_y + (2 * VERTICAL_STAGE_SPACING), "Zero_NOR")
        if zero_nor_id is None:
            return False

        # Set NOR gate to 8 inputs
        set_size = await self.mcp.send_command("change_input_size", {
            "element_id": zero_nor_id,
            "size": 8
        })
        if not set_size.success:
            self.log_error(f"Failed to set input_size=8 for Zero NOR: {set_size.error}")
            return False

        # Connect the FINAL per-bit results to the NOR (F26: this used to tap
        # the 5-way selector outputs, so for NOT/SHL/SHR the flag reflected
        # A XOR B instead of the actual result).
        for i in range(8):
            if not await self.connect(final_result_muxes[i], zero_nor_id, source_port_label="Out", target_port=i):
                return False
        await self.log("  ✓ Created Zero flag (8-input NOR over the final results)")

        # Create Zero flag LED
        zero_led_id = await self.create_element("Led", selector_x + HORIZONTAL_GATE_SPACING, selector_y + (2 * VERTICAL_STAGE_SPACING), "Zero")
        if zero_led_id is None:
            return False

        if not await self.connect(zero_nor_id, zero_led_id):
            return False

        await self.log("  ✓ Created Zero flag")

        # Create Negative flag (Result[7])
        await self.log(f"  🔍 Creating Negative flag from Result[7]")
        negative_led_id = await self.create_element("Led", selector_x + (2 * HORIZONTAL_GATE_SPACING), selector_y + (2 * VERTICAL_STAGE_SPACING), "Negative")
        if negative_led_id is None:
            return False
        await self.log(f"  ✓ Created Negative LED (id={negative_led_id})")

        await self.log(f"  🔍 Connecting final Result[7] to Negative LED")
        # F26: tap the final result bit, not the 5-way selector output.
        if not await self.connect(final_result_muxes[7], negative_led_id, source_port_label="Out"):
            return False

        await self.log("  ✓ Created Negative flag")

        # Create Carry flag (CarryOut from high ALU)
        await self.log(f"  🔍 Creating Carry flag from high ALU CarryOut")
        carry_led_id = await self.create_element("Led", selector_x + (3 * HORIZONTAL_GATE_SPACING), selector_y + (2 * VERTICAL_STAGE_SPACING), "Carry")
        if carry_led_id is None:
            return False
        await self.log(f"  ✓ Created Carry LED (id={carry_led_id})")

        await self.log(f"  🔍 Connecting high ALU CarryOut to Carry LED")
        if not await self.connect(alu_high, carry_led_id, source_port_label="CarryOut"):
            return False

        await self.log("  ✓ Created Carry flag")

        # SubCarryOut (F26): exposes the SUB chain's carry so two 8-bit ALUs
        # can be cascaded into a 16-bit subtractor.
        subcarry_led_id = await self.create_element("Led", selector_x + (4 * HORIZONTAL_GATE_SPACING), selector_y + (2 * VERTICAL_STAGE_SPACING), "SubCarryOut")
        if subcarry_led_id is None:
            return False
        if not await self.connect(alu_high, subcarry_led_id, source_port_label="SubCarryOut"):
            return False

        await self.log("  ✓ Created SubCarryOut")

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level6_alu_8bit.panda")
        await self.log(f"  🔍 Saving circuit to {output_file}")
        if not await self.save_circuit(output_file):
            self.log_error(f"Failed to save circuit")
            return False

        await self.log(f"✅ Successfully created 8-bit ALU IC ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Architecture: 2× level4_ripple_alu_4bit + 8× XOR + 8× NOT + 8× level3_alu_selector_5way")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = ALU8BitBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback
    try:
        exit_code = asyncio.run(run_ic_builder(build, "8-bit ALU IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
