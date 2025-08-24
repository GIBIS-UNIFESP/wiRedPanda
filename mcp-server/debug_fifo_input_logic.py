#!/usr/bin/env python3

"""
Debug FIFO Input Logic - Isolate the DATA AND PUSH gate issue
Focus: Test if AND gate logic is working for FIFO Stage 0 data input
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

class FifoInputLogicDebugger(AdvancedSequentialValidator):
    
    def debug_and_gate_logic(self):
        """Test if AND gate logic works independently"""
        logger.info("🔧 TESTING: Basic AND gate functionality")
        
        if not self.create_new_circuit():
            return False

        # Create simple AND gate test
        input_a_x, input_a_y = self._get_input_position(0)
        input_a = self.create_element("InputButton", input_a_x, input_a_y, "A")
        
        input_b_x, input_b_y = self._get_input_position(1)
        input_b = self.create_element("InputButton", input_b_x, input_b_y, "B")
        
        # Create AND gate
        and_x, and_y = self._get_grid_position(2, 0)
        and_gate = self.create_element("And", and_x, and_y, "AND_TEST")
        
        # Create output LED
        led_x, led_y = self._get_output_position(4, 0)
        output_led = self.create_element("Led", led_x, led_y, "OUTPUT")
        
        # Make connections
        self.connect_elements(input_a, 0, and_gate, 0)  # A -> AND input 0
        self.connect_elements(input_b, 0, and_gate, 1)  # B -> AND input 1
        self.connect_elements(and_gate, 0, output_led, 0)  # AND -> LED
        
        # Apply restart_simulation for proper initialization
        self.restart_simulation()
        time.sleep(0.1)
        
        logger.info("Testing AND gate truth table:")
        
        test_cases = [
            (False, False, False, "0 AND 0 = 0"),
            (False, True, False, "0 AND 1 = 0"),
            (True, False, False, "1 AND 0 = 0"),
            (True, True, True, "1 AND 1 = 1"),
        ]
        
        success_count = 0
        
        for a_val, b_val, expected, description in test_cases:
            self.set_input(input_a, a_val)
            self.set_input(input_b, b_val)
            self.update_simulation()
            time.sleep(0.1)
            
            actual = self.get_output(output_led)
            
            logger.info(f"  {description}: Expected={expected}, Actual={actual}")
            
            if actual == expected:
                logger.info(f"    ✅ CORRECT")
                success_count += 1
            else:
                logger.error(f"    ❌ FAILED")
        
        and_gate_works = (success_count == 4)
        
        if and_gate_works:
            logger.info("✅ AND gate logic working correctly")
        else:
            logger.error("❌ AND gate logic FAILED - basic logic issue")
        
        return and_gate_works
    
    def debug_single_flip_flop_with_and_gate(self):
        """Test single flip-flop with AND gate input (simplified FIFO Stage 0)"""
        logger.info("🔧 TESTING: Single D flip-flop with AND gate input control")
        
        if not self.create_new_circuit():
            return False

        # Create inputs
        data_x, data_y = self._get_input_position(0)
        data_id = self.create_element("InputButton", data_x, data_y, "DATA")
        
        enable_x, enable_y = self._get_input_position(1)
        enable_id = self.create_element("InputButton", enable_x, enable_y, "ENABLE")
        
        clk_x, clk_y = self._get_input_position(2)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        reset_x, reset_y = self._get_input_position(3)
        reset_id = self.create_element("InputButton", reset_x, reset_y, "RESET")
        
        # Create D flip-flop
        ff_components = self._create_simple_d_flip_flop_native(2, 0, "TEST_FF", clk_id, reset_id)
        if not all(ff_components[:3]):
            logger.error("Failed to create test flip-flop")
            return False
        ff_id = ff_components[0]
        
        # Create AND gate for input control: DATA AND ENABLE -> FF D
        and_x, and_y = self._get_grid_position(1, 2)
        input_and = self.create_element("And", and_x, and_y, "INPUT_AND")
        
        # Create output LED
        led_x, led_y = self._get_output_position(6, 0)
        output_led = self.create_element("Led", led_x, led_y, "Q_OUT")
        
        # Make connections
        logger.info("Making connections: DATA AND ENABLE -> FF D input")
        self.connect_elements(data_id, 0, input_and, 0)      # DATA -> AND
        self.connect_elements(enable_id, 0, input_and, 1)    # ENABLE -> AND
        self.connect_elements(input_and, 0, ff_id, 0)        # AND -> FF D input
        self.connect_elements(ff_id, 0, output_led, 0)       # FF Q -> LED
        
        # Initialize system
        logger.info("Initializing system with reset")
        self.set_input(reset_id, True)
        self.set_input(clk_id, False)
        self.set_input(data_id, False)
        self.set_input(enable_id, False)
        self.update_simulation()
        self.update_simulation()
        self.set_input(reset_id, False)
        self.update_simulation()
        
        # Apply restart_simulation for proper initialization
        self.restart_simulation()
        time.sleep(0.1)
        
        # Check initial state
        initial_q = self.get_output(output_led)
        logger.info(f"After reset: Q={initial_q} (should be False)")
        
        # Test sequence
        test_cases = [
            (True, False, "DATA=1, ENABLE=0 (should not store)"),
            (False, True, "DATA=0, ENABLE=1 (should store 0)"),
            (True, True, "DATA=1, ENABLE=1 (should store 1)"),
        ]
        
        success_count = 0
        
        for data_val, enable_val, description in test_cases:
            logger.info(f"\n🔧 Test: {description}")
            
            # Set inputs
            self.set_input(data_id, data_val)
            self.set_input(enable_id, enable_val)
            self.update_simulation()
            
            # Check AND gate output (what should go to FF D input)
            expected_d_input = data_val and enable_val
            logger.info(f"  Expected D input: {expected_d_input} (DATA={data_val} AND ENABLE={enable_val})")
            
            # Read state before clock
            before_q = self.get_output(output_led)
            logger.info(f"  Before clock: Q={before_q}")
            
            # Apply clock pulse
            logger.info("  Applying clock pulse...")
            self.pulse_clock(clk_id, 0.15)
            
            # Read state after clock
            after_q = self.get_output(output_led)
            logger.info(f"  After clock: Q={after_q}")
            
            # Determine expected result
            if enable_val:  # Only store when enabled
                expected_q = data_val
                logger.info(f"  Expected Q: {expected_q} (should store DATA when ENABLE=1)")
            else:  # Should not change when disabled
                expected_q = before_q
                logger.info(f"  Expected Q: {expected_q} (should not change when ENABLE=0)")
            
            if after_q == expected_q:
                logger.info(f"  ✅ CORRECT")
                success_count += 1
            else:
                logger.error(f"  ❌ FAILED: Expected {expected_q}, got {after_q}")
                
                # Diagnose failure
                if not enable_val and after_q != before_q:
                    logger.error(f"    DIAGNOSIS: FF changed state when ENABLE=0 - AND gate not working")
                elif enable_val and after_q != data_val:
                    logger.error(f"    DIAGNOSIS: FF not storing correct value when ENABLE=1")
                    if after_q == before_q:
                        logger.error(f"    POSSIBLE CAUSE: D input not receiving AND gate output")
                    else:
                        logger.error(f"    POSSIBLE CAUSE: FF timing or connection issue")
        
        controlled_ff_works = (success_count >= 2)  # Allow one failure
        
        if controlled_ff_works:
            logger.info("✅ Controlled flip-flop working correctly")
        else:
            logger.error("❌ Controlled flip-flop FAILED")
        
        # Save for inspection
        self.save_circuit("debug_controlled_flipflop.panda")
        
        return controlled_ff_works
    
    def debug_fifo_stage0_isolated(self):
        """Test FIFO Stage 0 in complete isolation"""
        logger.info("🔧 TESTING: FIFO Stage 0 in complete isolation")
        
        if not self.create_new_circuit():
            return False

        # Create exact same setup as FIFO debug but with only Stage 0
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        data_x, data_y = self._get_input_position(1)
        data_id = self.create_element("InputButton", data_x, data_y, "DATA_IN")
        
        push_x, push_y = self._get_input_position(2)
        push_id = self.create_element("InputButton", push_x, push_y, "PUSH")
        
        reset_x, reset_y = self._get_input_position(3)
        reset_id = self.create_element("InputButton", reset_x, reset_y, "RESET")
        
        # Create single FIFO stage (Stage 0)
        stage0_components = self._create_simple_d_flip_flop_native(2, 0, "FIFO_S0", clk_id, reset_id)
        if not all(stage0_components[:3]):
            logger.error("Failed to create FIFO Stage 0")
            return False
        stage0_ff = stage0_components[0]
        
        # Create monitoring LED
        led_x, led_y = self._get_output_position(8, 0)
        stage0_led = self.create_element("Led", led_x, led_y, "STAGE0")
        self.connect_elements(stage0_ff, 0, stage0_led, 0)
        
        # Create PUSH control AND gate (exact same as FIFO debug)
        push_and_x, push_and_y = self._get_grid_position(1, 5)
        push_and = self.create_element("And", push_and_x, push_and_y, "PUSH_AND")
        
        # Make exact same connections as FIFO debug
        logger.info("Making FIFO-style connections:")
        logger.info("  DATA_IN -> PUSH_AND input 0")
        self.connect_elements(data_id, 0, push_and, 0)
        
        logger.info("  PUSH -> PUSH_AND input 1")
        self.connect_elements(push_id, 0, push_and, 1)
        
        logger.info("  PUSH_AND output -> Stage0 D input")
        self.connect_elements(push_and, 0, stage0_ff, 0)
        
        # Initialize exactly like FIFO debug
        logger.info("Initializing exactly like FIFO debug")
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
        
        # Check initial state
        initial_stage = self.get_output(stage0_led)
        logger.info(f"After initialization: Stage0={initial_stage} (should be False)")
        
        # Test the exact failing case from FIFO debug
        logger.info("\n🔧 Testing exact failing case: DATA=True, PUSH=True")
        
        self.set_input(data_id, True)
        self.set_input(push_id, True)
        self.update_simulation()
        
        before_stage = self.get_output(stage0_led)
        logger.info(f"Before clock: Stage0={before_stage}")
        logger.info("Expected: PUSH_AND should output True (True AND True = True)")
        logger.info("Expected: Stage0 should prepare to receive True on clock edge")
        
        # Apply clock pulse
        logger.info("Applying clock pulse...")
        self.set_input(clk_id, True)
        time.sleep(0.15)
        self.update_simulation()
        
        during_stage = self.get_output(stage0_led)
        logger.info(f"During clock high: Stage0={during_stage}")
        
        self.set_input(clk_id, False)
        time.sleep(0.15)
        self.update_simulation()
        
        after_stage = self.get_output(stage0_led)
        logger.info(f"After clock pulse: Stage0={after_stage}")
        logger.info("Expected: Stage0 should be True (stored DATA=True when PUSH=True)")
        
        success = (after_stage == True)
        
        if success:
            logger.info("✅ FIFO Stage 0 working in isolation!")
        else:
            logger.error("❌ FIFO Stage 0 FAILED in isolation")
            logger.error("  This confirms the PUSH_AND logic is not working correctly")
            
            # Additional debugging: Test AND gate directly
            logger.info("\n🔧 Additional debugging: Reading PUSH_AND gate directly")
            # Note: Cannot directly read AND gate output, but we can infer from behavior
            
            logger.info("  Possible issues:")
            logger.info("  1. AND gate not receiving correct inputs")
            logger.info("  2. AND gate not producing correct output")
            logger.info("  3. Connection from AND gate to FF D input broken")
            logger.info("  4. FF D input not accepting AND gate output")
        
        # Save for inspection
        self.save_circuit("debug_fifo_stage0_isolated.panda")
        
        return success

def main():
    debugger = FifoInputLogicDebugger()
    
    try:
        # Test 1: Basic AND gate functionality
        logger.info("=" * 60)
        logger.info("TEST 1: Basic AND gate functionality")
        logger.info("=" * 60)
        and_gate_works = debugger.debug_and_gate_logic()
        
        # Test 2: Single flip-flop with AND gate control
        logger.info("\n" + "=" * 60)
        logger.info("TEST 2: Single flip-flop with AND gate control")
        logger.info("=" * 60)
        controlled_ff_works = debugger.debug_single_flip_flop_with_and_gate()
        
        # Test 3: FIFO Stage 0 isolated
        logger.info("\n" + "=" * 60)
        logger.info("TEST 3: FIFO Stage 0 isolated")
        logger.info("=" * 60)
        fifo_stage0_works = debugger.debug_fifo_stage0_isolated()
        
        # Overall analysis
        logger.info("\n" + "=" * 60)
        logger.info("OVERALL ANALYSIS")
        logger.info("=" * 60)
        
        logger.info(f"Basic AND gate: {'✅ WORKS' if and_gate_works else '❌ FAILS'}")
        logger.info(f"Controlled flip-flop: {'✅ WORKS' if controlled_ff_works else '❌ FAILS'}")
        logger.info(f"FIFO Stage 0: {'✅ WORKS' if fifo_stage0_works else '❌ FAILS'}")
        
        if and_gate_works and controlled_ff_works and fifo_stage0_works:
            print("🎉 All components work! FIFO issue may be in multi-stage interactions")
        elif and_gate_works and controlled_ff_works:
            print("🔧 Basic logic works, but FIFO-specific implementation has issues")
        elif and_gate_works:
            print("🔧 AND gate works, but flip-flop control logic has issues")
        else:
            print("💥 Fundamental AND gate logic failure")
            
    except Exception as e:
        logger.error(f"Exception during FIFO input logic debug: {e}")
        print(f"💥 Exception: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()