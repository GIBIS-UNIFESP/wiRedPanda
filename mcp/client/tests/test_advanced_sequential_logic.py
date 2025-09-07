#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Advanced Sequential Logic Tests

Tests for complex sequential logic elements and timing behavior including:
- SR latch and D flip-flop validation
- JK and T flip-flop state table validation
- Setup/hold timing and critical path analysis
- Sequential element state transitions

MCP test implementation
"""

import asyncio
import time
from typing import Any, Dict, List, Union

from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class AdvancedSequentialLogicTests(MCPTestBase):
    """Tests for complex sequential logic elements and timing behavior"""

    async def run_category_tests(self) -> bool:
        """Run all advanced sequential logic tests

        Returns:
            bool: True if all tests passed, False otherwise
        """
        tests = [
            self.test_sequential_sr_latch_validation,
            self.test_sequential_dflipflop_validation,
            self.test_jk_flipflop_state_table_validation,
            self.test_tflipflop_divide_by_2_operation,
            self.test_setup_hold_timing_validation,
            self.test_critical_path_timing_analysis,
        ]

        print("\n" + "=" * 60)
        print("🧪 RUNNING ADVANCED SEQUENTIAL LOGIC TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    # ==================== TEST METHODS ====================
    # Test method implementations

    @beartype
    async def test_sequential_sr_latch_validation(self) -> bool:
        """Test SR Latch sequential circuit state validation"""
        print("\n=== SR Latch Sequential Circuit Test ===")
        self.set_test_context("test_sequential_sr_latch_validation")

        all_passed: bool = True

        # Test SR Latch (Set-Reset Latch)
        print("\nTesting SR Latch...")  # SR Latch using NOR gates: Q = ~(R + ~Q), ~Q = ~(S + Q)
        sr_latch_circuit = {
            "elements": [
                # Inputs
                {"id": 1, "type": "InputButton", "x": 50, "y": 100, "label": "S"},  # Set
                {"id": 2, "type": "InputButton", "x": 50, "y": 200, "label": "R"},  # Reset
                {"id": 3, "type": "InputButton", "x": 50, "y": 300, "label": "EN"},  # Enable (Clock)
                # Logic gates for SR Latch with Enable
                {"id": 4, "type": "And", "x": 150, "y": 100, "label": "S_AND_EN"},  # S AND EN
                {"id": 5, "type": "And", "x": 150, "y": 200, "label": "R_AND_EN"},  # R AND EN
                {"id": 6, "type": "Nor", "x": 250, "y": 120, "label": "NOR1"},  # First NOR
                {"id": 7, "type": "Nor", "x": 250, "y": 180, "label": "NOR2"},  # Second NOR
                # Outputs
                {"id": 8, "type": "Led", "x": 350, "y": 120, "label": "Q"},  # Q output
                {"id": 9, "type": "Led", "x": 350, "y": 180, "label": "Q_NOT"},  # ~Q output
            ],
            "connections": [
                # Input connections
                {"source": 1, "target": 4, "source_port": 0, "target_port": 0},  # S -> S_AND_EN
                {"source": 3, "target": 4, "source_port": 0, "target_port": 1},  # EN -> S_AND_EN
                {"source": 2, "target": 5, "source_port": 0, "target_port": 0},  # R -> R_AND_EN
                {"source": 3, "target": 5, "source_port": 0, "target_port": 1},  # EN -> R_AND_EN
                # Cross-coupled NOR gates (SR Latch core)
                {"source": 5, "target": 6, "source_port": 0, "target_port": 0},  # R_AND_EN -> NOR1
                {"source": 7, "target": 6, "source_port": 0, "target_port": 1},  # NOR2 -> NOR1 (feedback)
                {"source": 4, "target": 7, "source_port": 0, "target_port": 0},  # S_AND_EN -> NOR2
                {"source": 6, "target": 7, "source_port": 0, "target_port": 1},  # NOR1 -> NOR2 (feedback)
                # Output connections
                {"source": 6, "target": 8, "source_port": 0, "target_port": 0},  # NOR1 -> Q
                {"source": 7, "target": 9, "source_port": 0, "target_port": 0},  # NOR2 -> ~Q
            ],
        }

        mapping = await self.create_circuit_from_spec(sr_latch_circuit)

        # Early return if SR Latch circuit creation failed
        if len(mapping) != 9:
            print(f"❌ SR Latch circuit creation failed: {len(mapping)}/9 elements created")
            return False

        resp = await self.send_command("simulation_control", {"action": "start"})
        all_passed &= await self.assert_success(resp, "Start SR Latch simulation")

        inputs = [mapping[1], mapping[2]]  # S, R
        outputs = [mapping[8], mapping[9]]  # Q, ~Q
        clock_id = mapping[3]  # Enable

        # SR Latch state table with Enable
        sr_latch_state_table = [
            {
                "description": "Initial state - Enable low (no change)",
                "inputs": [False, False],
                "outputs": [False, True],  # Assume initial state Q=0, ~Q=1
                "clock_edge": "level",
                "clock_level": False,
            },
            {
                "description": "Set operation - S=1, R=0, EN=1",
                "inputs": [True, False],
                "outputs": [True, False],  # Q=1, ~Q=0 (Set state)
                "clock_edge": "level",
                "clock_level": True,
            },
            {
                "description": "Hold state - S=0, R=0, EN=1",
                "inputs": [False, False],
                "outputs": [True, False],  # Q=1, ~Q=0 (Hold previous)
                "clock_edge": "level",
                "clock_level": True,
            },
            {
                "description": "Reset operation - S=0, R=1, EN=1",
                "inputs": [False, True],
                "outputs": [False, True],  # Q=0, ~Q=1 (Reset state)
                "clock_edge": "level",
                "clock_level": True,
            },
        ]

        validation_result = await self.validate_sequential_state_table(inputs, outputs, clock_id, sr_latch_state_table, "SR Latch")
        all_passed &= validation_result
        if validation_result:
            self.infrastructure.output.success("✅ SR Latch state table validation passed")
            # Note: validate_sequential_state_table() method already handles counting
        else:
            print("📋 SR Latch exposes sequential circuit limitation")
            print("   Expected: Cross-coupled feedback and memory behavior")
            print("   Analysis: Sequential state transitions fail")
            print("   Impact: Critical - sequential logic education impossible")
            print("   Documentation: See .claude/WIREDPANDA_LIMITATIONS.md #2")
            self.test_results.known_issues.append("Test: R Latch: Sequential state transitions fail")
            # Note: validate_sequential_state_table() method already handles counting

        return all_passed

    @beartype
    async def test_sequential_dflipflop_validation(self) -> bool:
        """Test D Flip-Flop sequential circuit state validation"""
        print("\n=== D Flip-Flop Sequential Circuit Test ===")
        self.set_test_context("test_sequential_dflipflop_validation")

        all_passed: bool = True

        # Test D Flip-Flop behavior
        print("\nTesting D Flip-Flop...")

        # Check if DFlipFlop element exists in wiRedPanda
        resp = await self.send_command(
            "create_element",
            {
                "type": "DFlipFlop",
                "x": 200,
                "y": 150,
                "label": "DFF",
            },
        )

        if not resp.success:
            print("📋 D Flip-Flop element not available")
            print("   Expected: Native DFlipFlop element support")
            print("   Analysis: Missing fundamental sequential element")
            print("   Impact: Severe - core sequential logic unavailable")
            print("   Status: Element availability limitation")
            all_passed = False  # Element not available counts as test failure
            return all_passed

        # Use native DFlipFlop element
        dff_circuit = {
            "elements": [
                {"id": 1, "type": "InputButton", "x": 50, "y": 100, "label": "D"},  # Data
                {"id": 2, "type": "InputButton", "x": 50, "y": 200, "label": "CLK"},  # Clock
                {"id": 3, "type": "DFlipFlop", "x": 200, "y": 150, "label": "DFF"},  # D Flip-Flop
                {"id": 4, "type": "Led", "x": 350, "y": 130, "label": "Q"},  # Q output
                {"id": 5, "type": "Led", "x": 350, "y": 170, "label": "Q_NOT"},  # ~Q output
            ],
            "connections": [
                {"source": 1, "target": 3, "source_port": 0, "target_port": 0},  # D -> DFF.D
                {"source": 2, "target": 3, "source_port": 0, "target_port": 1},  # CLK -> DFF.CLK
                {"source": 3, "target": 4, "source_port": 0, "target_port": 0},  # DFF.Q -> LED
                {"source": 3, "target": 5, "source_port": 1, "target_port": 0},  # DFF.~Q -> LED
            ],
        }

        # Clear the test element first
        mapping = await self.create_circuit_from_spec(dff_circuit)

        # Early return if D Flip-Flop circuit creation failed
        if len(mapping) != 5:
            print(f"❌ D Flip-Flop circuit creation failed: {len(mapping)}/5 elements created")
            return False

        resp = await self.send_command("simulation_control", {"action": "start"})
        all_passed &= await self.assert_success(resp, "Start D Flip-Flop simulation")

        inputs = [mapping[1]]  # D input
        outputs = [mapping[4], mapping[5]]  # Q, ~Q
        clock_id = mapping[2]  # Clock

        # D Flip-Flop state table
        dff_state_table: List[Dict[str, Any]] = [
            {
                "description": "Initial state - Q=0",
                "inputs": [False],
                "outputs": [False, True],
                "clock_edge": "rising",
            },
            {
                "description": "Load D=1 on rising edge",
                "inputs": [True],
                "outputs": [True, False],
                "clock_edge": "rising",
            },
            {
                "description": "Hold state with D=0 (no clock edge)",
                "inputs": [False],
                "outputs": [True, False],  # Should hold previous state
                "clock_edge": "level",
                "clock_level": True,
            },
            {
                "description": "Load D=0 on rising edge",
                "inputs": [False],
                "outputs": [False, True],
                "clock_edge": "rising",
            },
        ]

        validation_result = await self.validate_sequential_state_table(inputs, outputs, clock_id, dff_state_table, "D Flip-Flop")
        all_passed &= validation_result
        if not validation_result:
            print("📋 D Flip-Flop exposes sequential circuit limitation")
            print("   Expected: Edge-triggered memory behavior")
            print("   Analysis: Sequential state transitions fail")
            print("   Impact: Critical - sequential logic education impossible")
            print("   Documentation: See .claude/WIREDPANDA_LIMITATIONS.md #3")
            self.test_results.known_issues.append("Test:  Flip-Flop: Sequential state transitions fail")
            # Note: validate_sequential_state_table() method already handles counting
            return all_passed

        self.infrastructure.output.success("✅ D Flip-Flop state table validation passed")
        # Note: validate_sequential_state_table() method already handles counting

        return all_passed

    @beartype
    async def test_jk_flipflop_state_table_validation(self) -> bool:
        """Test JK flip-flop toggle functionality and state table validation"""
        print("\n=== JK Flip-Flop State Table Validation Test ===")
        self.set_test_context("test_jk_flipflop_state_table_validation")

        all_passed: bool = True

        print("Testing JK flip-flop toggle functionality...")

        # Test: K Flip-Flop State Table Validation
        jk_circuit_spec: Dict[str, Any] = {
            "elements": [
                {"id": 1, "type": "InputButton", "label": "J", "x": 0, "y": 0},
                {"id": 2, "type": "InputButton", "label": "K", "x": 0, "y": 100},
                {"id": 3, "type": "InputButton", "label": "CLK", "x": 0, "y": 200},
                {"id": 4, "type": "JKFlipFlop", "label": "JK_FF", "x": 200, "y": 100},
                {"id": 5, "type": "Led", "label": "Q", "x": 400, "y": 50},
                {"id": 6, "type": "Led", "label": "Q_NOT", "x": 400, "y": 150},
            ],
            "connections": [
                {"source": 1, "target": 4, "source_port": 0, "target_port": 0},  # J -> JK_FF.J (port 0)
                {"source": 2, "target": 4, "source_port": 0, "target_port": 2},  # K -> JK_FF.K (port 2)
                {"source": 3, "target": 4, "source_port": 0, "target_port": 1},  # CLK -> JK_FF.CLK (port 1)
                {"source": 4, "target": 5, "source_port": 0, "target_port": 0},  # JK_FF.Q -> Q
                {"source": 4, "target": 6, "source_port": 1, "target_port": 0},  # JK_FF.Q_NOT -> Q_NOT
            ],
        }

        # JK Flip-Flop State Table
        jk_state_table: List[Dict[str, Any]] = [
            # J, K, CLK_edge -> Expected Q, Q_NOT, Description
            {"inputs": [0, 0, 1], "expected_q": None, "expected_qn": None, "description": "Hold state (no change)"},
            {"inputs": [1, 0, 1], "expected_q": 1, "expected_qn": 0, "description": "Set state (Q=1)"},
            {"inputs": [0, 1, 1], "expected_q": 0, "expected_qn": 1, "description": "Reset state (Q=0)"},
            {"inputs": [1, 1, 1], "expected_q": "toggle", "expected_qn": "toggle", "description": "Toggle state"},
        ]

        element_mapping = await self.create_circuit_from_spec(jk_circuit_spec)

        # Early return if JK flip-flop circuit creation failed
        if not element_mapping:
            print("❌ Failed to create JK flip-flop circuit")
            return False

        all_passed &= True  # Circuit creation succeeded

        self.infrastructure.output.success("✅ Setup JK flip-flop circuit")

        resp = await self.send_command("simulation_control", {"action": "start"})
        all_passed &= await self.assert_success(resp, "Start JK flip-flop simulation")

        # Early return if simulation failed to start
        if not resp.success:
            print("❌ JK flip-flop simulation failed to start")
            return False

        print("\nTesting JK flip-flop state table behavior...")

        # Initialize to known state
        await self.send_command("set_input_value", {"element_id": element_mapping[2], "value": True})  # K=1
        await self.send_command("set_input_value", {"element_id": element_mapping[3], "value": False})  # CLK=0

        # Double read for propagation
        _temp_resp13 = await self.send_command("get_output_value", {"element_id": element_mapping[5]})
        _temp_resp14 = await self.send_command("get_output_value", {"element_id": element_mapping[5]})

        # Clock rising edge to reset to Q=0
        await self.send_command("set_input_value", {"element_id": element_mapping[3], "value": True})  # CLK rising edge
        _temp_resp15 = await self.send_command("get_output_value", {"element_id": element_mapping[5]})

        previous_q = False  # Start with Q=0 after reset
        jk_test_results = []

        # Test each state in the JK flip-flop state table
        for i, state in enumerate(jk_state_table):
            j_val, k_val, _ = state["inputs"]
            expected_q = state["expected_q"]
            expected_qn = state["expected_qn"]
            description = state["description"]

            print(f"\nTesting JK state {i + 1}: J={j_val}, K={k_val} -> {description}")

            # Set inputs
            await self.send_command("set_input_value", {"element_id": element_mapping[1], "value": bool(j_val)})
            await self.send_command("set_input_value", {"element_id": element_mapping[2], "value": bool(k_val)})

            # Clock transition (falling edge then rising edge)
            await self.send_command("set_input_value", {"element_id": element_mapping[3], "value": False})
            _temp_resp = await self.send_command("get_output_value", {"element_id": element_mapping[5]})

            await self.send_command("set_input_value", {"element_id": element_mapping[3], "value": True})

            # Double read for propagation
            _temp_resp16 = await self.send_command("get_output_value", {"element_id": element_mapping[5]})
            _temp_resp17 = await self.send_command("get_output_value", {"element_id": element_mapping[5]})

            # Read final outputs
            q_resp = await self.send_command("get_output_value", {"element_id": element_mapping[5]})
            qn_resp = await self.send_command("get_output_value", {"element_id": element_mapping[6]})

            actual_q = q_resp.result.get("value", False) if q_resp.success and q_resp.result else False
            actual_qn = qn_resp.result.get("value", False) if qn_resp.success and qn_resp.result else False

            # Determine expected values for toggle case
            if expected_q == "toggle":
                expected_q = not previous_q
                expected_qn = not expected_q
            elif expected_q is None:  # Hold state
                expected_q = previous_q
                expected_qn = not previous_q

            test_passed = (actual_q == expected_q) and (actual_qn == expected_qn)
            status = "✅" if test_passed else "❌"

            print(f"  {status} State {i}: J={j_val}, K={k_val} -> Q={actual_q}, Q_NOT={actual_qn} ({description})")
            if not test_passed:
                print(f"      Expected: Q={expected_q}, Q_NOT={expected_qn}")

            jk_test_results.append(
                {
                    "state": i,
                    "inputs": [j_val, k_val],
                    "expected": [expected_q, expected_qn],
                    "actual": [actual_q, actual_qn],
                    "passed": test_passed,
                    "description": description,
                }
            )

            previous_q = actual_q

        # Analyze JK flip-flop behavior
        print("\n--- JK Flip-Flop Analysis ---")
        passed_tests = sum(1 for result in jk_test_results if result["passed"])
        total_tests = len(jk_test_results)
        pass_rate = (passed_tests / total_tests * 100) if total_tests > 0 else 0

        print(f"📋 JK flip-flop test results: {passed_tests}/{total_tests} passed ({pass_rate:.1f}%)")

        # Exact JK flip-flop validation - expect 100% pass rate
        expected_pass_rate = 100.0
        if pass_rate == expected_pass_rate:
            self.infrastructure.output.success(f"✅ JK flip-flop validation: {pass_rate:.1f}% pass rate")
            all_passed &= True
        elif pass_rate < expected_pass_rate:
            print(f"📋 JK flip-flop limitation detected: {pass_rate:.1f}% pass rate (expected {expected_pass_rate:.1f}%)")
            print("   Analysis: JK flip-flop state transitions not working correctly")
            print("   Impact: Advanced sequential logic elements unreliable")
            print(f"   Failed states: {[r['description'] for r in jk_test_results if not r['passed']]}")
            all_passed = False
        else:
            print("❌ JK flip-flop validation failed")
            return False

        return all_passed

    @beartype
    async def test_tflipflop_divide_by_2_operation(self) -> bool:
        """Test T flip-flop divide-by-2 counter functionality"""
        print("\n=== T Flip-Flop Divide-by-2 Operation Test ===")
        self.set_test_context("test_tflipflop_divide_by_2_operation")

        all_passed: bool = True

        # Create new circuit first
        print("Testing T flip-flop divide-by-2 operation...")

        # Test: T Flip-Flop Divide-by-2 Counter
        t_circuit_spec: Dict[str, Any] = {
            "elements": [
                {"id": 1, "type": "InputButton", "label": "T", "x": 0, "y": 0},
                {"id": 2, "type": "InputButton", "label": "CLK", "x": 0, "y": 100},
                {"id": 3, "type": "TFlipFlop", "label": "T_FF", "x": 200, "y": 50},
                {"id": 4, "type": "Led", "label": "Q_OUT", "x": 400, "y": 50},
            ],
            "connections": [
                {"source": 1, "target": 3, "source_port": 0, "target_port": 0},  # T -> T_FF.T (port 0)
                {"source": 2, "target": 3, "source_port": 0, "target_port": 1},  # CLK -> T_FF.CLK (port 1)
                {"source": 3, "target": 4, "source_port": 0, "target_port": 0},  # T_FF.Q -> Q_OUT
            ],
        }

        element_mapping = await self.create_circuit_from_spec(t_circuit_spec)
        if element_mapping:
            self.infrastructure.output.success("✅ Setup T flip-flop circuit")

            resp = await self.send_command("simulation_control", {"action": "start"})
            all_passed &= await self.assert_success(resp, "Start T flip-flop simulation")
            if resp.success:
                print("\nTesting T flip-flop divide-by-2 behavior...")

                # Initialize to known state: Set T=0, then create a clock edge to reset flip-flop
                await self.send_command("set_input_value", {"element_id": element_mapping[1], "value": False})  # T=0 (no toggle)
                await self.send_command("set_input_value", {"element_id": element_mapping[2], "value": False})  # CLK=0
                await self.send_command("set_input_value", {"element_id": element_mapping[2], "value": True})   # CLK edge (no effect since T=0)
                await self.send_command("set_input_value", {"element_id": element_mapping[2], "value": False})  # CLK=0
                
                # Get initial state (should be predictable now)
                initial_resp = await self.send_command("get_output_value", {"element_id": element_mapping[4]})
                initial_state = initial_resp.result.get("value", False) if initial_resp.success and initial_resp.result else False
                print(f"Initial T flip-flop state: Q={initial_state}")
                
                # Now enable T input for toggle mode
                await self.send_command("set_input_value", {"element_id": element_mapping[1], "value": True})  # T=1

                t_flip_states: List[Dict[str, Union[int, bool]]] = []

                for cycle in range(8):  # Test for 8 clock cycles
                    # Create clean clock transition (low to high) for edge triggering
                    await self.send_command("set_input_value", {"element_id": element_mapping[2], "value": False})
                    
                    # Rising edge - T Flip-Flop should toggle on this edge when T=1
                    await self.send_command("set_input_value", {"element_id": element_mapping[2], "value": True})
                    
                    # Read output after edge propagation
                    q_resp = await self.send_command("get_output_value", {"element_id": element_mapping[4]})
                    q_value = q_resp.result.get("value", False) if q_resp.success and q_resp.result else False

                    t_flip_states.append(
                        {
                            "cycle": cycle,
                            "q_output": q_value,
                        }
                    )

                    print(f"Cycle {cycle}: Q={q_value}")

                # Analyze divide-by-2 behavior
                print("\n--- T Flip-Flop Analysis ---")

                # Check for alternating pattern (divide-by-2)
                alternating_pattern = True
                if len(t_flip_states) > 1:
                    for i in range(1, len(t_flip_states)):
                        # Should toggle every cycle
                        if t_flip_states[i]["q_output"] == t_flip_states[i - 1]["q_output"]:
                            alternating_pattern = False
                            break

                transition_count = sum(1 for i in range(1, len(t_flip_states)) if t_flip_states[i]["q_output"] != t_flip_states[i - 1]["q_output"])
                expected_transitions = len(t_flip_states) - 1

                # T flip-flop should work 100% reliably - this is basic digital logic
                # With proper initialization, there should be no timing variations  
                if alternating_pattern and transition_count == expected_transitions:
                    self.infrastructure.output.success("✅ T flip-flop divide-by-2 operation working correctly")
                    print(f"   Transitions: {transition_count}/{expected_transitions}")
                    all_passed &= True
                else:
                    print("❌ T flip-flop divide-by-2 operation failed")
                    print("   Analysis: Output not alternating correctly on each clock cycle")
                    print("   Expected: Perfect alternating pattern (divide-by-2)")
                    print(f"   Actual transitions: {transition_count}/{expected_transitions}")
                    print(f"   Alternating pattern: {alternating_pattern}")
                    print("   State sequence:")
                    for i, state in enumerate(t_flip_states):
                        marker = "❌" if i > 0 and state["q_output"] == t_flip_states[i-1]["q_output"] else "✅"
                        print(f"     {marker} Cycle {state['cycle']}: Q={state['q_output']}")
                    print("   This indicates a bug in T flip-flop implementation or test setup")
                    all_passed = False
        else:
            print("❌ Failed to create T flip-flop test circuit")
            return False

        return all_passed

    @beartype
    async def test_setup_hold_timing_validation(self) -> bool:
        """Test educational timing model consistency with D flip-flop circuit
        
        Validates that the zero-delay educational model provides consistent
        edge-triggered behavior without real-world timing complexity.
        """
        print("\n=== Educational Timing Model Validation Test ===")
        self.set_test_context("test_setup_hold_timing_validation")

        all_passed: bool = True

        # Create new circuit first
        print("Testing educational zero-delay timing model...")

        # Test: Setup/Hold Time Validation Circuit
        setup_hold_spec: Dict[str, Any] = {
            "elements": [
                {"id": 1, "type": "InputButton", "label": "DATA", "x": 0, "y": 0},
                {"id": 2, "type": "InputButton", "label": "CLK", "x": 0, "y": 100},
                {"id": 3, "type": "DFlipFlop", "label": "DFF", "x": 200, "y": 50},
                {"id": 4, "type": "Led", "label": "Q_OUT", "x": 400, "y": 50},
                {"id": 5, "type": "Led", "label": "CLK_MONITOR", "x": 400, "y": 100},
            ],
            "connections": [
                {"source": 1, "target": 3, "source_port": 0, "target_port": 0},  # DATA -> DFF.D
                {"source": 2, "target": 3, "source_port": 0, "target_port": 1},  # CLK -> DFF.CLK (port 1)
                {"source": 3, "target": 4, "source_port": 0, "target_port": 0},  # DFF.Q -> Q_OUT
                {"source": 2, "target": 5, "source_port": 0, "target_port": 0},  # CLK -> CLK_MONITOR
            ],
        }

        element_mapping = await self.create_circuit_from_spec(setup_hold_spec)
        if element_mapping:
            self.infrastructure.output.success("✅ Setup setup/hold timing test circuit")

            # Ensure simulation is stopped before starting
            await self.send_command("simulation_control", {"action": "stop"})
            resp = await self.send_command("simulation_control", {"action": "start"})
            all_passed &= await self.assert_success(resp, "Start setup/hold simulation")
            if resp.success:
                print("\nTesting educational model consistency...")

                timing_tests: List[Dict[str, Any]] = []

                # Test various data-to-clock scenarios in zero-delay educational model  
                # Focus on edge-triggered behavior consistency
                test_scenarios: List[Dict[str, Union[bool, bool, float, str]]] = [
                    {"data_value": True, "data_early": True, "description": "Data=1 setup before clock edge"},
                    {"data_value": False, "data_early": True, "description": "Data=0 setup before clock edge"},
                    {"data_value": True, "data_early": False, "description": "Data=1 change after clock edge"},
                ]

                for i, scenario in enumerate(test_scenarios):
                    print(f"\n  Testing scenario {i + 1}: {scenario['description']}")

                    # Reset to known state  
                    await self.send_command("set_input_value", {"element_id": element_mapping[1], "value": False})  # DATA=0
                    await self.send_command("set_input_value", {"element_id": element_mapping[2], "value": False})  # CLK=0
                    
                    # Force a clock edge with DATA=0 to reset the flip-flop
                    await self.send_command("set_input_value", {"element_id": element_mapping[2], "value": True})   # CLK rising with DATA=0
                    await self.send_command("set_input_value", {"element_id": element_mapping[2], "value": False})  # CLK back to 0

                    # Get initial state (should be Q=0 now)
                    initial_q_resp = await self.send_command("get_output_value", {"element_id": element_mapping[4]})
                    initial_q = initial_q_resp.result.get("value", False) if initial_q_resp.success and initial_q_resp.result else False

                    # Apply test scenario
                    data_value = scenario["data_value"]
                    if scenario["data_early"]:
                        # Data changes before clock
                        await self.send_command("set_input_value", {"element_id": element_mapping[1], "value": data_value})
                        
                        # Then clock edge
                        await self.send_command("set_input_value", {"element_id": element_mapping[2], "value": True})  # CLK rising
                    else:
                        # Clock edge first, then data change (should not affect output)
                        await self.send_command("set_input_value", {"element_id": element_mapping[2], "value": True})  # CLK rising
                        await self.send_command("set_input_value", {"element_id": element_mapping[1], "value": data_value})

                    # Allow propagation
                    await self.send_command("get_output_value", {"element_id": element_mapping[4]})
                    await self.send_command("get_output_value", {"element_id": element_mapping[4]})

                    final_q_resp = await self.send_command("get_output_value", {"element_id": element_mapping[4]})
                    final_q = final_q_resp.result.get("value", False) if final_q_resp.success and final_q_resp.result else False

                    # Analyze timing behavior
                    # Educational Model: D flip-flop captures data on clock edge regardless of timing
                    # This is pedagogically correct - students learn edge-triggered behavior without timing complexity
                    
                    # In educational model, D flip-flop captures D input value present at rising clock edge
                    # Educational behavior: consistent edge-triggered response
                    if scenario["data_early"]:
                        # DATA was set before CLK edge, so DFF should capture that DATA value
                        expected_q = scenario["data_value"]
                    else:
                        # CLK edge occurred before DATA change, so DFF captures old DATA value (0)
                        expected_q = False  # DATA was still 0 when CLK edge occurred
                        
                    timing_correct = (final_q == expected_q)
                    status = "✅" if timing_correct else "📋"

                    print(f"    {status} Initial Q={initial_q}, Final Q={final_q} (expected Q={expected_q})")
                    if not timing_correct:
                        print(f"        Expected Q: {expected_q}, Actual Q: {final_q}")

                    timing_tests.append(
                        {
                            "scenario": scenario["description"],
                            "expected_q": expected_q,
                            "actual_q": final_q,
                            "correct": timing_correct,
                        }
                    )

                # Analyze educational model behavior
                print("\n--- Educational Timing Model Analysis ---")

                correct_timing = sum(1 for test in timing_tests if test["correct"])
                total_timing = len(timing_tests)
                timing_accuracy = (correct_timing / total_timing * 100) if total_timing > 0 else 0

                print(f"📋 Educational model consistency: {timing_accuracy:.1f}% ({correct_timing}/{total_timing})")

                # Educational Model: Expect 100% accuracy with zero-delay abstraction
                expected_timing_accuracy = 100.0
                if timing_accuracy == expected_timing_accuracy:
                    self.infrastructure.output.success(f"✅ Educational timing model: {timing_accuracy:.1f}% consistency")
                    print("   ✅ Zero-delay model correctly abstracts timing complexity for education")
                    all_passed &= True
                elif timing_accuracy < expected_timing_accuracy:
                    print(f"📋 Educational model inconsistency: {timing_accuracy:.1f}% accuracy (expected {expected_timing_accuracy:.1f}%)")
                    print("   Analysis: Edge-triggered behavior not consistent")
                    print("   Impact: Educational model should be predictable")
                    failed_scenarios = [test["scenario"] for test in timing_tests if not test["correct"]]
                    print(f"   Inconsistent scenarios: {failed_scenarios}")
                    all_passed = False
                else:
                    print("❌ Educational timing model validation failed")
                    return False
        else:
            print("❌ Failed to create setup/hold timing test circuit")
            all_passed = False

        return all_passed

    @beartype
    async def test_critical_path_timing_analysis(self) -> bool:
        """Test critical path timing analysis through multi-gate logic chains"""
        print("\n=== Critical Path Timing Analysis Test ===")
        self.set_test_context("test_critical_path_timing_analysis")

        all_passed: bool = True

        # Create new circuit first
        print("Testing propagation delay effects...")

        # Test: Critical Path Timing Analysis
        critical_path_spec: Dict[str, Any] = {
            "elements": [
                {"id": 1, "type": "InputButton", "label": "A", "x": 0, "y": 0},
                {"id": 2, "type": "InputButton", "label": "B", "x": 0, "y": 100},
                {"id": 3, "type": "InputButton", "label": "C", "x": 0, "y": 200},
                {"id": 4, "type": "And", "label": "AND1", "x": 150, "y": 0},
                {"id": 5, "type": "And", "label": "AND2", "x": 150, "y": 100},
                {"id": 6, "type": "Or", "label": "OR1", "x": 300, "y": 50},
                {"id": 7, "type": "Not", "label": "NOT1", "x": 450, "y": 50},
                {"id": 8, "type": "Led", "label": "CRITICAL_OUT", "x": 600, "y": 50},
            ],
            "connections": [
                {"source": 1, "target": 4, "source_port": 0, "target_port": 0},  # A -> AND1
                {"source": 2, "target": 4, "source_port": 0, "target_port": 1},  # B -> AND1
                {"source": 2, "target": 5, "source_port": 0, "target_port": 0},  # B -> AND2
                {"source": 3, "target": 5, "source_port": 0, "target_port": 1},  # C -> AND2
                {"source": 4, "target": 6, "source_port": 0, "target_port": 0},  # AND1 -> OR1
                {"source": 5, "target": 6, "source_port": 0, "target_port": 1},  # AND2 -> OR1
                {"source": 6, "target": 7, "source_port": 0, "target_port": 0},  # OR1 -> NOT1
                {"source": 7, "target": 8, "source_port": 0, "target_port": 0},  # NOT1 -> CRITICAL_OUT
            ],
        }

        element_mapping = await self.create_circuit_from_spec(critical_path_spec)

        if element_mapping:
            self.infrastructure.output.success("✅ Setup critical path timing circuit")

            # Ensure simulation is stopped before starting
            await self.send_command("simulation_control", {"action": "stop"})
            resp = await self.send_command("simulation_control", {"action": "start"})
            all_passed &= await self.assert_success(resp, "Start critical path simulation")
            if resp.success:
                print("\nTesting critical path propagation timing...")

                # Test different input combinations and measure propagation
                test_vectors: List[List[bool]] = [
                    [True, True, False],  # A=1, B=1, C=0 -> AND1=1, AND2=0 -> OR1=1 -> NOT1=0
                    [False, True, True],  # A=0, B=1, C=1 -> AND1=0, AND2=1 -> OR1=1 -> NOT1=0
                    [False, False, False],  # A=0, B=0, C=0 -> AND1=0, AND2=0 -> OR1=0 -> NOT1=1
                ]

                propagation_times: List[float] = []
                logic_correctness: List[bool] = []

                for i, (a, b, c) in enumerate(test_vectors):
                    prop_start = time.time()

                    # Set inputs
                    await self.send_command("set_input_value", {"element_id": element_mapping[1], "value": a})
                    await self.send_command("set_input_value", {"element_id": element_mapping[2], "value": b})
                    await self.send_command("set_input_value", {"element_id": element_mapping[3], "value": c})

                    # Get output after propagation through critical path
                    output_resp = await self.send_command("get_output_value", {"element_id": element_mapping[8]})
                    output_val = output_resp.result.get("value", False) if output_resp.success and output_resp.result else False

                    prop_time = time.time() - prop_start

                    # Calculate expected output: NOT((A AND B) OR (B AND C))
                    expected = not ((a and b) or (b and c))
                    correct = output_val == expected

                    propagation_times.append(prop_time)
                    logic_correctness.append(correct)

                    status = "✅" if correct else "❌"
                    print(f"  {status} Vector {i + 1}: A={a}, B={b}, C={c} -> Output={output_val} (expected {expected}) [{prop_time:.3f}s]")

                # Analyze critical path performance
                print("\n--- Critical Path Timing Analysis ---")

                avg_prop_time = sum(propagation_times) / len(propagation_times)
                max_prop_time = max(propagation_times)
                logic_success_rate = sum(logic_correctness) / len(logic_correctness) * 100

                print("📋 Critical path metrics:")
                print(f"   Average propagation time: {avg_prop_time:.3f}s")
                print(f"   Maximum propagation time: {max_prop_time:.3f}s")
                print(f"   Logic correctness: {logic_success_rate:.1f}%")

                # Timing thresholds for critical path
                # Communication overhead timing validation - expect <=0.15s for 4-gate path + MCP round-trips
                # Note: This measures MCP command execution time, not pure logic propagation delay
                expected_max_critical_path_time = 0.15
                if avg_prop_time == expected_max_critical_path_time:
                    print(f"📋 Critical path timing at threshold: {avg_prop_time:.3f}s average")
                elif avg_prop_time > expected_max_critical_path_time:
                    print(f"📋 Critical path timing limitation: {avg_prop_time:.3f}s average (expected ≤{expected_max_critical_path_time:.1f}s) for 4-gate path + MCP overhead")
                else:
                    self.infrastructure.output.success(f"✅ Critical path timing good: {avg_prop_time:.3f}s average")

                # Exact logic success validation - expect 100% success rate
                expected_logic_success = 100.0
                if logic_success_rate == expected_logic_success:
                    self.infrastructure.output.success(f"✅ Critical path logic: {logic_success_rate:.1f}% success rate")
                elif logic_success_rate < expected_logic_success:
                    print(f"📋 Critical path logic limitation: {logic_success_rate:.1f}% success rate (expected {expected_logic_success:.1f}%)")
                    print("   Analysis: Multi-gate paths not computing correct Boolean functions")
                    print("   Impact: Complex logic circuits produce wrong results")

                # Check if both timing and logic meet exact requirements
                if avg_prop_time <= expected_max_critical_path_time and logic_success_rate == expected_logic_success:
                    self.infrastructure.output.success("✅ Critical path validation passed")
                    all_passed &= True
                else:
                    print("❌ Critical path validation failed")
                    all_passed = False

        else:
            print("❌ Failed to create critical path test circuit")
            return False

        return all_passed
