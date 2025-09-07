#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Element Operations Tests

Tests for element management functionality including:
- Element creation and validation
- Element listing and enumeration
- Element deletion and cleanup
- Basic element workflow

MCP test implementation
"""

import asyncio
from typing import List, Optional

from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class ElementOperationTests(MCPTestBase):
    """Tests for element operations and management"""

    async def run_category_tests(self) -> bool:
        """Run all element operation tests

        Returns:
            bool: True if all tests passed, False otherwise
        """
        tests = [
            self.test_element_operations,
            self.test_element_creation,
            self.test_element_listing,
            self.test_element_deletion,
            self.test_element_management_commands,
        ]

        print("\n" + "=" * 60)
        print("🧪 RUNNING ELEMENT OPERATIONS TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    # ==================== TEST METHODS ====================
    # Test method implementations

    @beartype
    async def test_element_operations(self) -> bool:
        """Basic element operations workflow"""
        print("\n=== Element Operations Test ===")
        self.set_test_context("test_element_operations")

        all_passed: bool = True

        # Create new circuit first
        # Test basic element workflow: create -> verify -> interact
        resp = await self.send_command(
            "create_element",
            {
                "type": "InputButton",
                "x": 100,
                "y": 100,
                "label": "WorkflowInput",
            },
        )
        input_id = await self.validate_element_creation_response(resp, "Create workflow input element")
        if input_id is None:
            all_passed = False

        resp = await self.send_command(
            "create_element",
            {
                "type": "Led",
                "x": 200,
                "y": 100,
                "label": "WorkflowOutput",
            },
        )
        output_id = await self.validate_element_creation_response(resp, "Create workflow output element")
        if output_id is None:
            all_passed = False

        # Test basic element interaction
        if input_id is not None:
            resp = await self.send_command("set_input_value", {"element_id": input_id, "value": True})
            all_passed &= await self.assert_success(resp, "Set input value in basic workflow")

        return all_passed

    @beartype
    async def test_element_creation(self) -> bool:
        """Test element creation with various types"""
        print("\n=== Element Creation Test ===")
        self.set_test_context("test_element_creation")

        all_passed: bool = True

        # Create new circuit first
        # Test valid element creation - known working types
        valid_element_types: List[str] = ["And", "Or", "Not", "InputButton", "Led"]
        created_elements: List[Optional[int]] = []

        for i, elem_type in enumerate(valid_element_types):
            x, y = 100 + i * 150, 100
            resp = await self.send_command(
                "create_element",
                {
                    "type": elem_type,
                    "x": x,
                    "y": y,
                    "label": f"Test{elem_type}",
                },
            )
            element_id = await self.validate_element_creation_response(resp, f"Create {elem_type} element")
            if element_id is not None:
                created_elements.append(element_id)
            else:
                all_passed = False

        # Test missing parameters instead
        resp = await self.send_command("create_element", {"type": "And"})  # Missing x, y
        all_passed &= await self.assert_failure(resp, "Missing required parameters rejection")

        return all_passed

    @beartype
    async def test_element_listing(self) -> bool:
        """Test element listing functionality"""
        print("\n=== Element Listing Test ===")
        self.set_test_context("test_element_listing")

        all_passed: bool = True

        # Create new circuit first
        # Create test elements for listing
        test_elements = ["InputButton", "And", "Or", "Led"]
        created_elements: List[Optional[int]] = []

        for i, elem_type in enumerate(test_elements):
            x, y = 100 + i * 100, 100
            resp = await self.send_command(
                "create_element",
                {
                    "type": elem_type,
                    "x": x,
                    "y": y,
                    "label": f"List{elem_type}",
                },
            )
            element_id = await self.validate_element_creation_response(resp, f"Create {elem_type} for listing")
            if element_id is not None:
                created_elements.append(element_id)
            else:
                all_passed = False

        # Test element listing
        resp = await self.send_command("list_elements", {})
        success = await self.assert_success_with_image(resp, "List elements", "test_element_listing")
        all_passed &= success
        if success:
            result = await self.get_response_result(resp)
            if result and "elements" in result:
                elements = result["elements"]
                expected_count = len([e for e in created_elements if e is not None])
                actual_count = len(elements)
                if actual_count == expected_count:
                    self.infrastructure.output.success(f"✅ Element count exact match: {actual_count}")
                    # Note: assert_success_with_image already incremented counters
                else:
                    print(f"❌ Element count mismatch: expected {expected_count}, got {actual_count}")
                    all_passed = False

        # Test empty circuit listing
        resp = await self.send_command("new_circuit", {})
        all_passed &= await self.assert_success(resp, "Create empty circuit for listing test")
        resp = await self.send_command("list_elements", {})
        success = await self.assert_success(resp, "List elements on empty circuit")
        all_passed &= success
        if success:
            result = await self.get_response_result(resp)
            if result and "elements" in result:
                elements = result["elements"]
                if len(elements) == 0:
                    self.infrastructure.output.success("✅ Empty circuit listing correct: 0 elements")
                    # Note: assert_success already incremented counters
                else:
                    print(f"❌ Empty circuit should have 0 elements, got {len(elements)}")
                    all_passed = False
                    # Note: This is additional validation, but assert_success already handled the counting

        return all_passed

    @beartype
    async def test_element_deletion(self) -> bool:
        """Test element deletion functionality and error handling"""
        print("\n=== Element Deletion Test ===")
        self.set_test_context("test_element_deletion")

        all_passed: bool = True

        # Create new circuit first
        # Create test elements for deletion
        test_elements = ["InputButton", "And", "Led"]
        created_elements: List[Optional[int]] = []

        for i, elem_type in enumerate(test_elements):
            x, y = 100 + i * 100, 100
            resp = await self.send_command(
                "create_element",
                {
                    "type": elem_type,
                    "x": x,
                    "y": y,
                    "label": f"Delete{elem_type}",
                },
            )
            element_id = await self.validate_element_creation_response(resp, f"Create {elem_type} for deletion")
            if element_id is not None:
                created_elements.append(element_id)
            else:
                all_passed = False

        # Test successful element deletion
        if created_elements and len(created_elements) >= 2:
            first_element = created_elements[0]
            second_element = created_elements[1]

            if first_element is not None:
                resp = await self.send_command("delete_element", {"element_id": first_element})
                all_passed &= await self.assert_success(resp, f"Delete element {first_element}")

                # Verify element was actually deleted by listing
                resp = await self.send_command("list_elements", {})
                success = await self.assert_success(resp, "List elements after deletion")
                all_passed &= success
                if success:
                    result = await self.get_response_result(resp)
                    if result and "elements" in result:
                        elements = result["elements"]
                        remaining_count = len(elements)
                        expected_remaining = len([e for e in created_elements if e is not None]) - 1
                        if remaining_count == expected_remaining:
                            self.infrastructure.output.success(f"✅ Element deletion verified: {remaining_count} elements remaining")
                            # Note: assert_success already incremented counters
                        else:
                            print(f"❌ Deletion verification failed: expected {expected_remaining}, got {remaining_count}")
                            all_passed = False

            # Test multiple deletions
            if second_element is not None:
                resp = await self.send_command("delete_element", {"element_id": second_element})
                all_passed &= await self.assert_success(resp, f"Delete element {second_element}")

        # Test delete non-existent element (error handling)
        resp = await self.send_command("delete_element", {"element_id": 99999})
        all_passed &= await self.assert_failure(resp, "Delete non-existent element")

        # Test delete with invalid parameters
        resp = await self.send_command("delete_element", {"element_id": -1})
        all_passed &= await self.assert_failure(resp, "Delete element with negative ID")

        # Test delete with missing parameters
        resp = await self.send_command("delete_element", {})
        all_passed &= await self.assert_failure(resp, "Delete element with missing ID parameter")

        return all_passed

    @beartype
    async def test_element_management_commands(self) -> bool:
        """Test element management commands: move_element, set_element_properties, disconnect_elements"""
        print("\n=== Element Management Commands Test ===")
        self.set_test_context("test_element_management_commands")

        all_passed: bool = True

        try:
            # Create elements for management testing
            elem1_id = await self.validate_element_creation_response(
                await self.send_command("create_element", {
                    "type": "And", "x": 100.0, "y": 100.0, "label": "TestAnd1"
                }), "create element for management"
            )

            elem2_id = await self.validate_element_creation_response(
                await self.send_command("create_element", {
                    "type": "Or", "x": 200.0, "y": 100.0, "label": "TestOr1"
                }), "create element for management"
            )

            # Early return if elements couldn't be created
            if not elem1_id or not elem2_id:
                print("❌ Failed to create elements for management testing")
                return False

            # Test move_element command
            move_resp = await self.send_command("move_element", {
                "element_id": elem1_id, "x": 150.0, "y": 150.0
            })
            if move_resp.success:
                self.infrastructure.output.success("✅ move_element succeeded")
                all_passed &= True
            else:
                print(f"❌ move_element failed: {move_resp.error}")
                all_passed = False

            # Test set_element_properties command
            props_resp = await self.send_command("set_element_properties", {
                "element_id": elem1_id,
                "label": "MovedAndGate",
                "input_size": 3
            })
            if props_resp.success:
                self.infrastructure.output.success("✅ set_element_properties succeeded")
                all_passed &= True
            else:
                print(f"❌ set_element_properties failed: {props_resp.error}")
                all_passed = False

            # Create a connection for disconnect testing
            connect_resp = await self.send_command("connect_elements", {
                "source_id": elem1_id, "source_port": 0,
                "target_id": elem2_id, "target_port": 0
            })

            if not connect_resp.success:
                print(f"❌ Could not test disconnect - connection failed: {connect_resp.error}")
                all_passed = False

            # Test disconnect_elements command
            disconnect_resp = await self.send_command("disconnect_elements", {
                "source_id": elem1_id, "target_id": elem2_id
            })
            if disconnect_resp.success:
                self.infrastructure.output.success("✅ disconnect_elements succeeded")
                all_passed &= True
            else:
                print(f"❌ disconnect_elements failed: {disconnect_resp.error}")
                all_passed = False

        except Exception as e:
            print(f"❌ Element management test failed with exception: {e}")
            self.test_results.errors.append(f"test_element_management_commands: Exception - {e}")
            all_passed = False

        return all_passed
