#!/usr/bin/env python3

"""
Debug Minimal Convergence - Focused test for convergence system
Focus: Create minimal feedback circuit to test convergence step-by-step
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

class MinimalConvergenceDebugger(AdvancedSequentialValidator):
    
    def debug_minimal_convergence_step_by_step(self):
        """Debug minimal convergence circuit with step-by-step analysis"""
        logger.info("🔧 MINIMAL CONVERGENCE STEP-BY-STEP DEBUG")
        logger.info("Creating simplest possible feedback circuit to test convergence")
        
        if not self.create_new_circuit():
            return False

        # Step 1: Create minimal feedback circuit
        logger.info("📍 STEP 1: Creating minimal feedback circuit")
        
        # Create clock input
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        # Create two D flip-flops for minimal feedback
        ff1_x, ff1_y = self._get_grid_position(3, 1)
        ff1_id = self.create_element("DFlipFlop", ff1_x, ff1_y, "FF1")
        
        ff2_x, ff2_y = self._get_grid_position(3, 3)  
        ff2_id = self.create_element("DFlipFlop", ff2_x, ff2_y, "FF2")
        
        # Create output LEDs
        out1_x, out1_y = self._get_output_position(6, 0)
        out1_led = self.create_element("Led", out1_x, out1_y, "Q1")
        
        out2_x, out2_y = self._get_output_position(6, 1)
        out2_led = self.create_element("Led", out2_x, out2_y, "Q2")
        
        logger.info(f"✅ Created elements: CLK={clk_id}, FF1={ff1_id}, FF2={ff2_id}")
        
        # Step 2: Connect clocks (no feedback yet)
        logger.info("📍 STEP 2: Connecting clocks")
        
        # Connect clocks to both flip-flops
        if not self.connect_elements(clk_id, 0, ff1_id, 1):  # CLK -> FF1 clock
            logger.error("❌ Failed to connect CLK -> FF1")
            return False
        if not self.connect_elements(clk_id, 0, ff2_id, 1):  # CLK -> FF2 clock
            logger.error("❌ Failed to connect CLK -> FF2")
            return False
        
        # Connect outputs to LEDs
        if not self.connect_elements(ff1_id, 0, out1_led, 0):  # FF1 Q -> LED1
            logger.error("❌ Failed to connect FF1 -> LED1")
            return False
        if not self.connect_elements(ff2_id, 0, out2_led, 0):  # FF2 Q -> LED2
            logger.error("❌ Failed to connect FF2 -> LED2")
            return False
        
        logger.info("✅ Connected clocks and outputs")
        
        # Step 3: Test without feedback (should work like single DFlipFlop)
        logger.info("📍 STEP 3: Testing without feedback")
        
        # Trigger ElementMapping rebuild
        self.trigger_circuit_update()
        
        # Initialize
        self.set_input(clk_id, False)
        self.update_simulation()
        
        initial_q1 = self.get_output(out1_led)
        initial_q2 = self.get_output(out2_led)
        logger.info(f"Initial state: Q1={initial_q1}, Q2={initial_q2}")
        
        # Test clock pulse without feedback
        self.set_input(clk_id, True)
        self.update_simulation()
        
        after_clock_q1 = self.get_output(out1_led)
        after_clock_q2 = self.get_output(out2_led)
        logger.info(f"After clock (no feedback): Q1={after_clock_q1}, Q2={after_clock_q2}")
        logger.info("Expected: Should remain same since D inputs are floating")
        
        # Step 4: Add minimal feedback (FF1 -> FF2, FF2 -> FF1)
        logger.info("📍 STEP 4: Adding minimal feedback loop")
        
        # Create feedback connections: FF1 Q -> FF2 D, FF2 ~Q -> FF1 D
        if not self.connect_elements(ff1_id, 0, ff2_id, 0):  # FF1 Q -> FF2 D
            logger.error("❌ Failed to connect FF1 -> FF2")
            return False
        logger.info("✅ Connected FF1 Q -> FF2 D")
        
        if not self.connect_elements(ff2_id, 1, ff1_id, 0):  # FF2 ~Q -> FF1 D
            logger.error("❌ Failed to connect FF2 ~Q -> FF1 D")
            return False
        logger.info("✅ Connected FF2 ~Q -> FF1 D (creates feedback loop)")
        
        # Step 5: Trigger convergence rebuild and analyze
        logger.info("📍 STEP 5: Triggering convergence system rebuild")
        self.trigger_circuit_update()
        
        # Reset to known state
        self.set_input(clk_id, False)
        self.update_simulation()
        
        feedback_initial_q1 = self.get_output(out1_led)
        feedback_initial_q2 = self.get_output(out2_led)
        logger.info(f"With feedback initial: Q1={feedback_initial_q1}, Q2={feedback_initial_q2}")
        
        # Step 6: Test convergence behavior
        logger.info("📍 STEP 6: Testing convergence behavior")
        
        # Apply clock pulse and watch convergence
        logger.info("Applying clock pulse with feedback...")
        self.set_input(clk_id, True)
        self.update_simulation()
        
        convergence_q1 = self.get_output(out1_led)
        convergence_q2 = self.get_output(out2_led)
        logger.info(f"After convergence clock: Q1={convergence_q1}, Q2={convergence_q2}")
        
        # Step 7: Comprehensive Sequential Feedback Timing Analysis
        logger.info("📍 STEP 7: Sequential Feedback Timing Analysis")
        logger.info("Testing hypothesis: Sequential elements use OLD values instead of CURRENT values")
        
        # Initialize to known state
        logger.info("\n🔧 Initializing to known state...")
        self.set_input(clk_id, False)
        self.update_simulation()
        
        initial_q1 = self.get_output(out1_led)
        initial_q2 = self.get_output(out2_led)
        logger.info(f"Initial state: FF1_Q={initial_q1}, FF2_Q={initial_q2}")
        
        # Test sequential feedback timing over extended cycles
        logger.info("\n🔧 Testing Sequential Feedback Timing (Ring Counter)")
        logger.info("Expected sequence for 2-FF ring counter with inversion:")
        logger.info("  State 0: FF1=F, FF2=F → FF1_D=~FF2_Q=T, FF2_D=FF1_Q=F → Next: FF1=T, FF2=F")
        logger.info("  State 1: FF1=T, FF2=F → FF1_D=~FF2_Q=T, FF2_D=FF1_Q=T → Next: FF1=T, FF2=T") 
        logger.info("  State 2: FF1=T, FF2=T → FF1_D=~FF2_Q=F, FF2_D=FF1_Q=T → Next: FF1=F, FF2=T")
        logger.info("  State 3: FF1=F, FF2=T → FF1_D=~FF2_Q=F, FF2_D=FF1_Q=F → Next: FF1=F, FF2=F")
        logger.info("  Should repeat: (F,F) → (T,F) → (T,T) → (F,T) → (F,F)...")
        
        expected_sequence = [
            (False, False, "Initial/Reset"),
            (True, False, "FF1 gets ~FF2=T, FF2 gets FF1=F"), 
            (True, True, "FF1 gets ~FF2=T, FF2 gets FF1=T"),
            (False, True, "FF1 gets ~FF2=F, FF2 gets FF1=T"),
            (False, False, "FF1 gets ~FF2=F, FF2 gets FF1=F - Back to start")
        ]
        
        states = []
        timing_analysis = []
        
        for cycle in range(7):  # Extended cycle test to show pattern
            logger.info(f"\n=== CLOCK CYCLE {cycle + 1} ===")
            
            # Read current state before clock
            pre_q1 = self.get_output(out1_led)
            pre_q2 = self.get_output(out2_led)
            
            # Calculate expected D inputs based on current outputs
            expected_ff1_d = not pre_q2  # FF1 D = ~FF2 Q
            expected_ff2_d = pre_q1      # FF2 D = FF1 Q
            
            logger.info(f"Pre-clock: FF1_Q={pre_q1}, FF2_Q={pre_q2}")
            logger.info(f"Expected D inputs: FF1_D=~FF2_Q=~{pre_q2}={expected_ff1_d}, FF2_D=FF1_Q={pre_q1}")
            
            # Expected next state
            expected_next_q1 = expected_ff1_d
            expected_next_q2 = expected_ff2_d
            logger.info(f"Expected next state: FF1_Q={expected_next_q1}, FF2_Q={expected_next_q2}")
            
            # Apply clock pulse and measure actual result
            logger.info("Applying clock pulse...")
            self.pulse_clock(clk_id, 0.1)
            
            # Read actual result
            post_q1 = self.get_output(out1_led)
            post_q2 = self.get_output(out2_led)
            logger.info(f"Actual result: FF1_Q={post_q1}, FF2_Q={post_q2}")
            
            # Analyze timing correctness
            ff1_correct = (post_q1 == expected_next_q1)
            ff2_correct = (post_q2 == expected_next_q2)
            cycle_correct = ff1_correct and ff2_correct
            
            logger.info(f"Correctness: FF1 {'✅' if ff1_correct else '❌'}, FF2 {'✅' if ff2_correct else '❌'}, Overall {'✅' if cycle_correct else '❌'}")
            
            # Check for expected sequence progression
            current_state = (post_q1, post_q2)
            expected_index = cycle % len(expected_sequence)
            expected_state = expected_sequence[expected_index][:2]
            sequence_correct = (current_state == expected_state)
            
            if cycle < len(expected_sequence):
                logger.info(f"Sequence check: Expected {expected_state} ({'✅' if sequence_correct else '❌'})")
                logger.info(f"  Context: {expected_sequence[expected_index][2]}")
            
            # Detect oscillation vs progression
            if cycle >= 2:
                # Check if we're stuck in 2-state oscillation
                prev_state = states[-2] if len(states) >= 2 else None
                if prev_state and current_state == prev_state and current_state != states[-1]:
                    logger.info("🚨 DETECTED 2-STATE OSCILLATION (Bug symptom!)")
            
            states.append(current_state)
            timing_analysis.append({
                "cycle": cycle + 1,
                "pre_state": (pre_q1, pre_q2),
                "expected_d": (expected_ff1_d, expected_ff2_d),
                "post_state": current_state,
                "expected_next": (expected_next_q1, expected_next_q2),
                "correct": cycle_correct,
                "sequence_correct": sequence_correct if cycle < len(expected_sequence) else None
            })
            
            logger.info("")  # Blank line for readability
        
        # Comprehensive Analysis
        logger.info("📊 SEQUENTIAL FEEDBACK TIMING ANALYSIS:")
        logger.info(f"Total cycles tested: {len(states)}")
        
        correct_cycles = sum(1 for analysis in timing_analysis if analysis["correct"])
        accuracy = (correct_cycles / len(timing_analysis)) * 100
        logger.info(f"Timing accuracy: {correct_cycles}/{len(timing_analysis)} = {accuracy:.1f}%")
        
        # Analyze state progression pattern
        unique_states = list(set(states))
        logger.info(f"Unique states observed: {len(unique_states)} {unique_states}")
        logger.info(f"Expected unique states: 4 [(False, False), (True, False), (True, True), (False, True)]")
        
        # Detect oscillation pattern
        if len(states) >= 4:
            # Check last 4 states for oscillation
            last_4 = states[-4:]
            if len(set(last_4)) == 2 and last_4[0] == last_4[2] and last_4[1] == last_4[3]:
                logger.info("🚨 CONFIRMED: 2-STATE OSCILLATION PATTERN")
                logger.info(f"   Oscillating between: {last_4[0]} ↔ {last_4[1]}")
                logger.info("   ROOT CAUSE: Sequential elements using OLD interconnection values")
            elif len(set(last_4)) == 4:
                logger.info("✅ PROPER: 4-STATE PROGRESSION PATTERN")  
                logger.info("   Sequential feedback timing working correctly")
            else:
                logger.info(f"⚠️  IRREGULAR: Pattern with {len(set(last_4))} states in last 4 cycles")
        
        # Expected vs Actual comparison
        logger.info("\n📈 STATE SEQUENCE ANALYSIS:")
        for i, analysis in enumerate(timing_analysis[:5]):  # First 5 cycles
            expected_state = expected_sequence[i][:2] if i < len(expected_sequence) else "N/A"
            actual_state = analysis["post_state"]
            match = "✅" if expected_state == actual_state else "❌"
            logger.info(f"  Cycle {i+1}: Expected {expected_state} → Actual {actual_state} {match}")
        
        # Root cause validation
        logger.info("\n🔍 ROOT CAUSE VALIDATION:")
        logger.info("If bug is present, we should see:")
        logger.info("  ❌ 2-state oscillation instead of 4-state progression")
        logger.info("  ❌ Both flip-flops receiving identical D values simultaneously")
        logger.info("  ❌ Values based on OLD outputs instead of CURRENT outputs")
        
        # Set final states for Step 8 analysis
        states = states[:5]  # Keep first 5 for comparison
        
        # Step 8: Analysis
        logger.info("📍 STEP 8: Detailed Convergence Analysis")
        
        # Check for expected toggle behavior (since FF2 ~Q -> FF1 D, FF1 Q -> FF2 D)
        # This should create a 2-bit ring counter: 00 -> 01 -> 10 -> 00 -> 01...
        expected_sequence = [
            (False, False),  # Initial
            (True, False),   # FF1 gets ~FF2 = ~False = True
            (True, True),    # FF2 gets FF1 = True  
            (False, True),   # FF1 gets ~FF2 = ~True = False
            (False, False)   # FF2 gets FF1 = False
        ]
        
        logger.info("🎯 CONVERGENCE ANALYSIS:")
        correct_transitions = 0
        
        for i, ((actual_q1, actual_q2), (exp_q1, exp_q2)) in enumerate(zip(states, expected_sequence)):
            if actual_q1 == exp_q1 and actual_q2 == exp_q2:
                logger.info(f"✅ Cycle {i + 1} CORRECT: Q1={actual_q1}, Q2={actual_q2}")
                correct_transitions += 1
            else:
                logger.info(f"❌ Cycle {i + 1} FAILED: Expected Q1={exp_q1}, Q2={exp_q2}, Got Q1={actual_q1}, Q2={actual_q2}")
        
        accuracy = (correct_transitions / len(expected_sequence)) * 100
        logger.info(f"🎯 MINIMAL CONVERGENCE RESULTS:")
        logger.info(f"   Correct transitions: {correct_transitions}/{len(expected_sequence)}")
        logger.info(f"   Accuracy: {accuracy:.1f}%")
        
        if accuracy >= 60:
            logger.info("✅ Minimal convergence working!")
            return True
        else:
            logger.error("❌ Minimal convergence failed")
            return False

def main():
    debugger = MinimalConvergenceDebugger()
    
    try:
        success = debugger.debug_minimal_convergence_step_by_step()
        
        if success:
            print("🎉 Minimal convergence debug PASSED!")
        else:
            print("💥 Minimal convergence debug FAILED!")
            print("   Check step-by-step analysis above")
            
    except Exception as e:
        logger.error(f"Exception during minimal convergence debug: {e}")
        print(f"💥 Exception: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()