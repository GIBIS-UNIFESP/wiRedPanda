#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Basic Protocol Tests

Tests for fundamental MCP protocol functionality including:
- Protocol workflow and integration
- Communication patterns
- JSON validation
- Error handling

Tests for basic MCP protocol functionality
"""

import asyncio
import json

from beartype import beartype
# Import models - needed for test_invalid_json_handling
from mcp_models import MCPResponse

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class BasicProtocolTests(MCPTestBase):
    """Tests for basic MCP protocol functionality"""

    async def run_category_tests(self) -> bool:
        """Run all basic protocol tests

        Returns:
            bool: True if all tests passed, False otherwise
        """
        tests = [
            self.test_basic_protocol,
            self.test_protocol_communication,
            self.test_error_handling,
            self.test_invalid_json_handling,
        ]

        print("\n" + "=" * 60)
        print("🧪 RUNNING BASIC PROTOCOL TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    # ==================== TEST METHODS ====================
    # Test method implementations

    @beartype
    async def test_basic_protocol(self) -> bool:
        """Basic protocol workflow and integration"""
        print("\n=== Basic Protocol Test ===")

        all_passed = True

        # Test basic protocol workflow: command -> verification

        # Test command chaining
        resp = await self.send_command("list_elements", {})
        all_passed &= await self.assert_success(resp, "Protocol workflow: list empty circuit")
        if resp.success:
            result = await self.get_response_result(resp)
            if result and "elements" in result:
                if len(result["elements"]) == 0:
                    self.infrastructure.output.success("✅ Protocol workflow: empty circuit confirmed")
                else:
                    print("❌ Protocol workflow: expected empty circuit")
                    all_passed = False
            else:
                print("❌ Protocol workflow: invalid list_elements response")
                all_passed = False

        # Test protocol response structure consistency
        commands_to_test = ["new_circuit", "list_elements", "simulation_control"]
        for cmd in commands_to_test:
            if cmd == "simulation_control":
                resp = await self.send_command(cmd, {"action": "stop"})
            else:
                resp = await self.send_command(cmd, {})

            # Verify all responses have consistent structure
            if hasattr(resp, "success") and hasattr(resp, "result"):
                self.infrastructure.output.success(f"✅ Protocol consistency: {cmd} structure valid")
            else:
                print(f"❌ Protocol consistency: {cmd} structure invalid")
                all_passed = False

        return all_passed

    @beartype
    async def test_protocol_communication(self) -> bool:
        """Test basic MCP protocol communication functionality"""
        print("\n=== Protocol Communication Test ===")

        all_passed = True

        # Test unknown command handling
        resp = await self.send_command("unknown_command", {})
        all_passed &= await self.assert_failure(resp, "Unknown command handling")

        # Test valid command with empty parameters
        resp = await self.send_command("list_elements", {})
        all_passed &= await self.assert_success(resp, "Valid command with empty parameters")

        return all_passed

    @beartype
    async def test_error_handling(self) -> bool:
        """Malformed JSON request handling"""
        print("\n=== Error Handling Test ===")

        all_passed = True

        # Test malformed JSON requests
        try:
            test_cases = [
                '{"jsonrpc": "2.0", "method": "", "params": {}, "id": 1}',  # empty method
                '{"jsonrpc": "2.0", "params": {}, "id": 1}',  # missing method
                '{"jsonrpc": "2.0", "method": "test", "id": 1}',  # missing params
            ]

            for test_case in test_cases:
                if not self.process or self.process.stdin is None or self.process.stdout is None:
                    raise RuntimeError("MCP process not available for malformed request test")

                # Write using async methods
                self.process.stdin.write((test_case + "\n").encode('utf-8'))
                await self.process.stdin.drain()

                # Read response with timeout using async methods
                try:
                    response_line_bytes = await asyncio.wait_for(
                        self.process.stdout.readline(),
                        timeout=5.0
                    )
                    response_line = response_line_bytes.decode('utf-8').strip()
                except asyncio.TimeoutError:
                    response_line = ""

                if response_line:
                    resp = json.loads(response_line)
                    if not resp.get("success", False):
                        self.infrastructure.output.success(f"✅ Rejected malformed request: {test_case[:30]}...")
                    else:
                        print(f"❌ Accepted malformed request: {test_case[:30]}...")
                        all_passed = False
                else:
                    print(f"❌ Timeout on: {test_case[:30]}...")
                    all_passed = False
        except Exception as e:
            print(f"❌ Error handling test failed: {e}")
            all_passed = False

        return all_passed

    @beartype
    async def test_invalid_json_handling(self) -> bool:
        """Test invalid JSON parsing and error handling"""
        print("\n=== Invalid JSON Handling Test ===")

        all_passed = True

        # Test invalid JSON (manual test since we can't use send_command)
        try:
            if not self.process or self.process.stdin is None or self.process.stdout is None:
                raise RuntimeError("MCP process not available for invalid JSON test")

            # Write using async methods
            self.process.stdin.write("invalid json\n".encode('utf-8'))
            await self.process.stdin.drain()

            # Read response with timeout using async methods
            try:
                response_line_bytes = await asyncio.wait_for(
                    self.process.stdout.readline(),
                    timeout=1.0
                )
                response_line = response_line_bytes.decode('utf-8').strip()
            except asyncio.TimeoutError:
                print("❌ No response to invalid JSON")
                return False

            if response_line:
                try:
                    raw_resp = json.loads(response_line)
                    resp = MCPResponse(**raw_resp)
                    all_passed &= await self.assert_failure(resp, "Invalid JSON handling")
                except json.JSONDecodeError:
                    # This is expected for invalid JSON input - the server might return an error message
                    print(f"❌ Invalid JSON test failed: '{response_line}'")
                    all_passed = False
            else:
                print("❌ No response to invalid JSON")
                all_passed = False

        except Exception as e:
            print(f"❌ Invalid JSON test failed: {e}")
            all_passed = False

        # Test completed successfully if we got here
        if all_passed:
            self.infrastructure.output.success("✅ Invalid JSON handling test completed")

        return all_passed
