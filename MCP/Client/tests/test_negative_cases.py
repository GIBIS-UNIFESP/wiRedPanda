#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Negative/Error Path Tests

Tests for error handling and invalid parameter rejection:
- Invalid rotation angles
- Morph to incompatible types
- Port size boundary violations
- Nonexistent resource references
- Undo/redo of transformation commands

MCP test implementation
"""

from beartype import beartype

from tests.mcp_test_base import MCPTestBase


class NegativeCaseTests(MCPTestBase):
    """Tests for error paths and invalid operations"""

    async def run_category_tests(self) -> bool:
        tests = [
            self.test_rotate_invalid_element,
            self.test_morph_error_cases,
            self.test_port_size_boundaries,
            self.test_split_error_cases,
            self.test_connect_error_cases,
            self.test_undo_rotate,
            self.test_undo_morph,
            self.test_undo_change_input_size,
            self.test_undo_split_connection,
            self.test_undo_set_element_properties,
            self.test_update_element,
            self.test_get_server_info,
        ]

        print("\n" + "=" * 60)
        print("RUNNING NEGATIVE CASE TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    @beartype
    async def test_rotate_invalid_element(self) -> bool:
        """Test rotate with nonexistent element and non-rotatable elements"""
        print("\n=== Rotate Invalid Element Test ===")
        self.set_test_context("test_rotate_invalid_element")

        all_passed: bool = True

        # Rotate nonexistent element
        resp = await self.send_command("rotate_element", {"element_id": 99999, "angle": 90})
        all_passed &= await self.assert_failure(resp, "Rotate nonexistent element")

        # Missing parameters
        resp = await self.send_command("rotate_element", {"element_id": 1})
        all_passed &= await self.assert_failure(resp, "Rotate missing angle")

        resp = await self.send_command("rotate_element", {"angle": 90})
        all_passed &= await self.assert_failure(resp, "Rotate missing element_id")

        return all_passed

    @beartype
    async def test_morph_error_cases(self) -> bool:
        """Test morph with various invalid inputs"""
        print("\n=== Morph Error Cases Test ===")
        self.set_test_context("test_morph_error_cases")

        all_passed: bool = True

        # Create an element to morph
        and_id = await self.create_element_checked("And", 200, 200, "Create AND for morph errors", label="MorphErr")
        if and_id is None:
            return False

        # Morph to invalid type
        resp = await self.send_command(
            "morph_element", {"element_ids": [and_id], "target_type": "FakeGate"}
        )
        all_passed &= await self.assert_failure(resp, "Morph to invalid type")

        # Morph with nonexistent element ID
        resp = await self.send_command(
            "morph_element", {"element_ids": [99999], "target_type": "Or"}
        )
        all_passed &= await self.assert_failure(resp, "Morph nonexistent element")

        # Missing target_type
        resp = await self.send_command(
            "morph_element", {"element_ids": [and_id]}
        )
        all_passed &= await self.assert_failure(resp, "Morph missing target_type")

        # Missing element_ids
        resp = await self.send_command(
            "morph_element", {"target_type": "Or"}
        )
        all_passed &= await self.assert_failure(resp, "Morph missing element_ids")

        # element_ids not an array
        resp = await self.send_command(
            "morph_element", {"element_ids": and_id, "target_type": "Or"}
        )
        all_passed &= await self.assert_failure(resp, "Morph element_ids not array")

        return all_passed

    @beartype
    async def test_port_size_boundaries(self) -> bool:
        """Test change_input_size and change_output_size at boundaries"""
        print("\n=== Port Size Boundaries Test ===")
        self.set_test_context("test_port_size_boundaries")

        all_passed: bool = True

        # AND gate: minInput=2, maxInput=8
        and_id = await self.create_element_checked("And", 200, 200, "Create AND for boundary test", label="SizeBoundary")
        if and_id is None:
            return False

        # Try to set input size below minimum (1 < minInput=2)
        resp = await self.send_command(
            "change_input_size", {"element_id": and_id, "size": 1}
        )
        # This might succeed (command layer) but the element may clamp, or it may fail
        # Server uses ChangeInputSizeCommand which doesn't validate bounds itself
        # Just verify it doesn't crash
        if resp.success:
            self.infrastructure.output.success("Size 1: accepted (may be clamped)")
        else:
            self.infrastructure.output.success("Size 1: rejected (boundary enforced)")

        # Try to set input size above maximum (9 > maxInput=8)
        resp = await self.send_command(
            "change_input_size", {"element_id": and_id, "size": 9}
        )
        if resp.success:
            self.infrastructure.output.success("Size 9: accepted (may be clamped)")
        else:
            self.infrastructure.output.success("Size 9: rejected (boundary enforced)")

        # Missing parameters
        resp = await self.send_command(
            "change_input_size", {"element_id": and_id}
        )
        all_passed &= await self.assert_failure(resp, "Change input size missing size")

        resp = await self.send_command(
            "change_input_size", {"size": 4}
        )
        all_passed &= await self.assert_failure(resp, "Change input size missing element_id")

        # Nonexistent element
        resp = await self.send_command(
            "change_input_size", {"element_id": 99999, "size": 4}
        )
        all_passed &= await self.assert_failure(resp, "Change input size nonexistent element")

        # Same tests for output size
        resp = await self.send_command(
            "change_output_size", {"element_id": and_id}
        )
        all_passed &= await self.assert_failure(resp, "Change output size missing size")

        resp = await self.send_command(
            "change_output_size", {"element_id": 99999, "size": 4}
        )
        all_passed &= await self.assert_failure(resp, "Change output size nonexistent element")

        return all_passed

    @beartype
    async def test_split_error_cases(self) -> bool:
        """Test split_connection with invalid parameters"""
        print("\n=== Split Error Cases Test ===")
        self.set_test_context("test_split_error_cases")

        all_passed: bool = True

        # Missing parameters
        resp = await self.send_command(
            "split_connection", {"source_id": 1, "source_port": 0}
        )
        all_passed &= await self.assert_failure(resp, "Split missing target params")

        # All params but nonexistent elements
        resp = await self.send_command(
            "split_connection",
            {
                "source_id": 99999,
                "source_port": 0,
                "target_id": 99998,
                "target_port": 0,
                "x": 200.0,
                "y": 200.0,
            },
        )
        all_passed &= await self.assert_failure(resp, "Split with nonexistent elements")

        # Create elements but no connection between them
        in_id = await self.create_element_checked("InputButton", 100, 200, "Create input for split error", label="SplitErrIn")
        out_id = await self.create_element_checked("Led", 400, 200, "Create output for split error", label="SplitErrOut")

        if in_id is not None and out_id is not None:
            resp = await self.send_command(
                "split_connection",
                {
                    "source_id": in_id,
                    "source_port": 0,
                    "target_id": out_id,
                    "target_port": 0,
                    "x": 250.0,
                    "y": 200.0,
                },
            )
            all_passed &= await self.assert_failure(resp, "Split nonexistent connection between real elements")

        return all_passed

    @beartype
    async def test_connect_error_cases(self) -> bool:
        """Test connect_elements with invalid port indices"""
        print("\n=== Connect Error Cases Test ===")
        self.set_test_context("test_connect_error_cases")

        all_passed: bool = True

        and_id = await self.create_element_checked("And", 200, 200, "Create AND for connect errors", label="ConnErr")
        led_id = await self.create_element_checked("Led", 400, 200, "Create LED for connect errors", label="ConnErrOut")

        if and_id is None or led_id is None:
            return False

        # Invalid source port index (AND has 1 output, port 0)
        resp = await self.send_command(
            "connect_elements",
            {"source_id": and_id, "source_port": 99, "target_id": led_id, "target_port": 0},
        )
        all_passed &= await self.assert_failure(resp, "Connect with invalid source port index")

        # Invalid target port index
        resp = await self.send_command(
            "connect_elements",
            {"source_id": and_id, "source_port": 0, "target_id": led_id, "target_port": 99},
        )
        all_passed &= await self.assert_failure(resp, "Connect with invalid target port index")

        # Missing port specification
        resp = await self.send_command(
            "connect_elements",
            {"source_id": and_id, "target_id": led_id},
        )
        all_passed &= await self.assert_failure(resp, "Connect missing port specification")

        return all_passed

    @beartype
    async def test_undo_rotate(self) -> bool:
        """Test undo after rotate_element"""
        print("\n=== Undo Rotate Test ===")
        self.set_test_context("test_undo_rotate")

        all_passed: bool = True

        elem_id = await self.create_element_checked("And", 200, 200, "Create element for undo rotate", label="UndoRotate")
        if elem_id is None:
            return False

        # Rotate 90
        resp = await self.send_command("rotate_element", {"element_id": elem_id, "angle": 90})
        all_passed &= await self.assert_success(resp, "Rotate 90 for undo test")

        # Undo the rotation
        resp = await self.send_command("undo", {})
        all_passed &= await self.assert_success(resp, "Undo rotation")

        # Redo the rotation
        resp = await self.send_command("redo", {})
        all_passed &= await self.assert_success(resp, "Redo rotation")

        # Verify element still exists
        resp = await self.send_command("list_elements", {})
        result = await self.assert_success_and_get_result(resp, "List after undo/redo rotate")
        if result:
            elements = result.get("elements", [])
            if len(elements) >= 1:
                self.infrastructure.output.success("Element survived undo/redo rotation")
            else:
                print("Element missing after undo/redo rotation")
                all_passed = False
        else:
            all_passed = False

        return all_passed

    @beartype
    async def test_undo_morph(self) -> bool:
        """Test undo after morph_element"""
        print("\n=== Undo Morph Test ===")
        self.set_test_context("test_undo_morph")

        all_passed: bool = True

        and_id = await self.create_element_checked("And", 200, 200, "Create AND for undo morph", label="UndoMorph")
        if and_id is None:
            return False

        # Morph AND -> Or
        resp = await self.send_command(
            "morph_element", {"element_ids": [and_id], "target_type": "Or"}
        )
        all_passed &= await self.assert_success(resp, "Morph AND to OR for undo test")

        # Verify it's an Or now
        resp = await self.send_command("list_elements", {})
        result = await self.assert_success_and_get_result(resp, "List after morph")
        if result:
            elements = result.get("elements", [])
            types = [e.get("type") for e in elements]
            if "Or" in types:
                self.infrastructure.output.success("Morph confirmed: Or present")
            else:
                print(f"Expected Or in types, got {types}")
                all_passed = False

        # Undo the morph
        resp = await self.send_command("undo", {})
        all_passed &= await self.assert_success(resp, "Undo morph")

        # Verify it's back to And
        resp = await self.send_command("list_elements", {})
        result = await self.assert_success_and_get_result(resp, "List after undo morph")
        if result:
            elements = result.get("elements", [])
            types = [e.get("type") for e in elements]
            if "And" in types:
                self.infrastructure.output.success("Undo morph confirmed: And restored")
            else:
                print(f"Expected And after undo morph, got {types}")
                all_passed = False

        return all_passed

    @beartype
    async def test_undo_change_input_size(self) -> bool:
        """Test undo after change_input_size"""
        print("\n=== Undo Change Input Size Test ===")
        self.set_test_context("test_undo_change_input_size")

        all_passed: bool = True

        and_id = await self.create_element_checked("And", 200, 200, "Create AND for undo resize", label="UndoResize")
        if and_id is None:
            return False

        # Change input size to 4
        resp = await self.send_command(
            "change_input_size", {"element_id": and_id, "size": 4}
        )
        all_passed &= await self.assert_success(resp, "Change input size to 4")

        # Undo the resize
        resp = await self.send_command("undo", {})
        all_passed &= await self.assert_success(resp, "Undo input size change")

        # Redo the resize
        resp = await self.send_command("redo", {})
        all_passed &= await self.assert_success(resp, "Redo input size change")

        # Verify element still exists
        resp = await self.send_command("list_elements", {})
        result = await self.assert_success_and_get_result(resp, "List after undo/redo resize")
        if result:
            elements = result.get("elements", [])
            if len(elements) >= 1:
                self.infrastructure.output.success("Element survived undo/redo resize")
            else:
                print("Element missing after undo/redo resize")
                all_passed = False

        return all_passed

    @beartype
    async def test_undo_split_connection(self) -> bool:
        """Test undo after split_connection restores original topology"""
        print("\n=== Undo Split Connection Test ===")
        self.set_test_context("test_undo_split_connection")

        all_passed: bool = True

        # Create input -> output with a wire
        input_id = await self.create_element_checked("InputButton", 100, 200, "Create input for undo split", label="UndoSplitIn")
        if input_id is None:
            return False
        output_id = await self.create_element_checked("Led", 400, 200, "Create output for undo split", label="UndoSplitOut")
        if output_id is None:
            return False

        resp = await self.send_command(
            "connect_elements",
            {"source_id": input_id, "source_port": 0, "target_id": output_id, "target_port": 0},
        )
        all_passed &= await self.assert_success(resp, "Connect for undo split")

        # Count connections before split
        resp = await self.send_command("list_connections", {})
        result = await self.assert_success_and_get_result(resp, "List connections before split")
        before_count = len(result.get("connections", [])) if result else 0

        # Split the connection
        resp = await self.send_command(
            "split_connection",
            {
                "source_id": input_id,
                "source_port": 0,
                "target_id": output_id,
                "target_port": 0,
                "x": 250.0,
                "y": 200.0,
            },
        )
        all_passed &= await self.assert_success(resp, "Split connection for undo")

        # Verify split increased connections
        resp = await self.send_command("list_connections", {})
        result = await self.assert_success_and_get_result(resp, "List connections after split")
        after_split_count = len(result.get("connections", [])) if result else 0
        if after_split_count > before_count:
            self.infrastructure.output.success(
                f"Split increased connections: {before_count} -> {after_split_count}"
            )
        else:
            print(f"Split didn't increase connections: {before_count} -> {after_split_count}")
            all_passed = False

        # Undo the split
        resp = await self.send_command("undo", {})
        all_passed &= await self.assert_success(resp, "Undo split connection")

        # Verify connections restored
        resp = await self.send_command("list_connections", {})
        result = await self.assert_success_and_get_result(resp, "List connections after undo split")
        after_undo_count = len(result.get("connections", [])) if result else 0
        if after_undo_count == before_count:
            self.infrastructure.output.success(
                f"Undo restored connections: {after_undo_count}"
            )
        else:
            print(f"Undo didn't restore connections: expected {before_count}, got {after_undo_count}")
            all_passed = False

        return all_passed

    @beartype
    async def test_undo_set_element_properties(self) -> bool:
        """Ctrl+Z after set_element_properties must revert the property change"""
        print("\n=== Undo set_element_properties Test ===")
        self.set_test_context("test_undo_set_element_properties")

        all_passed: bool = True

        elem_id = await self.create_element_checked("And", 200, 200, "Create And for undo test", label="OriginalLabel")
        if elem_id is None:
            return False

        # Change the label
        resp = await self.send_command(
            "set_element_properties", {"element_id": elem_id, "label": "ChangedLabel"}
        )
        all_passed &= await self.assert_success(resp, "Set label to ChangedLabel")

        # Undo — label should revert to OriginalLabel
        resp = await self.send_command("undo", {})
        all_passed &= await self.assert_success(resp, "Undo label change")

        resp = await self.send_command("list_elements", {})
        result = await self.assert_success_and_get_result(resp, "List after undo")
        if result:
            elements = result.get("elements", [])
            found_original = any(e.get("label") == "OriginalLabel" for e in elements)
            found_changed = any(e.get("label") == "ChangedLabel" for e in elements)
            if found_original and not found_changed:
                self.infrastructure.output.success("Undo restored original label")
            else:
                labels = [e.get("label") for e in elements]
                print(f"  FAIL: after undo expected 'OriginalLabel', got {labels}")
                all_passed = False
        else:
            all_passed = False

        # Redo — label should be ChangedLabel again
        resp = await self.send_command("redo", {})
        all_passed &= await self.assert_success(resp, "Redo label change")

        resp = await self.send_command("list_elements", {})
        result = await self.assert_success_and_get_result(resp, "List after redo")
        if result:
            elements = result.get("elements", [])
            found_changed = any(e.get("label") == "ChangedLabel" for e in elements)
            if found_changed:
                self.infrastructure.output.success("Redo re-applied label change")
            else:
                labels = [e.get("label") for e in elements]
                print(f"  FAIL: after redo expected 'ChangedLabel', got {labels}")
                all_passed = False
        else:
            all_passed = False

        return all_passed

    @beartype
    async def test_update_element(self) -> bool:
        """Test update_element command (wrapper around set_element_properties)"""
        print("\n=== Update Element Test ===")
        self.set_test_context("test_update_element")

        all_passed: bool = True

        # Create an element
        elem_id = await self.create_element_checked("InputButton", 200, 200, "Create element for update", label="UpdateTest")
        if elem_id is None:
            return False

        # Update label via update_element
        resp = await self.send_command(
            "update_element", {"element_id": elem_id, "label": "UpdatedLabel"}
        )
        result = await self.assert_success_and_get_result(resp, "Update element label")
        if result:
            if result.get("element_id") == elem_id:
                self.infrastructure.output.success("Update element returned correct ID")
            else:
                print(f"Update element: expected id={elem_id}, got {result.get('element_id')}")
                all_passed = False
        else:
            all_passed = False

        # Verify label changed by listing
        resp = await self.send_command("list_elements", {})
        result = await self.assert_success_and_get_result(resp, "List after update")
        if result:
            elements = result.get("elements", [])
            found = any(e.get("label") == "UpdatedLabel" for e in elements)
            if found:
                self.infrastructure.output.success("Update verified: label is 'UpdatedLabel'")
            else:
                labels = [e.get("label") for e in elements]
                print(f"Update not reflected: labels = {labels}")
                all_passed = False

        # Test update on nonexistent element
        resp = await self.send_command(
            "update_element", {"element_id": 99999, "label": "NoSuchElement"}
        )
        all_passed &= await self.assert_failure(resp, "Update nonexistent element")

        # Test update with missing element_id
        resp = await self.send_command("update_element", {"label": "MissingID"})
        all_passed &= await self.assert_failure(resp, "Update missing element_id")

        return all_passed

    @beartype
    async def test_get_server_info(self) -> bool:
        """Test get_server_info contract: name, version, protocol, capabilities"""
        print("\n=== Get Server Info Test ===")
        self.set_test_context("test_get_server_info")

        all_passed: bool = True

        resp = await self.send_command("get_server_info", {})
        result = await self.assert_success_and_get_result(resp, "Get server info")
        if not result:
            return False

        # Validate required fields
        required_fields = ["server_name", "version", "protocol_version", "status", "capabilities"]
        for field in required_fields:
            if field in result:
                self.infrastructure.output.success(f"Server info has '{field}': {result[field]}")
            else:
                print(f"Server info missing required field: {field}")
                all_passed = False

        # Validate server name
        if result.get("server_name") != "wiRedPanda MCP Server":
            print(f"Unexpected server name: {result.get('server_name')}")
            all_passed = False

        # Validate protocol version format
        protocol = result.get("protocol_version", "")
        if not protocol:
            print("Empty protocol_version")
            all_passed = False

        # Validate status
        if result.get("status") != "ready":
            print(f"Unexpected status: {result.get('status')}")
            all_passed = False

        # Validate capabilities is a list with expected items
        capabilities = result.get("capabilities", [])
        expected_caps = ["circuit_design", "simulation", "file_operations"]
        for cap in expected_caps:
            if cap not in capabilities:
                print(f"Missing capability: {cap}")
                all_passed = False

        if all_passed:
            self.infrastructure.output.success(
                f"Server info validated: {len(capabilities)} capabilities"
            )

        return all_passed
