#!/usr/bin/env python3

"""
Debug OR Gate Floating Input - Test if unused OR gate inputs cause logic failures
Focus: Test OR gate behavior with unused inputs vs tied inputs
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

class OrGateFloatingInputDebugger(AdvancedSequentialValidator):
    
    def test_or_gate_floating_vs_tied_inputs(self):
        """Test OR gate behavior with floating vs tied unused inputs"""
        logger.info("🔧 TESTING: OR gate behavior - floating vs tied unused inputs")
        
        if not self.create_new_circuit():
            return False

        # Create inputs
        input_a_x, input_a_y = self._get_input_position(0)
        input_a = self.create_element("InputButton", input_a_x, input_a_y, "A")
        
        # Create GND for tying unused inputs
        gnd_x, gnd_y = self._get_input_position(1)
        gnd = self.create_element("InputGnd", gnd_x, gnd_y, "GND")
        
        # Test 1: OR gate with floating second input
        logger.info("Creating OR gate with FLOATING second input")
        or1_x, or1_y = self._get_grid_position(2, 0)
        or_floating = self.create_element("Or", or1_x, or1_y, "OR_FLOATING")
        
        led1_x, led1_y = self._get_output_position(4, 0)
        led_floating = self.create_element("Led", led1_x, led1_y, "FLOATING_OUT")
        
        # Connect only input A, leave input B floating
        self.connect_elements(input_a, 0, or_floating, 0)  # A -> OR input 0
        # Input 1 deliberately left unconnected (floating)
        self.connect_elements(or_floating, 0, led_floating, 0)  # OR -> LED
        
        # Test 2: OR gate with tied second input  
        logger.info("Creating OR gate with TIED second input")
        or2_x, or2_y = self._get_grid_position(2, 5)
        or_tied = self.create_element("Or", or2_x, or2_y, "OR_TIED")
        
        led2_x, led2_y = self._get_output_position(4, 1)
        led_tied = self.create_element("Led", led2_x, led2_y, "TIED_OUT")
        
        # Connect input A and tie input B to GND
        self.connect_elements(input_a, 0, or_tied, 0)    # A -> OR input 0
        self.connect_elements(gnd, 0, or_tied, 1)        # GND -> OR input 1 (tied low)
        self.connect_elements(or_tied, 0, led_tied, 0)   # OR -> LED
        
        # Initialize
        self.restart_simulation()
        time.sleep(0.1)
        
        logger.info("Testing OR gate behavior with A=False:")
        self.set_input(input_a, False)
        self.update_simulation()
        time.sleep(0.1)
        
        floating_result_false = self.get_output(led_floating)
        tied_result_false = self.get_output(led_tied)
        
        logger.info(f"  OR with floating input: {floating_result_false}")
        logger.info(f"  OR with tied input:     {tied_result_false}")
        logger.info(f"  Expected (False OR ?):  False")
        
        logger.info("Testing OR gate behavior with A=True:")
        self.set_input(input_a, True)
        self.update_simulation()
        time.sleep(0.1)
        
        floating_result_true = self.get_output(led_floating)
        tied_result_true = self.get_output(led_tied)
        
        logger.info(f"  OR with floating input: {floating_result_true}")
        logger.info(f"  OR with tied input:     {tied_result_true}")
        logger.info(f"  Expected (True OR ?):   True")
        
        # Analysis
        logger.info("\n🔍 ANALYSIS:")
        
        tied_correct = (tied_result_false == False) and (tied_result_true == True)
        floating_correct = (floating_result_false == False) and (floating_result_true == True)
        
        if tied_correct:
            logger.info("✅ OR gate with tied input works correctly")
        else:
            logger.error("❌ OR gate with tied input FAILED")
        
        if floating_correct:
            logger.info("✅ OR gate with floating input works correctly")
        else:
            logger.error("❌ OR gate with floating input FAILED")
            logger.error("   This could be the FIFO issue!")
        
        # Save circuit for inspection
        self.save_circuit("debug_or_gate_floating_input.panda")
        
        return tied_correct and floating_correct
    
    def test_fifo_with_tied_or_gates(self):
        """Test simplified FIFO with properly tied OR gate inputs"""
        logger.info("🔧 TESTING: Simplified FIFO with tied OR gate inputs")
        
        if not self.create_new_circuit():
            return False

        # Create basic FIFO test - single stage with tied OR gate
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        data_x, data_y = self._get_input_position(1)
        data_id = self.create_element("InputButton", data_x, data_y, "DATA_IN")
        
        push_x, push_y = self._get_input_position(2)
        push_id = self.create_element("InputButton", push_x, push_y, "PUSH")
        
        reset_x, reset_y = self._get_input_position(3)
        reset_id = self.create_element("InputButton", reset_x, reset_y, "RESET")
        
        # Create GND for tying unused OR inputs
        gnd_x, gnd_y = self._get_input_position(4)
        gnd_id = self.create_element("InputGnd", gnd_x, gnd_y, "GND")
        
        # Create single FIFO stage
        stage0_components = self._create_simple_d_flip_flop_native(3, 0, "STAGE0", clk_id, reset_id)
        if not all(stage0_components[:3]):
            logger.error("Failed to create FIFO Stage 0")
            return False
        stage0_ff = stage0_components[0]
        
        # Create PUSH_AND gate
        push_and_x, push_and_y = self._get_grid_position(2, 0)
        push_and = self.create_element("And", push_and_x, push_and_y, "PUSH_AND")
        
        # Create OR gate with TIED second input
        logger.info("Creating OR gate with TIED unused input to GND")
        data_or_x, data_or_y = self._get_grid_position(1, 5)
        data_or = self.create_element("Or", data_or_x, data_or_y, "DATA_OR_TIED")
        
        # Create output LED
        led_x, led_y = self._get_output_position(6, 0)
        stage_led = self.create_element("Led", led_x, led_y, "STAGE0")
        
        # Make connections - EXACT same as working simple test BUT with tied OR gate
        logger.info("Making connections with tied OR gate:")
        logger.info("  DATA_IN -> PUSH_AND input 0")
        self.connect_elements(data_id, 0, push_and, 0)
        
        logger.info("  PUSH -> PUSH_AND input 1") 
        self.connect_elements(push_id, 0, push_and, 1)
        
        logger.info("  PUSH_AND -> DATA_OR input 0")
        self.connect_elements(push_and, 0, data_or, 0)
        
        logger.info("  GND -> DATA_OR input 1 (TIE UNUSED INPUT)")
        self.connect_elements(gnd_id, 0, data_or, 1)
        
        logger.info("  DATA_OR -> STAGE0 D input")
        self.connect_elements(data_or, 0, stage0_ff, 0)
        
        logger.info("  STAGE0 Q -> LED")
        self.connect_elements(stage0_ff, 0, stage_led, 0)
        
        # Initialize
        logger.info("Initializing FIFO with tied OR gate")
        self.set_input(reset_id, True)
        self.set_input(clk_id, False)
        self.set_input(data_id, False)
        self.set_input(push_id, False)
        self.update_simulation()
        self.update_simulation()
        self.set_input(reset_id, False)
        self.update_simulation()
        
        self.restart_simulation()
        time.sleep(0.1)
        
        # Test the exact failing case
        logger.info("\n🔧 Testing: DATA=True, PUSH=True with tied OR gate")
        
        self.set_input(data_id, True)
        self.set_input(push_id, True)
        self.update_simulation()
        
        before_stage = self.get_output(stage_led)
        logger.info(f"Before clock: Stage0={before_stage}")
        logger.info("Expected logic: DATA=True AND PUSH=True -> True OR False -> True")
        
        # Apply clock pulse
        logger.info("Applying clock pulse...")
        self.set_input(clk_id, True)
        time.sleep(0.15)
        self.update_simulation()
        
        during_stage = self.get_output(stage_led)
        logger.info(f"During clock high: Stage0={during_stage}")
        
        self.set_input(clk_id, False)
        time.sleep(0.15)
        self.update_simulation()
        
        after_stage = self.get_output(stage_led)
        logger.info(f"After clock pulse: Stage0={after_stage}")
        logger.info("Expected: Stage0 should be True")
        
        success = (after_stage == True)
        
        if success:
            logger.info("✅ FIFO with tied OR gate works!")
            logger.info("   SOLUTION FOUND: OR gates need tied unused inputs")
        else:
            logger.error("❌ FIFO with tied OR gate still fails")
            logger.error("   The issue is deeper than floating OR inputs")
        
        # Save circuit for inspection
        self.save_circuit("debug_fifo_tied_or_gate.panda")
        
        return success

def main():
    debugger = OrGateFloatingInputDebugger()
    
    try:
        # Test 1: OR gate floating vs tied inputs
        logger.info("=" * 60)
        logger.info("TEST 1: OR gate floating vs tied inputs")
        logger.info("=" * 60)
        or_test_passed = debugger.test_or_gate_floating_vs_tied_inputs()
        
        # Test 2: FIFO with tied OR gates
        logger.info("\n" + "=" * 60)
        logger.info("TEST 2: FIFO with tied OR gates")
        logger.info("=" * 60)
        fifo_tied_passed = debugger.test_fifo_with_tied_or_gates()
        
        # Overall analysis
        logger.info("\n" + "=" * 60)
        logger.info("OVERALL ANALYSIS")
        logger.info("=" * 60)
        
        logger.info(f"OR gate behavior test: {'✅ WORKS' if or_test_passed else '❌ FAILS'}")
        logger.info(f"FIFO with tied OR gates: {'✅ WORKS' if fifo_tied_passed else '❌ FAILS'}")
        
        if fifo_tied_passed:
            print("🎉 SOLUTION FOUND: FIFO needs tied OR gate inputs!")
            print("   The complex FIFO fails because OR gates have floating unused inputs")
        elif or_test_passed:
            print("🔧 OR gates work fine, but FIFO issue is more complex")
        else:
            print("💥 Fundamental OR gate issue detected")
            
    except Exception as e:
        logger.error(f"Exception during OR gate debug: {e}")
        print(f"💥 Exception: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()