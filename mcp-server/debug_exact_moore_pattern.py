#!/usr/bin/env python3

"""
Debug Exact Moore Pattern - Test with EXACT same sequence as working Moore machine
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

class ExactMoorePatternDebugger(AdvancedSequentialValidator):
    
    def test_exact_moore_pattern(self):
        """Test using EXACT same pattern as working Moore machine"""
        logger.info("🔧 Testing with EXACT Moore machine pattern")
        
        if not self.create_new_circuit():
            return False

        # EXACT same sequence as Moore machine
        # Inputs (same order and naming)
        input_x, input_y = self._get_input_position(0)
        input_id = self.create_element("InputButton", input_x, input_y, "INPUT")
        clk_x, clk_y = self._get_input_position(1)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        reset_x, reset_y = self._get_input_position(2)
        reset_id = self.create_element("InputButton", reset_x, reset_y, "RESET")

        # Single flip-flop for state storage with FIXED native D flip-flop (EXACT same call)
        ff_components = self._create_simple_d_flip_flop_native(1, 0, "STATE_FF", clk_id, reset_id)
        if not all(ff_components[:3]):
            return {"success": False, "error": "Failed to create state flip-flop"}
        ff_d_id, ff_q_id, ff_q_not_id = ff_components[:3]

        # Initialize with reset (EXACT same sequence)
        self.set_input(reset_id, True)  # Apply reset
        self.set_input(clk_id, False)   # Clock low during reset
        self.update_simulation()
        self.update_simulation()
        self.set_input(reset_id, False) # Release reset
        self.update_simulation()

        # Moore machine logic: Next state = current_input (EXACT same connections)
        # Native DFF: Port 0 = D input
        self.connect_elements(input_id, 0, ff_d_id, 0)

        # Output LED (Moore output = current state) (EXACT same position and connection)
        out_x, out_y = self._get_output_position(4, 0)
        output_led = self.create_element("Led", out_x, out_y, "OUTPUT")
        # Native DFF: Port 0 = Q output
        self.connect_elements(ff_q_id, 0, output_led, 0)

        # State display LED (EXACT same as Moore machine)
        state_x, state_y = self._get_output_position(4, 1)
        state_led = self.create_element("Led", state_x, state_y, "STATE")
        # Native DFF: Port 0 = Q output
        self.connect_elements(ff_q_id, 0, state_led, 0)

        logger.info("✅ Test circuit created with EXACT Moore pattern")

        # CRITICAL: Initialize EXACT same as Moore machine
        self.set_input(input_id, False)
        self.set_input(clk_id, False)
        self.restart_simulation()  # THIS WAS MISSING IN MY ORIGINAL TEST!
        time.sleep(0.1)

        logger.info("🔧 Test 1: Set INPUT=1 and apply clock pulse")
        
        # Set input to True (want to store 1 in flip-flop)
        self.set_input(input_id, True)
        self.update_simulation()
        
        # Check state before clock
        state_before = self.get_output(state_led)
        output_before = self.get_output(output_led)
        logger.info(f"Before clock: State={state_before}, Output={output_before}")
        
        # Apply clock pulse
        self.pulse_clock(clk_id, 0.1)
        
        # Check state after clock
        state_after = self.get_output(state_led)
        output_after = self.get_output(output_led)
        logger.info(f"After clock: State={state_after}, Output={output_after}")
        
        # Should change from False to True
        success_1 = (state_before == False) and (state_after == True) and (output_after == True)
        
        if success_1:
            logger.info("✅ Test 1 PASSED: INPUT=1 -> State=1 after clock")
        else:
            logger.error("❌ Test 1 FAILED: State did not change as expected")
            return False
            
        logger.info("🔧 Test 2: Set INPUT=0 and apply clock pulse")
        
        # Set input to False (want to store 0 in flip-flop)
        self.set_input(input_id, False)
        self.update_simulation()
        
        # Check state before second clock
        state_before_2 = self.get_output(state_led)
        output_before_2 = self.get_output(output_led)
        logger.info(f"Before 2nd clock: State={state_before_2}, Output={output_before_2}")
        
        # Apply second clock pulse
        self.pulse_clock(clk_id, 0.1)
        
        # Check state after second clock
        state_after_2 = self.get_output(state_led)
        output_after_2 = self.get_output(output_led)
        logger.info(f"After 2nd clock: State={state_after_2}, Output={output_after_2}")
        
        # Should change from True to False
        success_2 = (state_before_2 == True) and (state_after_2 == False) and (output_after_2 == False)
        
        if success_2:
            logger.info("✅ Test 2 PASSED: INPUT=0 -> State=0 after clock")
        else:
            logger.error("❌ Test 2 FAILED: State did not change as expected")
            return False
            
        logger.info("🎉 BOTH TESTS PASSED: D flip-flop works with exact Moore pattern!")
        return True

def main():
    debugger = ExactMoorePatternDebugger()
    
    try:
        success = debugger.test_exact_moore_pattern()
        
        if success:
            logger.info("🎯 CONFIRMED: D flip-flops work when using EXACT Moore machine pattern!")
            logger.info("   Key difference was likely: restart_simulation() after connections")
        else:
            logger.info("🔧 Even exact Moore pattern fails - deeper investigation needed")
            
    except Exception as e:
        logger.error(f"Exception during test: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()