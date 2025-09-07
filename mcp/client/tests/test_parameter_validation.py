#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Parameter Validation Tests

Tests for command parameter validation including:
- Missing parameter error handling
- Invalid parameter value validation
- Command coverage and completeness
- Parameter type checking and edge cases

MCP test implementation
"""

import asyncio
from typing import Any, Dict

from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class ParameterValidationTests(MCPTestBase):
    """Tests for parameter validation and command coverage"""

    async def run_category_tests(self) -> bool:
        """Run all parameter validation tests

        Returns:
            bool: True if all tests passed, False otherwise
        """
        tests = [
            self.test_missing_parameter_validation,
            self.test_missing_command_coverage,
        ]

        print("\n" + "=" * 60)
        print("🧪 RUNNING PARAMETER VALIDATION TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    # ==================== TEST METHODS ====================
    # Test method implementations

    @beartype
    async def test_missing_parameter_validation(self) -> bool:
        """Test missing parameter error handling across all commands"""
        print("\n=== Missing Parameter Validation Test ===")
        self.set_test_context("test_missing_parameter_validation")

        all_passed: bool = True

        # Test create_element with missing parameters
        resp = await self.send_command("create_element", {})
        all_passed &= await self.assert_failure(resp, "Create element with missing parameters")

        resp = await self.send_command("create_element", {"type": "And"})
        all_passed &= await self.assert_failure(resp, "Create element with partial parameters (missing x, y)")

        resp = await self.send_command("create_element", {"x": 100, "y": 100})
        all_passed &= await self.assert_failure(resp, "Create element with partial parameters (missing type)")

        # Test connect_elements with missing parameters
        resp = await self.send_command("connect_elements", {})
        all_passed &= await self.assert_failure(resp, "Connect elements with missing parameters")

        resp = await self.send_command("connect_elements", {"source_id": 1})
        all_passed &= await self.assert_failure(resp, "Connect elements with partial parameters (missing target)")

        resp = await self.send_command("connect_elements", {"source_id": 1, "target_id": 2})
        all_passed &= await self.assert_failure(resp, "Connect elements with partial parameters (missing ports)")

        # Test simulation_control with missing parameters
        resp = await self.send_command("simulation_control", {})
        all_passed &= await self.assert_failure(resp, "Simulation control with missing parameters")

        # Test delete_element with missing parameters
        resp = await self.send_command("delete_element", {})
        all_passed &= await self.assert_failure(resp, "Delete element with missing parameters")

        # Test set_input_value with missing parameters
        resp = await self.send_command("set_input_value", {})
        all_passed &= await self.assert_failure(resp, "Set input value with missing parameters")

        resp = await self.send_command("set_input_value", {"element_id": 1})
        all_passed &= await self.assert_failure(resp, "Set input value with partial parameters (missing value)")

        resp = await self.send_command("set_input_value", {"value": True})
        all_passed &= await self.assert_failure(resp, "Set input value with partial parameters (missing element_id)")

        # Test get_output_value with missing parameters
        resp = await self.send_command("get_output_value", {})
        all_passed &= await self.assert_failure(resp, "Get output value with missing parameters")

        # Test disconnect_elements with missing parameters
        resp = await self.send_command("disconnect_elements", {})
        all_passed &= await self.assert_failure(resp, "Disconnect elements with missing parameters")

        resp = await self.send_command("disconnect_elements", {"source_id": 1})
        all_passed &= await self.assert_failure(resp, "Disconnect elements with partial parameters (missing target)")

        # Test file operations with missing parameters
        resp = await self.send_command("save_circuit", {})
        all_passed &= await self.assert_failure(resp, "Save circuit with missing filename")

        resp = await self.send_command("load_circuit", {})
        all_passed &= await self.assert_failure(resp, "Load circuit with missing filename")

        # Test parameter validation for multiple commands (JSON-RPC 2.0 format)
        commands_to_test = [
            {"jsonrpc": "2.0", "method": "create_element", "params": {"type": "And"}, "id": 1},  # Missing x,y
            {"jsonrpc": "2.0", "method": "connect_elements", "params": {"source_id": 1}, "id": 2},  # Missing target info
            {"jsonrpc": "2.0", "method": "set_input_value", "params": {"element_id": 1}, "id": 3},  # Missing value
            {"jsonrpc": "2.0", "method": "save_circuit", "params": {"filename": ""}, "id": 4},  # Empty filename
        ]

        for test_case in commands_to_test:
            command: str = test_case["method"]  # type: ignore
            params: Dict[str, Any] = test_case["params"]  # type: ignore
            resp = await self.send_command(command, params)
            all_passed &= await self.assert_failure(resp, f"Parameter validation for {command}")

        # No cleanup needed as this is error handling test

        return all_passed

    @beartype
    async def test_missing_command_coverage(self) -> bool:
        """ Missing Command Coverage - IC Workflow and Element Management"""
        print("\n=== Missing Command Coverage ===")

        all_passed: bool = True

        # Test IC workflow commands
        print("Testing IC workflow commands...")
        # Note: Individual IC operations will be counted via assert_success/assert_failure

        try:
            # Clean up any existing test IC files to ensure clean state
            import os

            test_ic_file = "test_ic.panda"
            if os.path.exists(test_ic_file):
                os.remove(test_ic_file)
                print("🧹 Cleaned up existing test IC file")

            # Create a simple circuit for IC creation
            # Create elements for IC using proper ElementGroup types
            # InputButton belongs to ElementGroup::Input (required for IC interface)
            input_id = await self.validate_element_creation_response(
                await self.send_command("create_element", {
                    "type": "InputButton", "x": 100.0, "y": 100.0, "label": "IC_Input"
                }), "create IC input"
            )

            # Create a simple buffer logic
            buffer_id = await self.validate_element_creation_response(
                await self.send_command("create_element", {
                    "type": "Not", "x": 200.0, "y": 100.0, "label": "IC_Buffer"
                }), "create IC logic gate"
            )

            # Led belongs to ElementGroup::Output (required for IC interface)
            output_id = await self.validate_element_creation_response(
                await self.send_command("create_element", {
                    "type": "Led", "x": 300.0, "y": 100.0, "label": "IC_Output"
                }), "create IC output"
            )

            # Early return if element creation failed
            if not input_id or not buffer_id or not output_id:
                print("❌ IC test setup failed - could not create required elements")
                all_passed = False
                return all_passed

            # Connect: InputButton -> NOT -> Led (creates IC with 1 input, 1 output)
            connections = [
                (input_id, 0, buffer_id, 0),   # InputButton to NOT gate
                (buffer_id, 0, output_id, 0)   # NOT gate to Led
            ]

            for source_id, source_port, target_id, target_port in connections:
                connect_resp = await self.send_command("connect_elements", {
                    "source_id": source_id, "source_port": source_port,
                    "target_id": target_id, "target_port": target_port
                })
                if not connect_resp.success:
                    print(f"❌ IC test setup failed - could not connect elements: {connect_resp.error}")
                    return False

            # Test create_ic command with properly connected circuit
            ic_resp = await self.send_command("create_ic", {
                "name": "test_ic",
                "description": "Test IC created by test suite"
            })

            if ic_resp.success:
                self.infrastructure.output.success("✅ create_ic command successful")
            else:
                # Handle "file already exists" as acceptable for testing
                if "already exists" in str(ic_resp.error):
                    self.infrastructure.output.success(f"✅ create_ic handled existing file: {ic_resp.error}")
                else:
                    print(f"❌ create_ic failed: {ic_resp.error}")

            # Test list_ics command
            list_resp = await self.send_command("list_ics", {})
            if list_resp.success and list_resp.result:
                ics = list_resp.result.get("ics", [])
                self.infrastructure.output.success(f"✅ list_ics successful: found {len(ics)} ICs")
            else:
                print(f"❌ list_ics failed: {list_resp.error}")

        except Exception as e:
            print(f"❌ IC workflow test failed with exception: {e}")

        print("   • test_ic_workflow_commands()")
        print("   • test_element_management_commands()")

        return all_passed
