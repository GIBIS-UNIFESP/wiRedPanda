#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Edge Cases and Validation Tests

Tests for edge cases, boundary conditions, and input validation including:
- Extreme coordinate testing and boundary conditions
- Invalid parameter and port validation
- Null value handling and error cases
- Label validation and string processing
- Element count validation and limits
- Nonexistent element handling

MCP test implementation
"""

import asyncio
from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class EdgeCasesValidationTests(MCPTestBase):
    """Tests for edge cases, boundary conditions, and input validation"""

    async def run_category_tests(self) -> bool:
        """Run all edge cases and validation tests

        Returns:
            bool: True if all tests passed, False otherwise
        """
        tests = [
            self.test_workflow_element_count_validation,
            self.test_edge_cases_extreme_coordinates,
            self.test_edge_cases_label_validation,
            self.test_edge_cases_invalid_ports,
            self.test_edge_cases_nonexistent_elements,
            self.test_edge_cases_null_values,
            self.test_edge_cases_float_coordinates,
        ]

        print("\n" + "=" * 60)
        print("🧪 RUNNING EDGE CASES AND VALIDATION TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    # ==================== TEST METHODS ====================
    # Test method implementations

    @beartype
    async def test_workflow_element_count_validation(self) -> bool:
        """Test element count validation across different scenarios"""
        print("\n=== Element Count Validation Test ===")
        self.set_test_context("test_workflow_element_count_validation")

        all_passed: bool = True

        # Test: Empty circuit should have 0 elements
        resp = await self.send_command("list_elements", {})
        success = await self.assert_success(resp, "List elements in empty circuit")
        all_passed &= success
        if success:
            result = await self.get_response_result(resp)
            if result and "elements" in result:
                element_count = len(result["elements"])
                if element_count == 0:
                    self.infrastructure.output.success(f"✅ Empty circuit element count: {element_count}")
                else:
                    print(f"❌ Empty circuit unexpected count: {element_count} (expected 0)")
                    all_passed = False
            else:
                element_count = -1  # Default value when result is None
                print(f"❌ Empty circuit unexpected count: {element_count} (expected 0)")
                all_passed = False

        # Test: Single element circuit should have 1 element
        input_resp = await self.send_command(
            "create_element",
            {
                "type": "InputButton",
                "x": 100,
                "y": 100,
                "label": "SINGLE_INPUT",
            },
        )
        input_id = await self.validate_element_creation_response(input_resp, "Create single element")
        if input_id is None:
            all_passed = False

        resp = await self.send_command("list_elements", {})
        success = await self.assert_success(resp, "List elements after single creation")
        all_passed &= success
        if success:
            result = await self.get_response_result(resp)
            if result and "elements" in result:
                element_count = len(result["elements"])
                if element_count == 1:
                    self.infrastructure.output.success(f"✅ Single element circuit count: {element_count}")
                    # Note: assert_success already incremented counters
                else:
                    print(f"❌ Single element unexpected count: {element_count} (expected 1)")
                    all_passed = False

        # Test: Dual element circuit should have 2 elements
        output_resp = await self.send_command(
            "create_element",
            {
                "type": "Led",
                "x": 300,
                "y": 100,
                "label": "DUAL_OUTPUT",
            },
        )
        output_id = await self.validate_element_creation_response(output_resp, "Create second element")
        if output_id is None:
            all_passed = False

        resp = await self.send_command("list_elements", {})
        success = await self.assert_success(resp, "List elements after dual creation")
        all_passed &= success
        if success:
            result = await self.get_response_result(resp)
            if result and "elements" in result:
                element_count = len(result["elements"])
                if element_count == 2:
                    self.infrastructure.output.success(f"✅ Dual element circuit count: {element_count}")
                    # Note: assert_success already incremented counters
                else:
                    print(f"❌ Dual element unexpected count: {element_count} (expected 2)")
                    all_passed = False

        # Test: After element deletion should have 1 element
        if input_id:
            resp = await self.send_command("delete_element", {"element_id": input_id})
            all_passed &= await self.assert_success(resp, "Delete first element")

            resp = await self.send_command("list_elements", {})
            success = await self.assert_success(resp, "List elements after deletion")
            all_passed &= success
            if success:
                result = await self.get_response_result(resp)
                if result and "elements" in result:
                    element_count = len(result["elements"])
                    if element_count == 1:
                        self.infrastructure.output.success(f"✅ Post-deletion element count: {element_count}")
                        # Note: assert_success already incremented counters
                    else:
                        print(f"❌ Post-deletion unexpected count: {element_count} (expected 1)")
                        all_passed = False

        # Test: Multi-element types validation
        logic_resp = await self.send_command(
            "create_element",
            {
                "type": "And",
                "x": 200,
                "y": 200,
                "label": "LOGIC_GATE",
            },
        )
        logic_id = await self.validate_element_creation_response(logic_resp, "Create logic gate")
        if logic_id is None:
            all_passed = False

        switch_resp = await self.send_command(
            "create_element",
            {
                "type": "InputSwitch",
                "x": 50,
                "y": 200,
                "label": "SWITCH_INPUT",
            },
        )
        switch_id = await self.validate_element_creation_response(switch_resp, "Create switch input")
        if switch_id is None:
            all_passed = False

        resp = await self.send_command("list_elements", {})
        success = await self.assert_success(resp, "List elements with multiple types")
        all_passed &= success
        if success:
            result = await self.get_response_result(resp)
            if result and "elements" in result:
                element_count = len(result["elements"])
                expected_count = 3  # LED + Logic + Switch
                if element_count == expected_count:
                    self.infrastructure.output.success(f"✅ Multi-type element count: {element_count}")
                    # Note: assert_success already incremented counters
                else:
                    print(f"❌ Multi-type unexpected count: {element_count} (expected {expected_count})")
                    all_passed = False

        return all_passed

    @beartype
    async def test_edge_cases_extreme_coordinates(self) -> bool:
        """Test extreme coordinate boundary conditions"""
        print("\n=== Extreme Coordinates Edge Case Test ===")
        self.set_test_context("test_edge_cases_extreme_coordinates")

        all_passed: bool = True

        # Test extreme negative coordinates (near INT_MIN)
        resp = await self.send_command(
            "create_element",
            {
                "type": "And",
                "x": -2147483600,
                "y": -2147483600,
                "label": "ExtremeNegative",
            },
        )
        extreme_neg_id = await self.validate_element_creation_response(resp, "Create element with extreme negative coordinates")
        if extreme_neg_id is None:
            all_passed = False

        # Test extreme positive coordinates (near INT_MAX)
        resp = await self.send_command(
            "create_element",
            {
                "type": "Or",
                "x": 2147483600,
                "y": 2147483600,
                "label": "ExtremePositive",
            },
        )
        extreme_pos_id = await self.validate_element_creation_response(resp, "Create element with extreme positive coordinates")
        if extreme_pos_id is None:
            all_passed = False

        # Check if any validation failed and fail the test accordingly
        if not extreme_neg_id or not extreme_pos_id:
            print("❌ Extreme coordinates test failed: One or more validations failed")
            all_passed = False

        return all_passed

    @beartype
    async def test_edge_cases_label_validation(self) -> bool:
        """Test label validation edge cases"""
        print("\n=== Label Validation Edge Case Test ===")
        self.set_test_context("test_edge_cases_label_validation")

        all_passed: bool = True

        # Test empty label
        resp = await self.send_command(
            "create_element",
            {
                "type": "Not",
                "x": 200,
                "y": 200,
                "label": "",
            },
        )
        empty_label_id = await self.validate_element_creation_response(resp, "Create element with empty label")
        if empty_label_id is None:
            all_passed = False

        # Test special character label
        resp = await self.send_command(
            "create_element",
            {
                "type": "Led",
                "x": 300,
                "y": 300,
                "label": "Special!@#$%^&*()Chars",
            },
        )
        special_char_id = await self.validate_element_creation_response(resp, "Create element with special character label")
        if special_char_id is None:
            all_passed = False

        # Test very long label
        long_label = "VeryLongLabel" * 20  # 260 characters
        resp = await self.send_command(
            "create_element",
            {
                "type": "InputButton",
                "x": 400,
                "y": 400,
                "label": long_label,
            },
        )
        long_label_id = await self.validate_element_creation_response(resp, "Create element with very long label")
        if long_label_id is None:
            all_passed = False

        # Check if any validation failed and fail the test accordingly
        if not empty_label_id or not special_char_id or not long_label_id:
            print("❌ Label validation test failed: One or more validations failed")
            all_passed = False

        return all_passed

    @beartype
    async def test_edge_cases_invalid_ports(self) -> bool:
        """Test invalid port number edge cases"""
        print("\n=== Invalid Port Numbers Edge Case Test ===")
        self.set_test_context("test_edge_cases_invalid_ports")

        all_passed: bool = True

        # Setup
        # Create elements for port testing
        resp = await self.send_command(
            "create_element",
            {
                "type": "And",
                "x": 100,
                "y": 100,
                "label": "Source",
            },
        )
        source_id = await self.validate_element_creation_response(resp, "Create source element for port test")

        resp = await self.send_command(
            "create_element",
            {
                "type": "Or",
                "x": 200,
                "y": 100,
                "label": "Target",
            },
        )
        target_id = await self.validate_element_creation_response(resp, "Create target element for port test")

        # Early return if element creation failed
        if not source_id or not target_id:
            print("⚠️  Skipping port tests - element creation failed")
            return False

        # Test invalid negative port
        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": source_id,
                "source_port": -1,  # Invalid negative port
                "target_id": target_id,
                "target_port": 0,
            },
        )
        all_passed &= await self.assert_failure(resp, "Connect with invalid negative source port")

        # Test invalid high port number
        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": source_id,
                "source_port": 0,
                "target_id": target_id,
                "target_port": 999,  # Invalid high port number
            },
        )
        all_passed &= await self.assert_failure(resp, "Connect with invalid high target port")

        return all_passed

    @beartype
    async def test_edge_cases_nonexistent_elements(self) -> bool:
        """Test operations on non-existent elements"""
        print("\n=== Non-Existent Elements Edge Case Test ===")
        self.set_test_context("test_edge_cases_nonexistent_elements")

        all_passed: bool = True

        # Setup# Test operations on non-existent elements
        resp = await self.send_command("delete_element", {"element_id": -1})
        all_passed &= await self.assert_failure(resp, "Delete element with negative ID")

        resp = await self.send_command("set_input_value", {"element_id": 999999, "value": True})
        all_passed &= await self.assert_failure(resp, "Set value on non-existent element")

        resp = await self.send_command("get_output_value", {"element_id": -999})
        all_passed &= await self.assert_failure(resp, "Get value from element with negative ID")

        return all_passed

    @beartype
    async def test_edge_cases_null_values(self) -> bool:
        """Test null and undefined value handling"""
        print("\n=== Null Values Edge Case Test ===")
        self.set_test_context("test_edge_cases_null_values")

        all_passed: bool = True

        # Setup# Test null/undefined values
        resp = await self.send_command(
            "create_element",
            {
                "type": "And",
                "x": None,
                "y": 100,
                "label": "NullX",
            },
        )
        all_passed &= await self.assert_failure(resp, "Create element with null X coordinate")

        return all_passed

    @beartype
    async def test_edge_cases_float_coordinates(self) -> bool:
        """Test floating point coordinate handling"""
        print("\n=== Float Coordinates Edge Case Test ===")
        self.set_test_context("test_edge_cases_float_coordinates")

        all_passed: bool = True

        # Setup
        # Test floating point coordinates (should be handled gracefully)
        resp = await self.send_command(
            "create_element",
            {
                "type": "Or",
                "x": 100.5,
                "y": 200.7,
                "label": "FloatCoords",
            },
        )
        element_id = await self.validate_element_creation_response(resp, "Create element with float coordinates")
        if element_id:
            self.infrastructure.output.success("✅ Float coordinates accepted and element created")
            all_passed &= True
        else:
            print("❌ Float coordinates validation failed")
            all_passed = False

        return all_passed
