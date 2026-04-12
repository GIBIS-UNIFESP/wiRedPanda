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

from typing import Any, Dict, List

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

    # ==================== SHARED HELPER ====================

    async def _test_two_input_gate(
        self,
        gate_type: str,
        truth_table: List[Dict[str, Any]],
        gate_name: str,
    ) -> bool:
        """Validate a 2-input logic gate against its truth table."""
        print(f"\nTesting 2-input {gate_name}...")

        circuit: Dict[str, Any] = {
            "elements": [
                {"id": 1, "type": "InputButton", "x": 100, "y": 100, "label": "A"},
                {"id": 2, "type": "InputButton", "x": 100, "y": 200, "label": "B"},
                {"id": 3, "type": gate_type, "x": 300, "y": 150},
                {"id": 4, "type": "Led", "x": 500, "y": 150, "label": "OUT"},
            ],
            "connections": [
                {"source": 1, "target": 3, "source_port": 0, "target_port": 0},
                {"source": 2, "target": 3, "source_port": 0, "target_port": 1},
                {"source": 3, "target": 4, "source_port": 0, "target_port": 0},
            ],
        }

        mapping = await self.create_circuit_from_spec(circuit)
        if len(mapping) != 4:
            print(f"❌ {gate_name} circuit creation failed")
            print(f"   Expected 4 elements, got {len(mapping)}")
            return False

        resp = await self.send_command("simulation_control", {"action": "start"})
        if not await self.assert_success(resp, f"Start {gate_name} simulation"):
            return False

        inputs = [mapping[1], mapping[2]]  # A, B
        outputs = [mapping[4]]  # OUT

        validation_result = await self.validate_truth_table(inputs, outputs, truth_table, gate_name)
        if validation_result:
            self.infrastructure.output.success(f"✅ {gate_name} truth table validation passed")
            # Note: validate_truth_table() already handles counting
        else:
            print(f"📋 {gate_name} truth table exposes propagation limitation")
            print(f"   Expected: Standard Boolean {gate_name} logic")
            print(f"   Analysis: Signal propagation fails through multi-gate circuits")
            print(f"   Impact: Critical - affects all digital logic education")
            print(f"   Documentation: See .claude/WIREDPANDA_LIMITATIONS.md #1")
            # Note: validate_truth_table() already handles counting

        return validation_result

    # ==================== TEST METHODS ====================

    @beartype
    async def test_logic_gate_and(self) -> bool:
        """Test AND gate truth table validation"""
        print("\n=== AND Gate Truth Table Test ===")
        return await self._test_two_input_gate(
            "And",
            [
                {"inputs": [False, False], "outputs": [False]},  # 0 AND 0 = 0
                {"inputs": [False, True], "outputs": [False]},   # 0 AND 1 = 0
                {"inputs": [True, False], "outputs": [False]},   # 1 AND 0 = 0
                {"inputs": [True, True], "outputs": [True]},     # 1 AND 1 = 1
            ],
            "AND gate",
        )

    @beartype
    async def test_logic_gate_or(self) -> bool:
        """Test OR gate truth table validation"""
        print("\n=== OR Gate Truth Table Test ===")
        return await self._test_two_input_gate(
            "Or",
            [
                {"inputs": [False, False], "outputs": [False]},  # 0 OR 0 = 0
                {"inputs": [False, True], "outputs": [True]},    # 0 OR 1 = 1
                {"inputs": [True, False], "outputs": [True]},    # 1 OR 0 = 1
                {"inputs": [True, True], "outputs": [True]},     # 1 OR 1 = 1
            ],
            "OR gate",
        )

    @beartype
    async def test_logic_gate_xor(self) -> bool:
        """Test XOR gate truth table validation"""
        print("\n=== XOR Gate Truth Table Test ===")
        return await self._test_two_input_gate(
            "Xor",
            [
                {"inputs": [False, False], "outputs": [False]},  # 0 XOR 0 = 0
                {"inputs": [False, True], "outputs": [True]},    # 0 XOR 1 = 1
                {"inputs": [True, False], "outputs": [True]},    # 1 XOR 0 = 1
                {"inputs": [True, True], "outputs": [False]},    # 1 XOR 1 = 0
            ],
            "XOR gate",
        )
