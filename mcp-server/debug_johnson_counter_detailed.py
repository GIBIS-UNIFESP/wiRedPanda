#!/usr/bin/env python3

"""
Debug Johnson Counter - Detailed analysis of 3-bit ring counter with inverted feedback
Focus: Isolate circular feedback loop race conditions in Johnson Counter
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

class JohnsonCounterDetailedDebugger(AdvancedSequentialValidator):
    
    def debug_johnson_counter_step_by_step(self):
        """Debug Johnson Counter with detailed step-by-step analysis of each flip-flop"""
        logger.info("🔧 JOHNSON COUNTER DETAILED DEBUG")
        logger.info("Expected sequence: 000 -> 001 -> 011 -> 111 -> 110 -> 100 -> 000")
        
        if not self.create_new_circuit():
            return False

        # Create inputs
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        reset_x, reset_y = self._get_input_position(1)
        reset_id = self.create_element("InputButton", reset_x, reset_y, "RESET")
        
        # Create 3 D flip-flops with maximum spacing to avoid interference
        logger.info("Creating 3 D flip-flops with maximum spacing")
        ff0_components = self._create_simple_d_flip_flop_native(1, 0, "JFF0", clk_id, reset_id)
        if not all(ff0_components[:3]):
            logger.error("Failed to create FF0")
            return False
        ff0_id = ff0_components[0]
        
        ff1_components = self._create_simple_d_flip_flop_native(1, 20, "JFF1", clk_id, reset_id)
        if not all(ff1_components[:3]):
            logger.error("Failed to create FF1")
            return False
        ff1_id = ff1_components[0]
        
        ff2_components = self._create_simple_d_flip_flop_native(1, 40, "JFF2", clk_id, reset_id)
        if not all(ff2_components[:3]):
            logger.error("Failed to create FF2")
            return False
        ff2_id = ff2_components[0]
        
        # Create output LEDs for monitoring
        out0_x, out0_y = self._get_output_position(10, 0)
        out0_led = self.create_element("Led", out0_x, out0_y, "Q0")
        out1_x, out1_y = self._get_output_position(10, 1)
        out1_led = self.create_element("Led", out1_x, out1_y, "Q1")
        out2_x, out2_y = self._get_output_position(10, 2)
        out2_led = self.create_element("Led", out2_x, out2_y, "Q2")
        
        # Connect outputs to monitoring LEDs
        self.connect_elements(ff0_id, 0, out0_led, 0)  # FF0 Q -> LED0
        self.connect_elements(ff1_id, 0, out1_led, 0)  # FF1 Q -> LED1
        self.connect_elements(ff2_id, 0, out2_led, 0)  # FF2 Q -> LED2
        
        # Initialize with reset - CRITICAL STEP
        logger.info("🔧 Phase 1: Initializing with reset pulse")
        self.set_input(reset_id, True)
        self.set_input(clk_id, False)
        self.update_simulation()
        self.update_simulation()
        self.set_input(reset_id, False)
        self.update_simulation()
        
        # Apply restart_simulation for proper initialization
        self.restart_simulation()
        time.sleep(0.1)
        
        # Check initial state (should be 000 after reset)
        initial_q0 = self.get_output(out0_led)
        initial_q1 = self.get_output(out1_led)
        initial_q2 = self.get_output(out2_led)
        
        logger.info(f"After reset: Q2={initial_q2}, Q1={initial_q1}, Q0={initial_q0}")
        
        if initial_q0 or initial_q1 or initial_q2:
            logger.warning("⚠️ Initial state not 000 - applying additional reset")
            # Additional reset cycles if needed
            for i in range(2):
                self.set_input(reset_id, True)
                self.update_simulation()
                time.sleep(0.1)
                self.set_input(reset_id, False)
                self.update_simulation()
                time.sleep(0.1)
                
                check_q0 = self.get_output(out0_led)
                check_q1 = self.get_output(out1_led)
                check_q2 = self.get_output(out2_led)
                logger.info(f"After reset cycle {i+1}: Q2={check_q2}, Q1={check_q1}, Q0={check_q0}")
        
        # NOW make Johnson Counter feedback connections (AFTER initialization)
        logger.info("🔧 Phase 2: Creating Johnson Counter feedback connections")
        logger.info("Johnson Counter Logic:")
        logger.info("  D0 = ~Q2  (inverted feedback from last to first)")
        logger.info("  D1 = Q0   (shift: stage 0 -> stage 1)")
        logger.info("  D2 = Q1   (shift: stage 1 -> stage 2)")
        
        # Make feedback connections
        logger.info("Connecting FF2 ~Q (port 1) -> FF0 D (port 0) - INVERTED FEEDBACK")
        if not self.connect_elements(ff2_id, 1, ff0_id, 0):
            logger.error("Failed to connect FF2 ~Q -> FF0 D")
            return False
            
        logger.info("Connecting FF0 Q (port 0) -> FF1 D (port 0) - SHIFT")
        if not self.connect_elements(ff0_id, 0, ff1_id, 0):
            logger.error("Failed to connect FF0 Q -> FF1 D")
            return False
            
        logger.info("Connecting FF1 Q (port 0) -> FF2 D (port 0) - SHIFT")
        if not self.connect_elements(ff1_id, 0, ff2_id, 0):
            logger.error("Failed to connect FF1 Q -> FF2 D")
            return False
        
        logger.info("✅ All Johnson Counter feedback connections completed")
        
        # CRITICAL: Stabilization after feedback connections
        logger.info("🔧 Phase 3: Stabilizing feedback loops")
        for stab_cycle in range(5):
            self.update_simulation()
            time.sleep(0.1)
            
            stab_q0 = self.get_output(out0_led)
            stab_q1 = self.get_output(out1_led)
            stab_q2 = self.get_output(out2_led)
            logger.info(f"Stabilization {stab_cycle+1}: Q2={stab_q2}, Q1={stab_q1}, Q0={stab_q0}")
        
        # Test Johnson Counter sequence step by step
        logger.info("🔧 Phase 4: Testing Johnson Counter sequence")
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
                logger.info(f"\n🕐 Applying clock pulse {step}")
                
                # Detailed clock pulse analysis
                logger.info("  Before clock pulse:")
                pre_q0 = self.get_output(out0_led)
                pre_q1 = self.get_output(out1_led)
                pre_q2 = self.get_output(out2_led)
                logger.info(f"    Current state: Q2={pre_q2}, Q1={pre_q1}, Q0={pre_q0}")
                logger.info(f"    Expected D inputs: D0=~Q2=~{pre_q2}={not pre_q2}, D1=Q0={pre_q0}, D2=Q1={pre_q1}")
                
                # Apply clock pulse with detailed timing
                self.set_input(clk_id, True)
                time.sleep(0.15)  # Longer settling time
                self.update_simulation()
                
                during_q0 = self.get_output(out0_led)
                during_q1 = self.get_output(out1_led)
                during_q2 = self.get_output(out2_led)
                logger.info(f"  During clock high: Q2={during_q2}, Q1={during_q1}, Q0={during_q0}")
                
                self.set_input(clk_id, False)
                time.sleep(0.15)  # Longer settling time
                self.update_simulation()
            
            # Read final outputs
            actual_q0 = self.get_output(out0_led)
            actual_q1 = self.get_output(out1_led)
            actual_q2 = self.get_output(out2_led)
            
            logger.info(f"\n📊 Step {step}: {description}")
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
                    logger.error(f"    DIAGNOSIS: All outputs identical ({actual_q0}) - suggests feedback loop race condition")
                elif step == 1 and not actual_q0:
                    logger.error(f"    DIAGNOSIS: Q0 should be True from ~Q2=~False=True, but got {actual_q0}")
                    logger.error(f"    POSSIBLE CAUSE: Inverted feedback connection not working")
                else:
                    logger.error(f"    DIAGNOSIS: Partial shift register failure")
            
            results.append({
                "step": step,
                "expected": (exp_q0, exp_q1, exp_q2),
                "actual": (actual_q0, actual_q1, actual_q2),
                "correct": correct,
                "description": description
            })
        
        # Final analysis
        accuracy = (correct_count / len(expected_sequence)) * 100
        logger.info(f"\n🎯 JOHNSON COUNTER DEBUG RESULTS:")
        logger.info(f"   Correct steps: {correct_count}/{len(expected_sequence)}")
        logger.info(f"   Accuracy: {accuracy:.1f}%")
        
        if accuracy >= 80:
            logger.info("✅ Johnson Counter working correctly!")
        elif accuracy >= 40:
            logger.warning("⚠️ Johnson Counter partially working - feedback loop timing issues")
        else:
            logger.error("❌ Johnson Counter failed - fundamental feedback problem")
        
        # Save circuit for manual inspection
        self.save_circuit("debug_johnson_counter_detailed.panda")
        logger.info("💾 Debug circuit saved as: debug_johnson_counter_detailed.panda")
        
        return accuracy >= 60  # Accept partial success for feedback loops

def main():
    debugger = JohnsonCounterDetailedDebugger()
    
    try:
        success = debugger.debug_johnson_counter_step_by_step()
        
        if success:
            print("🎉 Johnson Counter detailed debug PASSED!")
        else:
            print("💥 Johnson Counter detailed debug FAILED!")
            print("   Check logs for detailed analysis of feedback loop behavior")
            
    except Exception as e:
        logger.error(f"Exception during Johnson Counter debug: {e}")
        print(f"💥 Exception: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()