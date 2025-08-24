#!/usr/bin/env python3

"""
Debug Simple Gate Cycle - Test cycle detection with atomic logic gates
Focus: Verify cycle detection works with basic gates (NOT, AND, OR)
"""

import sys
import os
import time
import logging

# Add the mcp-server directory to the Python path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from cpu_level_4_advanced_sequential import AdvancedSequentialValidator

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class SimpleGateCycleDebugger(AdvancedSequentialValidator):
    
    def debug_simple_gate_cycle(self):
        """Test cycle detection with basic atomic logic gates"""
        logger.info("🔧 SIMPLE GATE CYCLE DEBUG")
        logger.info("Testing cycle detection with atomic logic gates (NOT gates)")
        logger.info("This should work if the algorithm is correct for atomic elements")
        
        if not self.create_new_circuit():
            return False

        # Create a simple cycle: NOT A → NOT B → NOT C → NOT A
        logger.info("🔧 Creating 3 NOT gates")
        not_a_x, not_a_y = self._get_grid_position(3, 1)
        not_a_id = self.create_element("Not", not_a_x, not_a_y, "NOT_A")
        
        not_b_x, not_b_y = self._get_grid_position(5, 1)
        not_b_id = self.create_element("Not", not_b_x, not_b_y, "NOT_B")
        
        not_c_x, not_c_y = self._get_grid_position(7, 1)
        not_c_id = self.create_element("Not", not_c_x, not_c_y, "NOT_C")
        
        logger.info("🔧 Creating simple gate cycle: NOT_A → NOT_B → NOT_C → NOT_A")
        
        # Create the cycle
        if not self.connect_elements(not_a_id, 0, not_b_id, 0):
            logger.error("Failed to connect NOT_A → NOT_B")
            return False
            
        if not self.connect_elements(not_b_id, 0, not_c_id, 0):
            logger.error("Failed to connect NOT_B → NOT_C")
            return False
            
        if not self.connect_elements(not_c_id, 0, not_a_id, 0):
            logger.error("Failed to connect NOT_C → NOT_A")
            return False
        
        logger.info("✅ Simple gate cycle created!")
        logger.info("Expected: 'CYCLE DETECTED! Path size: 3' for 3-element NOT gate cycle")
        
        # Create LEDs to monitor outputs
        led_a_x, led_a_y = self._get_output_position(10, 0)
        led_a = self.create_element("Led", led_a_x, led_a_y, "LED_A")
        led_b_x, led_b_y = self._get_output_position(10, 1)
        led_b = self.create_element("Led", led_b_x, led_b_y, "LED_B")
        led_c_x, led_c_y = self._get_output_position(10, 2)
        led_c = self.create_element("Led", led_c_x, led_c_y, "LED_C")
        
        # Connect to LEDs for monitoring
        self.connect_elements(not_a_id, 0, led_a, 0)
        self.connect_elements(not_b_id, 0, led_b, 0)
        self.connect_elements(not_c_id, 0, led_c, 0)
        
        # Test the cycle behavior
        logger.info("🔧 Testing gate cycle behavior")
        for i in range(5):
            a = self.get_output(led_a)
            b = self.get_output(led_b)
            c = self.get_output(led_c)
            logger.info(f"Cycle {i}: A={a}, B={b}, C={c}")
            
            self.update_simulation()
            time.sleep(0.1)
        
        # Save circuit for inspection
        self.save_circuit("debug_simple_gate_cycle.panda")
        logger.info("💾 Debug circuit saved as: debug_simple_gate_cycle.panda")
        
        logger.info("✅ Check debug logs above for cycle detection results")
        logger.info("If NO 'CYCLE DETECTED' message: confirms DFlipFlop composite element issue")
        logger.info("If YES 'CYCLE DETECTED' message: proves algorithm works with atomic elements")
        
        return True

def main():
    debugger = SimpleGateCycleDebugger()
    
    try:
        success = debugger.debug_simple_gate_cycle()
        
        if success:
            print("🎉 Simple gate cycle debug completed!")
            print("   Check logs for cycle detection results")
        else:
            print("💥 Simple gate cycle debug FAILED!")
            
    except Exception as e:
        logger.error(f"Exception during debug: {e}")
        print(f"💥 Exception: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()