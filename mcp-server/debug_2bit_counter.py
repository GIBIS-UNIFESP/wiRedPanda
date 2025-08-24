#!/usr/bin/env python3

"""
Debug 2-Bit Counter - Analyze feedback detection for mixed sequential/combinational circuits
Focus: Understand why mixed feedback circuits aren't being detected correctly
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

class TwoBitCounterDebugger(AdvancedSequentialValidator):
    
    def debug_2bit_counter_feedback_detection(self):
        """Debug 2-bit counter feedback detection step by step"""
        logger.info("🔧 2-BIT COUNTER FEEDBACK DETECTION DEBUG")
        logger.info("Expected: 2-bit counter with XOR feedback should be detected as feedback group")
        
        if not self.create_new_circuit():
            return False

        # Create inputs
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        reset_x, reset_y = self._get_input_position(1)
        reset_id = self.create_element("InputButton", reset_x, reset_y, "RESET")
        
        logger.info("🔧 Creating 2-bit counter components")
        
        # Create 2 D flip-flops for 2-bit counter
        ff0_components = self._create_simple_d_flip_flop_native(1, 0, "FF0", clk_id, reset_id)
        if not all(ff0_components[:3]):
            logger.error("Failed to create FF0")
            return False
        ff0_id = ff0_components[0]
        
        ff1_components = self._create_simple_d_flip_flop_native(1, 10, "FF1", clk_id, reset_id)
        if not all(ff1_components[:3]):
            logger.error("Failed to create FF1")
            return False
        ff1_id = ff1_components[0]
        
        # Create XOR gate for 2-bit counter logic
        xor_x, xor_y = self._get_grid_position(5, 5)
        xor_id = self.create_element("Xor", xor_x, xor_y, "XOR_FEEDBACK")
        
        logger.info("🔧 Creating 2-bit counter feedback connections")
        logger.info("2-bit Counter Logic:")
        logger.info("  D0 = ~Q0  (toggle FF0)")
        logger.info("  D1 = Q0 XOR Q1  (FF1 toggles when FF0=1)")
        
        # Connect NOT Q0 -> D0 (FF0 toggle)
        logger.info("Connecting FF0 ~Q (port 1) -> FF0 D (port 0) - TOGGLE")
        if not self.connect_elements(ff0_id, 1, ff0_id, 0):
            logger.error("Failed to connect FF0 toggle feedback")
            return False
            
        # Connect Q0 -> XOR input 0
        logger.info("Connecting FF0 Q (port 0) -> XOR input (port 0)")
        if not self.connect_elements(ff0_id, 0, xor_id, 0):
            logger.error("Failed to connect FF0 Q -> XOR")
            return False
            
        # Connect Q1 -> XOR input 1  
        logger.info("Connecting FF1 Q (port 0) -> XOR input (port 1)")
        if not self.connect_elements(ff1_id, 0, xor_id, 1):
            logger.error("Failed to connect FF1 Q -> XOR")
            return False
            
        # Connect XOR output -> D1
        logger.info("Connecting XOR output (port 0) -> FF1 D (port 0)")
        if not self.connect_elements(xor_id, 0, ff1_id, 0):
            logger.error("Failed to connect XOR -> FF1 D")
            return False
        
        # Create output LEDs
        out0_x, out0_y = self._get_output_position(10, 0)
        out0_led = self.create_element("Led", out0_x, out0_y, "Q0")
        out1_x, out1_y = self._get_output_position(10, 1)
        out1_led = self.create_element("Led", out1_x, out1_y, "Q1")
        
        self.connect_elements(ff0_id, 0, out0_led, 0)
        self.connect_elements(ff1_id, 0, out1_led, 0)
        
        logger.info("✅ 2-bit counter circuit created with mixed feedback")
        logger.info("Expected feedback groups:")
        logger.info("  Group 1: FF0 -> FF0 (self-feedback)")  
        logger.info("  Group 2: FF0 -> XOR -> FF1, FF1 -> XOR (mixed sequential/combinational)")
        
        # Initialize and check feedback detection
        self.set_input(reset_id, True)
        self.set_input(clk_id, False)
        self.update_simulation()
        self.update_simulation()
        self.set_input(reset_id, False)
        self.update_simulation()
        
        logger.info("✅ 2-bit counter initialization complete")
        logger.info("Check debug logs above for feedback group detection results")
        
        # Test basic counting behavior (should be 00 -> 01 -> 10 -> 11 -> 00)
        logger.info("🔧 Testing 2-bit counter sequence")
        expected_sequence = [
            (False, False, "Initial: 00"),
            (True, False, "After 1 clock: 01"), 
            (False, True, "After 2 clocks: 10"),
            (True, True, "After 3 clocks: 11"),
            (False, False, "After 4 clocks: 00 (wrap)"),
        ]
        
        results = []
        correct_count = 0
        
        for step, (exp_q0, exp_q1, description) in enumerate(expected_sequence):
            if step > 0:  # Apply clock pulse for all except initial
                logger.info(f"🕐 Applying clock pulse {step}")
                self.pulse_clock(clk_id, 0.1)
                
            # Read outputs
            actual_q0 = self.get_output(out0_led)
            actual_q1 = self.get_output(out1_led)
            
            logger.info(f"📊 Step {step}: {description}")
            logger.info(f"    Expected: Q1={exp_q1}, Q0={exp_q0}")
            logger.info(f"    Actual:   Q1={actual_q1}, Q0={actual_q0}")
            
            correct = (actual_q0 == exp_q0) and (actual_q1 == exp_q1)
            
            if correct:
                logger.info(f"    ✅ CORRECT")
                correct_count += 1
            else:
                logger.info(f"    ❌ FAILED")
                
                # Diagnose failure
                if not actual_q0 and not actual_q1:
                    logger.error(f"    DIAGNOSIS: Both outputs stuck at 0 - flip-flops not responding to clock")
                elif actual_q0 == actual_q1:
                    logger.error(f"    DIAGNOSIS: Both outputs identical ({actual_q0}) - suggests feedback loop issue")
                else:
                    logger.error(f"    DIAGNOSIS: Partial counter failure")
        
        # Final results
        accuracy = (correct_count / len(expected_sequence)) * 100
        logger.info(f"🎯 2-BIT COUNTER DEBUG RESULTS:")
        logger.info(f"   Correct steps: {correct_count}/{len(expected_sequence)}")
        logger.info(f"   Accuracy: {accuracy:.1f}%")
        
        if accuracy >= 80:
            logger.info("✅ 2-bit counter working correctly!")
        elif accuracy >= 40:
            logger.warning("⚠️ 2-bit counter partially working - feedback/timing issues")
        else:
            logger.error("❌ 2-bit counter failed - fundamental feedback/sequential problem")
        
        # Save circuit for inspection
        self.save_circuit("debug_2bit_counter.panda")
        logger.info("💾 Debug circuit saved as: debug_2bit_counter.panda")
        
        return accuracy >= 60

def main():
    debugger = TwoBitCounterDebugger()
    
    try:
        success = debugger.debug_2bit_counter_feedback_detection()
        
        if success:
            print("🎉 2-bit Counter feedback debug PASSED!")
        else:
            print("💥 2-bit Counter feedback debug FAILED!")
            print("   Check logs for detailed analysis of feedback detection and sequential behavior")
            
    except Exception as e:
        logger.error(f"Exception during 2-bit counter debug: {e}")
        print(f"💥 Exception: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()