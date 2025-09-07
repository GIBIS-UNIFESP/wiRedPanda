#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Circuit Connections Tests

Tests for circuit connection functionality including:
- Basic element connections
- Connection error handling and validation
- Connection verification and topology
- Disconnect functionality

MCP test implementation
"""


from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class CircuitConnectionTests(MCPTestBase):
    """Tests for circuit connection operations"""

    async def run_category_tests(self) -> bool:
        """Run all circuit connection tests

        Returns:
            bool: True if all tests passed, False otherwise
        """
        tests = [
            self.test_circuit_connections,
            self.test_connection_basic,
            self.test_connection_error_handling,
        ]

        print("\n" + "=" * 60)
        print("🧪 RUNNING CIRCUIT CONNECTION TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    # ==================== TEST METHODS ====================
    # Test method implementations

    @beartype
    async def test_circuit_connections(self) -> bool:
        """ Connection error handling and disconnect functionality"""
        print("\n=== Circuit Connections ===")
        self.set_test_context("test_circuit_connections")

        all_passed: bool = True

        # Create new circuit
        # Create minimal elements for error testing
        resp1 = await self.send_command(
            "create_element",
            {
                "type": "InputButton",
                "x": 100,
                "y": 100,
                "label": "ErrorInput",
            },
        )
        input_id = await self.validate_element_creation_response(resp1, "Create input for error testing")
        if input_id is None:
            all_passed = False

        resp2 = await self.send_command(
            "create_element",
            {
                "type": "And",
                "x": 300,
                "y": 100,
                "label": "ErrorGate",
            },
        )
        gate_id = await self.validate_element_creation_response(resp2, "Create gate for error testing")
        if gate_id is None:
            all_passed = False

        # Early return if element creation failed
        if not input_id or not gate_id:
            print("❌ Failed to create required elements for connection disconnect testing")
            return False

        # First establish a connection for disconnect testing
        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": input_id,
                "source_port": 0,
                "target_id": gate_id,
                "target_port": 0,
            },
        )
        all_passed &= await self.assert_success(resp, "Create connection for disconnect test")

        # Test disconnect functionality
        resp = await self.send_command(
            "disconnect_elements",
            {
                "source_id": input_id,
                "target_id": gate_id,
            },
        )
        all_passed &= await self.assert_success(resp, f"Disconnect elements {input_id} and {gate_id}")

        return all_passed

    @beartype
    async def test_connection_basic(self) -> bool:
        """Test basic element connection functionality"""
        print("\n=== Basic Connection Test ===")
        self.set_test_context("test_connection_basic")

        all_passed: bool = True

        # Create new circuit
        # Create elements for connection
        resp1 = await self.send_command(
            "create_element",
            {
                "type": "InputButton",
                "x": 100,
                "y": 100,
                "label": "BasicInput",
            },
        )
        input_id = await self.validate_element_creation_response(resp1, "Create input for basic connection")
        if input_id is None:
            all_passed = False

        resp2 = await self.send_command(
            "create_element",
            {
                "type": "And",
                "x": 300,
                "y": 100,
                "label": "BasicGate",
            },
        )
        gate_id = await self.validate_element_creation_response(resp2, "Create gate for basic connection")
        if gate_id is None:
            all_passed = False

        resp3 = await self.send_command(
            "create_element",
            {
                "type": "Led",
                "x": 500,
                "y": 100,
                "label": "BasicOutput",
            },
        )
        output_id = await self.validate_element_creation_response(resp3, "Create output for basic connection")
        if output_id is None:
            all_passed = False

        # Early return if element creation failed
        if not input_id or not gate_id or not output_id:
            print("❌ Failed to create required elements for basic connection test")
            return False

        # Test first connection: input -> gate
        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": input_id,
                "source_port": 0,
                "target_id": gate_id,
                "target_port": 0,
            },
        )
        all_passed &= await self.assert_success_with_image(resp, f"Connect input {input_id} to gate {gate_id}", "test_connection_basic")

        # Verify connection was established
        connection_verified = await self.verify_connection_exists(input_id, gate_id, f"Verify connection {input_id}->{gate_id}")
        all_passed &= connection_verified
        if not connection_verified:
            print(f"❌ Connection not found: {input_id} -> {gate_id}")
        else:
            self.infrastructure.output.success(f"✅ Connection verified: {input_id} -> {gate_id}")

        # Test second connection: gate -> output
        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": gate_id,
                "source_port": 0,
                "target_id": output_id,
                "target_port": 0,
            },
        )
        all_passed &= await self.assert_success_with_image(resp, f"Connect gate {gate_id} to output {output_id}", "test_connection_basic")

        # Verify second connection was established
        second_connection_verified = await self.verify_connection_exists(gate_id, output_id, f"Verify connection {gate_id}->{output_id}")
        all_passed &= second_connection_verified
        if not second_connection_verified:
            print(f"❌ Connection not found: {gate_id} -> {output_id}")
        else:
            self.infrastructure.output.success(f"✅ Connection verified: {gate_id} -> {output_id}")

        return all_passed

    @beartype
    async def test_connection_error_handling(self) -> bool:
        """Test connection error handling and validation"""
        print("\n=== Connection Error Handling Test ===")
        self.set_test_context("test_connection_error_handling")

        all_passed: bool = True

        # Create new circuit
        # Create valid elements for error testing
        resp1 = await self.send_command(
            "create_element",
            {
                "type": "InputButton",
                "x": 100,
                "y": 100,
                "label": "ValidInput",
            },
        )
        input_id = await self.validate_element_creation_response(resp1, "Create valid input for error tests")
        if input_id is None:
            all_passed = False

        resp2 = await self.send_command(
            "create_element",
            {
                "type": "And",
                "x": 300,
                "y": 100,
                "label": "ValidGate",
            },
        )
        gate_id = await self.validate_element_creation_response(resp2, "Create valid gate for error tests")
        if gate_id is None:
            all_passed = False

        # Early return if element creation failed
        if not input_id or not gate_id:
            print("❌ Failed to create required elements for connection error handling test")
            return False

        # Test connection with non-existent source element
        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": 99999,
                "source_port": 0,
                "target_id": gate_id,
                "target_port": 0,
            },
        )
        all_passed &= await self.assert_failure(resp, "Connection with non-existent source element")

        # Test connection with non-existent target element
        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": input_id,
                "source_port": 0,
                "target_id": 88888,
                "target_port": 0,
            },
        )
        all_passed &= await self.assert_failure(resp, "Connection with non-existent target element")

        # Test connection with invalid port numbers
        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": input_id,
                "source_port": 99,  # Invalid high port
                "target_id": gate_id,
                "target_port": 0,
            },
        )
        all_passed &= await self.assert_failure(resp, "Connection with invalid source port")

        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": input_id,
                "source_port": 0,
                "target_id": gate_id,
                "target_port": 99,  # Invalid high port
            },
        )
        all_passed &= await self.assert_failure(resp, "Connection with invalid target port")

        # Test connection with missing parameters
        resp = await self.send_command("connect_elements", {})
        all_passed &= await self.assert_failure(resp, "Connection with missing parameters")

        # Test connection with partial parameters
        resp = await self.send_command("connect_elements", {"source_id": input_id})
        all_passed &= await self.assert_failure(resp, "Connection with partial parameters")

        return all_passed
