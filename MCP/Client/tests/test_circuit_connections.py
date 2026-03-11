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
            self.test_connection_by_label,
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
        """Connection error handling and disconnect functionality"""
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
        all_passed &= await self.assert_success_with_image(
            resp, f"Connect input {input_id} to gate {gate_id}", "test_connection_basic"
        )

        # Verify connection was established
        connection_verified = await self.verify_connection_exists(
            input_id, gate_id, f"Verify connection {input_id}->{gate_id}"
        )
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
        all_passed &= await self.assert_success_with_image(
            resp, f"Connect gate {gate_id} to output {output_id}", "test_connection_basic"
        )

        # Verify second connection was established
        second_connection_verified = await self.verify_connection_exists(
            gate_id, output_id, f"Verify connection {gate_id}->{output_id}"
        )
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

    @beartype
    async def test_connection_by_label(self) -> bool:
        """Test connection using port labels instead of indices"""
        print("\n=== Connection By Label Test ===")
        self.set_test_context("test_connection_by_label")

        all_passed: bool = True

        # Create a new circuit for this test
        resp = await self.send_command("new_circuit", {})
        all_passed &= await self.assert_success(resp, "Create new circuit for label test")

        # Create an InputButton (output port has no explicit name, will test with index)
        resp1 = await self.send_command(
            "create_element",
            {
                "type": "InputButton",
                "x": 100,
                "y": 200,
                "label": "ClockInput",
            },
        )
        clock_input_id = await self.validate_element_creation_response(resp1, "Create clock input")
        if clock_input_id is None:
            all_passed = False

        # Create a JK Flip-Flop which has named ports:
        # Inputs: "J" (0), "Clock" (1), "K" (2), "~Preset" (3), "~Clear" (4)
        # Outputs: "Q" (0), "~Q" (1)
        resp2 = await self.send_command(
            "create_element",
            {
                "type": "JKFlipFlop",
                "x": 300,
                "y": 200,
                "label": "JK_FF",
            },
        )
        jkff_id = await self.validate_element_creation_response(resp2, "Create JK Flip-Flop")
        if jkff_id is None:
            all_passed = False

        # Create a LED to connect to the Q output
        resp3 = await self.send_command(
            "create_element",
            {
                "type": "Led",
                "x": 500,
                "y": 200,
                "label": "Q_Output",
            },
        )
        led_id = await self.validate_element_creation_response(resp3, "Create LED output")
        if led_id is None:
            all_passed = False

        # Early return if element creation failed
        if not clock_input_id or not jkff_id or not led_id:
            print("❌ Failed to create required elements for connection by label test")
            return False

        # Test 1: Connect input to JK flip-flop Clock port using label
        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": clock_input_id,
                "source_port": 0,  # InputButton output port (no label)
                "target_id": jkff_id,
                "target_port_label": "Clock",  # Use label instead of index
            },
        )
        all_passed &= await self.assert_success(resp, "Connect input to JK flip-flop Clock port by label")

        # Verify connection was established
        connection_verified = await self.verify_connection_exists(clock_input_id, jkff_id, "Verify Clock connection")
        all_passed &= connection_verified
        if connection_verified:
            self.infrastructure.output.success("✅ Connection by label verified: input -> JK.Clock")
        else:
            print("❌ Connection by label not found: input -> JK.Clock")

        # Test 2: Connect JK flip-flop Q output to LED using output port label
        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": jkff_id,
                "source_port_label": "Q",  # Use label for output port
                "target_id": led_id,
                "target_port": 0,  # LED input port
            },
        )
        all_passed &= await self.assert_success(resp, "Connect JK flip-flop Q output by label to LED")

        # Verify second connection
        connection_verified = await self.verify_connection_exists(jkff_id, led_id, "Verify Q->LED connection")
        all_passed &= connection_verified
        if connection_verified:
            self.infrastructure.output.success("✅ Connection by label verified: JK.Q -> LED")
        else:
            print("❌ Connection by label not found: JK.Q -> LED")

        # Test 3: Connect using labels on both ends
        # Create another input for J port
        resp4 = await self.send_command(
            "create_element",
            {
                "type": "InputButton",
                "x": 100,
                "y": 100,
                "label": "J_Input",
            },
        )
        j_input_id = await self.validate_element_creation_response(resp4, "Create J input")
        if j_input_id is None:
            all_passed = False
            return all_passed

        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": j_input_id,
                "source_port": 0,
                "target_id": jkff_id,
                "target_port_label": "J",
            },
        )
        all_passed &= await self.assert_success(resp, "Connect input to JK flip-flop J port by label")

        # Test 4: Error handling - invalid port label
        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": clock_input_id,
                "source_port": 0,
                "target_id": jkff_id,
                "target_port_label": "InvalidPortName",
            },
        )
        all_passed &= await self.assert_failure(resp, "Connection with invalid target port label")

        # Test 5: Error handling - invalid source port label
        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": jkff_id,
                "source_port_label": "NonExistentOutput",
                "target_id": led_id,
                "target_port": 0,
            },
        )
        all_passed &= await self.assert_failure(resp, "Connection with invalid source port label")

        # Test 6: Connect to ~Q output using label with special character
        resp5 = await self.send_command(
            "create_element",
            {
                "type": "Led",
                "x": 500,
                "y": 300,
                "label": "NotQ_Output",
            },
        )
        notq_led_id = await self.validate_element_creation_response(resp5, "Create ~Q LED output")
        if notq_led_id is None:
            all_passed = False
            return all_passed

        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": jkff_id,
                "source_port_label": "~Q",  # Label with special character
                "target_id": notq_led_id,
                "target_port": 0,
            },
        )
        all_passed &= await self.assert_success(resp, "Connect JK flip-flop ~Q output by label")

        # Verify ~Q connection
        connection_verified = await self.verify_connection_exists(jkff_id, notq_led_id, "Verify ~Q->LED connection")
        all_passed &= connection_verified
        if connection_verified:
            self.infrastructure.output.success("✅ Connection by label verified: JK.~Q -> LED")
        else:
            print("❌ Connection by label not found: JK.~Q -> LED")

        return all_passed
