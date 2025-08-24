#!/usr/bin/env python3
"""Test FullAdder creation with debug output"""

import sys
import os
sys.path.append('.')

from cpu_level_5_integrated_cpu_components import IntegratedCPUValidator

def test_with_debug():
    validator = IntegratedCPUValidator()
    
    try:
        print("=== Testing FullAdder creation with debug output ===")
        validator._ensure_bridge()
        validator.bridge.new_circuit()
        
        print("\n1. Testing And gate (should work)...")
        and_id = validator.create_element("And", 100.0, 100.0, "TEST_AND")
        print(f"And result: {and_id}")
        
        print("\n2. Testing FullAdder (should fail gracefully)...")
        fa_id = validator.create_element("FullAdder", 200.0, 100.0, "TEST_FA")
        print(f"FullAdder result: {fa_id}")
        
        if fa_id:
            print("❌ FullAdder creation succeeded - this should not happen!")
        else:
            print("✅ FullAdder creation failed as expected - checking if error was returned properly")
            
    except Exception as e:
        print(f"Exception occurred: {e}")
        import traceback
        traceback.print_exc()
    finally:
        validator.cleanup_bridge()

if __name__ == "__main__":
    test_with_debug()