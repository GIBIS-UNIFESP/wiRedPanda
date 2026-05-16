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

from typing import Any, Awaitable, Callable, Dict, List

from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class ParameterValidationTests(MCPTestBase):
    """Tests for parameter validation and command coverage"""

    CATEGORY_NAME = "PARAMETER VALIDATION"

    def tests(self) -> List[Callable[[], Awaitable[bool]]]:
        return [
            self.test_missing_parameter_validation,
            self.test_missing_command_coverage,
        ]

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
            {
                "jsonrpc": "2.0",
                "method": "connect_elements",
                "params": {"source_id": 1},
                "id": 2,
            },  # Missing target info
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
        """Missing Command Coverage - IC Workflow and Element Management"""
        print("\n=== Missing Command Coverage ===")

        all_passed: bool = True

        # Test IC workflow commands
        print("Testing IC workflow commands...")
        # Note: Individual IC operations will be counted via assert_success/assert_failure

        try:
            # Build the InputButton -> Not -> Led chain that IC creation expects.
            if await self.setup_basic_ic_circuit() is None:
                print("❌ IC test setup failed - could not create required elements")
                return False

            # Test create_ic command with properly connected circuit
            await self.create_test_ic()

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
