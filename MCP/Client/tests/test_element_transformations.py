#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Element Transformation Tests

Tests for element transformation and configuration commands:
- rotate_element
- flip_element
- morph_element
- change_input_size / change_output_size
- toggle_truth_table_output

MCP test implementation
"""

from beartype import beartype

from tests.mcp_test_base import MCPTestBase


class ElementTransformationTests(MCPTestBase):
    """Tests for element transformation operations"""

    async def run_category_tests(self) -> bool:
        tests = [
            self.test_rotate_element,
            self.test_flip_element,
            self.test_morph_element,
            self.test_morph_display14_to_display7_removes_dangling_connections,
            self.test_change_input_size,
            self.test_change_output_size,
            self.test_toggle_truth_table_output,
        ]

        print("\n" + "=" * 60)
        print("RUNNING ELEMENT TRANSFORMATION TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    @beartype
    async def test_rotate_element(self) -> bool:
        """Test rotate_element through all valid angles"""
        print("\n=== Rotate Element Test ===")
        self.set_test_context("test_rotate_element")

        all_passed: bool = True

        # Create an AND gate to rotate
        resp = await self.send_command(
            "create_element", {"type": "And", "x": 200, "y": 200, "label": "RotateTest"}
        )
        elem_id = await self.validate_element_creation_response(resp, "Create element for rotation")
        if elem_id is None:
            return False

        # Test rotation at 90 degrees
        resp = await self.send_command("rotate_element", {"element_id": elem_id, "angle": 90})
        result = await self.assert_success_and_get_result(resp, "Rotate 90 degrees")
        if result:
            if result.get("angle") == 90:
                self.infrastructure.output.success("Rotate 90: angle matches")
            else:
                print(f"Rotate 90: expected angle=90, got {result.get('angle')}")
                all_passed = False
        else:
            all_passed = False

        # Test rotation at 180 degrees
        resp = await self.send_command("rotate_element", {"element_id": elem_id, "angle": 180})
        result = await self.assert_success_and_get_result(resp, "Rotate 180 degrees")
        if result:
            if result.get("angle") == 180:
                self.infrastructure.output.success("Rotate 180: angle matches")
            else:
                print(f"Rotate 180: expected angle=180, got {result.get('angle')}")
                all_passed = False
        else:
            all_passed = False

        # Test rotation at 270 degrees
        resp = await self.send_command("rotate_element", {"element_id": elem_id, "angle": 270})
        all_passed &= await self.assert_success(resp, "Rotate 270 degrees")

        # Test rotation at 0 degrees (identity)
        resp = await self.send_command("rotate_element", {"element_id": elem_id, "angle": 0})
        all_passed &= await self.assert_success(resp, "Rotate 0 degrees")

        # Test negative angle (should normalize to positive)
        resp = await self.send_command("rotate_element", {"element_id": elem_id, "angle": -90})
        result = await self.assert_success_and_get_result(resp, "Rotate -90 degrees (normalize)")
        if result:
            if result.get("angle") == 270:
                self.infrastructure.output.success("Rotate -90: normalized to 270")
            else:
                print(f"Rotate -90: expected normalized angle=270, got {result.get('angle')}")
                all_passed = False
        else:
            all_passed = False

        # Verify element still exists and is listable after rotations
        resp = await self.send_command("list_elements", {})
        all_passed &= await self.assert_success(resp, "List elements after rotation")

        return all_passed

    @beartype
    async def test_flip_element(self) -> bool:
        """Test flip_element horizontal and vertical"""
        print("\n=== Flip Element Test ===")
        self.set_test_context("test_flip_element")

        all_passed: bool = True

        # Create element and connect it to verify connections survive flip
        resp = await self.send_command(
            "create_element", {"type": "And", "x": 200, "y": 200, "label": "FlipTest"}
        )
        gate_id = await self.validate_element_creation_response(resp, "Create gate for flip")
        if gate_id is None:
            return False

        resp = await self.send_command(
            "create_element", {"type": "InputButton", "x": 100, "y": 200, "label": "FlipInput"}
        )
        input_id = await self.validate_element_creation_response(resp, "Create input for flip")
        if input_id is None:
            return False

        # Connect input to gate
        resp = await self.send_command(
            "connect_elements",
            {"source_id": input_id, "source_port": 0, "target_id": gate_id, "target_port": 0},
        )
        all_passed &= await self.assert_success(resp, "Connect before flip")

        # Flip horizontal (axis=0)
        resp = await self.send_command("flip_element", {"element_id": gate_id, "axis": 0})
        result = await self.assert_success_and_get_result(resp, "Flip horizontal")
        if result:
            if result.get("axis") == "horizontal":
                self.infrastructure.output.success("Flip horizontal: axis correct")
            else:
                print(f"Flip horizontal: expected axis='horizontal', got {result.get('axis')}")
                all_passed = False
        else:
            all_passed = False

        # Flip vertical (axis=1)
        resp = await self.send_command("flip_element", {"element_id": gate_id, "axis": 1})
        result = await self.assert_success_and_get_result(resp, "Flip vertical")
        if result:
            if result.get("axis") == "vertical":
                self.infrastructure.output.success("Flip vertical: axis correct")
            else:
                print(f"Flip vertical: expected axis='vertical', got {result.get('axis')}")
                all_passed = False
        else:
            all_passed = False

        # Test invalid axis value
        resp = await self.send_command("flip_element", {"element_id": gate_id, "axis": 2})
        all_passed &= await self.assert_failure(resp, "Flip with invalid axis=2")

        # Verify connection still exists after flips
        resp = await self.send_command("list_connections", {})
        result = await self.assert_success_and_get_result(resp, "List connections after flip")
        if result:
            connections = result.get("connections", [])
            if len(connections) >= 1:
                self.infrastructure.output.success(f"Connections survived flip: {len(connections)}")
            else:
                print(f"Connection lost after flip: {len(connections)} connections")
                all_passed = False
        else:
            all_passed = False

        return all_passed

    @beartype
    async def test_morph_element(self) -> bool:
        """Test morph_element type transformation"""
        print("\n=== Morph Element Test ===")
        self.set_test_context("test_morph_element")

        all_passed: bool = True

        # Create an AND gate to morph
        resp = await self.send_command(
            "create_element", {"type": "And", "x": 200, "y": 200, "label": "MorphTest"}
        )
        and_id = await self.validate_element_creation_response(resp, "Create AND for morph")
        if and_id is None:
            return False

        # Morph AND -> OR
        resp = await self.send_command(
            "morph_element", {"element_ids": [and_id], "target_type": "Or"}
        )
        result = await self.assert_success_and_get_result(resp, "Morph AND to OR")
        if result:
            if result.get("target_type") == "Or":
                self.infrastructure.output.success("Morph: target_type is Or")
            else:
                print(f"Morph: expected target_type='Or', got {result.get('target_type')}")
                all_passed = False

            morphed_ids = result.get("morphed_elements", [])
            if len(morphed_ids) == 1:
                self.infrastructure.output.success(f"Morph: got 1 new element id={morphed_ids[0]}")
            else:
                print(f"Morph: expected 1 morphed element, got {len(morphed_ids)}")
                all_passed = False
        else:
            all_passed = False

        # Verify the new element is listed as Or type
        resp = await self.send_command("list_elements", {})
        result = await self.assert_success_and_get_result(resp, "List after morph")
        if result:
            elements = result.get("elements", [])
            or_elements = [e for e in elements if e.get("type") == "Or"]
            if len(or_elements) >= 1:
                self.infrastructure.output.success("Morph verified: Or element exists in scene")
            else:
                print("Morph verification failed: no Or element found")
                all_passed = False
        else:
            all_passed = False

        # Test morph with invalid target type
        # Create a fresh element first
        resp = await self.send_command(
            "create_element", {"type": "Not", "x": 300, "y": 200, "label": "MorphInvalid"}
        )
        not_id = await self.validate_element_creation_response(resp, "Create NOT for invalid morph")
        if not_id is not None:
            resp = await self.send_command(
                "morph_element", {"element_ids": [not_id], "target_type": "NonExistentType"}
            )
            all_passed &= await self.assert_failure(resp, "Morph to invalid type")

        # Test morph with empty element_ids
        resp = await self.send_command(
            "morph_element", {"element_ids": [], "target_type": "And"}
        )
        all_passed &= await self.assert_failure(resp, "Morph with empty element_ids")

        return all_passed

    @beartype
    async def test_morph_display14_to_display7_removes_dangling_connections(self) -> bool:
        """Regression test: morphing Display14 to Display7 must remove connections on dropped ports.

        Bug: When Display14 (15 inputs) is morphed to Display7 (8 inputs), connections
        wired to ports 8-14 are not removed — the wire remains but the port is gone.
        """
        print("\n=== Morph Display14->Display7: dangling connection cleanup ===")
        self.set_test_context("test_morph_display14_to_display7_removes_dangling_connections")

        all_passed: bool = True

        # Create a Display14 (15 inputs: 14 segments + decimal point, ports 0-14)
        resp = await self.send_command(
            "create_element", {"type": "Display14", "x": 300, "y": 200, "label": "Disp14"}
        )
        disp14_id = await self.validate_element_creation_response(resp, "Create Display14")
        if disp14_id is None:
            return False

        # Create an InputSwitch to drive the last port of the Display14
        resp = await self.send_command(
            "create_element", {"type": "InputSwitch", "x": 100, "y": 200, "label": "Sw"}
        )
        sw_id = await self.validate_element_creation_response(resp, "Create InputSwitch")
        if sw_id is None:
            return False

        # Connect the switch output (port 0) to the LAST input port of Display14 (port 14)
        resp = await self.send_command(
            "connect_elements",
            {"source_id": sw_id, "source_port": 0, "target_id": disp14_id, "target_port": 14},
        )
        all_passed &= await self.assert_success(resp, "Connect switch to Display14 port 14")

        # Verify the connection exists before morph
        resp = await self.send_command("list_connections", {})
        result = await self.assert_success_and_get_result(resp, "List connections before morph")
        if result:
            connections_before = result.get("connections", [])
            if len(connections_before) == 1:
                self.infrastructure.output.success(
                    "Connection to Display14 port 14 confirmed before morph"
                )
            else:
                print(f"Expected 1 connection before morph, got {len(connections_before)}")
                all_passed = False
        else:
            all_passed = False

        # Morph Display14 -> Display7 (8 inputs: 7 segments + decimal point, ports 0-7)
        # Ports 8-14 are removed; the connection to port 14 must be cleaned up
        resp = await self.send_command(
            "morph_element", {"element_ids": [disp14_id], "target_type": "Display7"}
        )
        result = await self.assert_success_and_get_result(resp, "Morph Display14 to Display7")
        if result is None:
            return False

        if result.get("target_type") == "Display7":
            self.infrastructure.output.success("Morph: target_type is Display7")
        else:
            print(f"Morph: expected target_type='Display7', got {result.get('target_type')}")
            all_passed = False

        # After morph, the connection to the removed port must no longer exist
        resp = await self.send_command("list_connections", {})
        result = await self.assert_success_and_get_result(
            resp, "List connections after morph (expect 0)"
        )
        if result:
            connections_after = result.get("connections", [])
            if len(connections_after) == 0:
                self.infrastructure.output.success(
                    "Dangling connection correctly removed after morph"
                )
            else:
                print(
                    f"BUG: {len(connections_after)} dangling connection(s) remain after morph "
                    f"to Display7 — port was removed but wire was not"
                )
                all_passed = False
        else:
            all_passed = False

        return all_passed

    @beartype
    async def test_change_input_size(self) -> bool:
        """Test change_input_size for variable-port elements"""
        print("\n=== Change Input Size Test ===")
        self.set_test_context("test_change_input_size")

        all_passed: bool = True

        # AND gate: minInput=2, maxInput=8
        resp = await self.send_command(
            "create_element", {"type": "And", "x": 200, "y": 200, "label": "InputSizeTest"}
        )
        and_id = await self.validate_element_creation_response(resp, "Create AND for input resize")
        if and_id is None:
            return False

        # Increase to 4 inputs
        resp = await self.send_command(
            "change_input_size", {"element_id": and_id, "size": 4}
        )
        result = await self.assert_success_and_get_result(resp, "Change AND inputs to 4")
        if result:
            if result.get("new_size") == 4:
                self.infrastructure.output.success("Input size changed to 4")
            else:
                print(f"Expected new_size=4, got {result.get('new_size')}")
                all_passed = False
        else:
            all_passed = False

        # Increase to max (8 inputs)
        resp = await self.send_command(
            "change_input_size", {"element_id": and_id, "size": 8}
        )
        all_passed &= await self.assert_success(resp, "Change AND inputs to 8 (max)")

        # Decrease back to 2 (min)
        resp = await self.send_command(
            "change_input_size", {"element_id": and_id, "size": 2}
        )
        all_passed &= await self.assert_success(resp, "Change AND inputs to 2 (min)")

        # Test with Demux (minInput=2, maxInput=4)
        resp = await self.send_command(
            "create_element", {"type": "Demux", "x": 400, "y": 200, "label": "DemuxResize"}
        )
        demux_id = await self.validate_element_creation_response(resp, "Create Demux for input resize")
        if demux_id is not None:
            resp = await self.send_command(
                "change_input_size", {"element_id": demux_id, "size": 3}
            )
            all_passed &= await self.assert_success(resp, "Change Demux inputs to 3")

        return all_passed

    @beartype
    async def test_change_output_size(self) -> bool:
        """Test change_output_size for variable-port elements"""
        print("\n=== Change Output Size Test ===")
        self.set_test_context("test_change_output_size")

        all_passed: bool = True

        # Demux: minOutput=2, maxOutput=8, default output=2
        resp = await self.send_command(
            "create_element", {"type": "Demux", "x": 200, "y": 200, "label": "OutputSizeTest"}
        )
        demux_id = await self.validate_element_creation_response(resp, "Create Demux for output resize")
        if demux_id is None:
            return False

        # Increase to 4 outputs
        resp = await self.send_command(
            "change_output_size", {"element_id": demux_id, "size": 4}
        )
        result = await self.assert_success_and_get_result(resp, "Change Demux outputs to 4")
        if result:
            if result.get("new_size") == 4:
                self.infrastructure.output.success("Output size changed to 4")
            else:
                print(f"Expected new_size=4, got {result.get('new_size')}")
                all_passed = False
        else:
            all_passed = False

        # Increase to max (8 outputs)
        resp = await self.send_command(
            "change_output_size", {"element_id": demux_id, "size": 8}
        )
        all_passed &= await self.assert_success(resp, "Change Demux outputs to 8 (max)")

        # Decrease back to min (2)
        resp = await self.send_command(
            "change_output_size", {"element_id": demux_id, "size": 2}
        )
        all_passed &= await self.assert_success(resp, "Change Demux outputs to 2 (min)")

        return all_passed

    @beartype
    async def test_toggle_truth_table_output(self) -> bool:
        """Test toggle_truth_table_output for TruthTable elements"""
        print("\n=== Toggle Truth Table Output Test ===")
        self.set_test_context("test_toggle_truth_table_output")

        all_passed: bool = True

        # Create a TruthTable element
        resp = await self.send_command(
            "create_element", {"type": "TruthTable", "x": 200, "y": 200, "label": "TTTest"}
        )
        tt_id = await self.validate_element_creation_response(resp, "Create TruthTable")
        if tt_id is None:
            return False

        # Toggle position 0
        resp = await self.send_command(
            "toggle_truth_table_output", {"element_id": tt_id, "position": 0}
        )
        result = await self.assert_success_and_get_result(resp, "Toggle truth table position 0")
        if result:
            if result.get("position") == 0:
                self.infrastructure.output.success("Toggle position 0: correct")
            else:
                print(f"Toggle: expected position=0, got {result.get('position')}")
                all_passed = False
        else:
            all_passed = False

        # Toggle position 1
        resp = await self.send_command(
            "toggle_truth_table_output", {"element_id": tt_id, "position": 1}
        )
        all_passed &= await self.assert_success(resp, "Toggle truth table position 1")

        # Test toggle on non-TruthTable element (should fail or handle gracefully)
        resp = await self.send_command(
            "create_element", {"type": "And", "x": 400, "y": 200, "label": "NotTT"}
        )
        and_id = await self.validate_element_creation_response(resp, "Create AND for TT toggle test")
        if and_id is not None:
            resp = await self.send_command(
                "toggle_truth_table_output", {"element_id": and_id, "position": 0}
            )
            # This may succeed or fail depending on implementation -- just verify no crash
            if resp.success:
                self.infrastructure.output.success("Toggle on non-TT element: handled gracefully")
            else:
                self.infrastructure.output.success(f"Toggle on non-TT element: rejected as expected")

        return all_passed
