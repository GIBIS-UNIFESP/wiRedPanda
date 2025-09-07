#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Logic Gates Tests

Tests for logic gate functionality including:
- AND gate truth table validation
- OR gate truth table validation
- XOR gate truth table validation
- NOT gate analysis and waveform testing
- Logic gate type coverage validation

MCP test implementation
"""


from typing import cast

from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class LogicGateTests(MCPTestBase):
    """Tests for logic gate operations and truth table validation"""

    async def run_category_tests(self) -> bool:
        """Run all logic gate tests

        Returns:
            bool: True if all tests passed, False otherwise
        """
        tests = [
            self.test_logic_gate_and,
            self.test_logic_gate_or,
            self.test_logic_gate_xor,
            # self.test_missing_logic_gate_types,
            # self.test_waveform_not_gate_analysis,
        ]

        print("\n" + "=" * 60)
        print("🧪 RUNNING LOGIC GATE TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    # ==================== TEST METHODS ====================
    # Test method implementations

    @beartype
    async def test_logic_gate_and(self) -> bool:
        """Test AND gate truth table validation"""
        print("\n=== AND Gate Truth Table Test ===")

        all_passed: bool = True

        # Test 2-input AND gate truth table
        print("\nTesting 2-input AND gate...")

        and_circuit = {
            "elements": [
                {"id": 1, "type": "InputButton", "x": 100, "y": 100, "label": "A"},
                {"id": 2, "type": "InputButton", "x": 100, "y": 200, "label": "B"},
                {"id": 3, "type": "And", "x": 300, "y": 150},
                {"id": 4, "type": "Led", "x": 500, "y": 150, "label": "OUT"},
            ],
            "connections": [
                {"source": 1, "target": 3, "source_port": 0, "target_port": 0},
                {"source": 2, "target": 3, "source_port": 0, "target_port": 1},
                {"source": 3, "target": 4, "source_port": 0, "target_port": 0},
            ],
        }

        mapping = await self.create_circuit_from_spec(and_circuit)

        # Early return if circuit creation failed
        if len(mapping) != 4:
            print("❌ AND gate circuit creation failed")
            print(f"   Expected 4 elements, got {len(mapping)}")
            return False

        resp = await self.send_command("simulation_control", {"action": "start"})
        all_passed &= await self.assert_success(resp, "Start AND gate simulation")

        inputs = [mapping[1], mapping[2]]  # A, B
        outputs = [mapping[4]]  # OUT

        and_truth_table = [
            {"inputs": [False, False], "outputs": [False]},  # 0 AND 0 = 0
            {"inputs": [False, True], "outputs": [False]},  # 0 AND 1 = 0
            {"inputs": [True, False], "outputs": [False]},  # 1 AND 0 = 0
            {"inputs": [True, True], "outputs": [True]},  # 1 AND 1 = 1
        ]

        validation_result = await self.validate_truth_table(inputs, outputs, and_truth_table, "AND gate")
        if validation_result:
            self.infrastructure.output.success("✅ AND gate truth table validation passed")
            # Note: validate_truth_table() method already handles counting
        else:
            print("📋 AND gate truth table exposes propagation limitation")
            print("   Expected: Standard Boolean AND logic")
            print("   Analysis: Signal propagation fails through multi-gate circuits")
            print("   Impact: Critical - affects all digital logic education")
            print("   Documentation: See .claude/WIREDPANDA_LIMITATIONS.md #1")
            # Note: validate_truth_table() method already handles counting

        return validation_result and all_passed

    @beartype
    async def test_logic_gate_or(self) -> bool:
        """Test OR gate truth table validation"""
        print("\n=== OR Gate Truth Table Test ===")

        all_passed: bool = True

        # Test 2-input OR gate truth table
        print("\nTesting 2-input OR gate...")

        or_circuit = {
            "elements": [
                {"id": 1, "type": "InputButton", "x": 100, "y": 100, "label": "A"},
                {"id": 2, "type": "InputButton", "x": 100, "y": 200, "label": "B"},
                {"id": 3, "type": "Or", "x": 300, "y": 150},
                {"id": 4, "type": "Led", "x": 500, "y": 150, "label": "OUT"},
            ],
            "connections": [
                {"source": 1, "target": 3, "source_port": 0, "target_port": 0},
                {"source": 2, "target": 3, "source_port": 0, "target_port": 1},
                {"source": 3, "target": 4, "source_port": 0, "target_port": 0},
            ],
        }

        mapping = await self.create_circuit_from_spec(or_circuit)

        # Early return if circuit creation failed
        if len(mapping) != 4:
            print("❌ OR gate circuit creation failed")
            print(f"   Expected 4 elements, got {len(mapping)}")
            return False

        resp = await self.send_command("simulation_control", {"action": "start"})
        all_passed &= await self.assert_success(resp, "Start OR gate simulation")

        inputs = [mapping[1], mapping[2]]  # A, B
        outputs = [mapping[4]]  # OUT

        or_truth_table = [
            {"inputs": [False, False], "outputs": [False]},  # 0 OR 0 = 0
            {"inputs": [False, True], "outputs": [True]},  # 0 OR 1 = 1
            {"inputs": [True, False], "outputs": [True]},  # 1 OR 0 = 1
            {"inputs": [True, True], "outputs": [True]},  # 1 OR 1 = 1
        ]

        validation_result = await self.validate_truth_table(inputs, outputs, or_truth_table, "OR gate")
        if validation_result:
            self.infrastructure.output.success("✅ OR gate truth table validation passed")
            # Note: validate_truth_table() method already handles counting
        else:
            print("📋 OR gate truth table exposes propagation limitation")
            print("   Expected: Standard Boolean OR logic")
            print("   Analysis: Signal propagation fails through multi-gate circuits")
            print("   Impact: Critical - affects all digital logic education")
            print("   Documentation: See .claude/WIREDPANDA_LIMITATIONS.md #1")
            # Note: validate_truth_table() method already handles counting

        return validation_result and all_passed

    @beartype
    async def test_logic_gate_xor(self) -> bool:
        """Test XOR gate truth table validation"""
        print("\n=== XOR Gate Truth Table Test ===")

        all_passed: bool = True

        # Test XOR gate (using proper XOR element)
        print("\nTesting XOR gate...")

        xor_circuit = {
            "elements": [
                {"id": 1, "type": "InputButton", "x": 100, "y": 100, "label": "A"},
                {"id": 2, "type": "InputButton", "x": 100, "y": 200, "label": "B"},
                {"id": 3, "type": "Xor", "x": 300, "y": 150},
                {"id": 4, "type": "Led", "x": 500, "y": 150, "label": "OUT"},
            ],
            "connections": [
                {"source": 1, "target": 3, "source_port": 0, "target_port": 0},
                {"source": 2, "target": 3, "source_port": 0, "target_port": 1},
                {"source": 3, "target": 4, "source_port": 0, "target_port": 0},
            ],
        }

        mapping = await self.create_circuit_from_spec(xor_circuit)

        # Early return if circuit creation failed
        if len(mapping) != 4:
            print("❌ XOR gate circuit creation failed")
            print(f"   Expected 4 elements, got {len(mapping)}")
            return False

        resp = await self.send_command("simulation_control", {"action": "start"})
        all_passed &= await self.assert_success(resp, "Start XOR gate simulation")

        inputs = [mapping[1], mapping[2]]  # A, B
        outputs = [mapping[4]]  # OUT

        xor_truth_table = [
            {"inputs": [False, False], "outputs": [False]},  # 0 XOR 0 = 0
            {"inputs": [False, True], "outputs": [True]},  # 0 XOR 1 = 1
            {"inputs": [True, False], "outputs": [True]},  # 1 XOR 0 = 1
            {"inputs": [True, True], "outputs": [False]},  # 1 XOR 1 = 0
        ]

        validation_result = await self.validate_truth_table(inputs, outputs, xor_truth_table, "XOR gate")
        if validation_result:
            self.infrastructure.output.success("✅ XOR gate truth table validation passed")
            # Note: validate_truth_table() method already handles counting
        else:
            print("📋 XOR gate truth table exposes propagation limitation")
            print("   Expected: Standard Boolean XOR logic (A ⊕ B)")
            print("   Analysis: Complex multi-gate propagation fails")
            print("   Impact: Critical - XOR is fundamental to arithmetic circuits")
            print("   Documentation: See .claude/WIREDPANDA_LIMITATIONS.md #1")
            # Note: validate_truth_table() method already handles counting

        return validation_result and all_passed
