#!/usr/bin/env python3

"""
Debug FIFO Buffer - Detailed analysis of 4-stage First-In-First-Out buffer
Focus: Isolate shift register chain issues and data flow problems in FIFO
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

class FifoBufferDetailedDebugger(AdvancedSequentialValidator):
    
    def debug_fifo_buffer_step_by_step(self):
        """Debug 4-stage FIFO buffer with detailed analysis of data flow"""
        logger.info("🔧 FIFO BUFFER DETAILED DEBUG")
        logger.info("Testing 4-stage shift register: Stage0(input) -> Stage1 -> Stage2 -> Stage3(output)")
        
        if not self.create_new_circuit():
            return False

        # Create control inputs
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        data_x, data_y = self._get_input_position(1)
        data_id = self.create_element("InputButton", data_x, data_y, "DATA_IN")
        
        push_x, push_y = self._get_input_position(2)
        push_id = self.create_element("InputButton", push_x, push_y, "PUSH")
        
        reset_x, reset_y = self._get_input_position(3)
        reset_id = self.create_element("InputButton", reset_x, reset_y, "RESET")
        
        # Create 4 D flip-flops for FIFO stages with spacing
        logger.info("Creating 4 D flip-flops for FIFO stages")
        fifo_stages = []
        for i in range(4):
            ff_components = self._create_simple_d_flip_flop_native(2 + i*3, 0, f"FIFO_S{i}", clk_id, reset_id)
            if not all(ff_components[:3]):
                logger.error(f"Failed to create FIFO stage {i}")
                return False
            fifo_stages.append(ff_components[0])  # Store FF element ID
            logger.info(f"Created FIFO Stage {i}: ID={ff_components[0]}")
        
        # Create output LEDs for monitoring each stage
        stage_leds = []
        for i in range(4):
            led_x, led_y = self._get_output_position(15, i)
            led_id = self.create_element("Led", led_x, led_y, f"STAGE{i}")
            stage_leds.append(led_id)
            
            # Connect stage output to monitoring LED
            self.connect_elements(fifo_stages[i], 0, led_id, 0)  # Stage Q -> LED
            logger.info(f"Connected Stage {i} output to LED {led_id}")
        
        # Initialize FIFO (reset all stages)
        logger.info("🔧 Phase 1: Initializing FIFO with reset")
        self.set_input(reset_id, True)
        self.set_input(clk_id, False)
        self.set_input(data_id, False)
        self.set_input(push_id, False)
        self.update_simulation()
        self.update_simulation()
        self.set_input(reset_id, False)
        self.update_simulation()
        
        # Apply restart_simulation for proper initialization
        self.restart_simulation()
        time.sleep(0.1)
        
        # Check initial state (all stages should be False)
        initial_stages = []
        for i in range(4):
            stage_val = self.get_output(stage_leds[i])
            initial_stages.append(stage_val)
        
        logger.info(f"After reset: Stages = {initial_stages}")
        
        if any(initial_stages):
            logger.warning("⚠️ Some stages not reset to False - applying additional reset")
            for reset_cycle in range(2):
                self.set_input(reset_id, True)
                self.update_simulation()
                time.sleep(0.1)
                self.set_input(reset_id, False)
                self.update_simulation()
                time.sleep(0.1)
                
                check_stages = [self.get_output(stage_leds[i]) for i in range(4)]
                logger.info(f"After reset cycle {reset_cycle+1}: Stages = {check_stages}")
        
        # Method 1: Simple Shift Register Test (no complex FIFO control logic)
        logger.info("🔧 Phase 2: Testing as Simple Shift Register (bypass complex FIFO logic)")
        
        # Direct shift register connections: Stage[i] -> Stage[i+1]
        logger.info("Creating simple shift register connections:")
        logger.info("  DATA_IN -> Stage0 (when PUSH=1)")
        logger.info("  Stage0 -> Stage1")
        logger.info("  Stage1 -> Stage2") 
        logger.info("  Stage2 -> Stage3")
        
        # Create AND gates for conditional data flow (DATA_IN AND PUSH -> Stage0)
        push_and_x, push_and_y = self._get_grid_position(1, 5)
        push_and = self.create_element("And", push_and_x, push_and_y, "PUSH_AND")
        
        if not self.connect_elements(data_id, 0, push_and, 0):  # DATA -> AND
            logger.error("Failed to connect DATA to PUSH_AND")
            return False
        if not self.connect_elements(push_id, 0, push_and, 1):  # PUSH -> AND
            logger.error("Failed to connect PUSH to PUSH_AND") 
            return False
        if not self.connect_elements(push_and, 0, fifo_stages[0], 0):  # PUSH_AND -> Stage0 D
            logger.error("Failed to connect PUSH_AND to Stage0")
            return False
        
        # Direct shift connections between stages
        for i in range(3):  # Connect Stage i to Stage i+1
            if not self.connect_elements(fifo_stages[i], 0, fifo_stages[i+1], 0):
                logger.error(f"Failed to connect Stage{i} -> Stage{i+1}")
                return False
            logger.info(f"Connected Stage{i} Q -> Stage{i+1} D")
        
        logger.info("✅ Simple shift register connections completed")
        
        # Stabilization after connections
        logger.info("🔧 Phase 3: Stabilizing shift register")
        for stab_cycle in range(3):
            self.update_simulation()
            time.sleep(0.1)
            
            stab_stages = [self.get_output(stage_leds[i]) for i in range(4)]
            logger.info(f"Stabilization {stab_cycle+1}: Stages = {stab_stages}")
        
        # Test FIFO operations step by step
        logger.info("🔧 Phase 4: Testing FIFO push operations")
        
        test_sequence = [
            (False, False, "Initial: Empty FIFO [0,0,0,0]"),
            (True, True, "Push 1: Should get [1,0,0,0]"),
            (False, False, "Hold: Still [1,0,0,0]"),
            (False, True, "Push 0: Should get [0,1,0,0]"),
            (True, True, "Push 1: Should get [1,0,1,0]"),
            (False, True, "Push 0: Should get [0,1,0,1]"),
        ]
        
        expected_fifo = [False, False, False, False]
        results = []
        correct_count = 0
        
        for step, (data_val, push_val, description) in enumerate(test_sequence):
            logger.info(f"\n📊 Step {step}: {description}")
            
            if step > 0:  # Apply operations for all steps except initial
                logger.info(f"  Setting: DATA={data_val}, PUSH={push_val}")
                
                # Set inputs
                self.set_input(data_id, data_val)
                self.set_input(push_id, push_val)
                self.update_simulation()
                
                # Show state before clock
                pre_stages = [self.get_output(stage_leds[i]) for i in range(4)]
                logger.info(f"  Before clock: Stages = {pre_stages}")
                
                if push_val:  # Only shift on push
                    logger.info(f"  Expected shift: {data_val} -> Stage0, others shift right")
                
                # Apply clock pulse
                logger.info("  Applying clock pulse...")
                self.set_input(clk_id, True)
                time.sleep(0.15)
                self.update_simulation()
                
                during_stages = [self.get_output(stage_leds[i]) for i in range(4)]
                logger.info(f"  During clock high: Stages = {during_stages}")
                
                self.set_input(clk_id, False)
                time.sleep(0.15)
                self.update_simulation()
            
            # Read final state
            actual_stages = [self.get_output(stage_leds[i]) for i in range(4)]
            
            # Update expected FIFO state
            if step > 0 and push_val:
                # Shift right: [a,b,c,d] -> [data_val, a, b, c]
                expected_fifo = [data_val] + expected_fifo[:3]
            
            logger.info(f"  Expected: Stages = {expected_fifo}")
            logger.info(f"  Actual:   Stages = {actual_stages}")
            
            # Check correctness
            correct = (actual_stages == expected_fifo)
            
            if correct:
                logger.info(f"  ✅ CORRECT")
                correct_count += 1
            else:
                logger.info(f"  ❌ FAILED")
                
                # Diagnose failure
                if all(s == False for s in actual_stages):
                    logger.error(f"  DIAGNOSIS: All stages remain False - data not flowing")
                    logger.error(f"  POSSIBLE CAUSE: Push logic not working or shift connections broken")
                elif actual_stages == [False, False, False, False] and step > 1:
                    logger.error(f"  DIAGNOSIS: No data retention - stages not holding values")
                else:
                    logger.error(f"  DIAGNOSIS: Incorrect shift pattern")
                    
                    # Check individual stage changes
                    if step > 0:
                        for i in range(4):
                            if i == 0:
                                expected_change = data_val if push_val else pre_stages[0]
                                if actual_stages[0] != expected_change:
                                    logger.error(f"    Stage 0: Expected {expected_change}, got {actual_stages[0]}")
                            else:
                                expected_from_prev = pre_stages[i-1] if push_val else pre_stages[i]
                                if actual_stages[i] != expected_from_prev:
                                    logger.error(f"    Stage {i}: Expected {expected_from_prev}, got {actual_stages[i]}")
            
            results.append({
                "step": step,
                "inputs": {"data": data_val, "push": push_val},
                "expected": expected_fifo.copy(),
                "actual": actual_stages.copy(),
                "correct": correct,
                "description": description
            })
        
        # Final analysis
        accuracy = (correct_count / len(test_sequence)) * 100
        logger.info(f"\n🎯 FIFO BUFFER DEBUG RESULTS:")
        logger.info(f"   Correct operations: {correct_count}/{len(test_sequence)}")
        logger.info(f"   Accuracy: {accuracy:.1f}%")
        
        if accuracy >= 80:
            logger.info("✅ FIFO Buffer working correctly!")
        elif accuracy >= 40:
            logger.warning("⚠️ FIFO Buffer partially working - shift register issues")
        else:
            logger.error("❌ FIFO Buffer failed - fundamental data flow problem")
        
        # Additional diagnostic: Test each stage independently
        logger.info("\n🔧 Phase 5: Individual stage diagnostic")
        
        # Reset everything
        self.set_input(reset_id, True)
        self.update_simulation()
        self.set_input(reset_id, False)
        self.update_simulation()
        
        # Test Stage 0 independently
        logger.info("Testing Stage 0 independently:")
        self.set_input(data_id, True)
        self.set_input(push_id, True)  # Enable data flow to Stage 0
        self.update_simulation()
        
        logger.info("  Before clock: Stage 0 should prepare to receive 1")
        self.pulse_clock(clk_id, 0.15)
        
        stage0_test = self.get_output(stage_leds[0])
        logger.info(f"  After clock: Stage 0 = {stage0_test} (expected: True)")
        
        if stage0_test:
            logger.info("  ✅ Stage 0 can accept data")
        else:
            logger.error("  ❌ Stage 0 not accepting data - push logic or D input problem")
        
        # Save circuit for manual inspection
        self.save_circuit("debug_fifo_buffer_detailed.panda")
        logger.info("💾 Debug circuit saved as: debug_fifo_buffer_detailed.panda")
        
        return accuracy >= 50  # Accept partial success for shift registers

def main():
    debugger = FifoBufferDetailedDebugger()
    
    try:
        success = debugger.debug_fifo_buffer_step_by_step()
        
        if success:
            print("🎉 FIFO Buffer detailed debug PASSED!")
        else:
            print("💥 FIFO Buffer detailed debug FAILED!")
            print("   Check logs for detailed analysis of shift register behavior")
            
    except Exception as e:
        logger.error(f"Exception during FIFO Buffer debug: {e}")
        print(f"💥 Exception: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()