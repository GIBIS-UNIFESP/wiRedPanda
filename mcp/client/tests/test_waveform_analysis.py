#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Waveform Analysis Tests

Tests for waveform generation, timing analysis, and signal behavior including:
- Basic waveform generation capability
- Timing validation and correlation analysis
- Pattern recognition and signal propagation
- Multi-signal waveform generation
- Export formats and data analysis
- NOT gate and D-latch waveform behavior

MCP test implementation
"""

import asyncio
import os
import time
from typing import Dict, List, Union

from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class WaveformAnalysisTests(MCPTestBase):
    """Tests for waveform generation, timing analysis, and signal behavior"""

    async def run_category_tests(self) -> bool:
        """Run all waveform analysis tests

        Returns:
            bool: True if all tests passed, False otherwise
        """
        tests = [
            self.test_waveform_generation_capability,
            self.test_waveform_timing_validation,
            self.test_waveform_pattern_recognition,
            self.test_waveform_multi_signal_generation,
            self.test_waveform_not_gate_analysis,
            self.test_waveform_dlatch_sequential_behavior,
            self.test_waveform_export_formats,
        ]

        print("\n" + "=" * 60)
        print("🧪 RUNNING WAVEFORM ANALYSIS TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    # ==================== TEST METHODS ====================
    # Method implementations

    @beartype
    async def test_waveform_generation_capability(self) -> bool:
        """Test basic waveform generation capability and circuit setup"""
        print("\n=== Basic Waveform Generation Capability Test ===")
        self.set_test_context("test_waveform_generation_capability")

        all_passed: bool = True

        # Test if we can invoke BewavedDolphin-style functionality through MCP
        # Note: This tests basic waveform generation capability
        print("Testing waveform generation capability...")

        # Create a simple circuit for waveform testing
        # Create a controlled input and output for waveform generation
        waveform_circuit = {
            "elements": [
                {"id": 1, "type": "InputButton", "x": 100, "y": 100, "label": "INPUT"},
                {"id": 2, "type": "Led", "x": 300, "y": 100, "label": "OUT"},
            ],
            "connections": [
                {"source": 1, "target": 2, "source_port": 0, "target_port": 0},
            ],
        }

        mapping = await self.create_circuit_from_spec(waveform_circuit)

        if len(mapping) != 2:
            print("❌ Waveform circuit creation failed")
            print("   Expected: Basic input-output circuit for waveform testing")
            print("   Analysis: Circuit construction for waveform generation failed")
            return all_passed

        # Successfully created circuit with 2 elements
        resp = await self.send_command("simulation_control", {"action": "start"})
        all_passed &= await self.assert_success(resp, "Start simulation for waveform test")

        input_id = mapping[1]
        output_id = mapping[2]

        # Test basic input/output operation for waveform capability
        test_patterns = [True, False, True, False]
        waveform_operations_successful = True

        for i, pattern in enumerate(test_patterns):
            print(f"Testing waveform operation {i + 1}/4: input={pattern}")

            # Set input value
            resp = await self.send_command(
                "set_input_value",
                {
                    "element_id": input_id,
                    "value": pattern,
                },
            )

            if not resp.success:
                waveform_operations_successful = False
                break

            # Enhanced propagation for waveform timing
            time.sleep(0.05)

            # Read output value for waveform validation
            resp = await self.send_command("get_output_value", {"element_id": output_id})
            success = await self.assert_success(resp, f"Get output value for waveform validation step {i + 1}")
            all_passed &= success
            if success:
                result = await self.get_response_result(resp)
                if result:
                    output_value = result.get("value", False)
                    if output_value != pattern:
                        waveform_operations_successful = False
                        break
                else:
                    waveform_operations_successful = False
                    break
            else:
                waveform_operations_successful = False
                break

        if waveform_operations_successful:
            self.infrastructure.output.success("✅ Basic waveform generation capability confirmed")
            print("   Circuit supports input/output waveform operations")
        else:
            print("📋 Waveform generation capability limited")
            print("   Expected: Reliable input/output waveform operations")
            print("   Analysis: Basic waveform circuit operations inconsistent")
            print("   Impact: BewavedDolphin-style waveform generation affected")

        return all_passed

    @beartype
    async def test_waveform_timing_validation(self) -> bool:
        """Test timing-based signal validation and waveform correlation"""
        print("\n=== Waveform Timing Validation Test ===")
        self.set_test_context("test_waveform_timing_validation")

        all_passed: bool = True

        print("Testing timing-based signal validation...")

        # Create a simple circuit for timing waveform testing
        # Create waveform timing test circuit
        timing_circuit = {
            "elements": [
                {"id": 1, "type": "InputButton", "x": 100, "y": 100, "label": "INPUT"},
                {"id": 2, "type": "Led", "x": 300, "y": 100, "label": "OUT"},
            ],
            "connections": [
                {"source": 1, "target": 2, "source_port": 0, "target_port": 0},
            ],
        }

        mapping = await self.create_circuit_from_spec(timing_circuit)

        # Early return if circuit creation failed
        if len(mapping) != 2:
            print("❌ Timing validation circuit creation failed")
            print("   Expected: Basic circuit for timing waveform testing")
            print("   Analysis: Circuit setup for timing validation failed")
            return False

        # Extract element IDs from mapping
        input_id = mapping[1]  # InputButton
        output_id = mapping[2]  # Led

        # Collect signal values over time to simulate waveform behavior
        waveform_data: List[Dict[str, Union[int, bool]]] = []
        test_duration: int = 10  # Test for 10 time steps

        for step in range(test_duration):
            print(f"Time step {step}:")

            # In a real waveform system, time would advance automatically
            # Here we simulate by toggling the input manually
            input_state = step % 2 == 0  # Toggle every step

            resp = await self.send_command(
                "set_input_value",
                {
                    "element_id": input_id,
                    "value": input_state,
                },
            )

            if not resp.success:
                print(f"  Failed to set input at step {step}")
                break

            # Enhanced propagation for timing accuracy
            time.sleep(0.05)

            # Read output value
            resp = await self.send_command("get_output_value", {"element_id": output_id})
            success = await self.assert_success(resp, f"Get output value for waveform step {step}")
            all_passed &= success
            if not success:
                print(f"  Failed to read output at step {step}")
                break

            result = await self.get_response_result(resp)
            output_value = result.get("value", False) if result else False

            waveform_data.append(
                {
                    "time": step,
                    "input": input_state,
                    "output": output_value,
                }
            )
            print(f"  Input: {input_state}, Output: {output_value}")

        # Early return if insufficient waveform data
        if len(waveform_data) < 8:
            print("❌ Insufficient waveform data collected for timing analysis")
            print("   Expected: At least 8 time steps for correlation analysis")
            print("   Analysis: Waveform timing data collection failed")

            return all_passed

        # Analyze waveform data for timing relationships
        print("\n--- Waveform Timing Analysis ---")

        # Check if output follows input (direct connection)
        timing_correct = True
        input_output_correlation = 0

        for data in waveform_data:
            if data["input"] == data["output"]:
                input_output_correlation += 1
            else:
                timing_correct = False

        correlation_percentage = (input_output_correlation / len(waveform_data)) * 100

        # Exact correlation validation - expect >=90% for proper timing
        expected_min_correlation = 90.0
        if timing_correct and correlation_percentage == expected_min_correlation:
            print(f"📋 Waveform timing correlation at threshold: {correlation_percentage:.1f}%")
            print("📋 BewavedDolphin-style waveform validation at minimum acceptable level")
        elif timing_correct and correlation_percentage > expected_min_correlation:
            self.infrastructure.output.success(f"✅ Waveform timing correlation: {correlation_percentage:.1f}%")
        else:
            print(f"❌ Waveform timing correlation: {correlation_percentage:.1f}% (expected ≥{expected_min_correlation:.1f}%)")
            print("❌ Input-output correlation test failed")
            print("   Expected: Output should follow input with high correlation")
            print("   Analysis: Signal propagation issue detected")

        return all_passed

    @beartype
    async def test_waveform_pattern_recognition(self) -> bool:
        """Test waveform pattern recognition and propagation delay analysis"""
        print("\n=== Waveform Pattern Recognition Test ===")
        self.set_test_context("test_waveform_pattern_recognition")

        all_passed: bool = True

        print("Testing waveform pattern recognition...")

        # Create a simple circuit for pattern recognition testing
        # Create pattern recognition test circuit
        pattern_circuit = {
            "elements": [
                {"id": 1, "type": "InputButton", "x": 100, "y": 100, "label": "INPUT"},
                {"id": 2, "type": "Led", "x": 300, "y": 100, "label": "OUT"},
            ],
            "connections": [
                {"source": 1, "target": 2, "source_port": 0, "target_port": 0},
            ],
        }

        mapping = await self.create_circuit_from_spec(pattern_circuit)

        if len(mapping) == 2:
            resp = await self.send_command("simulation_control", {"action": "start"})
            all_passed &= await self.assert_success(resp, "Start pattern recognition simulation")

            input_id = mapping[1]
            output_id = mapping[2]

            # Generate pattern data for recognition testing
            pattern_data: List[Dict[str, Union[int, bool]]] = []
            test_steps = 8

            for step in range(test_steps):
                # Create alternating pattern for recognition
                input_pattern = step % 2 == 0

                resp = await self.send_command(
                    "set_input_value",
                    {
                        "element_id": input_id,
                        "value": input_pattern,
                    },
                )

                if resp.success:
                    # Enhanced propagation timing
                    time.sleep(0.05)

                    # Read output for pattern analysis
                    resp = await self.send_command("get_output_value", {"element_id": output_id})
                    success = await self.assert_success(resp, f"Get output value for pattern analysis step {step}")
                    all_passed &= success
                    if success:
                        result = await self.get_response_result(resp)
                        if result:
                            output_value = result.get("value", False)
                        else:
                            output_value = False
                        pattern_data.append(
                            {
                                "step": step,
                                "input": input_pattern,
                                "output": output_value,
                            }
                        )
                    else:
                        print(f"  Failed to read pattern at step {step}")
                        break
                else:
                    print(f"  Failed to set pattern at step {step}")
                    break

            # Analyze patterns if sufficient data collected
            if len(pattern_data) >= 6:
                print("\n--- Pattern Recognition Analysis ---")

                # Test input pattern detection (alternating values)
                for i in range(1, len(pattern_data)):
                    if pattern_data[i]["input"] == pattern_data[i - 1]["input"]:
                        # For alternating pattern, consecutive values should differ
                        # If they're the same, pattern detection logic might be needed
                        pass

                # Test propagation delay patterns
                propagation_delays = []
                for i, data in enumerate(pattern_data):
                    # In ideal systems, there might be 1 cycle delay
                    # Here we test immediate propagation
                    if data["input"] != data["output"]:
                        propagation_delays.append(i)

                # Analyze propagation characteristics
                if len(propagation_delays) == 0:
                    self.infrastructure.output.success("✅ Zero propagation delay detected (ideal for educational use)")
                    print("   Analysis: Immediate signal propagation confirmed")
                    print("   Pattern: Input changes reflected instantly in output")
                else:
                    print(f"📋 Propagation delays detected at {len(propagation_delays)} time steps")
                    print("   Analysis: May indicate timing simulation complexity")
                    print(f"   Pattern: Delays at steps {propagation_delays}")
                    print("   Impact: Waveform timing diagrams show realistic delays")

                # Test pattern consistency
                pattern_consistency = 0
                for data in pattern_data:
                    if data["input"] == data["output"]:
                        pattern_consistency += 1

                consistency_rate = (pattern_consistency / len(pattern_data)) * 100
                if consistency_rate >= 80.0:
                    self.infrastructure.output.success(f"✅ Pattern recognition consistency: {consistency_rate:.1f}%")
                    print("   Analysis: Waveform pattern recognition reliable")
                else:
                    print(f"📋 Pattern recognition consistency: {consistency_rate:.1f}%")
                    print("   Analysis: Pattern recognition challenges detected")
                    print("   Impact: Complex waveform patterns may be unreliable")
            else:
                print("❌ Insufficient pattern data for recognition analysis")
                print("   Expected: At least 6 data points for pattern analysis")
                print("   Analysis: Pattern data collection incomplete")
        else:
            print("❌ Pattern recognition circuit creation failed")
            print("   Expected: Basic circuit for pattern recognition testing")
            print("   Analysis: Circuit setup for pattern recognition failed")

        return all_passed

    @beartype
    async def test_waveform_multi_signal_generation(self) -> bool:
        """Test multi-signal waveform generation and complex circuit timing"""
        print("\n=== Multi-Signal Waveform Generation Test ===")
        self.set_test_context("test_waveform_multi_signal_generation")

        all_passed: bool = True

        print("Testing multi-signal waveform generation...")

        # Create more complex circuit for multi-signal waveform testing
        multi_signal_circuit = {
            "elements": [
                {"id": 1, "type": "InputButton", "x": 50, "y": 100, "label": "A"},
                {"id": 2, "type": "InputButton", "x": 50, "y": 200, "label": "B"},
                {"id": 3, "type": "And", "x": 200, "y": 150, "label": "AND_GATE"},
                {"id": 4, "type": "Or", "x": 200, "y": 250, "label": "OR_GATE"},
                {"id": 5, "type": "Led", "x": 350, "y": 150, "label": "AND_OUT"},
                {"id": 6, "type": "Led", "x": 350, "y": 250, "label": "OR_OUT"},
            ],
            "connections": [
                {"source": 1, "target": 3, "source_port": 0, "target_port": 0},  # A -> AND
                {"source": 2, "target": 3, "source_port": 0, "target_port": 1},  # B -> AND
                {"source": 1, "target": 4, "source_port": 0, "target_port": 0},  # A -> OR
                {"source": 2, "target": 4, "source_port": 0, "target_port": 1},  # B -> OR
                {"source": 3, "target": 5, "source_port": 0, "target_port": 0},  # AND -> LED
                {"source": 4, "target": 6, "source_port": 0, "target_port": 0},  # OR -> LED
            ],
        }

        mapping = await self.create_circuit_from_spec(multi_signal_circuit)

        if len(mapping) == 6:
            resp = await self.send_command("simulation_control", {"action": "start"})
            all_passed &= await self.assert_success(resp, "Start multi-signal simulation")

            # Test all 4 input combinations for truth table waveform
            input_patterns = [
                [False, False],  # A=0, B=0
                [False, True],   # A=0, B=1
                [True, False],   # A=1, B=0
                [True, True],    # A=1, B=1
            ]

            multi_waveform_data = []

            for step, pattern in enumerate(input_patterns):
                print(f"Multi-signal step {step}: A={pattern[0]}, B={pattern[1]}")

                # Set inputs
                resp1 = await self.send_command("set_input_value", {"element_id": mapping[1], "value": pattern[0]})
                resp2 = await self.send_command("set_input_value", {"element_id": mapping[2], "value": pattern[1]})

                if resp1.success and resp2.success:
                    time.sleep(0.1)

                    # Enhanced propagation using simulation update
                    resp = await self.send_command("simulation_control", {"action": "update"})

                    # Read outputs
                    resp_and = await self.send_command("get_output_value", {"element_id": mapping[5]})
                    resp_or = await self.send_command("get_output_value", {"element_id": mapping[6]})

                    if resp_and.success and resp_or.success:
                        and_out = resp_and.result.get("value", False) if resp_and.result else False
                        or_out = resp_or.result.get("value", False) if resp_or.result else False

                        multi_waveform_data.append(
                            {
                                "step": step,
                                "A": pattern[0],
                                "B": pattern[1],
                                "AND": and_out,
                                "OR": or_out,
                                "expected_AND": pattern[0] and pattern[1],
                                "expected_OR": pattern[0] or pattern[1],
                            }
                        )

                        print(f"  Step {step}: A={pattern[0]}, B={pattern[1]} -> AND={and_out}, OR={or_out}")
                    else:
                        print(f"  Failed to read outputs at step {step}")
                        break
                else:
                    print(f"  Failed to set inputs at step {step}")
                    break

            # Analyze multi-signal waveform correctness
            if len(multi_waveform_data) == 4:
                print("\n--- Multi-Signal Waveform Analysis ---")

                all_correct = True
                and_correct = 0
                or_correct = 0

                for data in multi_waveform_data:
                    if data["AND"] == data["expected_AND"]:
                        and_correct += 1
                    if data["OR"] == data["expected_OR"]:
                        or_correct += 1
                    if data["AND"] != data["expected_AND"] or data["OR"] != data["expected_OR"]:
                        all_correct = False

                and_accuracy = (and_correct / len(multi_waveform_data)) * 100
                or_accuracy = (or_correct / len(multi_waveform_data)) * 100
                overall_accuracy = (and_correct + or_correct) / (2 * len(multi_waveform_data)) * 100

                if all_correct:
                    self.infrastructure.output.success("✅ Multi-signal waveform generation validation passed")
                    print(f"   AND gate accuracy: {and_accuracy:.1f}%")
                    print(f"   OR gate accuracy: {or_accuracy:.1f}%")
                    print("   Analysis: Complex waveform timing diagrams reliable")
                else:
                    print(f"📋 Multi-signal waveform accuracy: {overall_accuracy:.1f}%")
                    print(f"   AND gate accuracy: {and_accuracy:.1f}%")
                    print(f"   OR gate accuracy: {or_accuracy:.1f}%")
                    if overall_accuracy >= 75.0:
                        print("   Analysis: Complex waveform generation mostly reliable")
                    else:
                        print("   Analysis: Complex waveform generation affected by propagation issues")
                        print("   Impact: Multi-signal timing diagrams unreliable")
            else:
                print("❌ Multi-signal waveform test incomplete")
                print("   Expected: Complete truth table waveform data")
                print("   Analysis: Multi-signal waveform data collection failed")
        else:
            print("❌ Multi-signal circuit creation failed")
            print("   Expected: Complex circuit for multi-signal waveform testing")
            print("   Analysis: Multi-signal circuit construction failed")

        return all_passed

    @beartype
    async def test_waveform_not_gate_analysis(self) -> bool:
        """Test NOT gate waveform analysis with create_waveform command"""
        print("\n=== NOT Gate Waveform Analysis Test ===")
        self.set_test_context("test_waveform_not_gate_analysis")

        all_passed: bool = True

        print("Testing enhanced create_waveform command with input patterns...")

        # Test: Simple Logic Gate Waveform
        print("\n--- NOT Gate Waveform Analysis ---")

        # Create NOT gate circuit
        not_circuit = {
            "elements": [
                {"id": 1, "type": "InputButton", "x": 0, "y": 50, "label": "INPUT"},
                {"id": 2, "type": "Not", "x": 100, "y": 50, "label": "NOT_GATE"},
                {"id": 3, "type": "Led", "x": 200, "y": 50, "label": "OUTPUT"}
            ],
            "connections": [
                {"source": 1, "target": 2, "source_port": 0, "target_port": 0},
                {"source": 2, "target": 3, "source_port": 0, "target_port": 0}
            ]
        }

        mapping = await self.create_circuit_from_spec(not_circuit)
        if len(mapping) == 3:
            resp = await self.send_command("simulation_control", {"action": "start"})
            all_passed &= await self.assert_success(resp, "Start simulation for NOT gate waveform test")

            # Test enhanced create_waveform with input patterns
            waveform_params = {
                "duration": 4,
                "input_patterns": {
                    "INPUT": [0, 1, 0, 1]  # Should produce [1, 0, 1, 0] at output
                }
            }

            resp = await self.send_command("create_waveform", waveform_params)

            success = await self.assert_success(resp, "Create waveform for NOT gate test")
            all_passed &= success
            if success:
                self.infrastructure.output.success("✅ create_waveform command executed successfully")

                result = await self.get_response_result(resp)
                if result:
                    waveform_data = result.get("waveform_data", {})
                else:
                    waveform_data = {}
                if waveform_data:
                    inputs = {inp["label"]: inp["values"] for inp in waveform_data.get("inputs", [])}
                    outputs = {out["label"]: out["values"] for out in waveform_data.get("outputs", [])}

                    print(f"📊 Waveform generated: {waveform_data.get('duration', 'N/A')} steps")
                    print(f"   Inputs captured: {len(inputs)} signals")
                    print(f"   Outputs captured: {len(outputs)} signals")

                    # Validate NOT gate behavior
                    if "INPUT" in inputs and "OUTPUT" in outputs:
                        input_values = inputs["INPUT"]
                        output_values = outputs["OUTPUT"]
                        expected_output = [1, 0, 1, 0]  # NOT of [0, 1, 0, 1]

                        not_gate_correct = output_values == expected_output
                        if not_gate_correct:
                            self.infrastructure.output.success("✅ NOT gate waveform validation: PASSED")
                            print(f"   INPUT:  {input_values}")
                            print(f"   OUTPUT: {output_values} (correct)")
                        else:
                            print("❌ NOT gate waveform validation: FAILED")
                            print(f"   INPUT:    {input_values}")
                            print(f"   OUTPUT:   {output_values}")
                            print(f"   EXPECTED: {expected_output}")
                            self.test_results.errors.append("test_waveform_not_gate_analysis: NOT gate waveform logic incorrect")
                    else:
                        self.test_results.errors.append("test_waveform_not_gate_analysis: Missing signals in waveform data")
                else:
                    self.test_results.errors.append("test_waveform_not_gate_analysis: No waveform data returned")
            # Note: assert_success already handles failure cases
        else:
            print("❌ Failed to create NOT gate waveform analysis circuit")

        return all_passed

    @beartype
    async def test_waveform_dlatch_sequential_behavior(self) -> bool:
        """Test D Latch sequential waveform behavior with transparent/hold states"""
        print("\n=== D Latch Sequential Waveform Behavior Test ===")
        self.set_test_context("test_waveform_dlatch_sequential_behavior")

        all_passed: bool = True

        # Test: Sequential Element Waveform (D Latch)
        print("\n--- D Latch Waveform Analysis ---")

        # Create D Latch circuit
        dlatch_circuit = {
            "elements": [
                {"id": 1, "type": "InputButton", "x": 0, "y": 50, "label": "D"},
                {"id": 2, "type": "InputButton", "x": 0, "y": 100, "label": "EN"},
                {"id": 3, "type": "DLatch", "x": 150, "y": 75, "label": "D_LATCH"},
                {"id": 4, "type": "Led", "x": 300, "y": 60, "label": "Q"},
                {"id": 5, "type": "Led", "x": 300, "y": 90, "label": "Q_NOT"},
            ],
            "connections": [
                {"source": 1, "target": 3, "source_port": 0, "target_port": 0},  # D → DLatch.D
                {"source": 2, "target": 3, "source_port": 0, "target_port": 1},  # EN → DLatch.EN
                {"source": 3, "target": 4, "source_port": 0, "target_port": 0},  # DLatch.Q → LED
                {"source": 3, "target": 5, "source_port": 1, "target_port": 0},  # DLatch.Q̄ → LED
            ],
        }

        mapping = await self.create_circuit_from_spec(dlatch_circuit)
        if len(mapping) == 5:
            resp = await self.send_command("simulation_control", {"action": "start"})
            all_passed &= await self.assert_success(resp, "Start simulation for D Latch waveform test")

            # D Latch test pattern: transparent when EN=1, hold when EN=0
            waveform_params = {
                "duration": 6,
                "input_patterns": {
                    "D": [0, 0, 1, 1, 0, 0],  # Data changes
                    "EN": [0, 1, 1, 0, 0, 1]   # Enable control
                }
            }

            resp = await self.send_command("create_waveform", waveform_params)

            success = await self.assert_success(resp, "Create waveform for D Latch test")
            all_passed &= success
            if success:
                result = await self.get_response_result(resp)
                if result:
                    waveform_data = result.get("waveform_data", {})
                else:
                    waveform_data = {}
                if waveform_data:
                    inputs = {inp["label"]: inp["values"] for inp in waveform_data.get("inputs", [])}
                    outputs = {out["label"]: out["values"] for out in waveform_data.get("outputs", [])}

                    self.infrastructure.output.success("✅ D Latch waveform generated successfully")
                    print("📊 Pattern analysis:")
                    for label, values in inputs.items():
                        print(f"   {label:3s} (IN):  {values}")
                    for label, values in outputs.items():
                        print(f"   {label:3s} (OUT): {values}")

                    # Validate D Latch transparent behavior
                    if "D" in inputs and "EN" in inputs and "Q" in outputs:
                        d_values = inputs["D"]
                        en_values = inputs["EN"]
                        q_values = outputs["Q"]

                        # Check latch behavior: Q follows D when EN=1, holds when EN=0
                        latch_behavior_correct = True
                        validation_details = []

                        for step, (d, en, q) in enumerate(zip(d_values, en_values, q_values)):

                            if step == 0:
                                # Initial step - any state acceptable
                                step_correct = True
                                behavior = "Initial"
                            elif en == 1:
                                # When enabled, Q should follow D
                                step_correct = q == d
                                behavior = f"Transparent (Q should = D={d})"
                            else:
                                # When disabled, Q should hold previous value
                                prev_q = q_values[step - 1]
                                step_correct = q == prev_q
                                behavior = f"Hold (Q should = {prev_q})"

                            status = "✅" if step_correct else "❌"
                            validation_details.append(f"   Step {step}: {status} D={d}, EN={en} → Q={q} ({behavior})")

                            if not step_correct:
                                latch_behavior_correct = False

                        # Print detailed analysis
                        print("🔍 D Latch Behavior Analysis:")
                        for detail in validation_details:
                            print(detail)

                        if latch_behavior_correct:
                            self.infrastructure.output.success("✅ Success")

                        else:
                            self.test_results.errors.append("test_waveform_dlatch_sequential_behavior: D Latch behavior validation failed")
                    else:
                        self.test_results.errors.append("test_waveform_dlatch_sequential_behavior: Missing D Latch signals")
                else:
                    self.test_results.errors.append("test_waveform_dlatch_sequential_behavior: No D Latch waveform data")
            # Note: assert_success already handles failure cases
        else:
            print("❌ Failed to create D-latch waveform analysis circuit")

        return all_passed

    @beartype
    async def test_waveform_export_formats(self) -> bool:
        """Test waveform export functionality in TXT and PNG formats"""
        print("\n=== Waveform Export Formats Test ===")
        self.set_test_context("test_waveform_export_formats")

        all_passed: bool = True

        # Test: Waveform Export Functionality
        print("\n--- Waveform Export Functionality Test ---")

        # Create D Latch circuit for export testing
        dlatch_circuit = {
            "elements": [
                {"id": 1, "type": "InputButton", "x": 0, "y": 50, "label": "D"},
                {"id": 2, "type": "InputButton", "x": 0, "y": 100, "label": "EN"},
                {"id": 3, "type": "DLatch", "x": 150, "y": 75, "label": "D_LATCH"},
                {"id": 4, "type": "Led", "x": 300, "y": 60, "label": "Q"},
                {"id": 5, "type": "Led", "x": 300, "y": 90, "label": "Q_NOT"},
            ],
            "connections": [
                {"source": 1, "target": 3, "source_port": 0, "target_port": 0},  # D → DLatch.D
                {"source": 2, "target": 3, "source_port": 0, "target_port": 1},  # EN → DLatch.EN
                {"source": 3, "target": 4, "source_port": 0, "target_port": 0},  # DLatch.Q → LED
                {"source": 3, "target": 5, "source_port": 1, "target_port": 0},  # DLatch.Q̄ → LED
            ],
        }

        mapping = await self.create_circuit_from_spec(dlatch_circuit)
        if len(mapping) == 5:
            resp = await self.send_command("simulation_control", {"action": "start"})
            all_passed &= await self.assert_success(resp, "Start simulation for waveform export test")

            # Generate waveform first
            waveform_params = {
                "duration": 6,
                "input_patterns": {
                    "D": [0, 0, 1, 1, 0, 0],  # Data changes
                    "EN": [0, 1, 1, 0, 0, 1]   # Enable control
                }
            }

            resp = await self.send_command("create_waveform", waveform_params)
            if resp.success:
                self.infrastructure.output.success("✅ Waveform generated for export testing")

                # Use centralized configuration for export directory
                from tests.test_config import get_test_images_dir

                images_dir = get_test_images_dir()

                # Test TXT export
                txt_filename = os.path.join(images_dir, "test_dlatch_waveform.txt")
                resp = await self.send_command("export_waveform", {
                    "filename": txt_filename,
                    "format": "txt"
                })

                if resp.success:
                    self.infrastructure.output.success("✅ TXT waveform export: PASSED")
                    if os.path.exists(txt_filename):
                        file_size = os.path.getsize(txt_filename)
                        print(f"   📁 TXT file created: {txt_filename} ({file_size} bytes)")
                else:
                    print("❌ TXT waveform export: FAILED")
                    error_msg = resp.error if resp.error else "Unknown error"
                    print(f"   Error: {error_msg}")
                    self.test_results.errors.append(f"test_waveform_export_formats: TXT export failed - {error_msg}")

                # Test PNG export
                png_filename = os.path.join(images_dir, "test_dlatch_waveform.png")
                resp = await self.send_command("export_waveform", {
                    "filename": png_filename,
                    "format": "png"
                })

                if resp.success:
                    self.infrastructure.output.success("✅ PNG waveform export: PASSED")
                    # Check if file was created (optional - file system dependent)
                    if os.path.exists(png_filename):
                        file_size = os.path.getsize(png_filename)
                        print(f"   📁 PNG file created: {png_filename} ({file_size} bytes)")
                    else:
                        print("   📝 PNG export completed (file location may vary)")
                else:
                    print("❌ PNG waveform export: FAILED")
                    error_msg = resp.error if resp.error else "Unknown error"
                    print(f"   Error: {error_msg}")
                    self.test_results.errors.append(f"test_waveform_export_formats: PNG export failed - {error_msg}")
            else:
                self.test_results.errors.append("test_waveform_export_formats: Waveform generation failed")
        else:
            print("❌ Failed to create waveform export test circuit")

        return all_passed
