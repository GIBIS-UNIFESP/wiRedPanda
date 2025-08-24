#!/usr/bin/env python3

"""
Debug Simple Johnson Counter - Using native DFlipFlop elements like successful cycle tests
Focus: Test if Johnson Counter works with simple native elements vs complex implementations
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

class SimpleJohnsonCounterDebugger(AdvancedSequentialValidator):
    
    def debug_simple_johnson_counter(self):
        """Debug Johnson Counter using simple native DFlipFlop elements (like successful cycle tests)"""
        logger.info("🔧 SIMPLE JOHNSON COUNTER DEBUG")
        logger.info("Using native DFlipFlop elements - same approach as working cycle tests")
        logger.info("Expected sequence: 000 -> 001 -> 011 -> 111 -> 110 -> 100 -> 000")
        
        if not self.create_new_circuit():
            return False

        # Create inputs
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        reset_x, reset_y = self._get_input_position(1)
        reset_id = self.create_element("InputButton", reset_x, reset_y, "RESET")
        
        # Create 3 native D flip-flops (same as successful cycle tests)
        logger.info("🔧 Creating 3 native D flip-flops")
        ff0_x, ff0_y = self._get_grid_position(3, 1)
        ff0_id = self.create_element("DFlipFlop", ff0_x, ff0_y, "JFF0")
        
        ff1_x, ff1_y = self._get_grid_position(3, 3)
        ff1_id = self.create_element("DFlipFlop", ff1_x, ff1_y, "JFF1")
        
        ff2_x, ff2_y = self._get_grid_position(3, 5)
        ff2_id = self.create_element("DFlipFlop", ff2_x, ff2_y, "JFF2")
        
        # Connect clocks
        self.connect_elements(clk_id, 0, ff0_id, 1)  # CLK -> FF0 clock
        self.connect_elements(clk_id, 0, ff1_id, 1)  # CLK -> FF1 clock
        self.connect_elements(clk_id, 0, ff2_id, 1)  # CLK -> FF2 clock
        
        # Connect resets
        self.connect_elements(reset_id, 0, ff0_id, 2)  # RESET -> FF0 reset
        self.connect_elements(reset_id, 0, ff1_id, 2)  # RESET -> FF1 reset
        self.connect_elements(reset_id, 0, ff2_id, 2)  # RESET -> FF2 reset
        
        # Create output LEDs for monitoring
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
        
        logger.info("🔧 Creating Johnson Counter feedback connections")
        logger.info("Johnson Counter Logic:")
        logger.info("  FF0 D = ~FF2 Q  (inverted feedback from last to first)")
        logger.info("  FF1 D = FF0 Q   (shift: stage 0 -> stage 1)")
        logger.info("  FF2 D = FF1 Q   (shift: stage 1 -> stage 2)")
        
        # Create Johnson Counter feedback - EXACT same pattern as successful cycle tests
        logger.info("Creating Johnson cycle: FF0 -> FF1 -> FF2 -> FF0 (with inversion)")
        
        # Forward shift connections
        if not self.connect_elements(ff0_id, 0, ff1_id, 0):  # FF0 Q -> FF1 D
            logger.error("Failed to connect FF0 -> FF1")
            return False
        if not self.connect_elements(ff1_id, 0, ff2_id, 0):  # FF1 Q -> FF2 D
            logger.error("Failed to connect FF1 -> FF2")
            return False
        
        # Inverted feedback connection (closes the cycle)
        if not self.connect_elements(ff2_id, 1, ff0_id, 0):  # FF2 ~Q -> FF0 D
            logger.error("Failed to connect FF2 ~Q -> FF0 D (inverted feedback)")
            return False
        
        logger.info("✅ Johnson Counter cycle created! Expected: 3-element feedback group")
        
        # FINAL FIX: Rebuild ElementMapping after all elements and connections are created
        logger.info("🔧 Triggering full ElementMapping rebuild after complete circuit creation")
        self.trigger_circuit_update()
        
        # CRITICAL: Wait for circuit update to complete before starting simulation
        logger.info("🔧 Waiting for circuit update to complete...")
        self.update_simulation()  # Force one simulation cycle to ensure connections are active
        
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
        
        # Test Johnson Counter sequence step by step
        logger.info("🔧 Testing Johnson Counter sequence")
        expected_sequence = [
            (False, False, False, "Initial: 000"),
            (True, False, False, "After 1 clock: 001 (D0=~Q2=~False=True)"),
            (True, True, False, "After 2 clocks: 011 (D1=Q0=True)"),
            (True, True, True, "After 3 clocks: 111 (D2=Q1=True)"),
            (False, True, True, "After 4 clocks: 110 (D0=~Q2=~True=False)"),
            (False, False, True, "After 5 clocks: 100 (D1=Q0=False)"),
            (False, False, False, "After 6 clocks: 000 (D2=Q1=False)"),
        ]
        
        results = []
        correct_count = 0
        
        for step, (exp_q0, exp_q1, exp_q2, description) in enumerate(expected_sequence):
            if step > 0:  # Apply clock pulse for all steps except initial
                logger.info(f"🕐 Applying clock pulse {step}")
                
                # Before clock pulse
                pre_q0 = self.get_output(out0_led)
                pre_q1 = self.get_output(out1_led)
                pre_q2 = self.get_output(out2_led)
                logger.info(f"  Before clock: Q2={pre_q2}, Q1={pre_q1}, Q0={pre_q0}")
                logger.info(f"  Expected D inputs: D0=~Q2=~{pre_q2}={not pre_q2}, D1=Q0={pre_q0}, D2=Q1={pre_q1}")
                
                # Clock pulse
                self.pulse_clock(clk_id, 0.15)
                
            # Read final outputs
            actual_q0 = self.get_output(out0_led)
            actual_q1 = self.get_output(out1_led)
            actual_q2 = self.get_output(out2_led)
            
            logger.info(f"📊 Step {step}: {description}")
            logger.info(f"    Expected: Q2={exp_q2}, Q1={exp_q1}, Q0={exp_q0}")
            logger.info(f"    Actual:   Q2={actual_q2}, Q1={actual_q1}, Q0={actual_q0}")
            
            # Check correctness
            correct = (actual_q0 == exp_q0) and (actual_q1 == exp_q1) and (actual_q2 == exp_q2)
            
            if correct:
                logger.info(f"    ✅ CORRECT")
                correct_count += 1
            else:
                logger.info(f"    ❌ FAILED")
                
                # Diagnose failure pattern
                if actual_q0 == actual_q1 == actual_q2:
                    logger.error(f"    DIAGNOSIS: All outputs identical ({actual_q0}) - convergence or feedback issue")
                elif step == 1 and not actual_q0:
                    logger.error(f"    DIAGNOSIS: Q0 should be True from ~Q2=~False=True, but got {actual_q0}")
                else:
                    logger.error(f"    DIAGNOSIS: Johnson Counter behavior incorrect")
            
            results.append({
                "step": step,
                "expected": (exp_q0, exp_q1, exp_q2),
                "actual": (actual_q0, actual_q1, actual_q2),
                "correct": correct,
                "description": description
            })
        
        # Final analysis
        accuracy = (correct_count / len(expected_sequence)) * 100
        logger.info(f"🎯 SIMPLE JOHNSON COUNTER RESULTS:")
        logger.info(f"   Correct steps: {correct_count}/{len(expected_sequence)}")
        logger.info(f"   Accuracy: {accuracy:.1f}%")
        
        if accuracy >= 80:
            logger.info("✅ Simple Johnson Counter working correctly!")
        elif accuracy >= 40:
            logger.warning("⚠️ Simple Johnson Counter partially working")
        else:
            logger.error("❌ Simple Johnson Counter failed")
        
        # Save circuit for inspection
        self.save_circuit("debug_simple_johnson_counter.panda")
        logger.info("💾 Debug circuit saved as: debug_simple_johnson_counter.panda")
        
        return accuracy >= 60

def main():
    debugger = SimpleJohnsonCounterDebugger()
    
    try:
        success = debugger.debug_simple_johnson_counter()
        
        if success:
            print("🎉 Simple Johnson Counter debug PASSED!")
        else:
            print("💥 Simple Johnson Counter debug FAILED!")
            print("   Compare with complex implementation to identify differences")
            
    except Exception as e:
        logger.error(f"Exception during simple Johnson Counter debug: {e}")
        print(f"💥 Exception: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()