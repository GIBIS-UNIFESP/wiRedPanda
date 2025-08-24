#!/usr/bin/env python3

"""
Debug Johnson Counter - Simplified test to isolate the feedback issue
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

class JohnsonCounterDebugger(AdvancedSequentialValidator):
    
    def debug_simple_johnson_counter(self):
        """Debug Johnson Counter with detailed tracing"""
        logger.info("🔧 DEBUGGING: Simple Johnson Counter with step-by-step analysis")
        
        if not self.create_new_circuit():
            logger.error("Failed to create circuit")
            return False

        # Create clock and inputs
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        # Create 3 native D flip-flops without reset to eliminate reset interference
        logger.info("Creating 3 D flip-flops WITHOUT reset to isolate feedback issue")
        
        ff0_x, ff0_y = self._get_grid_position(2, 0)
        ff0_id = self.create_element("DFlipFlop", ff0_x, ff0_y, "FF0")
        
        ff1_x, ff1_y = self._get_grid_position(2, 8)
        ff1_id = self.create_element("DFlipFlop", ff1_x, ff1_y, "FF1")
        
        ff2_x, ff2_y = self._get_grid_position(2, 16)
        ff2_id = self.create_element("DFlipFlop", ff2_x, ff2_y, "FF2")
        
        if not all([ff0_id, ff1_id, ff2_id]):
            logger.error("Failed to create flip-flops")
            return False
            
        # Connect clocks
        logger.info("Connecting clocks to all flip-flops")
        if not self.connect_elements(clk_id, 0, ff0_id, 1):  # CLK -> FF0 CLK
            logger.error("Failed to connect clock to FF0")
            return False
        if not self.connect_elements(clk_id, 0, ff1_id, 1):  # CLK -> FF1 CLK  
            logger.error("Failed to connect clock to FF1")
            return False
        if not self.connect_elements(clk_id, 0, ff2_id, 1):  # CLK -> FF2 CLK
            logger.error("Failed to connect clock to FF2")
            return False
            
        # Create output LEDs FIRST before making feedback connections
        out0_x, out0_y = self._get_output_position(8, 0)
        out0_led = self.create_element("Led", out0_x, out0_y, "Q0")
        out1_x, out1_y = self._get_output_position(8, 1) 
        out1_led = self.create_element("Led", out1_x, out1_y, "Q1")
        out2_x, out2_y = self._get_output_position(8, 2)
        out2_led = self.create_element("Led", out2_x, out2_y, "Q2")
        
        # Connect outputs to LEDs
        if not self.connect_elements(ff0_id, 0, out0_led, 0):
            logger.error("Failed to connect FF0 output")
            return False
        if not self.connect_elements(ff1_id, 0, out1_led, 0):
            logger.error("Failed to connect FF1 output") 
            return False
        if not self.connect_elements(ff2_id, 0, out2_led, 0):
            logger.error("Failed to connect FF2 output")
            return False
            
        # Test initial state (should be all False)
        self.set_input(clk_id, False)
        self.update_simulation()
        self.update_simulation()
        
        initial_q0 = self.get_output(out0_led)
        initial_q1 = self.get_output(out1_led)
        initial_q2 = self.get_output(out2_led)
        
        logger.info(f"INITIAL STATE: Q2={initial_q2}, Q1={initial_q1}, Q0={initial_q0}")
        
        # Make Johnson Counter feedback connections step by step
        logger.info("🔧 Making Johnson Counter feedback connections:")
        logger.info("  D0 = ~Q2 (inverted feedback)")
        logger.info("  D1 = Q0 (shift right)")  
        logger.info("  D2 = Q1 (shift right)")
        
        # Connection 1: FF2 ~Q output -> FF0 D input (inverted feedback)
        logger.info("Connecting FF2 ~Q (port 1) -> FF0 D (port 0)")
        if not self.connect_elements(ff2_id, 1, ff0_id, 0):
            logger.error("Failed to connect FF2 ~Q -> FF0 D")
            return False
            
        # Connection 2: FF0 Q output -> FF1 D input (shift)
        logger.info("Connecting FF0 Q (port 0) -> FF1 D (port 0)")
        if not self.connect_elements(ff0_id, 0, ff1_id, 0):
            logger.error("Failed to connect FF0 Q -> FF1 D")
            return False
            
        # Connection 3: FF1 Q output -> FF2 D input (shift)
        logger.info("Connecting FF1 Q (port 0) -> FF2 D (port 0)")  
        if not self.connect_elements(ff1_id, 0, ff2_id, 0):
            logger.error("Failed to connect FF1 Q -> FF2 D")
            return False
            
        logger.info("✅ All Johnson Counter connections completed")
        
        # Test after connections but before clocking
        self.update_simulation()
        
        conn_q0 = self.get_output(out0_led)
        conn_q1 = self.get_output(out1_led) 
        conn_q2 = self.get_output(out2_led)
        
        logger.info(f"AFTER CONNECTIONS: Q2={conn_q2}, Q1={conn_q1}, Q0={conn_q0}")
        
        # Apply first clock pulse and analyze step by step
        logger.info("🕐 Applying first clock pulse...")
        logger.info("  Expected logic: D0 = ~Q2 = ~False = True, D1 = Q0 = False, D2 = Q1 = False")
        logger.info("  Expected result: Q0 = True, Q1 = False, Q2 = False")
        
        self.set_input(clk_id, True)
        time.sleep(0.1)
        self.update_simulation()
        
        mid_q0 = self.get_output(out0_led)
        mid_q1 = self.get_output(out1_led)
        mid_q2 = self.get_output(out2_led)
        
        logger.info(f"DURING CLOCK HIGH: Q2={mid_q2}, Q1={mid_q1}, Q0={mid_q0}")
        
        self.set_input(clk_id, False)
        time.sleep(0.1) 
        self.update_simulation()
        
        after_q0 = self.get_output(out0_led)
        after_q1 = self.get_output(out1_led)
        after_q2 = self.get_output(out2_led)
        
        logger.info(f"AFTER CLOCK PULSE 1: Q2={after_q2}, Q1={after_q1}, Q0={after_q0}")
        
        # Check if behavior is correct
        expected = [True, False, False]
        actual = [after_q0, after_q1, after_q2]
        
        if actual == expected:
            logger.info("✅ SUCCESS: Johnson Counter step 1 working correctly!")
            return True
        else:
            logger.error(f"❌ FAILED: Expected {expected}, got {actual}")
            
            # Diagnose the issue
            if after_q0 == after_q1 == after_q2:
                logger.error("DIAGNOSIS: All outputs identical - suggests all D inputs getting same signal")
                logger.error("POSSIBLE CAUSES: Connection interference, reset logic, or MCP connection bug")
            
            return False

def main():
    debugger = JohnsonCounterDebugger()
    
    try:
        success = debugger.debug_simple_johnson_counter()
        if success:
            print("🎉 Johnson Counter debug test PASSED!")
        else:
            print("💥 Johnson Counter debug test FAILED!")
            
    except Exception as e:
        logger.error(f"Exception during debug: {e}")
        print(f"💥 Exception: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()