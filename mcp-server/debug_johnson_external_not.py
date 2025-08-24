#!/usr/bin/env python3

"""
Debug Johnson Counter with External NOT - Test port-level dependency hypothesis
Focus: Use only port 0 (Q) outputs with external NOT gate for inversion
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

class JohnsonExternalNotDebugger(AdvancedSequentialValidator):
    
    def debug_johnson_external_not(self):
        """Test Johnson Counter using external NOT gate to avoid port 1 (~Q)"""
        logger.info("🔧 JOHNSON COUNTER WITH EXTERNAL NOT DEBUG")
        logger.info("Hypothesis: Cycle detection fails because port 0 ≠ port 1 in dependency tracking")
        logger.info("Solution: Use only port 0 (Q outputs) with external NOT gate")
        
        if not self.create_new_circuit():
            return False

        # Create inputs
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        reset_x, reset_y = self._get_input_position(1)
        reset_id = self.create_element("InputButton", reset_x, reset_y, "RESET")
        
        # Create 3 D flip-flops
        logger.info("🔧 Creating 3 D flip-flops")
        ff0_x, ff0_y = self._get_grid_position(3, 1)
        ff0_id = self.create_element("DFlipFlop", ff0_x, ff0_y, "JFF0")
        
        ff1_x, ff1_y = self._get_grid_position(3, 3)
        ff1_id = self.create_element("DFlipFlop", ff1_x, ff1_y, "JFF1")
        
        ff2_x, ff2_y = self._get_grid_position(3, 5)
        ff2_id = self.create_element("DFlipFlop", ff2_x, ff2_y, "JFF2")
        
        # Create external NOT gate for inversion
        logger.info("🔧 Creating external NOT gate for inversion")
        not_x, not_y = self._get_grid_position(5, 5)
        not_id = self.create_element("Not", not_x, not_y, "INVERT")
        
        # Connect clocks and resets
        self.connect_elements(clk_id, 0, ff0_id, 1)  # CLK -> FF0
        self.connect_elements(clk_id, 0, ff1_id, 1)  # CLK -> FF1
        self.connect_elements(clk_id, 0, ff2_id, 1)  # CLK -> FF2
        
        self.connect_elements(reset_id, 0, ff0_id, 2)  # RESET -> FF0
        self.connect_elements(reset_id, 0, ff1_id, 2)  # RESET -> FF1
        self.connect_elements(reset_id, 0, ff2_id, 2)  # RESET -> FF2
        
        # Create output LEDs
        out0_x, out0_y = self._get_output_position(10, 0)
        out0_led = self.create_element("Led", out0_x, out0_y, "Q0")
        out1_x, out1_y = self._get_output_position(10, 1)
        out1_led = self.create_element("Led", out1_x, out1_y, "Q1")
        out2_x, out2_y = self._get_output_position(10, 2)
        out2_led = self.create_element("Led", out2_x, out2_y, "Q2")
        
        # Connect outputs to LEDs
        self.connect_elements(ff0_id, 0, out0_led, 0)  # FF0 Q -> LED0
        self.connect_elements(ff1_id, 0, out1_led, 0)  # FF1 Q -> LED1
        self.connect_elements(ff2_id, 0, out2_led, 0)  # FF2 Q -> LED2
        
        logger.info("🔧 Creating Johnson Counter with ONLY port 0 connections")
        logger.info("Johnson Counter Logic (modified):")
        logger.info("  FF0 D = NOT(FF2 Q)  (external NOT gate)")
        logger.info("  FF1 D = FF0 Q       (direct connection)")
        logger.info("  FF2 D = FF1 Q       (direct connection)")
        
        # Create feedback using ONLY port 0 (Q outputs)
        logger.info("Step 1: FF0 Q -> FF1 D (port 0 to port 0)")
        if not self.connect_elements(ff0_id, 0, ff1_id, 0):
            logger.error("Failed to connect FF0 -> FF1")
            return False
            
        logger.info("Step 2: FF1 Q -> FF2 D (port 0 to port 0)")
        if not self.connect_elements(ff1_id, 0, ff2_id, 0):
            logger.error("Failed to connect FF1 -> FF2")
            return False
            
        logger.info("Step 3: FF2 Q -> NOT gate (port 0 to port 0)")
        if not self.connect_elements(ff2_id, 0, not_id, 0):
            logger.error("Failed to connect FF2 -> NOT")
            return False
            
        logger.info("Step 4: NOT output -> FF0 D (port 0 to port 0)")
        if not self.connect_elements(not_id, 0, ff0_id, 0):
            logger.error("Failed to connect NOT -> FF0")
            return False
        
        logger.info("✅ Johnson Counter with external NOT created!")
        logger.info("Expected: 4-element feedback group (FF0 → FF1 → FF2 → NOT → FF0)")
        
        # Initialize with reset
        logger.info("🔧 Initialize with reset")
        self.set_input(reset_id, True)
        self.set_input(clk_id, False)
        self.update_simulation()
        self.update_simulation()
        self.set_input(reset_id, False)
        self.update_simulation()
        
        # Check initial state
        initial_q0 = self.get_output(out0_led)
        initial_q1 = self.get_output(out1_led)
        initial_q2 = self.get_output(out2_led)
        
        logger.info(f"After reset: Q2={initial_q2}, Q1={initial_q1}, Q0={initial_q0}")
        
        # Test a few clock cycles
        logger.info("🔧 Testing with external NOT gate")
        for cycle in range(7):
            if cycle > 0:
                logger.info(f"🕐 Clock pulse {cycle}")
                self.pulse_clock(clk_id, 0.15)
                
            q0 = self.get_output(out0_led)
            q1 = self.get_output(out1_led)
            q2 = self.get_output(out2_led)
            
            logger.info(f"Cycle {cycle}: Q2={q2}, Q1={q1}, Q0={q0}")
        
        # Save circuit for inspection
        self.save_circuit("debug_johnson_external_not.panda")
        logger.info("💾 Debug circuit saved as: debug_johnson_external_not.panda")
        
        logger.info("✅ Check debug logs above for cycle detection results")
        logger.info("Expected: 'CYCLE DETECTED! Path size: 4' for 4-element cycle")
        
        return True

def main():
    debugger = JohnsonExternalNotDebugger()
    
    try:
        success = debugger.debug_johnson_external_not()
        
        if success:
            print("🎉 Johnson Counter with external NOT debug completed!")
            print("   Check logs for cycle detection - should show 4-element feedback group")
        else:
            print("💥 Johnson Counter with external NOT debug FAILED!")
            
    except Exception as e:
        logger.error(f"Exception during debug: {e}")
        print(f"💥 Exception: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()