#!/usr/bin/env python3

"""
Debug Self-Loop - Minimal test to check if flip-flop self-feedback is working
Focus: Single D flip-flop with ~Q -> D feedback (toggle behavior)
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

class SelfLoopDebugger(AdvancedSequentialValidator):
    
    def debug_simple_toggle_flipflop(self):
        """Debug minimal D flip-flop self-feedback toggle"""
        logger.info("🔧 SELF-LOOP TOGGLE DEBUG")
        logger.info("Creating D flip-flop with ~Q -> D (toggle on clock)")
        
        if not self.create_new_circuit():
            return False

        # Create clock and reset
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        reset_x, reset_y = self._get_input_position(1)
        reset_id = self.create_element("InputButton", reset_x, reset_y, "RESET")
        
        # Create single D flip-flop
        ff_components = self._create_simple_d_flip_flop_native(1, 0, "TOGGLE_FF", clk_id, reset_id)
        if not all(ff_components[:3]):
            logger.error("Failed to create toggle flip-flop")
            return False
        ff_id = ff_components[0]
        
        # Create LED for monitoring
        out_x, out_y = self._get_output_position(10, 0)
        out_led = self.create_element("Led", out_x, out_y, "Q")
        
        # Connect output to LED
        self.connect_elements(ff_id, 0, out_led, 0)  # FF Q -> LED
        
        logger.info("🔧 Creating self-loop feedback: ~Q -> D")
        # Connect ~Q (port 1) back to D (port 0) - this should create a toggle
        if not self.connect_elements(ff_id, 1, ff_id, 0):
            logger.error("Failed to connect ~Q -> D feedback")
            return False
        
        logger.info("✅ Self-loop created! Expected cycle detection...")
        
        # Initialize
        logger.info("🔧 Initialize with reset")
        self.set_input(reset_id, True)
        self.set_input(clk_id, False)
        self.update_simulation()
        self.update_simulation()
        self.set_input(reset_id, False)
        self.update_simulation()
        
        # Check initial state
        initial_q = self.get_output(out_led)
        logger.info(f"Initial state after reset: Q={initial_q} (expected: False)")
        
        # Test toggle sequence
        logger.info("🔧 Testing toggle behavior")
        expected_sequence = [
            (False, "After reset: Q=0"),
            (True, "After 1st clock: Q=1 (toggled)"),
            (False, "After 2nd clock: Q=0 (toggled)"), 
            (True, "After 3rd clock: Q=1 (toggled)"),
            (False, "After 4th clock: Q=0 (toggled)"),
        ]
        
        results = []
        correct_count = 0
        
        for step, (expected_q, description) in enumerate(expected_sequence):
            if step > 0:  # Apply clock pulse for all except initial
                logger.info(f"🕐 Applying clock pulse {step}")
                
                # Before clock pulse
                pre_q = self.get_output(out_led)
                logger.info(f"  Before clock: Q={pre_q}, D=~Q={not pre_q}")
                
                # Clock pulse
                self.pulse_clock(clk_id, 0.15)
                
                # After clock pulse  
                post_q = self.get_output(out_led)
                logger.info(f"  After clock: Q={post_q}")
                
            actual_q = self.get_output(out_led)
            
            logger.info(f"📊 Step {step}: {description}")
            logger.info(f"    Expected: Q={expected_q}")
            logger.info(f"    Actual:   Q={actual_q}")
            
            correct = (actual_q == expected_q)
            
            if correct:
                logger.info(f"    ✅ CORRECT")
                correct_count += 1
            else:
                logger.info(f"    ❌ FAILED")
                
                if actual_q == initial_q:
                    logger.error(f"    DIAGNOSIS: Output stuck at initial value - self-loop not working")
                else:
                    logger.error(f"    DIAGNOSIS: Toggle behavior incorrect")
                    
            results.append({
                "step": step,
                "expected": expected_q,
                "actual": actual_q,
                "correct": correct,
                "description": description
            })
        
        # Analysis
        accuracy = (correct_count / len(expected_sequence)) * 100
        logger.info(f"🎯 SELF-LOOP TOGGLE RESULTS:")
        logger.info(f"   Correct steps: {correct_count}/{len(expected_sequence)}")
        logger.info(f"   Accuracy: {accuracy:.1f}%")
        
        if accuracy >= 80:
            logger.info("✅ Self-loop toggle working correctly!")
        elif accuracy >= 40:
            logger.warning("⚠️ Self-loop partially working - timing/feedback issues")
        else:
            logger.error("❌ Self-loop failed - fundamental feedback problem")
        
        # Save for inspection
        self.save_circuit("debug_self_loop.panda")
        logger.info("💾 Debug circuit saved as: debug_self_loop.panda")
        
        return accuracy >= 60

def main():
    debugger = SelfLoopDebugger()
    
    try:
        success = debugger.debug_simple_toggle_flipflop()
        
        if success:
            print("🎉 Self-loop toggle debug PASSED!")
        else:
            print("💥 Self-loop toggle debug FAILED!")
            print("   Check logs for detailed analysis of self-feedback behavior")
            
    except Exception as e:
        logger.error(f"Exception during self-loop debug: {e}")
        print(f"💥 Exception: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()