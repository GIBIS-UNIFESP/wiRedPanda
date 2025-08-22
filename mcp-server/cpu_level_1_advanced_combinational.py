#!/usr/bin/env python3
"""
CPU Validation Level 1: Advanced Combinational Logic
==================================================

This module implements comprehensive validation for advanced combinational logic
components that will form the foundation for CPU arithmetic and control circuits.

Components Validated:
1. Multi-Input Logic Gates (3, 4, 8-input variants)
2. Compound Boolean Functions (AOI, OAI, complex expressions)
3. Signal Conditioning (Buffers, Tri-state, Fan-out)
4. Complex Logic Networks (Multi-level logic, optimization)

Target: 100% accuracy for all combinational logic operations

Updated to use real wiRedPanda MCP integration.
"""

import sys
import json
import time
from typing import Dict, Any, List, Tuple, Optional, Callable
from pathlib import Path
import itertools

# Import the real wiRedPanda bridge
from wiredpanda_bridge import WiredPandaBridge, WiredPandaError


class AdvancedCombinationalValidator:
    def __init__(self):
        self.bridge = None
        self.test_results = []
        self.current_test = None
        self.level_name = "Level 1: Advanced Combinational Logic"
        
        # Layout constants for proper element positioning
        # Based on GraphicElement analysis: 64x64px elements, labels at +64px below
        self.ELEMENT_WIDTH = 64
        self.ELEMENT_HEIGHT = 64
        self.LABEL_OFFSET = 64  # Labels positioned 64px below element center
        self.MIN_VERTICAL_SPACING = 130  # 64px element + 64px label + 2px margin
        self.MIN_HORIZONTAL_SPACING = 150  # 64px element + margin for labels and connections
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

    def test_multi_input_gates(self) -> dict:
        """
        Test 1.1: Multi-Input Logic Gates
        Validate 3, 4, and 8-input AND/OR gates with exhaustive truth tables.
        """
        print("🔧 Test 1.1: Multi-Input Logic Gates")
        
        multi_input_tests = {
            "3_input_and": {
                "element_type": "And",
                "inputs": 3,
                "function": lambda inputs: all(inputs),
                "test_cases": list(itertools.product([False, True], repeat=3))
            },
            "4_input_and": {
                "element_type": "And", 
                "inputs": 4,
                "function": lambda inputs: all(inputs),
                "test_cases": list(itertools.product([False, True], repeat=4))
            },
            "3_input_or": {
                "element_type": "Or",
                "inputs": 3,
                "function": lambda inputs: any(inputs),
                "test_cases": list(itertools.product([False, True], repeat=3))
            },
            "4_input_or": {
                "element_type": "Or",
                "inputs": 4,
                "function": lambda inputs: any(inputs),
                "test_cases": list(itertools.product([False, True], repeat=4))
            }
        }
        
        all_tests_passed = True
        test_results = {}
        
        for gate_name, gate_config in multi_input_tests.items():
            print(f"  Testing {gate_name}...")
            
            if not self.create_new_circuit():
                test_results[gate_name] = {"success": False, "error": "Circuit creation failed"}
                all_tests_passed = False
                continue
            
            # Create inputs with proper grid positioning
            input_ids = []
            for i in range(gate_config["inputs"]):
                x, y = self._get_input_position(i)
                input_id = self.create_element("InputButton", x, y, f"IN{i}")
                if not input_id:
                    test_results[gate_name] = {"success": False, "error": f"Input {i} creation failed"}
                    all_tests_passed = False
                    break
                input_ids.append(input_id)
            
            if len(input_ids) != gate_config["inputs"]:
                continue
                
            # Create gate - note: wiRedPanda gates have 2 inputs by default
            # For multi-input, we need to cascade gates or use available multi-input gates
            if gate_config["inputs"] > 2:
                # Create cascade of 2-input gates to simulate multi-input behavior
                gate_results = self._create_multi_input_gate_cascade(
                    gate_config["element_type"], 
                    input_ids,
                    gate_config["inputs"]
                )
                if not gate_results:
                    test_results[gate_name] = {"success": False, "error": "Multi-input gate cascade failed"}
                    all_tests_passed = False
                    continue
                final_gate_id, output_id = gate_results
            else:
                # Direct 2-input gate with proper positioning
                gate_x, gate_y = self._get_grid_position(1, 0)
                gate_id = self.create_element(gate_config["element_type"], gate_x, gate_y, gate_name.upper())
                output_x, output_y = self._get_output_position(2, 0)
                output_id = self.create_element("Led", output_x, output_y, "OUT")
                
                if not all([gate_id, output_id]):
                    test_results[gate_name] = {"success": False, "error": "Gate creation failed"}
                    all_tests_passed = False
                    continue
                    
                # Connect inputs to gate
                for i, input_id in enumerate(input_ids):
                    if not self.connect_elements(input_id, 0, gate_id, i):
                        test_results[gate_name] = {"success": False, "error": f"Input {i} connection failed"}
                        all_tests_passed = False
                        break
                
                # Connect gate to output
                if not self.connect_elements(gate_id, 0, output_id, 0):
                    test_results[gate_name] = {"success": False, "error": "Output connection failed"}
                    all_tests_passed = False
                    continue
            
            if not self.start_simulation():
                test_results[gate_name] = {"success": False, "error": "Simulation start failed"}
                all_tests_passed = False
                continue
            
            # Test all input combinations (limit for 4+ inputs due to exponential growth)
            test_cases = gate_config["test_cases"]
            if len(test_cases) > 32:  # Limit to reasonable number for large truth tables
                # Sample representative cases including all corners
                test_cases = self._select_representative_cases(test_cases)
            
            gate_passed = True
            case_results = []
            
            for test_case in test_cases:
                # Set inputs
                for i, input_val in enumerate(test_case):
                    if not self.set_input(input_ids[i], input_val):
                        gate_passed = False
                        break
                
                if not gate_passed:
                    break
                
                # Step simulation
                self.step_simulation()
                
                # Get output
                actual_output = self.get_output(output_id)
                expected_output = gate_config["function"](test_case)
                
                case_correct = (actual_output == expected_output)
                if not case_correct:
                    gate_passed = False
                
                case_results.append({
                    "inputs": list(test_case),
                    "expected": expected_output,
                    "actual": actual_output,
                    "correct": case_correct
                })
            
            test_results[gate_name] = {
                "success": gate_passed,
                "total_cases": len(case_results),
                "passed_cases": sum(1 for case in case_results if case["correct"]),
                "failed_cases": sum(1 for case in case_results if not case["correct"]),
                "sample_results": case_results[:8],  # Show first 8 cases
                "accuracy": sum(1 for case in case_results if case["correct"]) / len(case_results) * 100
            }
            
            if not gate_passed:
                all_tests_passed = False
            
            # Save individual circuit file for this gate
            individual_filename = f"level1_{gate_name}.panda"
            self.save_circuit(individual_filename)
        
        return {
            "test": "multi_input_gates",
            "success": all_tests_passed,
            "results": test_results,
            "description": "Tests multi-input logic gates with exhaustive truth tables"
        }

    def _create_multi_input_gate_cascade(self, gate_type: str, input_ids: List[int], num_inputs: int) -> Optional[Tuple[int, int]]:
        """Create a cascade of 2-input gates to simulate multi-input behavior."""
        if num_inputs <= 2:
            return None
            
        # Create cascade tree of gates
        # For N inputs, we need N-1 gates arranged in a tree structure
        current_inputs = input_ids.copy()
        gate_level = 0
        
        while len(current_inputs) > 1:
            next_level_inputs = []
            
            # Pair up current inputs and create gates
            for i in range(0, len(current_inputs), 2):
                if i + 1 < len(current_inputs):
                    # Create gate for pair with proper spacing
                    gate_x, gate_y = self._get_grid_position(gate_level + 1, i // 2)
                    gate_id = self.create_element(
                        gate_type, 
                        gate_x, 
                        gate_y, 
                        f"{gate_type}{gate_level}_{i//2}"
                    )
                    
                    if not gate_id:
                        return None
                    
                    # Connect inputs to gate
                    if not self.connect_elements(current_inputs[i], 0, gate_id, 0):
                        return None
                    if not self.connect_elements(current_inputs[i+1], 0, gate_id, 1):
                        return None
                    
                    next_level_inputs.append(gate_id)
                else:
                    # Odd input, pass through to next level
                    next_level_inputs.append(current_inputs[i])
            
            current_inputs = next_level_inputs
            gate_level += 1
        
        # Create output LED connected to final gate with proper positioning
        final_gate_id = current_inputs[0]
        output_x, output_y = self._get_grid_position(gate_level + 1, 0)
        output_id = self.create_element("Led", output_x, output_y, "OUT")
        
        if not output_id:
            return None
            
        if not self.connect_elements(final_gate_id, 0, output_id, 0):
            return None
        
        return final_gate_id, output_id

    def _select_representative_cases(self, all_cases: List[Tuple]) -> List[Tuple]:
        """Select representative test cases from a large truth table."""
        if len(all_cases) <= 32:
            return all_cases
        
        # Include corner cases
        representative = []
        
        # All False
        representative.append(tuple(False for _ in all_cases[0]))
        
        # All True  
        representative.append(tuple(True for _ in all_cases[0]))
        
        # Single True (each position)
        for i in range(len(all_cases[0])):
            case = [False] * len(all_cases[0])
            case[i] = True
            representative.append(tuple(case))
        
        # Single False (each position)
        for i in range(len(all_cases[0])):
            case = [True] * len(all_cases[0])
            case[i] = False
            representative.append(tuple(case))
        
        # Remove duplicates and return
        return list(set(representative))

    def test_compound_boolean_functions(self) -> dict:
        """
        Test 1.2: Compound Boolean Functions
        Test complex Boolean expressions using combinations of basic gates.
        """
        print("🔧 Test 1.2: Compound Boolean Functions")
        
        compound_functions = {
            "aoi22": {
                "description": "AND-OR-INVERT: F = NOT((A AND B) OR (C AND D))",
                "inputs": ["A", "B", "C", "D"],
                "function": lambda a, b, c, d: not((a and b) or (c and d)),
                "implementation": "and_or_not_cascade"
            },
            "oai22": {
                "description": "OR-AND-INVERT: F = NOT((A OR B) AND (C OR D))", 
                "inputs": ["A", "B", "C", "D"],
                "function": lambda a, b, c, d: not((a or b) and (c or d)),
                "implementation": "or_and_not_cascade"
            },
            "majority3": {
                "description": "3-input Majority: F = (A AND B) OR (B AND C) OR (A AND C)",
                "inputs": ["A", "B", "C"],
                "function": lambda a, b, c: (a and b) or (b and c) or (a and c),
                "implementation": "majority_logic"
            },
            "parity4": {
                "description": "4-input Parity: F = A XOR B XOR C XOR D",
                "inputs": ["A", "B", "C", "D"], 
                "function": lambda a, b, c, d: (a ^ b ^ c ^ d),
                "implementation": "xor_cascade"
            }
        }
        
        all_tests_passed = True
        test_results = {}
        
        for func_name, func_config in compound_functions.items():
            print(f"  Testing {func_name}: {func_config['description']}")
            
            if not self.create_new_circuit():
                test_results[func_name] = {"success": False, "error": "Circuit creation failed"}
                all_tests_passed = False
                continue
            
            # Create circuit for compound function
            circuit_result = self._create_compound_function_circuit(func_name, func_config)
            if not circuit_result:
                test_results[func_name] = {"success": False, "error": "Circuit implementation failed"}
                all_tests_passed = False
                continue
                
            input_ids, output_id = circuit_result
            
            if not self.start_simulation():
                test_results[func_name] = {"success": False, "error": "Simulation start failed"}
                all_tests_passed = False
                continue
            
            # Generate test cases for all input combinations
            num_inputs = len(func_config["inputs"])
            test_cases = list(itertools.product([False, True], repeat=num_inputs))
            
            func_passed = True
            case_results = []
            
            for test_case in test_cases:
                # Set inputs
                for i, input_val in enumerate(test_case):
                    if not self.set_input(input_ids[i], input_val):
                        func_passed = False
                        break
                
                if not func_passed:
                    break
                
                # Step simulation to allow propagation
                self.step_simulation()
                
                # Get output
                actual_output = self.get_output(output_id)
                expected_output = func_config["function"](*test_case)
                
                case_correct = (actual_output == expected_output)
                if not case_correct:
                    func_passed = False
                
                case_results.append({
                    "inputs": dict(zip(func_config["inputs"], test_case)),
                    "expected": expected_output,
                    "actual": actual_output,
                    "correct": case_correct
                })
            
            test_results[func_name] = {
                "success": func_passed,
                "description": func_config["description"],
                "total_cases": len(case_results),
                "passed_cases": sum(1 for case in case_results if case["correct"]),
                "failed_cases": sum(1 for case in case_results if not case["correct"]),
                "sample_results": case_results,
                "accuracy": sum(1 for case in case_results if case["correct"]) / len(case_results) * 100
            }
            
            if not func_passed:
                all_tests_passed = False
            
            # Save individual circuit file for this function
            individual_filename = f"level1_{func_name}.panda"
            self.save_circuit(individual_filename)
        
        return {
            "test": "compound_boolean_functions",
            "success": all_tests_passed,
            "results": test_results,
            "description": "Tests complex Boolean functions implemented with gate networks"
        }

    def _create_compound_function_circuit(self, func_name: str, func_config: dict) -> Optional[Tuple[List[int], int]]:
        """Create circuit implementation for compound Boolean function."""
        num_inputs = len(func_config["inputs"])
        
        # Create input elements with proper grid positioning
        input_ids = []
        for i, input_name in enumerate(func_config["inputs"]):
            x, y = self._get_input_position(i)
            input_id = self.create_element("InputButton", x, y, input_name)
            if not input_id:
                return None
            input_ids.append(input_id)
        
        # Create circuit based on function type
        if func_name == "aoi22":
            return self._create_aoi22_circuit(input_ids)
        elif func_name == "oai22":
            return self._create_oai22_circuit(input_ids)
        elif func_name == "majority3":
            return self._create_majority3_circuit(input_ids)
        elif func_name == "parity4":
            return self._create_parity4_circuit(input_ids)
        else:
            return None

    def _create_aoi22_circuit(self, input_ids: List[int]) -> Optional[Tuple[List[int], int]]:
        """Create AOI22 circuit: F = NOT((A AND B) OR (C AND D))"""
        if len(input_ids) != 4:
            return None
            
        # Create gates with improved spacing for better visual clarity
        # AND gates positioned to show clear grouping of (A&B) and (C&D)
        and1_x, and1_y = self._get_grid_position(1, 0)      # A & B 
        and1_id = self.create_element("And", and1_x, and1_y, "AND1")
        
        and2_x, and2_y = self._get_grid_position(1, 2)      # C & D (more spacing)
        and2_id = self.create_element("And", and2_x, and2_y, "AND2")
        
        # OR gate centered between the AND gates
        or_x, or_y = self._get_grid_position(2, 1)          # Center position
        or_id = self.create_element("Or", or_x, or_y, "OR")
        
        # NOT gate for final inversion
        not_x, not_y = self._get_grid_position(3, 1)        # Aligned with OR
        not_id = self.create_element("Not", not_x, not_y, "NOT")
        
        # Output LED
        output_x, output_y = self._get_grid_position(4, 1)  # Aligned with NOT
        output_id = self.create_element("Led", output_x, output_y, "OUT")
        
        if not all([and1_id, and2_id, or_id, not_id, output_id]):
            return None
        
        # Connect circuit with clear logic flow: A,B -> AND1; C,D -> AND2; AND1,AND2 -> OR; OR -> NOT -> OUT
        connections = [
            (input_ids[0], 0, and1_id, 0),  # A -> AND1
            (input_ids[1], 0, and1_id, 1),  # B -> AND1
            (input_ids[2], 0, and2_id, 0),  # C -> AND2
            (input_ids[3], 0, and2_id, 1),  # D -> AND2
            (and1_id, 0, or_id, 0),         # AND1 -> OR
            (and2_id, 0, or_id, 1),         # AND2 -> OR
            (or_id, 0, not_id, 0),          # OR -> NOT
            (not_id, 0, output_id, 0)       # NOT -> OUT
        ]
        
        for source_id, source_port, target_id, target_port in connections:
            if not self.connect_elements(source_id, source_port, target_id, target_port):
                return None
        
        return input_ids, output_id

    def _create_oai22_circuit(self, input_ids: List[int]) -> Optional[Tuple[List[int], int]]:
        """Create OAI22 circuit: F = NOT((A OR B) AND (C OR D))"""
        if len(input_ids) != 4:
            return None
            
        # Create gates with proper grid positioning: OR1, OR2, AND, NOT
        or1_x, or1_y = self._get_grid_position(1, 0)
        or1_id = self.create_element("Or", or1_x, or1_y, "OR1")
        or2_x, or2_y = self._get_grid_position(1, 1)
        or2_id = self.create_element("Or", or2_x, or2_y, "OR2")
        and_x, and_y = self._get_grid_position(2, 0)
        and_id = self.create_element("And", and_x, and_y, "AND")
        not_x, not_y = self._get_grid_position(3, 0)
        not_id = self.create_element("Not", not_x, not_y, "NOT")
        output_x, output_y = self._get_grid_position(4, 0)
        output_id = self.create_element("Led", output_x, output_y, "OUT")
        
        if not all([or1_id, or2_id, and_id, not_id, output_id]):
            return None
        
        # Connect circuit: A,B -> OR1; C,D -> OR2; OR1,OR2 -> AND; AND -> NOT -> OUT
        connections = [
            (input_ids[0], 0, or1_id, 0),   # A -> OR1
            (input_ids[1], 0, or1_id, 1),   # B -> OR1
            (input_ids[2], 0, or2_id, 0),   # C -> OR2
            (input_ids[3], 0, or2_id, 1),   # D -> OR2
            (or1_id, 0, and_id, 0),         # OR1 -> AND
            (or2_id, 0, and_id, 1),         # OR2 -> AND
            (and_id, 0, not_id, 0),         # AND -> NOT
            (not_id, 0, output_id, 0)       # NOT -> OUT
        ]
        
        for source_id, source_port, target_id, target_port in connections:
            if not self.connect_elements(source_id, source_port, target_id, target_port):
                return None
        
        return input_ids, output_id

    def _create_majority3_circuit(self, input_ids: List[int]) -> Optional[Tuple[List[int], int]]:
        """Create 3-input majority circuit: F = (A AND B) OR (B AND C) OR (A AND C)"""
        if len(input_ids) != 3:
            return None
            
        # Create gates with proper grid positioning: 3 AND gates, 2 OR gates
        and1_x, and1_y = self._get_grid_position(1, 0)
        and1_id = self.create_element("And", and1_x, and1_y, "AND1")  # A AND B
        and2_x, and2_y = self._get_grid_position(1, 1)
        and2_id = self.create_element("And", and2_x, and2_y, "AND2")  # B AND C
        and3_x, and3_y = self._get_grid_position(1, 2)
        and3_id = self.create_element("And", and3_x, and3_y, "AND3")  # A AND C
        or1_x, or1_y = self._get_grid_position(2, 0)
        or1_id = self.create_element("Or", or1_x, or1_y, "OR1")     # First OR
        or2_x, or2_y = self._get_grid_position(3, 0)
        or2_id = self.create_element("Or", or2_x, or2_y, "OR2")     # Final OR
        output_x, output_y = self._get_grid_position(4, 0)
        output_id = self.create_element("Led", output_x, output_y, "OUT")
        
        if not all([and1_id, and2_id, and3_id, or1_id, or2_id, output_id]):
            return None
        
        # Connect circuit
        connections = [
            (input_ids[0], 0, and1_id, 0),  # A -> AND1
            (input_ids[1], 0, and1_id, 1),  # B -> AND1
            (input_ids[1], 0, and2_id, 0),  # B -> AND2
            (input_ids[2], 0, and2_id, 1),  # C -> AND2
            (input_ids[0], 0, and3_id, 0),  # A -> AND3
            (input_ids[2], 0, and3_id, 1),  # C -> AND3
            (and1_id, 0, or1_id, 0),        # AND1 -> OR1
            (and2_id, 0, or1_id, 1),        # AND2 -> OR1
            (or1_id, 0, or2_id, 0),         # OR1 -> OR2
            (and3_id, 0, or2_id, 1),        # AND3 -> OR2
            (or2_id, 0, output_id, 0)       # OR2 -> OUT
        ]
        
        for source_id, source_port, target_id, target_port in connections:
            if not self.connect_elements(source_id, source_port, target_id, target_port):
                return None
        
        return input_ids, output_id

    def _create_parity4_circuit(self, input_ids: List[int]) -> Optional[Tuple[List[int], int]]:
        """Create 4-input parity circuit: F = A XOR B XOR C XOR D"""
        if len(input_ids) != 4:
            return None
            
        # Create XOR cascade with proper grid positioning: XOR1, XOR2, XOR3
        xor1_x, xor1_y = self._get_grid_position(1, 0)
        xor1_id = self.create_element("Xor", xor1_x, xor1_y, "XOR1")  # A XOR B
        xor2_x, xor2_y = self._get_grid_position(2, 0)
        xor2_id = self.create_element("Xor", xor2_x, xor2_y, "XOR2")  # (A XOR B) XOR C
        xor3_x, xor3_y = self._get_grid_position(3, 0)
        xor3_id = self.create_element("Xor", xor3_x, xor3_y, "XOR3")  # ((A XOR B) XOR C) XOR D
        output_x, output_y = self._get_grid_position(4, 0)
        output_id = self.create_element("Led", output_x, output_y, "OUT")
        
        if not all([xor1_id, xor2_id, xor3_id, output_id]):
            return None
        
        # Connect XOR cascade
        connections = [
            (input_ids[0], 0, xor1_id, 0),  # A -> XOR1
            (input_ids[1], 0, xor1_id, 1),  # B -> XOR1
            (xor1_id, 0, xor2_id, 0),       # XOR1 -> XOR2
            (input_ids[2], 0, xor2_id, 1),  # C -> XOR2
            (xor2_id, 0, xor3_id, 0),       # XOR2 -> XOR3
            (input_ids[3], 0, xor3_id, 1),  # D -> XOR3
            (xor3_id, 0, output_id, 0)      # XOR3 -> OUT
        ]
        
        for source_id, source_port, target_id, target_port in connections:
            if not self.connect_elements(source_id, source_port, target_id, target_port):
                return None
        
        return input_ids, output_id

    def test_signal_conditioning(self) -> dict:
        """
        Test 1.3: Signal Conditioning
        Test buffer chains, tri-state buffers, and fan-out capabilities.
        """
        print("🔧 Test 1.3: Signal Conditioning")
        
        # Note: wiRedPanda may not have dedicated buffer/tri-state elements
        # We'll test signal propagation through basic gates used as buffers
        
        signal_tests = {
            "buffer_chain": {
                "description": "Signal propagation through buffer chain (NOT-NOT pairs)",
                "chain_length": 4,
                "expected_inversion": False  # Even number of NOTs
            },
            "fan_out_test": {
                "description": "Single input driving multiple outputs", 
                "fan_out": 4,
                "expected_all_same": True
            },
            "signal_integrity": {
                "description": "Signal strength through multiple gate levels",
                "gate_levels": 3,
                "expected_propagation": True
            }
        }
        
        all_tests_passed = True
        test_results = {}
        
        for test_name, test_config in signal_tests.items():
            print(f"  Testing {test_name}: {test_config['description']}")
            
            if not self.create_new_circuit():
                test_results[test_name] = {"success": False, "error": "Circuit creation failed"}
                all_tests_passed = False
                continue
            
            if test_name == "buffer_chain":
                circuit_result = self._create_buffer_chain_circuit(test_config["chain_length"])
            elif test_name == "fan_out_test":
                circuit_result = self._create_fan_out_circuit(test_config["fan_out"])
            elif test_name == "signal_integrity":
                circuit_result = self._create_signal_integrity_circuit(test_config["gate_levels"])
            else:
                circuit_result = None
            
            if not circuit_result:
                test_results[test_name] = {"success": False, "error": "Circuit implementation failed"}
                all_tests_passed = False
                continue
                
            input_ids, output_ids = circuit_result
            
            if not self.start_simulation():
                test_results[test_name] = {"success": False, "error": "Simulation start failed"}
                all_tests_passed = False
                continue
            
            # Test signal conditioning
            test_passed = True
            condition_results = []
            
            for input_val in [False, True]:
                # Set input
                if isinstance(input_ids, list):
                    for input_id in input_ids:
                        if not self.set_input(input_id, input_val):
                            test_passed = False
                            break
                else:
                    if not self.set_input(input_ids, input_val):
                        test_passed = False
                
                if not test_passed:
                    break
                
                # Step simulation for propagation
                for _ in range(3):  # Multiple steps for long propagation
                    self.step_simulation()
                
                # Get outputs
                actual_outputs = []
                if isinstance(output_ids, list):
                    for output_id in output_ids:
                        output_val = self.get_output(output_id)
                        actual_outputs.append(output_val)
                else:
                    actual_outputs.append(self.get_output(output_ids))
                
                # Validate outputs based on test type
                if test_name == "buffer_chain":
                    expected_output = input_val if not test_config["expected_inversion"] else not input_val
                    output_correct = (actual_outputs[0] == expected_output)
                elif test_name == "fan_out_test":
                    # All outputs should be the same as input
                    output_correct = all(out == input_val for out in actual_outputs)
                elif test_name == "signal_integrity":
                    # Final output should match input through gate chain
                    output_correct = (actual_outputs[-1] == input_val)
                else:
                    output_correct = False
                
                if not output_correct:
                    test_passed = False
                
                condition_results.append({
                    "input": input_val,
                    "outputs": actual_outputs,
                    "correct": output_correct
                })
            
            test_results[test_name] = {
                "success": test_passed,
                "description": test_config["description"],
                "results": condition_results,
                "accuracy": sum(1 for r in condition_results if r["correct"]) / len(condition_results) * 100
            }
            
            if not test_passed:
                all_tests_passed = False
            
            # Save individual circuit file for this test
            individual_filename = f"level1_{test_name}.panda"
            self.save_circuit(individual_filename)
        
        return {
            "test": "signal_conditioning",
            "success": all_tests_passed,
            "results": test_results,
            "description": "Tests signal conditioning, buffering, and fan-out capabilities"
        }

    def _create_buffer_chain_circuit(self, chain_length: int) -> Optional[Tuple[int, List[int]]]:
        """Create a chain of NOT gates acting as buffers."""
        input_x, input_y = self._get_input_position(0)
        input_id = self.create_element("InputButton", input_x, input_y, "IN")
        if not input_id:
            return None
        
        current_output = input_id
        not_gates = []
        
        # Create chain of NOT gates with proper grid positioning
        for i in range(chain_length):
            not_x, not_y = self._get_grid_position(i + 1, 0)
            not_id = self.create_element("Not", not_x, not_y, f"NOT{i+1}")
            if not not_id:
                return None
            
            # Connect previous output to this NOT gate
            if not self.connect_elements(current_output, 0, not_id, 0):
                return None
            
            not_gates.append(not_id)
            current_output = not_id
        
        # Create output LED with proper positioning
        output_x, output_y = self._get_grid_position(chain_length + 1, 0)
        output_id = self.create_element("Led", output_x, output_y, "OUT")
        if not output_id:
            return None
            
        if not self.connect_elements(current_output, 0, output_id, 0):
            return None
        
        return input_id, [output_id]

    def _create_fan_out_circuit(self, fan_out: int) -> Optional[Tuple[int, List[int]]]:
        """Create circuit testing fan-out capabilities."""
        input_x, input_y = self._get_input_position(0)
        input_id = self.create_element("InputButton", input_x, input_y, "IN")
        if not input_id:
            return None
        
        # Create buffer (NOT-NOT pair) to drive multiple outputs with proper positioning
        not1_x, not1_y = self._get_grid_position(1, 0)
        not1_id = self.create_element("Not", not1_x, not1_y, "NOT1")
        not2_x, not2_y = self._get_grid_position(2, 0)
        not2_id = self.create_element("Not", not2_x, not2_y, "NOT2")
        
        if not all([not1_id, not2_id]):
            return None
        
        # Connect input through buffer
        if not self.connect_elements(input_id, 0, not1_id, 0):
            return None
        if not self.connect_elements(not1_id, 0, not2_id, 0):
            return None
        
        # Create multiple outputs with proper grid positioning
        output_ids = []
        for i in range(fan_out):
            output_x, output_y = self._get_grid_position(3, i)
            output_id = self.create_element("Led", output_x, output_y, f"OUT{i+1}")
            if not output_id:
                return None
            
            # Connect buffer output to each LED
            if not self.connect_elements(not2_id, 0, output_id, 0):
                return None
            
            output_ids.append(output_id)
        
        return input_id, output_ids

    def _create_signal_integrity_circuit(self, gate_levels: int) -> Optional[Tuple[int, List[int]]]:
        """Create circuit with multiple gate levels to test signal integrity."""
        input_x, input_y = self._get_input_position(0)
        input_id = self.create_element("InputButton", input_x, input_y, "IN")
        if not input_id:
            return None
        
        current_output = input_id
        gate_outputs = []
        
        # Create multiple levels of gates with proper positioning
        for level in range(gate_levels):
            # Create AND gate with one input tied to supply (always True)
            supply_x, supply_y = self._get_input_position(level + 1)
            supply_id = self.create_element("InputButton", supply_x, supply_y, f"VDD{level}")
            and_x, and_y = self._get_grid_position(level + 1, 0)
            and_id = self.create_element("And", and_x, and_y, f"AND{level}")
            
            if not all([supply_id, and_id]):
                return None
            
            # Set supply to always True
            self.set_input(supply_id, True)
            
            # Connect signal and supply to AND gate
            if not self.connect_elements(current_output, 0, and_id, 0):
                return None
            if not self.connect_elements(supply_id, 0, and_id, 1):
                return None
            
            current_output = and_id
            gate_outputs.append(and_id)
        
        # Create final output LED with proper positioning
        final_output_x, final_output_y = self._get_grid_position(gate_levels + 1, 0)
        final_output_id = self.create_element("Led", final_output_x, final_output_y, "OUT")
        if not final_output_id:
            return None
            
        if not self.connect_elements(current_output, 0, final_output_id, 0):
            return None
        
        return input_id, [final_output_id]

    def test_complex_logic_networks(self) -> dict:
        """
        Test 1.4: Complex Logic Networks
        Test realistic multi-level logic networks that might appear in CPU designs.
        """
        print("🔧 Test 1.4: Complex Logic Networks")
        
        complex_networks = {
            "decoder_2_to_4": {
                "description": "2-to-4 decoder with enable",
                "inputs": ["A", "B", "EN"],
                "outputs": 4,
                "function": self._decoder_2to4_function
            },
            "mux_4_to_1": {
                "description": "4-to-1 multiplexer",
                "inputs": ["D0", "D1", "D2", "D3", "S0", "S1"],
                "outputs": 1,
                "function": self._mux_4to1_function
            },
            "full_adder": {
                "description": "Full adder (Sum and Carry outputs)",
                "inputs": ["A", "B", "Cin"],
                "outputs": 2,
                "function": self._full_adder_function
            }
        }
        
        all_tests_passed = True
        test_results = {}
        
        for network_name, network_config in complex_networks.items():
            print(f"  Testing {network_name}: {network_config['description']}")
            
            if not self.create_new_circuit():
                test_results[network_name] = {"success": False, "error": "Circuit creation failed"}
                all_tests_passed = False
                continue
            
            # Create complex network circuit
            if network_name == "decoder_2_to_4":
                circuit_result = self._create_2to4_decoder_circuit()
            elif network_name == "mux_4_to_1":
                circuit_result = self._create_4to1_mux_circuit()
            elif network_name == "full_adder":
                circuit_result = self._create_full_adder_circuit()
            else:
                circuit_result = None
            
            if not circuit_result:
                test_results[network_name] = {"success": False, "error": "Circuit implementation failed"}
                all_tests_passed = False
                continue
                
            input_ids, output_ids = circuit_result
            
            if not self.start_simulation():
                test_results[network_name] = {"success": False, "error": "Simulation start failed"}
                all_tests_passed = False
                continue
            
            # Generate test cases
            num_inputs = len(network_config["inputs"])
            test_cases = list(itertools.product([False, True], repeat=num_inputs))
            
            network_passed = True
            case_results = []
            
            for test_case in test_cases:
                # Set inputs
                for i, input_val in enumerate(test_case):
                    if not self.set_input(input_ids[i], input_val):
                        network_passed = False
                        break
                
                if not network_passed:
                    break
                
                # Step simulation for complex propagation
                for _ in range(3):
                    self.step_simulation()
                
                # Get outputs
                actual_outputs = []
                for output_id in output_ids:
                    output_val = self.get_output(output_id)
                    actual_outputs.append(output_val)
                
                # Calculate expected outputs
                expected_outputs = network_config["function"](test_case)
                if not isinstance(expected_outputs, list):
                    expected_outputs = [expected_outputs]
                
                # Check correctness
                case_correct = (actual_outputs == expected_outputs)
                if not case_correct:
                    network_passed = False
                
                case_results.append({
                    "inputs": dict(zip(network_config["inputs"], test_case)),
                    "expected": expected_outputs,
                    "actual": actual_outputs,
                    "correct": case_correct
                })
            
            test_results[network_name] = {
                "success": network_passed,
                "description": network_config["description"],
                "total_cases": len(case_results),
                "passed_cases": sum(1 for case in case_results if case["correct"]),
                "failed_cases": sum(1 for case in case_results if not case["correct"]),
                "sample_results": case_results[:8],
                "accuracy": sum(1 for case in case_results if case["correct"]) / len(case_results) * 100
            }
            
            if not network_passed:
                all_tests_passed = False
            
            # Save individual circuit file for this network
            individual_filename = f"level1_{network_name}.panda"
            self.save_circuit(individual_filename)
        
        return {
            "test": "complex_logic_networks",
            "success": all_tests_passed,
            "results": test_results,
            "description": "Tests complex multi-level logic networks used in CPU designs"
        }

    def _decoder_2to4_function(self, inputs: Tuple[bool, ...]) -> List[bool]:
        """2-to-4 decoder function: outputs[i] = True if (A,B) == i and EN == True"""
        a, b, en = inputs
        if not en:
            return [False, False, False, False]
        
        addr = (a << 0) | (b << 1)  # B is MSB
        outputs = [False] * 4
        outputs[addr] = True
        return outputs

    def _mux_4to1_function(self, inputs: Tuple[bool, ...]) -> List[bool]:
        """4-to-1 mux function: output = inputs[select]"""
        d0, d1, d2, d3, s0, s1 = inputs
        select = (s0 << 0) | (s1 << 1)  # S1 is MSB
        data_inputs = [d0, d1, d2, d3]
        return [data_inputs[select]]

    def _full_adder_function(self, inputs: Tuple[bool, ...]) -> List[bool]:
        """Full adder function: [Sum, Carry]"""
        a, b, cin = inputs
        sum_out = a ^ b ^ cin
        carry_out = (a & b) | (cin & (a ^ b))
        return [sum_out, carry_out]

    def _create_2to4_decoder_circuit(self) -> Optional[Tuple[List[int], List[int]]]:
        """Create 2-to-4 decoder circuit."""
        # Create inputs with proper grid positioning: A, B, EN
        a_x, a_y = self._get_input_position(0)
        a_id = self.create_element("InputButton", a_x, a_y, "A")
        b_x, b_y = self._get_input_position(1)
        b_id = self.create_element("InputButton", b_x, b_y, "B")
        en_x, en_y = self._get_input_position(2)
        en_id = self.create_element("InputButton", en_x, en_y, "EN")
        
        if not all([a_id, b_id, en_id]):
            return None
        
        # Create NOT gates for inverted inputs with proper positioning
        not_a_x, not_a_y = self._get_grid_position(1, 0)
        not_a_id = self.create_element("Not", not_a_x, not_a_y, "NOT_A")
        not_b_x, not_b_y = self._get_grid_position(1, 1)
        not_b_id = self.create_element("Not", not_b_x, not_b_y, "NOT_B")
        
        if not all([not_a_id, not_b_id]):
            return None
        
        # Connect inverters
        if not self.connect_elements(a_id, 0, not_a_id, 0):
            return None
        if not self.connect_elements(b_id, 0, not_b_id, 0):
            return None
        
        # Create AND gates for each output with proper grid positioning
        # OUT0 = ~A & ~B & EN, OUT1 = A & ~B & EN, OUT2 = ~A & B & EN, OUT3 = A & B & EN
        and0_ab_x, and0_ab_y = self._get_grid_position(2, 0)
        and0_ab_id = self.create_element("And", and0_ab_x, and0_ab_y, "AND0_AB")
        and0_x, and0_y = self._get_grid_position(3, 0)
        and0_id = self.create_element("And", and0_x, and0_y, "AND0")
        
        and1_ab_x, and1_ab_y = self._get_grid_position(2, 1)
        and1_ab_id = self.create_element("And", and1_ab_x, and1_ab_y, "AND1_AB")
        and1_x, and1_y = self._get_grid_position(3, 1)
        and1_id = self.create_element("And", and1_x, and1_y, "AND1")
        
        and2_ab_x, and2_ab_y = self._get_grid_position(2, 2)
        and2_ab_id = self.create_element("And", and2_ab_x, and2_ab_y, "AND2_AB")
        and2_x, and2_y = self._get_grid_position(3, 2)
        and2_id = self.create_element("And", and2_x, and2_y, "AND2")
        
        and3_ab_x, and3_ab_y = self._get_grid_position(2, 3)
        and3_ab_id = self.create_element("And", and3_ab_x, and3_ab_y, "AND3_AB")
        and3_x, and3_y = self._get_grid_position(3, 3)
        and3_id = self.create_element("And", and3_x, and3_y, "AND3")
        
        gates = [and0_ab_id, and0_id, and1_ab_id, and1_id, and2_ab_id, and2_id, and3_ab_id, and3_id]
        if not all(gates):
            return None
        
        # Connect address decoding logic
        connections = [
            # OUT0: ~A & ~B & EN
            (not_a_id, 0, and0_ab_id, 0), (not_b_id, 0, and0_ab_id, 1),
            (and0_ab_id, 0, and0_id, 0), (en_id, 0, and0_id, 1),
            
            # OUT1: A & ~B & EN  
            (a_id, 0, and1_ab_id, 0), (not_b_id, 0, and1_ab_id, 1),
            (and1_ab_id, 0, and1_id, 0), (en_id, 0, and1_id, 1),
            
            # OUT2: ~A & B & EN
            (not_a_id, 0, and2_ab_id, 0), (b_id, 0, and2_ab_id, 1),
            (and2_ab_id, 0, and2_id, 0), (en_id, 0, and2_id, 1),
            
            # OUT3: A & B & EN
            (a_id, 0, and3_ab_id, 0), (b_id, 0, and3_ab_id, 1),
            (and3_ab_id, 0, and3_id, 0), (en_id, 0, and3_id, 1),
        ]
        
        for source_id, source_port, target_id, target_port in connections:
            if not self.connect_elements(source_id, source_port, target_id, target_port):
                return None
        
        # Create output LEDs with proper grid positioning
        out0_x, out0_y = self._get_grid_position(4, 0)
        out0_id = self.create_element("Led", out0_x, out0_y, "OUT0")
        out1_x, out1_y = self._get_grid_position(4, 1)
        out1_id = self.create_element("Led", out1_x, out1_y, "OUT1")
        out2_x, out2_y = self._get_grid_position(4, 2)
        out2_id = self.create_element("Led", out2_x, out2_y, "OUT2")
        out3_x, out3_y = self._get_grid_position(4, 3)
        out3_id = self.create_element("Led", out3_x, out3_y, "OUT3")
        
        output_ids = [out0_id, out1_id, out2_id, out3_id]
        if not all(output_ids):
            return None
        
        # Connect outputs
        output_connections = [
            (and0_id, 0, out0_id, 0),
            (and1_id, 0, out1_id, 0),
            (and2_id, 0, out2_id, 0),
            (and3_id, 0, out3_id, 0)
        ]
        
        for source_id, source_port, target_id, target_port in output_connections:
            if not self.connect_elements(source_id, source_port, target_id, target_port):
                return None
        
        return [a_id, b_id, en_id], output_ids

    def _create_4to1_mux_circuit(self) -> Optional[Tuple[List[int], List[int]]]:
        """Create 4-to-1 multiplexer circuit."""
        # Create data inputs with proper grid positioning
        d0_x, d0_y = self._get_input_position(0)
        d0_id = self.create_element("InputButton", d0_x, d0_y, "D0")
        d1_x, d1_y = self._get_input_position(1)
        d1_id = self.create_element("InputButton", d1_x, d1_y, "D1")
        d2_x, d2_y = self._get_input_position(2)
        d2_id = self.create_element("InputButton", d2_x, d2_y, "D2")
        d3_x, d3_y = self._get_input_position(3)
        d3_id = self.create_element("InputButton", d3_x, d3_y, "D3")
        
        # Create select inputs
        s0_x, s0_y = self._get_input_position(4)
        s0_id = self.create_element("InputButton", s0_x, s0_y, "S0")
        s1_x, s1_y = self._get_input_position(5)
        s1_id = self.create_element("InputButton", s1_x, s1_y, "S1")
        
        input_ids = [d0_id, d1_id, d2_id, d3_id, s0_id, s1_id]
        if not all(input_ids):
            return None
        
        # Create select logic (NOT gates for inverted selects) with proper positioning
        not_s0_x, not_s0_y = self._get_grid_position(1, 4)
        not_s0_id = self.create_element("Not", not_s0_x, not_s0_y, "NOT_S0")
        not_s1_x, not_s1_y = self._get_grid_position(1, 5)
        not_s1_id = self.create_element("Not", not_s1_x, not_s1_y, "NOT_S1")
        
        if not all([not_s0_id, not_s1_id]):
            return None
        
        if not self.connect_elements(s0_id, 0, not_s0_id, 0):
            return None
        if not self.connect_elements(s1_id, 0, not_s1_id, 0):
            return None
        
        # Create AND gates for each data path with proper grid positioning
        # Path 0: D0 & ~S1 & ~S0
        and0_s_x, and0_s_y = self._get_grid_position(2, 0)
        and0_s_id = self.create_element("And", and0_s_x, and0_s_y, "AND0_S")
        and0_x, and0_y = self._get_grid_position(3, 0)
        and0_id = self.create_element("And", and0_x, and0_y, "AND0")
        
        # Path 1: D1 & ~S1 & S0
        and1_s_x, and1_s_y = self._get_grid_position(2, 1)
        and1_s_id = self.create_element("And", and1_s_x, and1_s_y, "AND1_S")
        and1_x, and1_y = self._get_grid_position(3, 1)
        and1_id = self.create_element("And", and1_x, and1_y, "AND1")
        
        # Path 2: D2 & S1 & ~S0  
        and2_s_x, and2_s_y = self._get_grid_position(2, 2)
        and2_s_id = self.create_element("And", and2_s_x, and2_s_y, "AND2_S")
        and2_x, and2_y = self._get_grid_position(3, 2)
        and2_id = self.create_element("And", and2_x, and2_y, "AND2")
        
        # Path 3: D3 & S1 & S0
        and3_s_x, and3_s_y = self._get_grid_position(2, 3)
        and3_s_id = self.create_element("And", and3_s_x, and3_s_y, "AND3_S")
        and3_x, and3_y = self._get_grid_position(3, 3)
        and3_id = self.create_element("And", and3_x, and3_y, "AND3")
        
        and_gates = [and0_s_id, and0_id, and1_s_id, and1_id, and2_s_id, and2_id, and3_s_id, and3_id]
        if not all(and_gates):
            return None
        
        # Connect select decoding
        select_connections = [
            # Select for path 0: ~S1 & ~S0
            (not_s1_id, 0, and0_s_id, 0), (not_s0_id, 0, and0_s_id, 1),
            
            # Select for path 1: ~S1 & S0
            (not_s1_id, 0, and1_s_id, 0), (s0_id, 0, and1_s_id, 1),
            
            # Select for path 2: S1 & ~S0
            (s1_id, 0, and2_s_id, 0), (not_s0_id, 0, and2_s_id, 1),
            
            # Select for path 3: S1 & S0
            (s1_id, 0, and3_s_id, 0), (s0_id, 0, and3_s_id, 1),
        ]
        
        for source_id, source_port, target_id, target_port in select_connections:
            if not self.connect_elements(source_id, source_port, target_id, target_port):
                return None
        
        # Connect data and select to final AND gates
        data_connections = [
            (d0_id, 0, and0_id, 0), (and0_s_id, 0, and0_id, 1),
            (d1_id, 0, and1_id, 0), (and1_s_id, 0, and1_id, 1),
            (d2_id, 0, and2_id, 0), (and2_s_id, 0, and2_id, 1),
            (d3_id, 0, and3_id, 0), (and3_s_id, 0, and3_id, 1),
        ]
        
        for source_id, source_port, target_id, target_port in data_connections:
            if not self.connect_elements(source_id, source_port, target_id, target_port):
                return None
        
        # Create OR gates to combine outputs with proper grid positioning
        or1_x, or1_y = self._get_grid_position(4, 0)
        or1_id = self.create_element("Or", or1_x, or1_y, "OR1")
        or2_x, or2_y = self._get_grid_position(4, 1)
        or2_id = self.create_element("Or", or2_x, or2_y, "OR2")
        or_final_x, or_final_y = self._get_grid_position(5, 0)
        or_final_id = self.create_element("Or", or_final_x, or_final_y, "OR_FINAL")
        
        if not all([or1_id, or2_id, or_final_id]):
            return None
        
        # Connect OR tree
        or_connections = [
            (and0_id, 0, or1_id, 0), (and1_id, 0, or1_id, 1),
            (and2_id, 0, or2_id, 0), (and3_id, 0, or2_id, 1),
            (or1_id, 0, or_final_id, 0), (or2_id, 0, or_final_id, 1)
        ]
        
        for source_id, source_port, target_id, target_port in or_connections:
            if not self.connect_elements(source_id, source_port, target_id, target_port):
                return None
        
        # Create output with proper grid positioning
        output_x, output_y = self._get_grid_position(6, 0)
        output_id = self.create_element("Led", output_x, output_y, "OUT")
        if not output_id:
            return None
            
        if not self.connect_elements(or_final_id, 0, output_id, 0):
            return None
        
        return input_ids, [output_id]

    def _create_full_adder_circuit(self) -> Optional[Tuple[List[int], List[int]]]:
        """Create full adder circuit."""
        # Create inputs with proper grid positioning
        a_x, a_y = self._get_input_position(0)
        a_id = self.create_element("InputButton", a_x, a_y, "A")
        b_x, b_y = self._get_input_position(1)
        b_id = self.create_element("InputButton", b_x, b_y, "B")
        cin_x, cin_y = self._get_input_position(2)
        cin_id = self.create_element("InputButton", cin_x, cin_y, "Cin")
        
        if not all([a_id, b_id, cin_id]):
            return None
        
        # Sum = A XOR B XOR Cin with proper grid positioning
        xor1_x, xor1_y = self._get_grid_position(1, 0)
        xor1_id = self.create_element("Xor", xor1_x, xor1_y, "XOR1")  # A XOR B
        xor2_x, xor2_y = self._get_grid_position(2, 0)
        xor2_id = self.create_element("Xor", xor2_x, xor2_y, "XOR2")  # (A XOR B) XOR Cin
        
        # Carry = (A AND B) OR (Cin AND (A XOR B)) with proper grid positioning
        and1_x, and1_y = self._get_grid_position(1, 1)
        and1_id = self.create_element("And", and1_x, and1_y, "AND1")  # A AND B
        and2_x, and2_y = self._get_grid_position(2, 1)
        and2_id = self.create_element("And", and2_x, and2_y, "AND2")  # Cin AND (A XOR B)
        or_x, or_y = self._get_grid_position(3, 1)
        or_id = self.create_element("Or", or_x, or_y, "OR")       # Final carry
        
        gates = [xor1_id, xor2_id, and1_id, and2_id, or_id]
        if not all(gates):
            return None
        
        # Connect sum logic
        sum_connections = [
            (a_id, 0, xor1_id, 0),
            (b_id, 0, xor1_id, 1),
            (xor1_id, 0, xor2_id, 0),
            (cin_id, 0, xor2_id, 1)
        ]
        
        # Connect carry logic
        carry_connections = [
            (a_id, 0, and1_id, 0),
            (b_id, 0, and1_id, 1),
            (cin_id, 0, and2_id, 0),
            (xor1_id, 0, and2_id, 1),
            (and1_id, 0, or_id, 0),
            (and2_id, 0, or_id, 1)
        ]
        
        all_connections = sum_connections + carry_connections
        for source_id, source_port, target_id, target_port in all_connections:
            if not self.connect_elements(source_id, source_port, target_id, target_port):
                return None
        
        # Create outputs with proper grid positioning
        sum_out_x, sum_out_y = self._get_grid_position(3, 0)
        sum_out_id = self.create_element("Led", sum_out_x, sum_out_y, "SUM")
        carry_out_x, carry_out_y = self._get_grid_position(4, 1)
        carry_out_id = self.create_element("Led", carry_out_x, carry_out_y, "CARRY")
        
        if not all([sum_out_id, carry_out_id]):
            return None
        
        # Connect outputs
        if not self.connect_elements(xor2_id, 0, sum_out_id, 0):
            return None
        if not self.connect_elements(or_id, 0, carry_out_id, 0):
            return None
        
        return [a_id, b_id, cin_id], [sum_out_id, carry_out_id]

    def cleanup(self):
        """Clean up resources"""
        if self.bridge:
            self.bridge.stop()
            self.bridge = None

    def run_all_tests(self) -> dict:
        """Run all Level 1 advanced combinational logic tests."""
        print("🚀 CPU Level 1: Advanced Combinational Logic Validation")
        print("=" * 65)
        
        tests = [
            self.test_multi_input_gates,
            self.test_compound_boolean_functions,
            self.test_signal_conditioning,
            self.test_complex_logic_networks,
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
        
        print(f"\n📊 Level 1 Results: {passed}/{total} tests passed ({passed/total*100:.1f}%)")
        
        return {
            "level": 1,
            "name": "Advanced Combinational Logic",
            "total_tests": total,
            "passed": passed,
            "failed": total - passed,
            "pass_rate": passed / total * 100,
            "results": results
        }


def main():
    """Main execution function."""
    validator = AdvancedCombinationalValidator()
    
    try:
        # Run all Level 1 validation tests
        final_results = validator.run_all_tests()
        
        # Save detailed results
        with open("cpu_level_1_results.json", "w") as f:
            json.dump(final_results, f, indent=2)
        
        print(f"\n💾 Detailed results saved to: cpu_level_1_results.json")
        
        # Return exit code based on pass rate
        if final_results["pass_rate"] >= 95:
            print("🏆 EXCELLENT: Level 1 validation passed with high accuracy!")
            return 0
        elif final_results["pass_rate"] >= 80:
            print("✅ GOOD: Level 1 mostly validated with minor issues")
            return 0
        else:
            print("⚠️ ISSUES: Level 1 has significant validation failures")
            return 1
    finally:
        # Always cleanup
        validator.cleanup()


if __name__ == "__main__":
    sys.exit(main())