#!/usr/bin/env python3
"""
CPU Validation Level 2: Arithmetic Building Blocks
================================================

This module implements comprehensive validation for arithmetic circuits
that will form the foundation of the CPU's Arithmetic Logic Unit (ALU).

Components Validated:
1. Half Adder and Full Adder (fundamental arithmetic units)
2. Multi-bit Adders (4-bit and 8-bit ripple carry adders)
3. Subtraction Circuits (2's complement subtractors)
4. Comparator Circuits (equality, magnitude comparison)
5. Advanced Arithmetic (carry lookahead, overflow detection)

Target: 100% accuracy for all arithmetic operations
Prerequisites: Level 1 (Advanced Combinational Logic) - ✅ COMPLETED

Updated to use real wiRedPanda MCP integration.
"""

import sys
import json
import time
import random
from typing import Dict, Any, List, Tuple, Optional, Callable
from pathlib import Path
import itertools

# Import the real wiRedPanda bridge
from wiredpanda_bridge import WiredPandaBridge, WiredPandaError


class ArithmeticBlocksValidator:
    def __init__(self):
        self.bridge = None
        self.test_results = []
        self.current_test = None
        self.level_name = "Level 2: Arithmetic Building Blocks"
        
        # Layout constants for proper element positioning (IMPROVED FOR BETTER VISUAL CLARITY)
        # Based on GraphicElement analysis: 64x64px elements, labels at +64px below
        self.ELEMENT_WIDTH = 64
        self.ELEMENT_HEIGHT = 64
        self.LABEL_OFFSET = 64  # Labels positioned 64px below element center
        self.MIN_VERTICAL_SPACING = 150  # INCREASED for better readability
        self.MIN_HORIZONTAL_SPACING = 180  # INCREASED for cleaner visual flow
        self.GRID_START_X = 80
        self.GRID_START_Y = 100
    
    def _get_grid_position(self, col: int, row: int) -> Tuple[float, float]:
        """Get grid-based position with proper spacing for elements and labels."""
        x = float(self.GRID_START_X + col * self.MIN_HORIZONTAL_SPACING)
        y = float(self.GRID_START_Y + row * self.MIN_VERTICAL_SPACING)
        return x, y
    
    def _get_input_position(self, index: int) -> Tuple[float, float]:
        """Get position for input elements in a vertical stack."""
        return self._get_grid_position(0, index)
    
    def _get_output_position(self, col: int, row: int = 0) -> Tuple[float, float]:
        """Get position for output elements."""
        return self._get_grid_position(col, row)
        
    def _ensure_bridge(self):
        """Ensure bridge is connected"""
        if not self.bridge:
            self.bridge = WiredPandaBridge()
            self.bridge.start()
    
    def create_new_circuit(self) -> bool:
        """Create a new circuit by starting the application."""
        try:
            self._ensure_bridge()
            self.bridge.new_circuit()
            return True
        except WiredPandaError:
            return False
    
    def save_circuit(self, file_path: str) -> bool:
        """Save the current circuit to a .panda file."""
        try:
            self._ensure_bridge()
            self.bridge.save_circuit(file_path)
            print(f"  💾 Circuit saved: {file_path}")
            print(f"      Note: Real .panda file created by wiRedPanda")
            return True
        except WiredPandaError as e:
            print(f"  ❌ Failed to save circuit: {e}")
            return False
    
    def create_element(self, element_type: str, x: float, y: float, label: str = "") -> Optional[int]:
        """Create an element and return its ID."""
        try:
            self._ensure_bridge()
            return self.bridge.create_element(element_type, x, y, label)
        except WiredPandaError:
            return None
    
    def connect_elements(self, source_id: int, source_port: int, target_id: int, target_port: int) -> bool:
        """Connect two elements."""
        try:
            self._ensure_bridge()
            self.bridge.connect_elements(source_id, source_port, target_id, target_port)
            return True
        except WiredPandaError:
            return False
    
    def start_simulation(self) -> bool:
        """Start simulation."""
        try:
            self._ensure_bridge()
            self.bridge.start_simulation()
            return True
        except WiredPandaError:
            return False
    
    def step_simulation(self) -> bool:
        """Step simulation (single cycle)."""
        try:
            self._ensure_bridge()
            # Note: Real wiRedPanda doesn't have step mode, use restart instead
            self.bridge.restart_simulation()
            return True
        except WiredPandaError:
            return False
    
    def set_input(self, element_id: int, value: bool) -> bool:
        """Set input element value."""
        try:
            self._ensure_bridge()
            self.bridge.set_input_value(element_id, value)
            return True
        except WiredPandaError:
            return False
    
    def get_output(self, element_id: int) -> Optional[bool]:
        """Get output element value."""
        try:
            self._ensure_bridge()
            return self.bridge.get_output_value(element_id)
        except WiredPandaError:
            return None

    def test_basic_adders(self) -> dict:
        """
        Test 2.1: Basic Adders (Half Adder and Full Adder)
        Validate the fundamental building blocks of arithmetic circuits.
        """
        print("🔧 Test 2.1: Basic Adders (Half and Full Adders)")
        
        basic_adders = {
            "half_adder": {
                "description": "Half Adder: Sum = A XOR B, Carry = A AND B",
                "inputs": ["A", "B"],
                "outputs": ["Sum", "Carry"],
                "function": lambda a, b: ((a ^ b), (a & b)),
                "test_cases": [(False, False), (False, True), (True, False), (True, True)]
            },
            "full_adder": {
                "description": "Full Adder: Sum = A XOR B XOR Cin, Carry = (A AND B) OR (Cin AND (A XOR B))",
                "inputs": ["A", "B", "Cin"],
                "outputs": ["Sum", "Cout"],
                "function": lambda a, b, cin: ((a ^ b ^ cin), ((a & b) | (cin & (a ^ b)))),
                "test_cases": list(itertools.product([False, True], repeat=3))
            }
        }
        
        all_tests_passed = True
        test_results = {}
        
        for adder_name, adder_config in basic_adders.items():
            print(f"  Testing {adder_name}: {adder_config['description']}")
            
            if not self.create_new_circuit():
                test_results[adder_name] = {"success": False, "error": "Circuit creation failed"}
                all_tests_passed = False
                continue
            
            # Create adder circuit
            if adder_name == "half_adder":
                circuit_result = self._create_half_adder_circuit()
            elif adder_name == "full_adder":
                circuit_result = self._create_full_adder_circuit()
            else:
                circuit_result = None
            
            if not circuit_result:
                test_results[adder_name] = {"success": False, "error": "Circuit implementation failed"}
                all_tests_passed = False
                continue
                
            input_ids, output_ids = circuit_result
            
            if not self.start_simulation():
                test_results[adder_name] = {"success": False, "error": "Simulation start failed"}
                all_tests_passed = False
                continue
            
            # Test all input combinations
            adder_passed = True
            case_results = []
            
            for test_case in adder_config["test_cases"]:
                # Set inputs
                for i, input_val in enumerate(test_case):
                    if not self.set_input(input_ids[i], input_val):
                        adder_passed = False
                        break
                
                if not adder_passed:
                    break
                
                # Step simulation for propagation
                self.step_simulation()
                
                # Get outputs
                actual_outputs = []
                for output_id in output_ids:
                    output_val = self.get_output(output_id)
                    actual_outputs.append(output_val)
                
                # Calculate expected outputs
                expected_outputs = list(adder_config["function"](*test_case))
                
                # Check correctness
                case_correct = (actual_outputs == expected_outputs)
                if not case_correct:
                    adder_passed = False
                
                case_results.append({
                    "inputs": dict(zip(adder_config["inputs"], test_case)),
                    "expected": dict(zip(adder_config["outputs"], expected_outputs)),
                    "actual": dict(zip(adder_config["outputs"], actual_outputs)),
                    "correct": case_correct
                })
            
            test_results[adder_name] = {
                "success": adder_passed,
                "description": adder_config["description"],
                "total_cases": len(case_results),
                "passed_cases": sum(1 for case in case_results if case["correct"]),
                "failed_cases": sum(1 for case in case_results if not case["correct"]),
                "results": case_results,
                "accuracy": sum(1 for case in case_results if case["correct"]) / len(case_results) * 100
            }
            
            if not adder_passed:
                all_tests_passed = False
            
            # Save individual circuit file for this adder
            individual_filename = f"level2_{adder_name}.panda"
            self.save_circuit(individual_filename)
        
        return {
            "test": "basic_adders",
            "success": all_tests_passed,
            "results": test_results,
            "description": "Tests fundamental adder circuits (half and full adders)"
        }

    def _create_half_adder_circuit(self) -> Optional[Tuple[List[int], List[int]]]:
        """Create half adder circuit: Sum = A XOR B, Carry = A AND B"""
        # Create inputs with proper grid positioning
        a_x, a_y = self._get_input_position(0)
        a_id = self.create_element("InputButton", a_x, a_y, "A")
        b_x, b_y = self._get_input_position(1)
        b_id = self.create_element("InputButton", b_x, b_y, "B")
        
        if not all([a_id, b_id]):
            return None
        
        # Create logic gates with proper grid positioning
        xor_x, xor_y = self._get_grid_position(1, 0)
        xor_id = self.create_element("Xor", xor_x, xor_y, "XOR")  # Sum
        and_x, and_y = self._get_grid_position(1, 1)
        and_id = self.create_element("And", and_x, and_y, "AND")  # Carry
        
        if not all([xor_id, and_id]):
            return None
        
        # Create outputs with proper grid positioning
        sum_out_x, sum_out_y = self._get_grid_position(2, 0)
        sum_out_id = self.create_element("Led", sum_out_x, sum_out_y, "SUM")
        carry_out_x, carry_out_y = self._get_grid_position(2, 1)
        carry_out_id = self.create_element("Led", carry_out_x, carry_out_y, "CARRY")
        
        if not all([sum_out_id, carry_out_id]):
            return None
        
        # Connect circuit
        connections = [
            (a_id, 0, xor_id, 0),          # A -> XOR
            (b_id, 0, xor_id, 1),          # B -> XOR  
            (a_id, 0, and_id, 0),          # A -> AND
            (b_id, 0, and_id, 1),          # B -> AND
            (xor_id, 0, sum_out_id, 0),    # XOR -> SUM
            (and_id, 0, carry_out_id, 0)   # AND -> CARRY
        ]
        
        for source_id, source_port, target_id, target_port in connections:
            if not self.connect_elements(source_id, source_port, target_id, target_port):
                return None
        
        return [a_id, b_id], [sum_out_id, and_id]  # Return gate output, not LED

    def _create_full_adder_circuit(self) -> Optional[Tuple[List[int], List[int]]]:
        """Create full adder circuit: Sum = A XOR B XOR Cin, Carry = (A AND B) OR (Cin AND (A XOR B))"""
        # Create inputs with proper grid positioning
        a_x, a_y = self._get_input_position(0)
        a_id = self.create_element("InputButton", a_x, a_y, "A")
        b_x, b_y = self._get_input_position(1)
        b_id = self.create_element("InputButton", b_x, b_y, "B")
        cin_x, cin_y = self._get_input_position(2)
        cin_id = self.create_element("InputButton", cin_x, cin_y, "Cin")
        
        if not all([a_id, b_id, cin_id]):
            return None
        
        # Create logic gates for sum with proper grid positioning
        xor1_x, xor1_y = self._get_grid_position(1, 0)
        xor1_id = self.create_element("Xor", xor1_x, xor1_y, "XOR1")  # A XOR B
        xor2_x, xor2_y = self._get_grid_position(2, 0)
        xor2_id = self.create_element("Xor", xor2_x, xor2_y, "XOR2")  # (A XOR B) XOR Cin = Sum
        
        # Create logic gates for carry with proper grid positioning
        and1_x, and1_y = self._get_grid_position(1, 1)
        and1_id = self.create_element("And", and1_x, and1_y, "AND1")  # A AND B
        and2_x, and2_y = self._get_grid_position(2, 1)
        and2_id = self.create_element("And", and2_x, and2_y, "AND2")  # Cin AND (A XOR B)
        or_x, or_y = self._get_grid_position(3, 1)
        or_id = self.create_element("Or", or_x, or_y, "OR")       # Carry = AND1 OR AND2
        
        gates = [xor1_id, xor2_id, and1_id, and2_id, or_id]
        if not all(gates):
            return None
        
        # Create outputs with proper grid positioning
        sum_out_x, sum_out_y = self._get_grid_position(3, 0)
        sum_out_id = self.create_element("Led", sum_out_x, sum_out_y, "SUM")
        carry_out_x, carry_out_y = self._get_grid_position(4, 1)
        carry_out_id = self.create_element("Led", carry_out_x, carry_out_y, "CARRY")
        
        if not all([sum_out_id, carry_out_id]):
            return None
        
        # Connect circuit
        connections = [
            # Sum logic: A XOR B XOR Cin
            (a_id, 0, xor1_id, 0),         # A -> XOR1
            (b_id, 0, xor1_id, 1),         # B -> XOR1
            (xor1_id, 0, xor2_id, 0),      # XOR1 -> XOR2
            (cin_id, 0, xor2_id, 1),       # Cin -> XOR2
            (xor2_id, 0, sum_out_id, 0),   # XOR2 -> SUM
            
            # Carry logic: (A AND B) OR (Cin AND (A XOR B))
            (a_id, 0, and1_id, 0),         # A -> AND1
            (b_id, 0, and1_id, 1),         # B -> AND1
            (cin_id, 0, and2_id, 0),       # Cin -> AND2
            (xor1_id, 0, and2_id, 1),      # XOR1 -> AND2
            (and1_id, 0, or_id, 0),        # AND1 -> OR
            (and2_id, 0, or_id, 1),        # AND2 -> OR
            (or_id, 0, carry_out_id, 0)    # OR -> CARRY
        ]
        
        for source_id, source_port, target_id, target_port in connections:
            if not self.connect_elements(source_id, source_port, target_id, target_port):
                return None
        
        return [a_id, b_id, cin_id], [sum_out_id, or_id]  # Return gate output, not LED

    def test_multi_bit_adders(self) -> dict:
        """
        Test 2.2: Multi-bit Adders (4-bit and 8-bit Ripple Carry Adders)
        Validate arithmetic circuits that will be used in the CPU ALU.
        """
        print("🔧 Test 2.2: Multi-bit Adders (4-bit and 8-bit Ripple Carry)")
        
        multi_bit_adders = {
            "4_bit_adder": {
                "description": "4-bit Ripple Carry Adder",
                "bit_width": 4,
                "max_value": 15,
                "test_cases": self._generate_4bit_test_cases()
            },
            "8_bit_adder": {
                "description": "8-bit Ripple Carry Adder (complexity limit test)",
                "bit_width": 8, 
                "max_value": 255,
                "test_cases": self._generate_8bit_test_cases()
            }
        }
        
        all_tests_passed = True
        test_results = {}
        
        for adder_name, adder_config in multi_bit_adders.items():
            print(f"  Testing {adder_name}: {adder_config['description']}")
            
            if not self.create_new_circuit():
                test_results[adder_name] = {"success": False, "error": "Circuit creation failed"}
                all_tests_passed = False
                continue
            
            # Create multi-bit adder circuit
            circuit_result = self._create_ripple_carry_adder_circuit(adder_config["bit_width"])
            
            if not circuit_result:
                test_results[adder_name] = {"success": False, "error": "Circuit implementation failed"}
                all_tests_passed = False
                continue
                
            input_a_ids, input_b_ids, cin_id, sum_output_ids, cout_id, debug_xor1_id, debug_xor2_id = circuit_result
            
            if not self.start_simulation():
                test_results[adder_name] = {"success": False, "error": "Simulation start failed"}
                all_tests_passed = False
                continue
            
            # Test addition operations
            adder_passed = True
            case_results = []
            
            for a_val, b_val, cin_val in adder_config["test_cases"]:
                
                # Convert to bits for detailed logging
                a_bits = self._int_to_bits(a_val, adder_config["bit_width"])
                b_bits = self._int_to_bits(b_val, adder_config["bit_width"])
                
                # Set input A bits with logging
                for i, bit_val in enumerate(a_bits):
                    success = self.set_input(input_a_ids[i], bit_val)
                    if not success:
                        adder_passed = False
                        break
                
                # Set input B bits with logging
                for i, bit_val in enumerate(b_bits):
                    success = self.set_input(input_b_ids[i], bit_val)
                    if not success:
                        adder_passed = False
                        break
                
                # Set carry input with logging
                if cin_id:
                    cin_bool = bool(cin_val)  # Convert integer to boolean
                    success = self.set_input(cin_id, cin_bool)
                    if not success:
                        adder_passed = False
                
                if not adder_passed:
                    break
                
                # Restart simulation for proper propagation (like Level 1)
                self.bridge.restart_simulation()
                time.sleep(0.3)  # Extended LED settling time for multi-bit circuits
                
                # Get sum outputs with detailed logging
                actual_sum_bits = []
                for i, sum_id in enumerate(sum_output_ids):
                    sum_bit = self.get_output(sum_id)
                    actual_sum_bits.append(sum_bit)
                
                # Get carry output
                actual_carry = self.get_output(cout_id) if cout_id else False
                
                # Calculate expected result
                expected_sum = (a_val + b_val + cin_val)
                expected_sum_bits = self._int_to_bits(expected_sum % (2**adder_config["bit_width"]), adder_config["bit_width"])
                expected_carry = expected_sum >= (2**adder_config["bit_width"])
                
                
                # Check correctness
                sum_correct = (actual_sum_bits == expected_sum_bits)
                carry_correct = (actual_carry == expected_carry)
                case_correct = sum_correct and carry_correct
                
                if not case_correct:
                    if not sum_correct:
                        # Debug LSB failure by reading intermediate gates
                        if debug_xor1_id and debug_xor2_id and not actual_sum_bits[0] and expected_sum_bits[0]:
                            # Read intermediate XOR gate outputs
                            xor1_output = self.get_output(debug_xor1_id)
                            xor2_output = self.get_output(debug_xor2_id)
                            
                            # Verify expected calculations
                            expected_xor1 = a_bits[0] ^ b_bits[0]
                            expected_xor2 = expected_xor1 ^ cin_val
                
                if not case_correct:
                    adder_passed = False
                
                case_results.append({
                    "inputs": {
                        "A": a_val,
                        "B": b_val,
                        "Cin": cin_val
                    },
                    "expected": {
                        "Sum": expected_sum % (2**adder_config["bit_width"]),
                        "Sum_bits": expected_sum_bits,
                        "Carry": expected_carry
                    },
                    "actual": {
                        "Sum": self._bits_to_int(actual_sum_bits),
                        "Sum_bits": actual_sum_bits,
                        "Carry": actual_carry
                    },
                    "correct": case_correct
                })
            
            test_results[adder_name] = {
                "success": adder_passed,
                "description": adder_config["description"],
                "bit_width": adder_config["bit_width"],
                "total_cases": len(case_results),
                "passed_cases": sum(1 for case in case_results if case["correct"]),
                "failed_cases": sum(1 for case in case_results if not case["correct"]),
                "sample_results": case_results[:8],  # Show first 8 cases
                "accuracy": sum(1 for case in case_results if case["correct"]) / len(case_results) * 100
            }
            
            if not adder_passed:
                all_tests_passed = False
            
            # Save individual circuit file for this adder
            individual_filename = f"level2_{adder_name}.panda"
            self.save_circuit(individual_filename)
        
        return {
            "test": "multi_bit_adders",
            "success": all_tests_passed,
            "results": test_results,
            "description": "Tests multi-bit ripple carry adders for CPU ALU"
        }

    def _generate_4bit_test_cases(self) -> List[Tuple[int, int, int]]:
        """Generate comprehensive test cases for 4-bit adder."""
        test_cases = []
        
        # Corner cases
        test_cases.extend([
            (0, 0, 0),    # Minimum
            (15, 15, 1),  # Maximum with overflow
            (8, 8, 0),    # Mid-range
            (1, 1, 1),    # Simple carry
            (7, 1, 0),    # No carry
            (15, 1, 0),   # Overflow
            (0, 15, 1),   # One operand zero
        ])
        
        # Random sampling for broader coverage
        random.seed(42)  # Reproducible results
        for _ in range(25):  # Additional random cases
            a = random.randint(0, 15)
            b = random.randint(0, 15)
            cin = random.choice([0, 1])
            test_cases.append((a, b, cin))
        
        return test_cases

    def _generate_8bit_test_cases(self) -> List[Tuple[int, int, int]]:
        """Generate statistical test cases for 8-bit adder."""
        test_cases = []
        
        # Corner cases
        test_cases.extend([
            (0, 0, 0),      # Minimum
            (255, 255, 1),  # Maximum with overflow
            (128, 128, 0),  # Mid-range
            (127, 1, 0),    # Just under overflow
            (255, 1, 0),    # Definite overflow
            (0, 255, 1),    # One operand zero
            (170, 85, 0),   # Pattern testing (0xAA + 0x55)
        ])
        
        # Statistical sampling (can't test all 2^17 combinations)
        random.seed(123)  # Reproducible results
        for _ in range(100):  # Statistical sample
            a = random.randint(0, 255)
            b = random.randint(0, 255)
            cin = random.choice([0, 1])
            test_cases.append((a, b, cin))
        
        return test_cases

    def _create_ripple_carry_adder_circuit(self, bit_width: int) -> Optional[Tuple[List[int], List[int], Optional[int], List[int], Optional[int]]]:
        """Create n-bit ripple carry adder using full adder stages with improved visual layout."""
        if bit_width < 1 or bit_width > 8:
            return None
        
        # Create inputs in a more compact arrangement - group A and B inputs by bit position
        input_a_ids = []
        input_b_ids = []
        
        for i in range(bit_width):
            # Place A inputs in top row, B inputs in second row for clarity
            a_x, a_y = self._get_grid_position(i, 0)  # A inputs in row 0
            a_id = self.create_element("InputButton", a_x, a_y, f"A{i}")
            b_x, b_y = self._get_grid_position(i, 1)   # B inputs in row 1
            b_id = self.create_element("InputButton", b_x, b_y, f"B{i}")
            
            if not all([a_id, b_id]):
                return None
                
            input_a_ids.append(a_id)
            input_b_ids.append(b_id)
        
        # Create carry input
        cin_x, cin_y = self._get_grid_position(bit_width, 0)  # Cin next to A inputs
        cin_id = self.create_element("InputButton", cin_x, cin_y, "Cin")
        if not cin_id:
            return None
        
        # Create full adder stages with cleaner horizontal layout
        sum_output_ids = []
        carry_signals = [cin_id]
        debug_xor1_id = None  # For LSB debugging
        debug_xor2_id = None  # For LSB debugging
        
        for stage in range(bit_width):
            # Each stage is vertically organized: logic in rows 2-4, outputs in row 5
            stage_col = stage  # Each stage gets its own column
            
            # XOR1: A XOR B (row 2)
            xor1_x, xor1_y = self._get_grid_position(stage_col, 2)
            xor1_id = self.create_element("Xor", xor1_x, xor1_y, f"XOR1_{stage}")
            
            # AND1: A AND B (row 3)
            and1_x, and1_y = self._get_grid_position(stage_col, 3)
            and1_id = self.create_element("And", and1_x, and1_y, f"AND1_{stage}")
            
            # XOR2: (A XOR B) XOR Cin = Sum (row 4)  
            xor2_x, xor2_y = self._get_grid_position(stage_col, 4)
            xor2_id = self.create_element("Xor", xor2_x, xor2_y, f"SUM_{stage}")
            
            # AND2: Cin AND (A XOR B) (row 5)
            and2_x, and2_y = self._get_grid_position(stage_col, 5)
            and2_id = self.create_element("And", and2_x, and2_y, f"AND2_{stage}")
            
            # OR: (A AND B) OR (Cin AND (A XOR B)) = Carry (row 6)
            or_x, or_y = self._get_grid_position(stage_col, 6)
            or_id = self.create_element("Or", or_x, or_y, f"C{stage}")
            
            # Sum output (row 7)
            sum_out_x, sum_out_y = self._get_grid_position(stage_col, 7)
            sum_out_id = self.create_element("Led", sum_out_x, sum_out_y, f"S{stage}")
            
            gates = [xor1_id, xor2_id, and1_id, and2_id, or_id, sum_out_id]
            if not all(gates):
                return None
                
            # Connect the full adder logic
            connections = [
                # Sum path: A XOR B XOR Cin
                (input_a_ids[stage], 0, xor1_id, 0),
                (input_b_ids[stage], 0, xor1_id, 1),
                (xor1_id, 0, xor2_id, 0),
                (carry_signals[stage], 0, xor2_id, 1),
                (xor2_id, 0, sum_out_id, 0),
                
                # Carry path: (A AND B) OR (Cin AND (A XOR B))
                (input_a_ids[stage], 0, and1_id, 0),
                (input_b_ids[stage], 0, and1_id, 1),
                (carry_signals[stage], 0, and2_id, 0),
                (xor1_id, 0, and2_id, 1),
                (and1_id, 0, or_id, 0),
                (and2_id, 0, or_id, 1)
            ]
            
            for source_id, source_port, target_id, target_port in connections:
                if not self.connect_elements(source_id, source_port, target_id, target_port):
                    return None
            
            sum_output_ids.append(sum_out_id)
            carry_signals.append(or_id)
            
            # Store debug info for LSB stage (stage 0)
            if stage == 0:
                debug_xor1_id = xor1_id
                debug_xor2_id = xor2_id
        
        # Create final carry output (row 8)
        cout_x, cout_y = self._get_grid_position(bit_width, 8)
        cout_id = self.create_element("Led", cout_x, cout_y, "COUT")
        if not cout_id:
            return None
            
        # Connect final carry
        if not self.connect_elements(carry_signals[-1], 0, cout_id, 0):
            return None
        
        return input_a_ids, input_b_ids, cin_id, sum_output_ids, cout_id, debug_xor1_id, debug_xor2_id

    def _create_full_adder_stage(self, stage: int, a_id: int, b_id: int, cin_id: int) -> Optional[Tuple[int, int, int, int]]:
        """Create a single full adder stage within a multi-bit adder."""
        # Use grid positioning - each stage gets its own column group
        stage_col_offset = stage * 3  # 3 columns per stage (XOR1, XOR2+AND2, OR)
        base_col = 2  # Start at column 2 (after inputs)
        
        # Create gates for this stage with proper grid positioning
        xor1_x, xor1_y = self._get_grid_position(base_col + stage_col_offset, 0)
        xor1_id = self.create_element("Xor", xor1_x, xor1_y, f"XOR1_{stage}")
        xor2_x, xor2_y = self._get_grid_position(base_col + stage_col_offset + 1, 0)
        xor2_id = self.create_element("Xor", xor2_x, xor2_y, f"XOR2_{stage}")
        and1_x, and1_y = self._get_grid_position(base_col + stage_col_offset, 1)
        and1_id = self.create_element("And", and1_x, and1_y, f"AND1_{stage}")
        and2_x, and2_y = self._get_grid_position(base_col + stage_col_offset + 1, 1)
        and2_id = self.create_element("And", and2_x, and2_y, f"AND2_{stage}")
        or_x, or_y = self._get_grid_position(base_col + stage_col_offset + 2, 1)
        or_id = self.create_element("Or", or_x, or_y, f"OR_{stage}")
        
        gates = [xor1_id, xor2_id, and1_id, and2_id, or_id]
        if not all(gates):
            return None
        
        # Create outputs with proper grid positioning
        sum_out_x, sum_out_y = self._get_grid_position(base_col + stage_col_offset + 1, stage)
        sum_out_id = self.create_element("Led", sum_out_x, sum_out_y, f"S{stage}")
        carry_out_x, carry_out_y = self._get_grid_position(base_col + stage_col_offset + 2, stage + 2)
        carry_out_id = self.create_element("Led", carry_out_x, carry_out_y, f"C{stage}")
        
        if not all([sum_out_id, carry_out_id]):
            return None
        
        # Connect full adder logic
        connections = [
            # Sum path: A XOR B XOR Cin
            (a_id, 0, xor1_id, 0),
            (b_id, 0, xor1_id, 1),
            (xor1_id, 0, xor2_id, 0),
            (cin_id, 0, xor2_id, 1),
            (xor2_id, 0, sum_out_id, 0),
            
            # Carry path: (A AND B) OR (Cin AND (A XOR B))
            (a_id, 0, and1_id, 0),
            (b_id, 0, and1_id, 1),
            (cin_id, 0, and2_id, 0),
            (xor1_id, 0, and2_id, 1),
            (and1_id, 0, or_id, 0),
            (and2_id, 0, or_id, 1),
            (or_id, 0, carry_out_id, 0)
        ]
        
        for i, (source_id, source_port, target_id, target_port) in enumerate(connections):
            success = self.connect_elements(source_id, source_port, target_id, target_port)
            connection_names = [
                f"A→XOR1", f"B→XOR1", f"XOR1→XOR2", f"Cin→XOR2", f"XOR2→Sum_LED",
                f"A→AND1", f"B→AND1", f"Cin→AND2", f"XOR1→AND2", f"AND1→OR", f"AND2→OR", f"OR→Carry_LED"
            ]
            if not success:
                return None
        
        return sum_out_id, or_id, xor1_id, xor2_id  # Return gate outputs for debugging

    def _int_to_bits(self, value: int, bit_width: int) -> List[bool]:
        """Convert integer to list of bits (LSB first)."""
        bits = []
        for i in range(bit_width):
            bits.append(bool(value & (1 << i)))
        return bits

    def _bits_to_int(self, bits: List[bool]) -> int:
        """Convert list of bits (LSB first) to integer."""
        value = 0
        for i, bit in enumerate(bits):
            if bit:
                value |= (1 << i)
        return value

    def test_subtraction_circuits(self) -> dict:
        """
        Test 2.3: Subtraction Circuits (2's Complement Subtractors)
        Validate subtraction using 2's complement arithmetic.
        """
        print("🔧 Test 2.3: Subtraction Circuits (2's Complement)")
        
        subtraction_tests = {
            "4_bit_subtractor": {
                "description": "4-bit Subtractor using 2's complement (A - B = A + (~B + 1))",
                "bit_width": 4,
                "test_cases": [
                    (15, 1),   # 15 - 1 = 14
                    (8, 3),    # 8 - 3 = 5
                    (5, 5),    # 5 - 5 = 0
                    (3, 8),    # 3 - 8 = -5 (underflow in unsigned)
                    (0, 1),    # 0 - 1 = -1 (underflow)
                    (15, 15),  # 15 - 15 = 0
                    (10, 6),   # 10 - 6 = 4
                ]
            }
        }
        
        all_tests_passed = True
        test_results = {}
        
        for sub_name, sub_config in subtraction_tests.items():
            print(f"  Testing {sub_name}: {sub_config['description']}")
            
            if not self.create_new_circuit():
                test_results[sub_name] = {"success": False, "error": "Circuit creation failed"}
                all_tests_passed = False
                continue
            
            # Create subtractor circuit
            circuit_result = self._create_subtractor_circuit(sub_config["bit_width"])
            
            if not circuit_result:
                test_results[sub_name] = {"success": False, "error": "Circuit implementation failed"}
                all_tests_passed = False
                continue
                
            input_a_ids, input_b_ids, diff_output_ids, borrow_id, diff_gate_ids = circuit_result
            
            if not self.start_simulation():
                test_results[sub_name] = {"success": False, "error": "Simulation start failed"}
                all_tests_passed = False
                continue
            
            # Test subtraction operations
            sub_passed = True
            case_results = []
            
            for a_val, b_val in sub_config["test_cases"]:
                # Ensure the constant "1" input is set for 2's complement addition
                one_id = None
                for element in self.bridge.list_elements():
                    if element.get('label') == 'ONE':
                        one_id = element['id']
                        break
                if one_id:
                    self.set_input(one_id, True)
                
                # Set input A bits
                for i, bit_val in enumerate(self._int_to_bits(a_val, sub_config["bit_width"])):
                    if not self.set_input(input_a_ids[i], bit_val):
                        sub_passed = False
                        break
                
                # Set input B bits  
                for i, bit_val in enumerate(self._int_to_bits(b_val, sub_config["bit_width"])):
                    if not self.set_input(input_b_ids[i], bit_val):
                        sub_passed = False
                        break
                
                if not sub_passed:
                    break
                
                # Restart simulation for proper propagation (like Level 1)
                self.bridge.restart_simulation()
                time.sleep(0.3)  # Extended LED settling time for multi-bit circuits
                
                # Get difference outputs
                actual_diff_bits = []
                for diff_id in diff_output_ids:
                    diff_bit = self.get_output(diff_id)
                    actual_diff_bits.append(diff_bit)
                
                # Get borrow output
                actual_borrow = self.get_output(borrow_id) if borrow_id else False
                
                # Calculate expected result (2's complement arithmetic)
                expected_diff = (a_val - b_val)
                if expected_diff < 0:
                    # Negative result in 2's complement
                    # For 4-bit: -5 = 16 - 5 = 11 (binary 1011)
                    expected_diff_unsigned = expected_diff + (1 << sub_config["bit_width"])
                    expected_diff_bits = self._int_to_bits(expected_diff_unsigned, sub_config["bit_width"])
                    expected_borrow = True  # Negative result indicates borrow
                else:
                    expected_diff_bits = self._int_to_bits(expected_diff, sub_config["bit_width"])
                    expected_borrow = False
                
                
                # Manual verification for 3 - 8
                if a_val == 3 and b_val == 8:
                    # Debug: Check what the actual carry_out from MSB is
                    # Need to find the MSB carry output to check its value
                    pass
                
                # Check correctness
                diff_correct = (actual_diff_bits == expected_diff_bits)
                borrow_correct = (actual_borrow == expected_borrow)
                case_correct = diff_correct and borrow_correct
                
                if not case_correct:
                    sub_passed = False
                
                # For display, show the raw unsigned result from the circuit
                actual_diff_unsigned = self._bits_to_int(actual_diff_bits)
                
                case_results.append({
                    "inputs": {
                        "A": a_val,
                        "B": b_val
                    },
                    "expected": {
                        "Difference": expected_diff,
                        "Diff_bits": expected_diff_bits,
                        "Borrow": expected_borrow
                    },
                    "actual": {
                        "Difference": actual_diff_unsigned,
                        "Diff_bits": actual_diff_bits,
                        "Borrow": actual_borrow
                    },
                    "correct": case_correct
                })
            
            test_results[sub_name] = {
                "success": sub_passed,
                "description": sub_config["description"],
                "bit_width": sub_config["bit_width"],
                "total_cases": len(case_results),
                "passed_cases": sum(1 for case in case_results if case["correct"]),
                "failed_cases": sum(1 for case in case_results if not case["correct"]),
                "results": case_results,
                "accuracy": sum(1 for case in case_results if case["correct"]) / len(case_results) * 100
            }
            
            if not sub_passed:
                all_tests_passed = False
            
            # Save individual circuit file for this subtractor
            individual_filename = f"level2_{sub_name}.panda"
            self.save_circuit(individual_filename)
        
        return {
            "test": "subtraction_circuits",
            "success": all_tests_passed,
            "results": test_results,
            "description": "Tests 2's complement subtraction circuits for CPU ALU"
        }

    def _create_subtractor_circuit(self, bit_width: int) -> Optional[Tuple[List[int], List[int], List[int], Optional[int], List[int]]]:
        """Create n-bit subtractor using 2's complement: A - B = A + (~B + 1)"""
        if bit_width < 1 or bit_width > 8:
            return None
        
        # Create input arrays with proper grid positioning
        input_a_ids = []
        input_b_ids = []
        
        for i in range(bit_width):
            # Place A inputs in top row, B inputs in second row
            a_x, a_y = self._get_grid_position(i, 0)
            a_id = self.create_element("InputButton", a_x, a_y, f"A{i}")
            b_x, b_y = self._get_grid_position(i, 1)
            b_id = self.create_element("InputButton", b_x, b_y, f"B{i}")
            
            if not all([a_id, b_id]):
                return None
                
            input_a_ids.append(a_id)
            input_b_ids.append(b_id)
        
        # Create NOT gates to invert B (first step of 2's complement) with proper positioning
        not_b_ids = []
        for i in range(bit_width):
            not_x, not_y = self._get_grid_position(i, 2)  # NOT gates in row 2
            not_id = self.create_element("Not", not_x, not_y, f"NOT_B{i}")
            if not not_id:
                return None
            
            # Connect B to NOT gate
            if not self.connect_elements(input_b_ids[i], 0, not_id, 0):
                return None
            
            not_b_ids.append(not_id)
        
        # Create constant ONE input with proper positioning
        one_x, one_y = self._get_grid_position(bit_width, 0)  # ONE input next to other inputs
        one_id = self.create_element("InputButton", one_x, one_y, "ONE")
        if not one_id:
            return None
        
        # Set the constant 1 input
        self.set_input(one_id, True)
        
        # Create adder stages: A + ~B + 1 using improved layout
        diff_output_ids = []  # LED outputs
        diff_gate_ids = []    # XOR2 gate outputs (for magnitude comparator)
        carry_signals = [one_id]  # Start with carry = 1 for 2's complement
        
        for stage in range(bit_width):
            # Each stage is vertically organized similar to ripple carry adder
            stage_col = stage  # Each stage gets its own column
            
            # XOR1: A XOR ~B (row 3)
            xor1_x, xor1_y = self._get_grid_position(stage_col, 3)
            xor1_id = self.create_element("Xor", xor1_x, xor1_y, f"XOR1_{stage}")
            
            # AND1: A AND ~B (row 4)
            and1_x, and1_y = self._get_grid_position(stage_col, 4)
            and1_id = self.create_element("And", and1_x, and1_y, f"AND1_{stage}")
            
            # XOR2: (A XOR ~B) XOR Cin = Diff (row 5)  
            xor2_x, xor2_y = self._get_grid_position(stage_col, 5)
            xor2_id = self.create_element("Xor", xor2_x, xor2_y, f"DIFF_{stage}")
            
            # AND2: Cin AND (A XOR ~B) (row 6)
            and2_x, and2_y = self._get_grid_position(stage_col, 6)
            and2_id = self.create_element("And", and2_x, and2_y, f"AND2_{stage}")
            
            # OR: (A AND ~B) OR (Cin AND (A XOR ~B)) = Carry (row 7)
            or_x, or_y = self._get_grid_position(stage_col, 7)
            or_id = self.create_element("Or", or_x, or_y, f"C{stage}")
            
            # Diff output (row 8)
            diff_out_x, diff_out_y = self._get_grid_position(stage_col, 8)
            diff_out_id = self.create_element("Led", diff_out_x, diff_out_y, f"DIFF{stage}")
            
            gates = [xor1_id, xor2_id, and1_id, and2_id, or_id, diff_out_id]
            if not all(gates):
                return None
                
            # Connect the full adder logic for subtraction
            connections = [
                # Diff path: A XOR ~B XOR Cin
                (input_a_ids[stage], 0, xor1_id, 0),
                (not_b_ids[stage], 0, xor1_id, 1),
                (xor1_id, 0, xor2_id, 0),
                (carry_signals[stage], 0, xor2_id, 1),
                (xor2_id, 0, diff_out_id, 0),
                
                # Carry path: (A AND ~B) OR (Cin AND (A XOR ~B))
                (input_a_ids[stage], 0, and1_id, 0),
                (not_b_ids[stage], 0, and1_id, 1),
                (carry_signals[stage], 0, and2_id, 0),
                (xor1_id, 0, and2_id, 1),
                (and1_id, 0, or_id, 0),
                (and2_id, 0, or_id, 1)
            ]
            
            for source_id, source_port, target_id, target_port in connections:
                if not self.connect_elements(source_id, source_port, target_id, target_port):
                    return None
            
            diff_output_ids.append(diff_out_id)
            diff_gate_ids.append(xor2_id)
            carry_signals.append(or_id)
        
        # Create borrow output (NOT of final carry) with proper positioning
        borrow_not_x, borrow_not_y = self._get_grid_position(bit_width, 7)
        borrow_not_id = self.create_element("Not", borrow_not_x, borrow_not_y, "NOT_BORROW")
        borrow_out_x, borrow_out_y = self._get_grid_position(bit_width, 8)
        borrow_out_id = self.create_element("Led", borrow_out_x, borrow_out_y, "BORROW")
        
        if not all([borrow_not_id, borrow_out_id]):
            return None
            
        # Connect borrow logic: BORROW = NOT(final_carry)
        if not self.connect_elements(carry_signals[-1], 0, borrow_not_id, 0):
            return None
        if not self.connect_elements(borrow_not_id, 0, borrow_out_id, 0):
            return None
        
        return input_a_ids, input_b_ids, diff_output_ids, borrow_not_id, diff_gate_ids

    def _create_subtractor_with_inputs(self, input_a_ids: List[int], input_b_ids: List[int]) -> Optional[Tuple[List[int], int, List[int]]]:
        """Create n-bit subtractor using provided input arrays: A - B = A + (~B + 1)"""
        bit_width = len(input_a_ids)
        
        if bit_width != len(input_b_ids) or bit_width < 1 or bit_width > 8:
            return None
        
        # Create NOT gates to invert B (first step of 2's complement) with proper positioning
        not_b_ids = []
        for i in range(bit_width):
            not_x, not_y = self._get_grid_position(i, 2)  # NOT gates in row 2
            not_id = self.create_element("Not", not_x, not_y, f"NOT_B{i}")
            if not not_id:
                return None
            
            # Connect B to NOT gate
            if not self.connect_elements(input_b_ids[i], 0, not_id, 0):
                return None
            
            not_b_ids.append(not_id)
        
        # Create ripple carry adder to add A + (~B + 1)
        # The +1 is implemented by setting the initial carry input to 1
        one_x, one_y = self._get_grid_position(bit_width, 0)  # ONE input next to other inputs
        one_id = self.create_element("InputButton", one_x, one_y, "ONE")
        if not one_id:
            return None
        
        # Set the constant 1 input
        self.set_input(one_id, True)
        
        # Create adder stages: A + ~B + 1
        diff_output_ids = []  # LED outputs
        diff_gate_ids = []    # XOR2 gate outputs (for magnitude comparator)
        carry_signals = [one_id]  # Start with carry = 1 for 2's complement
        
        for stage in range(bit_width):
            if stage < bit_width - 1:
                # Regular stage
                stage_result = self._create_full_adder_stage(stage, input_a_ids[stage], not_b_ids[stage], carry_signals[stage])
                
                if not stage_result:
                    return None
                    
                diff_out_id, carry_out_id, _, xor2_id = stage_result
                diff_output_ids.append(diff_out_id)
                diff_gate_ids.append(xor2_id)  # Store XOR2 gate output
                carry_signals.append(carry_out_id)
            else:
                # Final stage with borrow output
                stage_result = self._create_full_adder_stage_with_borrow(stage, input_a_ids[stage], not_b_ids[stage], carry_signals[stage])
                if not stage_result:
                    return None
                    
                diff_out_id, carry_out_id, borrow_led_id, xor2_id, borrow_gate_id = stage_result
                diff_output_ids.append(diff_out_id)
                diff_gate_ids.append(xor2_id)  # Store XOR2 gate output
        
        return diff_output_ids, borrow_gate_id, diff_gate_ids

    def _create_full_adder_stage_with_borrow(self, stage: int, a_id: int, b_id: int, cin_id: int) -> Optional[Tuple[int, int, int, int, int]]:
        """Create a full adder stage with borrow output using grid layout."""
        
        # Use grid positioning similar to regular full adder stages
        stage_col = stage
        
        # XOR1: A XOR B (row 3)
        xor1_x, xor1_y = self._get_grid_position(stage_col, 3)
        xor1_id = self.create_element("Xor", xor1_x, xor1_y, f"XOR1_{stage}")
        
        # AND1: A AND B (row 4)
        and1_x, and1_y = self._get_grid_position(stage_col, 4)
        and1_id = self.create_element("And", and1_x, and1_y, f"AND1_{stage}")
        
        # XOR2: (A XOR B) XOR Cin = Sum (row 5)  
        xor2_x, xor2_y = self._get_grid_position(stage_col, 5)
        xor2_id = self.create_element("Xor", xor2_x, xor2_y, f"SUM_{stage}")
        
        # AND2: Cin AND (A XOR B) (row 6)
        and2_x, and2_y = self._get_grid_position(stage_col, 6)
        and2_id = self.create_element("And", and2_x, and2_y, f"AND2_{stage}")
        
        # OR: (A AND B) OR (Cin AND (A XOR B)) = Carry (row 7)
        or_x, or_y = self._get_grid_position(stage_col, 7)
        or_id = self.create_element("Or", or_x, or_y, f"CARRY_{stage}")
        
        gates = [xor1_id, xor2_id, and1_id, and2_id, or_id]
        if not all(gates):
            return None
        
        # Create outputs with proper grid positioning
        # Sum output (row 8)
        sum_out_x, sum_out_y = self._get_grid_position(stage_col, 8)
        sum_out_id = self.create_element("Led", sum_out_x, sum_out_y, f"S{stage}")
        
        # Carry output (row 9)
        carry_out_x, carry_out_y = self._get_grid_position(stage_col, 9)
        carry_out_id = self.create_element("Led", carry_out_x, carry_out_y, f"C{stage}")
        
        # NOT gate for borrow (row 10) - connects to OR output
        not_borrow_x, not_borrow_y = self._get_grid_position(stage_col, 10)
        not_borrow_id = self.create_element("Not", not_borrow_x, not_borrow_y, f"NOT_BORROW_{stage}")
        
        # Borrow output LED (row 11)
        borrow_out_x, borrow_out_y = self._get_grid_position(stage_col, 11)
        borrow_out_id = self.create_element("Led", borrow_out_x, borrow_out_y, f"BORROW")
        
        if not all([sum_out_id, carry_out_id, not_borrow_id, borrow_out_id]):
            return None
        
        # Connect full adder logic with improved organization
        connections = [
            # Sum path: A XOR B XOR Cin
            (a_id, 0, xor1_id, 0),
            (b_id, 0, xor1_id, 1),
            (xor1_id, 0, xor2_id, 0),
            (cin_id, 0, xor2_id, 1),
            (xor2_id, 0, sum_out_id, 0),
            
            # Carry path: (A AND B) OR (Cin AND (A XOR B))
            (a_id, 0, and1_id, 0),
            (b_id, 0, and1_id, 1),
            (cin_id, 0, and2_id, 0),
            (xor1_id, 0, and2_id, 1),
            (and1_id, 0, or_id, 0),
            (and2_id, 0, or_id, 1),
            (or_id, 0, carry_out_id, 0),
            
            # Borrow path: BORROW = NOT(carry_out)
            (or_id, 0, not_borrow_id, 0),      # OR -> NOT (not LED!)
            (not_borrow_id, 0, borrow_out_id, 0)  # NOT -> Borrow LED
        ]
        
        for source_id, source_port, target_id, target_port in connections:
            if not self.connect_elements(source_id, source_port, target_id, target_port):
                return None
        
        return sum_out_id, carry_out_id, borrow_out_id, xor2_id, not_borrow_id

    def _add_one_to_bits(self, bits: List[bool]) -> List[bool]:
        """Add 1 to a binary number represented as list of bits."""
        result = bits.copy()
        carry = True
        
        for i in range(len(result)):
            if carry:
                if result[i]:
                    result[i] = False  # 1 + 1 = 0 with carry
                else:
                    result[i] = True   # 0 + 1 = 1 no carry
                    carry = False
            else:
                break
        
        return result

    def test_comparator_circuits(self) -> dict:
        """
        Test 2.4: Comparator Circuits (Equality and Magnitude)
        Validate comparison operations used in conditional instructions.
        """
        print("🔧 Test 2.4: Comparator Circuits (Equality and Magnitude)")
        
        comparator_tests = {
            "equality_comparator": {
                "description": "4-bit Equality Comparator (A == B)",
                "bit_width": 4,
                "comparison": "equality",
                "test_cases": [
                    (0, 0, True),    # Equal: 0 == 0
                    (5, 5, True),    # Equal: 5 == 5  
                    (15, 15, True),  # Equal: 15 == 15
                    (3, 7, False),   # Not equal: 3 != 7
                    (10, 1, False),  # Not equal: 10 != 1
                    (0, 15, False),  # Not equal: 0 != 15
                ]
            },
            "magnitude_comparator": {
                "description": "4-bit Magnitude Comparator (A > B, A < B)",
                "bit_width": 4,
                "comparison": "magnitude",
                "test_cases": [
                    (8, 3, "greater"),   # 8 > 3
                    (2, 10, "less"),     # 2 < 10
                    (7, 7, "equal"),     # 7 == 7
                    (15, 0, "greater"),  # 15 > 0
                    (0, 15, "less"),     # 0 < 15
                    (12, 12, "equal"),   # 12 == 12
                ]
            }
        }
        
        all_tests_passed = True
        test_results = {}
        
        for comp_name, comp_config in comparator_tests.items():
            print(f"  Testing {comp_name}: {comp_config['description']}")
            
            if not self.create_new_circuit():
                test_results[comp_name] = {"success": False, "error": "Circuit creation failed"}
                all_tests_passed = False
                continue
            
            # Create comparator circuit
            if comp_config["comparison"] == "equality":
                circuit_result = self._create_equality_comparator_circuit(comp_config["bit_width"])
            elif comp_config["comparison"] == "magnitude":
                circuit_result = self._create_magnitude_comparator_circuit(comp_config["bit_width"])
            else:
                circuit_result = None
            
            if not circuit_result:
                test_results[comp_name] = {"success": False, "error": "Circuit implementation failed"}
                all_tests_passed = False
                continue
                
            if comp_config["comparison"] == "equality":
                input_a_ids, input_b_ids, eq_output_id = circuit_result
                output_ids = [eq_output_id]
            else:
                input_a_ids, input_b_ids, gt_output_id, lt_output_id, eq_output_id = circuit_result
                output_ids = [gt_output_id, lt_output_id, eq_output_id]
            
            if not self.start_simulation():
                test_results[comp_name] = {"success": False, "error": "Simulation start failed"}
                all_tests_passed = False
                continue
            
            # Test comparison operations
            comp_passed = True
            case_results = []
            
            for test_case in comp_config["test_cases"]:
                if comp_config["comparison"] == "equality":
                    a_val, b_val, expected_eq = test_case
                else:
                    a_val, b_val, expected_result = test_case
                
                # Set input A bits
                for i, bit_val in enumerate(self._int_to_bits(a_val, comp_config["bit_width"])):
                    if not self.set_input(input_a_ids[i], bit_val):
                        comp_passed = False
                        break
                
                # Set input B bits  
                for i, bit_val in enumerate(self._int_to_bits(b_val, comp_config["bit_width"])):
                    if not self.set_input(input_b_ids[i], bit_val):
                        comp_passed = False
                        break
                
                if not comp_passed:
                    break
                
                # Step simulation for comparison logic
                for _ in range(4):
                    self.step_simulation()
                
                # Get outputs
                actual_outputs = []
                for output_id in output_ids:
                    output_val = self.get_output(output_id)
                    actual_outputs.append(output_val)
                
                # Check correctness
                if comp_config["comparison"] == "equality":
                    case_correct = (actual_outputs[0] == expected_eq)
                    case_result = {
                        "inputs": {"A": a_val, "B": b_val},
                        "expected": {"Equal": expected_eq},
                        "actual": {"Equal": actual_outputs[0]},
                        "correct": case_correct
                    }
                else:
                    actual_gt, actual_lt, actual_eq = actual_outputs
                    expected_gt = (expected_result == "greater")
                    expected_lt = (expected_result == "less")
                    expected_eq = (expected_result == "equal")
                    
                    case_correct = (actual_gt == expected_gt and actual_lt == expected_lt and actual_eq == expected_eq)
                    case_result = {
                        "inputs": {"A": a_val, "B": b_val},
                        "expected": {"Greater": expected_gt, "Less": expected_lt, "Equal": expected_eq},
                        "actual": {"Greater": actual_gt, "Less": actual_lt, "Equal": actual_eq},
                        "correct": case_correct
                    }
                
                if not case_correct:
                    comp_passed = False
                
                case_results.append(case_result)
            
            test_results[comp_name] = {
                "success": comp_passed,
                "description": comp_config["description"],
                "bit_width": comp_config["bit_width"],
                "comparison_type": comp_config["comparison"],
                "total_cases": len(case_results),
                "passed_cases": sum(1 for case in case_results if case["correct"]),
                "failed_cases": sum(1 for case in case_results if not case["correct"]),
                "results": case_results,
                "accuracy": sum(1 for case in case_results if case["correct"]) / len(case_results) * 100
            }
            
            if not comp_passed:
                all_tests_passed = False
            
            # Save individual circuit file for this comparator
            individual_filename = f"level2_{comp_name}.panda"
            self.save_circuit(individual_filename)
        
        return {
            "test": "comparator_circuits",
            "success": all_tests_passed,
            "results": test_results,
            "description": "Tests equality and magnitude comparator circuits for CPU conditional operations"
        }

    def _create_equality_comparator_circuit(self, bit_width: int) -> Optional[Tuple[List[int], List[int], int]]:
        """Create n-bit equality comparator with improved layout: EQ = (A0 XNOR B0) AND (A1 XNOR B1) AND ... AND (An XNOR Bn)"""
        if bit_width < 1 or bit_width > 8:
            return None
        
        # Create inputs with clear horizontal layout
        input_a_ids = []
        input_b_ids = []
        
        for i in range(bit_width):
            # Place A inputs in top row, B inputs in second row for clear visual grouping
            a_x, a_y = self._get_grid_position(i, 0)
            a_id = self.create_element("InputButton", a_x, a_y, f"A{i}")
            b_x, b_y = self._get_grid_position(i, 1)
            b_id = self.create_element("InputButton", b_x, b_y, f"B{i}")
            
            if not all([a_id, b_id]):
                return None
                
            input_a_ids.append(a_id)
            input_b_ids.append(b_id)
        
        # Create XNOR gates for bit-wise equality in row 2
        xnor_ids = []
        for i in range(bit_width):
            xnor_x, xnor_y = self._get_grid_position(i, 2)  # XNOR gates aligned with inputs
            xnor_id = self.create_element("Xnor", xnor_x, xnor_y, f"EQ{i}")
            if not xnor_id:
                return None
            
            # Connect inputs to XNOR
            if not self.connect_elements(input_a_ids[i], 0, xnor_id, 0):
                return None
            if not self.connect_elements(input_b_ids[i], 0, xnor_id, 1):
                return None
            
            xnor_ids.append(xnor_id)
        
        # Create AND tree with clean horizontal progression
        if bit_width == 1:
            # Simple case: direct connection
            eq_output_x, eq_output_y = self._get_grid_position(1, 3)
            eq_output_id = self.create_element("Led", eq_output_x, eq_output_y, "EQ")
            if not eq_output_id:
                return None
            if not self.connect_elements(xnor_ids[0], 0, eq_output_id, 0):
                return None
        else:
            # Use AND tree for multiple bits with improved layout
            and_tree_result = self._create_and_tree_positioned(xnor_ids, "EQ_AND", 0, 3)
            if not and_tree_result:
                return None
            
            # Create final output LED
            final_output_col = bit_width + 2  # Position after AND tree
            eq_output_x, eq_output_y = self._get_grid_position(final_output_col, 3)
            eq_output_id = self.create_element("Led", eq_output_x, eq_output_y, "EQ")
            if not eq_output_id:
                return None
            if not self.connect_elements(and_tree_result, 0, eq_output_id, 0):
                return None
        
        return input_a_ids, input_b_ids, eq_output_id

    def _create_magnitude_comparator_circuit(self, bit_width: int) -> Optional[Tuple[List[int], List[int], int, int, int]]:
        """Create n-bit magnitude comparator with ultra-compact horizontal layout - maximum 6 rows total."""
        if bit_width < 1 or bit_width > 8:
            return None
        
        # Row allocation for ultra-compact design:
        # Row 0: A inputs
        # Row 1: B inputs  
        # Row 2: Adder logic (XOR1, AND gates)
        # Row 3: Sum/Diff outputs (XOR2) + Carry OR
        # Row 4: Comparison logic (borrow, zero detect, GT/LT logic)
        # Row 5: Final outputs (GT, LT, EQ LEDs)
        
        input_a_ids = []
        input_b_ids = []
        
        # Inputs with minimal spacing
        for i in range(bit_width):
            a_x, a_y = self._get_grid_position(i * 4, 0)  # 4x spacing for adder stage width
            a_id = self.create_element("InputButton", a_x, a_y, f"A{i}")
            b_x, b_y = self._get_grid_position(i * 4, 1)   
            b_id = self.create_element("InputButton", b_x, b_y, f"B{i}")
            
            if not all([a_id, b_id]):
                return None
                
            input_a_ids.append(a_id)
            input_b_ids.append(b_id)
        
        # Constant 1 input for 2's complement (far right)
        one_x, one_y = self._get_grid_position(bit_width * 4, 0)
        one_id = self.create_element("InputButton", one_x, one_y, "1")
        if not one_id:
            return None
        self.set_input(one_id, True)
        
        # Ultra-compact adder stages: each stage fits in 4 columns, 2 main rows
        diff_outputs = []
        carry_chain = [one_id]
        
        for i in range(bit_width):
            base_col = i * 4
            
            # Column 0: NOT gate for B (row 2)
            not_b_x, not_b_y = self._get_grid_position(base_col, 2)
            not_b_id = self.create_element("Not", not_b_x, not_b_y, f"~B{i}")
            if not not_b_id:
                return None
            if not self.connect_elements(input_b_ids[i], 0, not_b_id, 0):
                return None
            
            # Column 1: XOR1 (A XOR ~B) (row 2)
            xor1_x, xor1_y = self._get_grid_position(base_col + 1, 2)
            xor1_id = self.create_element("Xor", xor1_x, xor1_y, f"X1_{i}")
            if not xor1_id:
                return None
            if not self.connect_elements(input_a_ids[i], 0, xor1_id, 0):
                return None
            if not self.connect_elements(not_b_id, 0, xor1_id, 1):
                return None
            
            # Column 2: XOR2 (XOR1 XOR Cin = Sum) (row 3)
            xor2_x, xor2_y = self._get_grid_position(base_col + 2, 3)
            xor2_id = self.create_element("Xor", xor2_x, xor2_y, f"D{i}")
            if not xor2_id:
                return None
            if not self.connect_elements(xor1_id, 0, xor2_id, 0):
                return None
            if not self.connect_elements(carry_chain[i], 0, xor2_id, 1):
                return None
            diff_outputs.append(xor2_id)
            
            # Carry generation: AND1 (A AND ~B), AND2 (XOR1 AND Cin), OR (carry out)
            # Column 2: AND1 (row 2)
            and1_x, and1_y = self._get_grid_position(base_col + 2, 2)
            and1_id = self.create_element("And", and1_x, and1_y, f"&1_{i}")
            if not and1_id:
                return None
            if not self.connect_elements(input_a_ids[i], 0, and1_id, 0):
                return None
            if not self.connect_elements(not_b_id, 0, and1_id, 1):
                return None
            
            # Column 3: AND2 (row 2)
            and2_x, and2_y = self._get_grid_position(base_col + 3, 2)
            and2_id = self.create_element("And", and2_x, and2_y, f"&2_{i}")
            if not and2_id:
                return None
            if not self.connect_elements(xor1_id, 0, and2_id, 0):
                return None
            if not self.connect_elements(carry_chain[i], 0, and2_id, 1):
                return None
            
            # Column 3: OR (carry out) (row 3)
            or_x, or_y = self._get_grid_position(base_col + 3, 3)
            or_id = self.create_element("Or", or_x, or_y, f"C{i}")
            if not or_id:
                return None
            if not self.connect_elements(and1_id, 0, or_id, 0):
                return None
            if not self.connect_elements(and2_id, 0, or_id, 1):
                return None
            
            carry_chain.append(or_id)
        
        # Ultra-compact comparison section - right after adder stages
        comp_start = bit_width * 4 + 2
        final_carry = carry_chain[-1]
        
        # Row 4: Borrow detection
        borrow_x, borrow_y = self._get_grid_position(comp_start, 4)
        borrow_id = self.create_element("Not", borrow_x, borrow_y, "BOR")
        if not borrow_id:
            return None
        if not self.connect_elements(final_carry, 0, borrow_id, 0):
            return None
        
        # Row 4: Zero detection (all diff outputs OR'd then NOT'd)
        if len(diff_outputs) == 1:
            zero_or = diff_outputs[0]
        elif len(diff_outputs) == 2:
            # Simple 2-input OR
            or_x, or_y = self._get_grid_position(comp_start + 1, 4)
            zero_or = self.create_element("Or", or_x, or_y, "OR")
            if not zero_or:
                return None
            if not self.connect_elements(diff_outputs[0], 0, zero_or, 0):
                return None
            if not self.connect_elements(diff_outputs[1], 0, zero_or, 1):
                return None
        else:
            # Multi-input OR tree - keep compact
            zero_or = self._create_or_tree_positioned(diff_outputs, "OR", comp_start + 1, 4)
            if not zero_or:
                return None
        
        zero_x, zero_y = self._get_grid_position(comp_start + 3, 4)
        zero_id = self.create_element("Not", zero_x, zero_y, "EQ")
        if not zero_id:
            return None
        if not self.connect_elements(zero_or, 0, zero_id, 0):
            return None
        
        # Row 4: GT logic - NOT(borrow) AND NOT(zero)
        not_borrow_x, not_borrow_y = self._get_grid_position(comp_start + 4, 4)
        not_borrow_id = self.create_element("Not", not_borrow_x, not_borrow_y, "~B")
        if not not_borrow_id:
            return None
        if not self.connect_elements(borrow_id, 0, not_borrow_id, 0):
            return None
        
        not_zero_x, not_zero_y = self._get_grid_position(comp_start + 5, 4)
        not_zero_id = self.create_element("Not", not_zero_x, not_zero_y, "~Z")
        if not not_zero_id:
            return None
        if not self.connect_elements(zero_id, 0, not_zero_id, 0):
            return None
        
        gt_and_x, gt_and_y = self._get_grid_position(comp_start + 6, 4)
        gt_and_id = self.create_element("And", gt_and_x, gt_and_y, "GT")
        if not gt_and_id:
            return None
        if not self.connect_elements(not_borrow_id, 0, gt_and_id, 0):
            return None
        if not self.connect_elements(not_zero_id, 0, gt_and_id, 1):
            return None
        
        # Row 5: Final outputs - directly below logic
        gt_out_x, gt_out_y = self._get_grid_position(comp_start + 4, 5)
        gt_output_id = self.create_element("Led", gt_out_x, gt_out_y, "GT")
        
        lt_out_x, lt_out_y = self._get_grid_position(comp_start + 2, 5)
        lt_output_id = self.create_element("Led", lt_out_x, lt_out_y, "LT")
        
        eq_out_x, eq_out_y = self._get_grid_position(comp_start + 6, 5)
        eq_output_id = self.create_element("Led", eq_out_x, eq_out_y, "EQ")
        
        if not all([gt_output_id, lt_output_id, eq_output_id]):
            return None
        
        # Ultra-short connections
        final_connections = [
            (gt_and_id, 0, gt_output_id, 0),   # GT
            (borrow_id, 0, lt_output_id, 0),    # LT = borrow
            (zero_id, 0, eq_output_id, 0),      # EQ = zero
        ]
        
        for source_id, source_port, target_id, target_port in final_connections:
            if not self.connect_elements(source_id, source_port, target_id, target_port):
                return None
        
        return input_a_ids, input_b_ids, gt_output_id, lt_output_id, eq_output_id

    def _create_or_tree(self, signal_ids: List[int], prefix: str) -> Optional[int]:
        """Create OR tree to combine multiple signals."""
        return self._create_or_tree_positioned(signal_ids, prefix, 3, 0)
    
    def _create_or_tree_positioned(self, signal_ids: List[int], prefix: str, start_col: int, start_row: int) -> Optional[int]:
        """Create OR tree to combine multiple signals with specific positioning."""
        if len(signal_ids) == 1:
            return signal_ids[0]
        
        current_signals = signal_ids.copy()
        level = 0
        
        while len(current_signals) > 1:
            next_level_signals = []
            
            for i in range(0, len(current_signals), 2):
                if i + 1 < len(current_signals):
                    or_x, or_y = self._get_grid_position(start_col + level, start_row + (i // 2))
                    or_id = self.create_element("Or", or_x, or_y, f"{prefix}_{level}_{i//2}")
                    if not or_id:
                        return None
                    
                    if not self.connect_elements(current_signals[i], 0, or_id, 0):
                        return None
                    if not self.connect_elements(current_signals[i+1], 0, or_id, 1):
                        return None
                    
                    next_level_signals.append(or_id)
                else:
                    next_level_signals.append(current_signals[i])
            
            current_signals = next_level_signals
            level += 1
        
        return current_signals[0]
    
    def _create_and_tree_positioned(self, signal_ids: List[int], prefix: str, start_col: int, start_row: int) -> Optional[int]:
        """Create AND tree to combine multiple signals with specific positioning."""
        if len(signal_ids) == 1:
            return signal_ids[0]
        
        current_signals = signal_ids.copy()
        level = 0
        
        while len(current_signals) > 1:
            next_level_signals = []
            
            for i in range(0, len(current_signals), 2):
                if i + 1 < len(current_signals):
                    and_x, and_y = self._get_grid_position(start_col + level, start_row + (i // 2))
                    and_id = self.create_element("And", and_x, and_y, f"{prefix}_{level}_{i//2}")
                    if not and_id:
                        return None
                    
                    if not self.connect_elements(current_signals[i], 0, and_id, 0):
                        return None
                    if not self.connect_elements(current_signals[i+1], 0, and_id, 1):
                        return None
                    
                    next_level_signals.append(and_id)
                else:
                    next_level_signals.append(current_signals[i])
            
            current_signals = next_level_signals
            level += 1
        
        return current_signals[0]


    def cleanup(self):
        """Clean up resources"""
        if self.bridge:
            self.bridge.stop()
            self.bridge = None

    def run_all_tests(self) -> dict:
        """Run all Level 2 arithmetic building blocks tests."""
        print("🚀 CPU Level 2: Arithmetic Building Blocks Validation")
        print("=" * 60)
        
        tests = [
            self.test_basic_adders,
            self.test_multi_bit_adders,
            self.test_subtraction_circuits,
            self.test_comparator_circuits,
        ]
        
        results = []
        passed = 0
        total = len(tests)
        
        for test_func in tests:
            try:
                result = test_func()
                results.append(result)
                
                if result.get("success", False):
                    passed += 1
                    status = "✅ PASS"
                else:
                    status = "❌ FAIL"
                
                print(f"{status} - {result.get('test', 'unknown')}")
                
            except Exception as e:
                error_result = {
                    "test": test_func.__name__,
                    "success": False,
                    "error": str(e)
                }
                results.append(error_result)
                print(f"❌ ERROR - {test_func.__name__}: {e}")
        
        print(f"\n📊 Level 2 Results: {passed}/{total} tests passed ({passed/total*100:.1f}%)")
        
        return {
            "level": 2,
            "name": "Arithmetic Building Blocks",
            "total_tests": total,
            "passed": passed,
            "failed": total - passed,
            "pass_rate": passed / total * 100,
            "results": results
        }


def main():
    """Main execution function."""
    validator = ArithmeticBlocksValidator()
    
    try:
        # Run all Level 2 validation tests
        final_results = validator.run_all_tests()
        
        # Save detailed results
        with open("cpu_level_2_results.json", "w") as f:
            json.dump(final_results, f, indent=2)
        
        print(f"\n💾 Detailed results saved to: cpu_level_2_results.json")
        
        # Return exit code based on pass rate
        if final_results["pass_rate"] >= 95:
            print("🏆 EXCELLENT: Level 2 validation passed with high accuracy!")
            return 0
        elif final_results["pass_rate"] >= 80:
            print("✅ GOOD: Level 2 mostly validated with minor issues")
            return 0
        else:
            print("⚠️ ISSUES: Level 2 has significant validation failures")
            return 1
    finally:
        # Always cleanup
        validator.cleanup()


if __name__ == "__main__":
    sys.exit(main())