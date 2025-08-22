#!/usr/bin/env python3
"""
Example: Create a simple AND gate circuit using the real wiRedPanda MCP

This example demonstrates how to:
1. Create circuit elements (inputs, logic gate, output)
2. Connect elements together
3. Test the circuit with different input combinations
4. Save the circuit to a file

Updated to use real wiRedPanda MCP integration.
"""

import sys
import time
from pathlib import Path

# Add the src directory to the path
sys.path.append(str(Path(__file__).parent.parent / "src"))
from wiredpanda_bridge import WiredPandaBridge, WiredPandaError

def main():
    print("🔧 Creating Simple AND Gate Circuit with Real wiRedPanda")
    print("=" * 55)
    
    try:
        # Create bridge and start wiRedPanda
        with WiredPandaBridge() as bridge:
            
            # Step 1: Create input elements
            print("\n1. Creating input elements...")
            input1_id = bridge.create_input_button(50, 100, "Input1")
            input2_id = bridge.create_input_button(50, 150, "Input2")
            print(f"   Created Input1: ID {input1_id}")
            print(f"   Created Input2: ID {input2_id}")
            
            # Step 2: Create AND gate
            print("\n2. Creating AND gate...")
            and_gate_id = bridge.create_and_gate(150, 125, "AndGate")
            print(f"   Created AND gate: ID {and_gate_id}")
            
            # Step 3: Create output LED
            print("\n3. Creating output LED...")
            output_id = bridge.create_led(250, 125, "Output")
            print(f"   Created Output LED: ID {output_id}")
            
            # Step 4: Connect elements
            print("\n4. Connecting elements...")
            
            # Connect Input1 to AND gate input 0
            bridge.connect_elements(input1_id, 0, and_gate_id, 0)
            print(f"   Connected Input1 → AND gate")
            
            # Connect Input2 to AND gate input 1
            bridge.connect_elements(input2_id, 0, and_gate_id, 1)
            print(f"   Connected Input2 → AND gate")
            
            # Connect AND gate output to LED
            bridge.connect_elements(and_gate_id, 0, output_id, 0)
            print(f"   Connected AND gate → Output LED")
            
            # Step 5: Verify circuit structure
            print("\n5. Verifying circuit structure...")
            elements = bridge.list_elements()
            print(f"   Total elements: {len(elements)}")
            
            for elem in elements:
                print(f"   - {elem['type']} (ID: {elem['id']}) at ({elem['position']['x']}, {elem['position']['y']})")
            
            # Step 6: Test the circuit
            print("\n6. Testing AND gate logic...")
            
            # Start simulation
            bridge.start_simulation()
            print("   Simulation started")
            
            # Test all combinations
            test_cases = [
                (False, False, False),
                (False, True, False),
                (True, False, False),
                (True, True, True)
            ]
            
            print("\n   AND Gate Truth Table:")
            print("   Input1 | Input2 | Output | Status")
            print("   -------|--------|--------|-------")
            
            all_passed = True
            for in1, in2, expected in test_cases:
                # Set input values
                bridge.set_input_value(input1_id, in1)
                bridge.set_input_value(input2_id, in2)
                
                # Restart simulation to ensure proper propagation
                bridge.restart_simulation()
                time.sleep(0.1)
                
                # Read output
                actual = bridge.get_output_value(output_id)
                status = "✓ PASS" if actual == expected else "✗ FAIL"
                
                if actual != expected:
                    all_passed = False
                
                print(f"   {str(in1):6} | {str(in2):6} | {str(actual):6} | {status}")
            
            # Stop simulation
            bridge.stop_simulation()
            print(f"\n   Simulation stopped")
            
            # Step 7: Save the circuit
            print("\n7. Saving circuit...")
            bridge.save_circuit("simple_and_gate_real.panda")
            print("   Circuit saved as: simple_and_gate_real.panda")
            
            # Final results
            print(f"\n🎉 Circuit creation completed successfully!")
            print(f"Logic test result: {'✅ ALL PASSED' if all_passed else '❌ SOME FAILED'}")
            print("Files created:")
            print("  - simple_and_gate_real.panda (real wiRedPanda circuit file)")
            
            return 0 if all_passed else 1
            
    except WiredPandaError as e:
        print(f"❌ Error: {e}")
        return 1
    except Exception as e:
        print(f"❌ Unexpected error: {e}")
        return 1

if __name__ == "__main__":
    exit(main())