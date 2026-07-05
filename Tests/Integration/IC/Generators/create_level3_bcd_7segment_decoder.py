#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create BCD to 7-Segment Decoder IC

Inputs: BCD[0-3] (4-bit BCD value)
Outputs: segments[0-6] (7-segment display outputs - segment a through g)

Architecture:
Uses AND/OR gates to implement the 7-segment truth table.
Creates one AND gate per digit (0-9) to detect each digit value,
then ORs the appropriate digit detectors for each segment.

7-Segment Truth Table:
  aaa
 f   b
  ggg
 e   c
  ddd

Digit   BCD   a b c d e f g
0      0000  1 1 1 1 1 1 0
1      0001  0 1 1 0 0 0 0
2      0010  1 1 0 1 1 0 1
3      0011  1 1 1 1 0 0 1
4      0100  0 1 1 0 0 1 1
5      0101  1 0 1 1 0 1 1
6      0110  1 0 1 1 1 1 1
7      0111  1 1 1 0 0 0 0
8      1000  1 1 1 1 1 1 1
9      1001  1 1 1 1 0 1 1

Usage:
    python create_level3_bcd_7segment_decoder.py
"""

import asyncio

from element_spacing import HORIZONTAL_GATE_SPACING, VERTICAL_STAGE_SPACING
from ic_builder_base import IC_COMPONENTS_DIR, ICBuilderBase, run_ic_builder


class BCD7SegmentDecoderBuilder(ICBuilderBase):
    """Builder for BCD to 7-Segment Decoder IC"""

    async def create(self) -> bool:
        """Create the BCD to 7-Segment Decoder IC"""
        await self.begin_build("BCD 7-Segment Decoder")

        # Create new circuit
        if not await self.create_new_circuit():
            return False

        # Input/output positions
        input_x = 50.0
        input_y = 100.0
        output_x = input_x + (6 * HORIZONTAL_GATE_SPACING)
        output_y = input_y

        # Create input switches for BCD[0-3]
        bcd_inputs = []
        for i in range(4):
            element_id = await self.create_element(
                "InputSwitch", input_x + (i * HORIZONTAL_GATE_SPACING), input_y, f"BCD[{i}]"
            )
            if element_id is None:
                return False
            bcd_inputs.append(element_id)
            await self.log(f"  ✓ Created BCD[{i}] (id={element_id})")

        # Create output LEDs for segments a-g
        segment_names = ["a", "b", "c", "d", "e", "f", "g"]
        segment_outputs = []
        for i, segment in enumerate(segment_names):
            element_id = await self.create_element(
                "Led", output_x, output_y + (i * VERTICAL_STAGE_SPACING), f"segment_{segment}"
            )
            if element_id is None:
                return False
            segment_outputs.append(element_id)
            await self.log(f"  ✓ Created segment {segment} output (id={element_id})")

        # NOT gate positions
        # A full extra stage of horizontal clearance (2 * HORIZONTAL_GATE_SPACING
        # instead of 1) is inserted here so that or_gate_x below -- 3 stages right
        # of not_gate_x -- no longer lands on BCD[3]'s x. At only 1 stage of
        # clearance, or_gate_x == BCD[3]'s x exactly, and the OR gates for
        # segments needing up to 8 inputs grow tall enough (up to ~122px) that
        # their body reaches up into BCD[3]'s label. The extra stage shifts the
        # whole not/digit-detector/OR column chain right by one column, so it now
        # aliases BCD[2]/BCD[3]/(no BCD column) respectively instead of
        # BCD[1]/BCD[2]/BCD[3] -- the per-column row-push below still clears
        # each gate row from whichever BCD input row sits directly above it.
        not_gate_x = input_x + (2 * HORIZONTAL_GATE_SPACING)

        # Create NOT gates for inverted inputs
        # Row 0 of this column would otherwise land at input_y, exactly where the
        # BCD[2] input switch sits (not_gate_x == BCD[2]'s x) -- push the whole
        # column down by one stage so it clears the BCD input row above it.
        not_gates = []
        for i in range(4):
            element_id = await self.create_element(
                "Not", not_gate_x, input_y + ((i + 1) * VERTICAL_STAGE_SPACING), f"not_bcd{i}"
            )
            if element_id is None:
                return False
            not_gates.append(element_id)

            # Connect BCD input to NOT gate
            if not await self.connect(bcd_inputs[i], element_id):
                return False

        # Create AND gates for digit detection (one per digit 0-9)
        # Each AND gate detects a specific BCD input value
        # Pattern indices: [BCD[0], BCD[1], BCD[2], BCD[3]] (LSB to MSB)
        digit_detectors = []
        digit_patterns = [
            # Digit: which inputs are 1 (True) vs 0 (False) for BCD[0-3]
            (False, False, False, False),  # 0: 0000
            (True, False, False, False),  # 1: 0001
            (False, True, False, False),  # 2: 0010
            (True, True, False, False),  # 3: 0011
            (False, False, True, False),  # 4: 0100
            (True, False, True, False),  # 5: 0101
            (False, True, True, False),  # 6: 0110
            (True, True, True, False),  # 7: 0111
            (False, False, False, True),  # 8: 1000
            (True, False, False, True),  # 9: 1001
        ]

        # AND gate (digit detector) positions - arrange 10 detectors vertically or in grid
        # Same reasoning as the NOT-gate column: digit_detector_x coincides with
        # BCD[3]'s x, so row 0 needs to clear the BCD input row above it.
        digit_detector_x = not_gate_x + HORIZONTAL_GATE_SPACING
        for digit, pattern in enumerate(digit_patterns):
            # Stack digit detectors vertically
            digit_y = input_y + ((digit + 1) * VERTICAL_STAGE_SPACING)
            element_id = await self.create_element("And", digit_detector_x, digit_y, f"digit_{digit}")
            if element_id is None:
                return False
            digit_detectors.append(element_id)

            # Change input size to 4
            response = await self.mcp.send_command("change_input_size", {"element_id": element_id, "size": 4})
            if not response.success:
                self.log_error(f"Failed to change input size for digit_{digit}: {response.error}")
                return False

            # Connect BCD inputs to AND gate
            # If pattern[i] is True, use BCD[i]
            # If pattern[i] is False, use NOT(BCD[i])
            for bit_idx, bit_is_one in enumerate(pattern):
                source_id = bcd_inputs[bit_idx] if bit_is_one else not_gates[bit_idx]
                if not await self.connect(source_id, element_id, target_port=bit_idx):
                    return False

        # Define which digits light up each segment
        # Based on the standard 7-segment truth table
        segment_digit_map = [
            [0, 2, 3, 5, 6, 7, 8, 9],  # Segment a: all except 1,4
            [0, 1, 2, 3, 4, 7, 8, 9],  # Segment b: all except 5,6
            [0, 1, 3, 4, 5, 6, 7, 8, 9],  # Segment c: all except 2
            [0, 2, 3, 5, 6, 8, 9],  # Segment d: all except 1,4,7
            [0, 2, 6, 8],  # Segment e: only these
            [0, 4, 5, 6, 8, 9],  # Segment f: except 1,2,3,7
            [2, 3, 4, 5, 6, 8, 9],  # Segment g: all except 0,1,7
        ]

        # OR gate positions
        or_gate_x = digit_detector_x + HORIZONTAL_GATE_SPACING

        # Segment OR gates can grow up to 8 inputs (segments a/b/c need exactly
        # or over 8 active digits), and a gate's boundingRect grows past the
        # stock 64px once it has more than 4 input ports (up to ~122px tall at
        # 8 inputs) -- a flat VERTICAL_STAGE_SPACING between rows isn't enough
        # for two adjacent wide gates, so double it for this column. Row 0 is
        # also pushed down one stage for extra clearance from the BCD input row
        # above, on top of the extra horizontal stage added to the gate-pipeline
        # columns (see not_gate_x) that keeps or_gate_x clear of every BCD
        # input's x.
        or_row_spacing = 2 * VERTICAL_STAGE_SPACING
        or_row_y0 = output_y + VERTICAL_STAGE_SPACING

        # Create OR gates for each segment and connect digit detectors
        for segment_idx, segment_name in enumerate(segment_names):
            active_digits = segment_digit_map[segment_idx]

            # Handle case where more than 8 inputs needed (OR gate max is 8)
            if len(active_digits) <= 8:
                # Single OR gate is sufficient
                or_gate_id = await self.create_element(
                    "Or", or_gate_x, or_row_y0 + (segment_idx * or_row_spacing), f"or_seg_{segment_name}"
                )
                if or_gate_id is None:
                    return False

                # Change input size to match number of active digits
                if len(active_digits) > 2:
                    response = await self.mcp.send_command(
                        "change_input_size", {"element_id": or_gate_id, "size": len(active_digits)}
                    )
                    if not response.success:
                        self.log_error(f"Failed to change input size for OR[{segment_name}]: {response.error}")
                        return False

                # Connect digit detectors to OR gate
                for input_port, digit_idx in enumerate(active_digits):
                    if not await self.connect(digit_detectors[digit_idx], or_gate_id, target_port=input_port):
                        return False

                # Connect OR output to segment LED
                if not await self.connect(or_gate_id, segment_outputs[segment_idx]):
                    return False
            else:
                # Multiple OR gates needed for >8 inputs
                # Create first OR gate with 8 inputs
                or_gate_1 = await self.create_element(
                    "Or", or_gate_x, or_row_y0 + (segment_idx * or_row_spacing), f"or_seg_{segment_name}_1"
                )
                if or_gate_1 is None:
                    return False

                response = await self.mcp.send_command("change_input_size", {"element_id": or_gate_1, "size": 8})
                if not response.success:
                    self.log_error(f"Failed to set input size for OR[{segment_name}]_1: {response.error}")
                    return False

                # Connect first 8 digit detectors
                for input_port in range(8):
                    if not await self.connect(
                        digit_detectors[active_digits[input_port]], or_gate_1, target_port=input_port
                    ):
                        return False

                # Create second OR gate with remaining inputs
                remaining_digits = active_digits[8:]
                or_gate_2 = await self.create_element(
                    "Or",
                    or_gate_x + HORIZONTAL_GATE_SPACING,
                    or_row_y0 + (segment_idx * or_row_spacing),
                    f"or_seg_{segment_name}_2",
                )
                if or_gate_2 is None:
                    return False

                input_size_2 = len(remaining_digits) + 1  # remaining digits + output of OR gate 1
                response = await self.mcp.send_command(
                    "change_input_size", {"element_id": or_gate_2, "size": input_size_2}
                )
                if not response.success:
                    self.log_error(f"Failed to set input size for OR[{segment_name}]_2: {response.error}")
                    return False

                # Connect remaining digit detectors
                for input_port, digit_idx in enumerate(remaining_digits):
                    if not await self.connect(digit_detectors[digit_idx], or_gate_2, target_port=input_port):
                        return False

                # Connect first OR output to second OR gate
                if not await self.connect(or_gate_1, or_gate_2, target_port=len(remaining_digits)):
                    return False

                # Connect final OR output to segment LED
                if not await self.connect(or_gate_2, segment_outputs[segment_idx]):
                    return False

        # Save circuit as IC
        output_file = str(IC_COMPONENTS_DIR / "level3_bcd_7segment_decoder.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(
            f"✅ Successfully created BCD to 7-Segment Decoder IC"
            f"({self.element_count} elements, {self.connection_count} connections)"
        )
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point for the builder"""
    builder = BCD7SegmentDecoderBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import sys
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "BCD to 7-Segment Decoder IC"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
