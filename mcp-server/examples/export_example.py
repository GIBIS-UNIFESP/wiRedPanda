#!/usr/bin/env python3
"""
Example: Using wiRedPanda MCP Bridge Export Functionality

This example demonstrates how to create circuits and export them to various image formats.
"""

import sys
import os
from wiredpanda_bridge import WiredPandaBridge

def main():
    """Create circuits and export them to different formats"""
    
    print("wiRedPanda Circuit Export Example")
    print("=================================")
    
    # Initialize bridge in headless mode for automated export
    with WiredPandaBridge(headless=True) as bridge:
        
        # Example 1: Simple AND gate circuit
        print("\n1. Creating AND gate circuit...")
        
        # Create inputs
        a = bridge.create_input_button(50, 100, "A")
        b = bridge.create_input_button(50, 200, "B")
        
        # Create AND gate
        and_gate = bridge.create_and_gate(200, 150, "AND")
        
        # Create output LED
        output = bridge.create_led(350, 150, "Output")
        
        # Connect the circuit
        bridge.connect_elements(a, 0, and_gate, 0)
        bridge.connect_elements(b, 0, and_gate, 1)
        bridge.connect_elements(and_gate, 0, output, 0)
        
        # Export to different formats
        bridge.export_png("and_gate_circuit.png")
        bridge.export_pdf("and_gate_circuit.pdf") 
        bridge.export_svg("and_gate_circuit.svg")
        print("✓ AND gate circuit exported to PNG, PDF, and SVG")
        
        # Example 2: More complex circuit
        print("\n2. Creating XOR gate circuit...")
        bridge.new_circuit()  # Start fresh
        
        # Create XOR using basic gates: A⊕B = (A∧¬B) ∨ (¬A∧B)
        input_a = bridge.create_input_button(50, 150, "A")
        input_b = bridge.create_input_button(50, 250, "B")
        
        # Create NOT gates
        not_a = bridge.create_not_gate(150, 100, "¬A")
        not_b = bridge.create_not_gate(150, 300, "¬B")
        
        # Create AND gates
        and1 = bridge.create_and_gate(250, 125, "A∧¬B")  # A AND NOT B
        and2 = bridge.create_and_gate(250, 275, "¬A∧B")  # NOT A AND B
        
        # Create OR gate
        or_gate = bridge.create_or_gate(400, 200, "XOR")
        
        # Create output LED
        xor_output = bridge.create_led(550, 200, "A⊕B")
        
        # Wire up the circuit
        bridge.connect_elements(input_a, 0, not_a, 0)     # A -> NOT A
        bridge.connect_elements(input_b, 0, not_b, 0)     # B -> NOT B
        bridge.connect_elements(input_a, 0, and1, 0)      # A -> AND1
        bridge.connect_elements(not_b, 0, and1, 1)        # NOT B -> AND1
        bridge.connect_elements(not_a, 0, and2, 0)        # NOT A -> AND2
        bridge.connect_elements(input_b, 0, and2, 1)      # B -> AND2
        bridge.connect_elements(and1, 0, or_gate, 0)      # AND1 -> OR
        bridge.connect_elements(and2, 0, or_gate, 1)      # AND2 -> OR
        bridge.connect_elements(or_gate, 0, xor_output, 0) # OR -> Output
        
        # Export the complex circuit
        bridge.export_png("xor_gate_circuit.png")
        print("✓ XOR gate circuit exported to PNG")
        
        # Example 3: Test circuit with simulation
        print("\n3. Creating and testing circuit...")
        bridge.new_circuit()
        
        # Build a basic gate circuit using the convenience method
        inputs, gate, led = bridge.build_basic_gate_circuit("Or", 3)  # 3-input OR gate
        
        # Test truth table for 3-input OR gate
        truth_table = [
            ([False, False, False], False),
            ([False, False, True], True),
            ([False, True, False], True),
            ([False, True, True], True),
            ([True, False, False], True),
            ([True, False, True], True),
            ([True, True, False], True),
            ([True, True, True], True),
        ]
        
        results = bridge.test_truth_table(inputs, led, truth_table)
        print(f"✓ Truth table test: {results['passed']}/{results['total_tests']} passed")
        
        # Export the tested circuit
        bridge.export_png("tested_or_gate.png")
        print("✓ Tested OR gate circuit exported to PNG")
        
    print("\n✓ All exports completed successfully!")
    print("\nExported files:")
    for filename in ["and_gate_circuit.png", "and_gate_circuit.pdf", "and_gate_circuit.svg",
                     "xor_gate_circuit.png", "tested_or_gate.png"]:
        if os.path.exists(filename):
            size = os.path.getsize(filename)
            print(f"  {filename} ({size:,} bytes)")

if __name__ == "__main__":
    main()