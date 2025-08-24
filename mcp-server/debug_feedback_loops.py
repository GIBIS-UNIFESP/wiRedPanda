#!/usr/bin/env python3

"""
Debug Feedback Loops - Test if the issue is with feedback connections, not D flip-flops
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

class FeedbackLoopDebugger(AdvancedSequentialValidator):
    
    def test_simple_forward_connection(self):
        """Test 1: Simple forward connection (like Moore machine) - should work"""
        logger.info("🔧 TEST 1: Simple forward connection (INPUT -> DFF -> LED)")
        
        if not self.create_new_circuit():
            return False

        # Create input and clock
        input_x, input_y = self._get_input_position(0)
        input_id = self.create_element("InputButton", input_x, input_y, "INPUT")
        clk_x, clk_y = self._get_input_position(1)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        reset_x, reset_y = self._get_input_position(2)
        reset_id = self.create_element("InputButton", reset_x, reset_y, "RESET")
        
        # Create D flip-flop using same method as Moore machine
        ff_components = self._create_simple_d_flip_flop_native(1, 0, "FF", clk_id, reset_id)
        if not all(ff_components[:3]):
            logger.error("Failed to create flip-flop")
            return False
        ff_d_id, ff_q_id, ff_q_not_id = ff_components[:3]
        
        # Initialize with reset (same as Moore machine)
        self.set_input(reset_id, True)
        self.set_input(clk_id, False)
        self.update_simulation()
        self.update_simulation()
        self.set_input(reset_id, False)
        self.update_simulation()
        
        # Create output LED
        out_x, out_y = self._get_output_position(4, 0)
        out_led = self.create_element("Led", out_x, out_y, "OUTPUT")
        
        # Forward connections (no feedback loop)
        self.connect_elements(input_id, 0, ff_d_id, 0)  # INPUT -> D
        self.connect_elements(ff_q_id, 0, out_led, 0)   # Q -> LED
        
        logger.info("Testing INPUT=1, apply clock pulse")
        self.set_input(input_id, True)
        self.set_input(clk_id, False)
        self.update_simulation()
        
        initial_q = self.get_output(out_led)
        logger.info(f"Before clock: Q={initial_q}")
        
        # Apply clock pulse
        self.pulse_clock(clk_id, 0.1)
        
        final_q = self.get_output(out_led)
        logger.info(f"After clock: Q={final_q}")
        
        success = (initial_q == False) and (final_q == True)
        if success:
            logger.info("✅ TEST 1 PASSED: Simple forward connection works")
        else:
            logger.error("❌ TEST 1 FAILED: Even forward connection broken")
            
        return success
    
    def test_simple_feedback_loop(self):
        """Test 2: Simple 2-flip-flop feedback loop - may fail due to race conditions"""
        logger.info("🔧 TEST 2: Simple 2-flip-flop feedback loop")
        
        if not self.create_new_circuit():
            return False

        # Create clock only (no external inputs for this test)
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        reset_x, reset_y = self._get_input_position(1)
        reset_id = self.create_element("InputButton", reset_x, reset_y, "RESET")
        
        # Create two D flip-flops
        ff0_components = self._create_simple_d_flip_flop_native(1, 0, "FF0", clk_id, reset_id)
        if not all(ff0_components[:3]):
            return False
        ff0_d, ff0_q, ff0_qn = ff0_components[:3]
        
        ff1_components = self._create_simple_d_flip_flop_native(1, 8, "FF1", clk_id, reset_id)
        if not all(ff1_components[:3]):
            return False
        ff1_d, ff1_q, ff1_qn = ff1_components[:3]
        
        # Initialize with reset
        self.set_input(reset_id, True)
        self.set_input(clk_id, False)
        self.update_simulation()
        self.update_simulation()
        self.set_input(reset_id, False)
        self.update_simulation()
        
        # Create output LEDs
        out0_x, out0_y = self._get_output_position(4, 0)
        out0_led = self.create_element("Led", out0_x, out0_y, "Q0")
        out1_x, out1_y = self._get_output_position(4, 1)
        out1_led = self.create_element("Led", out1_x, out1_y, "Q1")
        
        self.connect_elements(ff0_q, 0, out0_led, 0)
        self.connect_elements(ff1_q, 0, out1_led, 0)
        
        # Create feedback connections: FF0 Q -> FF1 D, FF1 ~Q -> FF0 D (toggle pattern)
        logger.info("Creating feedback connections:")
        logger.info("  FF0 Q -> FF1 D (shift)")
        logger.info("  FF1 ~Q -> FF0 D (inverted feedback)")
        
        self.connect_elements(ff0_q, 0, ff1_d, 0)   # FF0 Q -> FF1 D
        self.connect_elements(ff1_qn, 1, ff0_d, 0)  # FF1 ~Q -> FF0 D (port 1 is ~Q)
        
        # Check initial state
        q0_init = self.get_output(out0_led)
        q1_init = self.get_output(out1_led)
        logger.info(f"Initial: Q0={q0_init}, Q1={q1_init}")
        
        # Apply clock pulse - should create: Q0=True (from ~Q1=~False=True), Q1=False (from Q0=False)
        logger.info("Applying first clock pulse...")
        logger.info("Expected: Q0 should become True (from ~Q1), Q1 should become False (from Q0)")
        
        self.pulse_clock(clk_id, 0.1)
        
        q0_after = self.get_output(out0_led)
        q1_after = self.get_output(out1_led)
        logger.info(f"After 1 clock: Q0={q0_after}, Q1={q1_after}")
        
        # Expected behavior: Q0=True, Q1=False
        expected = (q0_after == True, q1_after == False)
        
        if expected == (True, True):
            logger.info("✅ TEST 2 PASSED: Simple feedback loop works")
            return True
        else:
            logger.error("❌ TEST 2 FAILED: Feedback loop creates race conditions")
            
            # Diagnose the failure
            if q0_after == q1_after:
                logger.error("DIAGNOSIS: Both outputs same - suggests timing issue in feedback")
            if q0_after == q0_init and q1_after == q1_init:
                logger.error("DIAGNOSIS: No state change - flip-flops not responding to feedback")
                
            return False

def main():
    debugger = FeedbackLoopDebugger()
    
    try:
        # Test 1: Simple forward connection (should work like Moore machine)
        test1_success = debugger.test_simple_forward_connection()
        
        # Test 2: Simple feedback loop (may fail due to race conditions)
        test2_success = debugger.test_simple_feedback_loop()
        
        logger.info("🔍 ANALYSIS SUMMARY:")
        logger.info(f"  Forward connection (like Moore): {'✅ WORKS' if test1_success else '❌ FAILS'}")
        logger.info(f"  Feedback loop (like Johnson):    {'✅ WORKS' if test2_success else '❌ FAILS'}")
        
        if test1_success and not test2_success:
            logger.info("🎯 CONFIRMED: Issue is with FEEDBACK LOOPS, not D flip-flops!")
            logger.info("   Root cause: Race conditions in synchronous simulation with feedback")
        elif not test1_success:
            logger.info("🔧 Issue is more fundamental - even forward connections fail")
        else:
            logger.info("🤔 Both tests work - issue may be more specific to Johnson Counter")
            
    except Exception as e:
        logger.error(f"Exception during test: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()