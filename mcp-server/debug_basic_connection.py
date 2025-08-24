#!/usr/bin/env python3

"""
Debug Basic Connection - Test if MCP connections work at all
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

class ConnectionDebugger(AdvancedSequentialValidator):
    
    def debug_basic_connection(self):
        """Test if basic flip-flop connections work"""
        logger.info("🔧 DEBUGGING: Basic D flip-flop connection test")
        
        if not self.create_new_circuit():
            logger.error("Failed to create circuit")
            return False

        # Create clock and reset (matching Moore machine pattern)
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        # Create data input
        data_x, data_y = self._get_input_position(1)
        data_id = self.create_element("InputButton", data_x, data_y, "DATA")
        
        reset_x, reset_y = self._get_input_position(2)
        reset_id = self.create_element("InputButton", reset_x, reset_y, "RESET")
        
        # Create D flip-flop using the SAME method as working Moore machine
        logger.info("Creating D flip-flop using _create_simple_d_flip_flop_native (same as Moore machine)")
        ff_components = self._create_simple_d_flip_flop_native(1, 0, "TEST_FF", clk_id, reset_id)
        if not all(ff_components[:3]):
            logger.error("Failed to create D flip-flop using helper function")
            return False
        ff_d_id, ff_q_id, ff_q_not_id = ff_components[:3]
        
        logger.info(f"Created FF components: D={ff_d_id}, Q={ff_q_id}, Q_NOT={ff_q_not_id}")
        
        # Initialize with reset (exact same sequence as Moore machine)
        logger.info("Initializing with reset sequence (matching Moore machine)")
        self.set_input(reset_id, True)  # Apply reset
        self.set_input(clk_id, False)   # Clock low during reset
        self.update_simulation()
        self.update_simulation()
        self.set_input(reset_id, False) # Release reset
        self.update_simulation()
        
        # Create output LED
        out_x, out_y = self._get_output_position(8, 0)
        out_led = self.create_element("Led", out_x, out_y, "Q")
        
        if not all([clk_id, data_id, out_led]):
            logger.error("Failed to create elements")
            return False
        
        logger.info(f"Created elements: CLK={clk_id}, DATA={data_id}, FF_Q={ff_q_id}, LED={out_led}")
        
        # Make connections (DATA -> FF D input, FF Q -> LED)
        logger.info("Connecting DATA -> FF D input")
        if not self.connect_elements(data_id, 0, ff_d_id, 0):
            logger.error("Failed to connect DATA -> FF D")
            return False
            
        logger.info("Connecting FF Q output -> LED")
        if not self.connect_elements(ff_q_id, 0, out_led, 0):
            logger.error("Failed to connect FF Q -> LED")
            return False
            
        logger.info("✅ All basic connections completed")
        
        # Test sequence: DATA=0, CLK=0 -> should initialize to Q=0
        logger.info("Initial test: DATA=0, CLK=0")
        self.set_input(data_id, False)
        self.set_input(clk_id, False)
        self.update_simulation()
        
        initial_q = self.get_output(out_led)
        logger.info(f"Initial state: Q={initial_q}")
        
        # Test 1: DATA=1, CLK=0 -> Q should remain unchanged
        logger.info("Test 1: DATA=1, CLK=0 (no clock edge)")
        self.set_input(data_id, True)
        self.update_simulation()
        
        no_clock_q = self.get_output(out_led)
        logger.info(f"After DATA=1, CLK=0: Q={no_clock_q}")
        
        # Test 2: DATA=1, CLK=0->1 (rising edge) -> Q should become 1
        logger.info("Test 2: DATA=1, CLK=0->1 (rising edge)")
        self.set_input(clk_id, True)
        time.sleep(0.1)
        self.update_simulation()
        
        rising_edge_q = self.get_output(out_led)
        logger.info(f"After rising edge with DATA=1: Q={rising_edge_q}")
        
        # Test 3: DATA=0, CLK=1->0->1 (another rising edge) -> Q should become 0
        logger.info("Test 3: DATA=0, CLK=1->0->1 (rising edge with DATA=0)")
        self.set_input(data_id, False)
        self.set_input(clk_id, False)
        time.sleep(0.1)
        self.update_simulation()
        
        self.set_input(clk_id, True)
        time.sleep(0.1)
        self.update_simulation()
        
        second_edge_q = self.get_output(out_led)
        logger.info(f"After second rising edge with DATA=0: Q={second_edge_q}")
        
        # Analyze results
        logger.info("🔍 ANALYSIS:")
        logger.info(f"  Initial: Q={initial_q}")
        logger.info(f"  After DATA=1, no clock: Q={no_clock_q} (should be same as initial)")
        logger.info(f"  After rising edge with DATA=1: Q={rising_edge_q} (should be True)")
        logger.info(f"  After rising edge with DATA=0: Q={second_edge_q} (should be False)")
        
        # Check if basic D flip-flop behavior works
        success = True
        if no_clock_q != initial_q:
            logger.error("❌ FAILED: Q changed without clock edge")
            success = False
        
        if rising_edge_q != True:
            logger.error("❌ FAILED: Q did not become True after rising edge with DATA=1")
            success = False
            
        if second_edge_q != False:
            logger.error("❌ FAILED: Q did not become False after rising edge with DATA=0")  
            success = False
            
        if success:
            logger.info("✅ SUCCESS: Basic D flip-flop connections working correctly!")
        else:
            logger.error("❌ FAILED: Basic D flip-flop connections not working")
            
        return success

def main():
    debugger = ConnectionDebugger()
    
    try:
        success = debugger.debug_basic_connection()
        if success:
            print("🎉 Basic connection debug test PASSED!")
        else:
            print("💥 Basic connection debug test FAILED!")
            
    except Exception as e:
        logger.error(f"Exception during debug: {e}")
        print(f"💥 Exception: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()