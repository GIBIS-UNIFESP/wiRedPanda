#!/usr/bin/env python3
"""
Simple test to verify our iterative convergence implementation
"""

import sys
import os
sys.path.append('/workspace/mcp-server')

from wiredpanda_bridge import WiredPandaBridge
import time

def test_simple_sr_latch():
    """Test basic SR latch to see if convergence works"""
    print("🔧 Testing simple SR latch with iterative convergence...")
    
    bridge = WiredPandaBridge()
    bridge.start()
    
    try:
        # Create new circuit
        bridge.new_circuit()
        print("✅ Circuit created")
        
        # Create SR Latch elements
        s_id = bridge.create_element("InputButton", 100.0, 100.0, "S")
        r_id = bridge.create_element("InputButton", 100.0, 130.0, "R")
        nor1_id = bridge.create_element("Nor", 200.0, 100.0, "NOR1")
        nor2_id = bridge.create_element("Nor", 200.0, 130.0, "NOR2")
        q_id = bridge.create_element("Led", 300.0, 100.0, "Q")
        q_not_id = bridge.create_element("Led", 300.0, 130.0, "Q_NOT")
        
        print(f"✅ Elements created: S={s_id}, R={r_id}, NOR1={nor1_id}, NOR2={nor2_id}, Q={q_id}, Q_NOT={q_not_id}")
        
        # Connect SR Latch with cross-coupling
        print("Connecting cross-coupled SR latch...")
        bridge.connect_elements(r_id, 0, nor1_id, 0)      # R → NOR1
        bridge.connect_elements(s_id, 0, nor2_id, 0)      # S → NOR2
        bridge.connect_elements(nor2_id, 0, nor1_id, 1)   # Cross-couple Q̄ → NOR1
        bridge.connect_elements(nor1_id, 0, nor2_id, 1)   # Cross-couple Q → NOR2
        bridge.connect_elements(nor1_id, 0, q_id, 0)      # Q output
        bridge.connect_elements(nor2_id, 0, q_not_id, 0)  # Q̄ output
        print("✅ Cross-coupled connections completed")
        
        # Start simulation once and keep it running to maintain circuit state
        bridge.restart_simulation()
        
        # Test 1: Reset
        print("\n🧪 Test 1: Reset (R=1, S=0)")
        bridge.set_input_value(s_id, False)
        bridge.set_input_value(r_id, True)
        bridge.update_simulation()  # Update with new inputs - don't restart!
        
        q1 = bridge.get_output_value(q_id)
        q_not1 = bridge.get_output_value(q_not_id)
        print(f"   Result: Q={q1}, Q_NOT={q_not1}")
        print(f"   Expected: Q=False, Q_NOT=True")
        print(f"   Correct: {q1 == False and q_not1 == True}")
        
        # Test 2: Set  
        print("\n🧪 Test 2: Set (S=1, R=0)")
        bridge.set_input_value(s_id, True)
        bridge.set_input_value(r_id, False)
        bridge.update_simulation()  # Update with new inputs - circuit state preserved!
        
        q2 = bridge.get_output_value(q_id)
        q_not2 = bridge.get_output_value(q_not_id)
        print(f"   Result: Q={q2}, Q_NOT={q_not2}")
        print(f"   Expected: Q=True, Q_NOT=False")
        print(f"   Correct: {q2 == True and q_not2 == False}")
        
        # Test 3: Hold (the critical test) - this should now work!
        print("\n🧪 Test 3: Hold (S=0, R=0) - THE CRITICAL TEST")
        bridge.set_input_value(s_id, False)
        bridge.set_input_value(r_id, False)
        bridge.update_simulation()  # Update with hold inputs - state should be maintained!
        
        q3 = bridge.get_output_value(q_id)
        q_not3 = bridge.get_output_value(q_not_id)
        print(f"   Result: Q={q3}, Q_NOT={q_not3}")
        print(f"   Expected: Q=True, Q_NOT=False (should maintain set state)")
        print(f"   Correct: {q3 == True and q_not3 == False}")
        
        # Analysis
        hold_works = (q3 == True and q_not3 == False)
        complementary = (q3 != q_not3)
        
        print(f"\n📊 Analysis:")
        print(f"   Hold behavior works: {hold_works}")
        print(f"   Outputs complementary: {complementary}")
        
        if hold_works:
            print("🎉 SUCCESS: Iterative convergence is working!")
            return True
        else:
            print("❌ FAILURE: Cross-coupling limitation still present")
            return False
            
    except Exception as e:
        print(f"❌ Error: {e}")
        return False
    finally:
        bridge.stop()

if __name__ == "__main__":
    success = test_simple_sr_latch()
    sys.exit(0 if success else 1)