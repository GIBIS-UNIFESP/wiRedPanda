#!/usr/bin/env python3

"""
Debug FIFO Complex Implementation - Test the exact complex FIFO architecture from Level 4
Focus: Replicate the full FIFO implementation with all AND gates, OR gates, and complex logic paths
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

class FifoComplexImplementationDebugger(AdvancedSequentialValidator):
    
    def debug_complex_fifo_implementation(self):
        """Debug the exact complex FIFO implementation from Level 4 with detailed analysis"""
        logger.info("🔧 COMPLEX FIFO DEBUG - Exact Level 4 Implementation")
        logger.info("Testing full complexity: 4 stages, AND gates, OR gates, push/pop logic")
        
        if not self.create_new_circuit():
            return False

        # Create control inputs - EXACT same as Level 4
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        push_x, push_y = self._get_input_position(1)
        push_id = self.create_element("InputButton", push_x, push_y, "PUSH")
        
        pop_x, pop_y = self._get_input_position(2)
        pop_id = self.create_element("InputButton", pop_x, pop_y, "POP")
        
        data_in_x, data_in_y = self._get_input_position(3)
        data_in_id = self.create_element("InputButton", data_in_x, data_in_y, "DATA_IN")
        
        reset_x, reset_y = self._get_input_position(4)
        reset_id = self.create_element("InputButton", reset_x, reset_y, "RESET")

        # Create 4 FIXED native D flip-flops for 4-stage FIFO - EXACT same as Level 4
        logger.info("Creating 4 D flip-flops for FIFO stages - exact Level 4 implementation")
        stage_ffs = []
        for i in range(4):
            ff_components = self._create_simple_d_flip_flop_native(2 + i*2, 0, f"FIFO_STAGE{i}", clk_id, reset_id)
            if not all(ff_components[:3]):
                logger.error(f"Failed to create FIFO stage {i}")
                return False
            stage_ffs.append(ff_components[0])  # Store element ID only
            logger.info(f"Created FIFO Stage {i}: ID={ff_components[0]}")
        
        # FIFO Control Logic - EXACT same complexity as Level 4
        logger.info("Creating FIFO control logic - push/pop AND gates and data OR gates")
        
        # Create control logic gates for push/pop operations - EXACT same as Level 4
        push_and_gates = []
        pop_and_gates = []
        
        for i in range(4):
            # Push control: PUSH AND DATA for each stage
            push_and_x, push_and_y = self._get_grid_position(1, i + 4)
            push_and = self.create_element("And", push_and_x, push_and_y, f"PUSH_AND{i}")
            push_and_gates.append(push_and)
            logger.info(f"Created PUSH_AND{i}: ID={push_and}")
            
            # Pop control: POP AND STAGE_DATA for each stage  
            pop_and_x, pop_and_y = self._get_grid_position(1, i + 8)
            pop_and = self.create_element("And", pop_and_x, pop_and_y, f"POP_AND{i}")
            pop_and_gates.append(pop_and)
            logger.info(f"Created POP_AND{i}: ID={pop_and}")

        # Create OR gates for combining push/pop data paths - EXACT same as Level 4
        data_or_gates = []
        for i in range(4):
            or_x, or_y = self._get_grid_position(0, i + 12)
            data_or = self.create_element("Or", or_x, or_y, f"DATA_OR{i}")
            data_or_gates.append(data_or)
            logger.info(f"Created DATA_OR{i}: ID={data_or}")

        # Check all control elements created - EXACT same as Level 4
        all_control_elements = push_and_gates + pop_and_gates + data_or_gates
        if not all(all_control_elements):
            failed_count = sum(1 for el in all_control_elements if el is None)
            logger.error(f"Failed to create {failed_count} FIFO control elements")
            return False
        
        logger.info("✅ All FIFO control elements created successfully")

        # FIFO DATA PATH CONNECTIONS - EXACT same as Level 4
        logger.info("🔧 Making FIFO data path connections - exact Level 4 implementation")
        
        # Stage 0 (Input stage): Gets data from input or from stage 1 (pop operation)
        logger.info("Connecting Stage 0 input logic:")
        logger.info("  DATA_IN -> PUSH_AND0 input 0")
        if not self.connect_elements(data_in_id, 0, push_and_gates[0], 0):  # DATA_IN -> PUSH_AND0
            logger.error("Failed to connect DATA_IN to PUSH_AND0")
            return False
        
        logger.info("  PUSH -> PUSH_AND0 input 1")
        if not self.connect_elements(push_id, 0, push_and_gates[0], 1):     # PUSH -> PUSH_AND0
            logger.error("Failed to connect PUSH to PUSH_AND0")
            return False
        
        # Stage 1: Gets data from stage 0 Q output (push) or from stage 2 (pop)
        if len(stage_ffs) > 1:
            logger.info("Connecting Stage 1 shift logic:")
            logger.info("  STAGE0 Q -> PUSH_AND1 input 0")
            if not self.connect_elements(stage_ffs[0], 0, push_and_gates[1], 0):  # STAGE0_Q -> PUSH_AND1
                logger.error("Failed to connect STAGE0 Q to PUSH_AND1")
                return False
            
            logger.info("  PUSH -> PUSH_AND1 input 1")
            if not self.connect_elements(push_id, 0, push_and_gates[1], 1):   # PUSH -> PUSH_AND1  
                logger.error("Failed to connect PUSH to PUSH_AND1")
                return False
        
        # Stage 2: Gets data from stage 1 Q output (push) or from stage 3 (pop)
        if len(stage_ffs) > 2:
            logger.info("Connecting Stage 2 shift logic:")
            logger.info("  STAGE1 Q -> PUSH_AND2 input 0")
            if not self.connect_elements(stage_ffs[1], 0, push_and_gates[2], 0):  # STAGE1_Q -> PUSH_AND2
                logger.error("Failed to connect STAGE1 Q to PUSH_AND2")
                return False
            
            logger.info("  PUSH -> PUSH_AND2 input 1")
            if not self.connect_elements(push_id, 0, push_and_gates[2], 1):   # PUSH -> PUSH_AND2
                logger.error("Failed to connect PUSH to PUSH_AND2")
                return False
        
        # Stage 3 (Output stage): Gets data from stage 2 Q output (push only, no pop source)
        if len(stage_ffs) > 3:
            logger.info("Connecting Stage 3 shift logic:")
            logger.info("  STAGE2 Q -> PUSH_AND3 input 0")
            if not self.connect_elements(stage_ffs[2], 0, push_and_gates[3], 0):  # STAGE2_Q -> PUSH_AND3
                logger.error("Failed to connect STAGE2 Q to PUSH_AND3")
                return False
            
            logger.info("  PUSH -> PUSH_AND3 input 1")
            if not self.connect_elements(push_id, 0, push_and_gates[3], 1):   # PUSH -> PUSH_AND3
                logger.error("Failed to connect PUSH to PUSH_AND3")
                return False

        # Connect OR gates to flip-flop D inputs - EXACT same as Level 4
        logger.info("Connecting OR gates to flip-flop D inputs:")
        for i in range(4):
            logger.info(f"  PUSH_AND{i} -> DATA_OR{i} input 0")
            if not self.connect_elements(push_and_gates[i], 0, data_or_gates[i], 0):
                logger.error(f"Failed to connect PUSH_AND{i} to DATA_OR{i}")
                return False
            
            # Note: Pop logic would connect to OR gate input 1, but simplified for now
            logger.info(f"  DATA_OR{i} -> STAGE{i} D input")
            if not self.connect_elements(data_or_gates[i], 0, stage_ffs[i], 0):  # OR gate -> D input (port 0)
                logger.error(f"Failed to connect DATA_OR{i} to STAGE{i} D input")
                return False

        # Output displays for FIFO contents - EXACT same as Level 4
        logger.info("Creating output displays for FIFO monitoring:")
        output_leds = []
        for i in range(4):
            out_x, out_y = self._get_output_position(12, i)
            out_led = self.create_element("Led", out_x, out_y, f"FIFO_STAGE{i}")
            output_leds.append(out_led)
            
            if not self.connect_elements(stage_ffs[i], 0, out_led, 0):  # Q output (port 0) -> LED
                logger.error(f"Failed to connect STAGE{i} output to LED")
                return False
            
            logger.info(f"Connected STAGE{i} Q -> LED{i}")

        # Data output (from stage 3) - EXACT same as Level 4
        data_out_x, data_out_y = self._get_output_position(12, 4)
        data_out_led = self.create_element("Led", data_out_x, data_out_y, "DATA_OUT")
        if not self.connect_elements(stage_ffs[3], 0, data_out_led, 0):  # Stage 3 Q output -> DATA_OUT LED
            logger.error("Failed to connect FIFO DATA_OUT")
            return False
        
        logger.info("Connected STAGE3 Q -> DATA_OUT LED")
        logger.info("✅ Complex FIFO architecture completed - exact Level 4 implementation")

        # Initialize with ENHANCED stabilization - EXACT same as Level 4
        logger.info("🔧 Phase 1: Initializing complex FIFO with enhanced stabilization")
        self.set_input(clk_id, False)
        self.set_input(push_id, False)
        self.set_input(pop_id, False)
        self.set_input(data_in_id, False)
        self.restart_simulation()
        time.sleep(0.1)
        
        # ENHANCED stabilization for complex logic chains
        logger.info("Applying enhanced stabilization for complex FIFO logic chains")
        for stabilize_cycle in range(5):  # More cycles for complex logic
            self.update_simulation()
            time.sleep(0.1)
            
            # Monitor all stages during stabilization
            stage_states = []
            for i in range(4):
                stage_val = self.get_output(output_leds[i])
                stage_states.append(stage_val)
            
            logger.info(f"Stabilization cycle {stabilize_cycle + 1}: Stages={stage_states}")
        
        # Test FIFO operations - EXACT same sequence as Level 4
        logger.info("🔧 Phase 2: Testing complex FIFO operations")
        
        test_sequence = [
            # (data_in, push, pop, description)
            (False, False, False, "Initial: Empty FIFO"),
            (True, True, False, "Push 1 into FIFO"),
            (False, False, False, "Hold: FIFO contains [1,0,0,0]"),
            (False, True, False, "Push 0 into FIFO"),  
            (True, True, False, "Push 1 into FIFO"),
            (False, True, False, "Push 0 into FIFO - FIFO full [0,1,0,1]"),
        ]

        results = []
        correct_count = 0
        fifo_contents = [False, False, False, False]  # Track expected FIFO state

        for step, (data, push, pop, description) in enumerate(test_sequence):
            logger.info(f"\n📊 FIFO Test {step + 1}: {description}")
            logger.info(f"Setting inputs: DATA_IN={data}, PUSH={push}, POP={pop}")
            
            if step > 0:  # Apply operations for all steps except initial
                # Set inputs
                self.set_input(data_in_id, data)
                self.set_input(push_id, push)
                self.set_input(pop_id, pop)
                self.update_simulation()
                
                # Show expected logic flow
                if push:
                    logger.info("Expected logic flow:")
                    logger.info(f"  Stage 0: DATA_IN={data} AND PUSH={push} -> PUSH_AND0={data and push}")
                    logger.info(f"  Stage 1: STAGE0_Q AND PUSH -> PUSH_AND1=(prev_stage0 and push)")
                    logger.info(f"  Stage 2: STAGE1_Q AND PUSH -> PUSH_AND2=(prev_stage1 and push)")
                    logger.info(f"  Stage 3: STAGE2_Q AND PUSH -> PUSH_AND3=(prev_stage2 and push)")
                
                # Read states before clock
                before_stages = []
                for i in range(4):
                    stage_val = self.get_output(output_leds[i])
                    before_stages.append(stage_val)
                logger.info(f"Before clock: Stages={before_stages}")
                
                # Apply clock pulse
                logger.info("Applying clock pulse...")
                self.set_input(clk_id, True)
                time.sleep(0.2)  # Longer settling for complex logic
                self.update_simulation()
                
                during_stages = []
                for i in range(4):
                    stage_val = self.get_output(output_leds[i])
                    during_stages.append(stage_val)
                logger.info(f"During clock high: Stages={during_stages}")
                
                self.set_input(clk_id, False)
                time.sleep(0.2)  # Longer settling for complex logic
                self.update_simulation()
            
            # Read final states
            actual_stages = []
            for i in range(4):
                stage_val = self.get_output(output_leds[i])
                actual_stages.append(stage_val)
            
            # Update expected FIFO state using exact Level 4 logic
            if step > 0 and push:
                # FIFO push operation: shift data right
                # [a,b,c,d] + push(data) -> [data, a, b, c]
                fifo_contents = [data] + fifo_contents[:3]
            
            logger.info(f"Expected: Stages={fifo_contents}")
            logger.info(f"Actual:   Stages={actual_stages}")
            
            # Check correctness
            correct = (actual_stages == fifo_contents)
            
            if correct:
                logger.info("✅ CORRECT")
                correct_count += 1
            else:
                logger.error("❌ FAILED")
                
                # Detailed diagnosis for complex FIFO failure
                logger.error("🔍 DETAILED DIAGNOSIS:")
                
                if all(s == False for s in actual_stages):
                    logger.error("  All stages remain False - complex logic chain not propagating data")
                    logger.error("  POSSIBLE CAUSES:")
                    logger.error("    1. PUSH_AND gates not receiving correct inputs")
                    logger.error("    2. PUSH_AND gate logic not working")
                    logger.error("    3. DATA_OR gates not combining signals correctly")
                    logger.error("    4. OR gate to flip-flop D input connections broken")
                    logger.error("    5. Complex timing issues in multi-gate logic chains")
                elif actual_stages[0] == False and push:
                    logger.error("  Stage 0 not accepting input - first stage of complex logic failed")
                    logger.error("  DIAGNOSIS: DATA_IN AND PUSH logic not reaching Stage 0")
                else:
                    logger.error("  Partial propagation - some stages working, others failing")
                    
                    # Check each stage individually
                    for i in range(4):
                        if i == 0:
                            expected_stage = data if (step > 0 and push) else fifo_contents[0]
                        else:
                            expected_stage = fifo_contents[i]
                        
                        if actual_stages[i] != expected_stage:
                            logger.error(f"    Stage {i}: Expected {expected_stage}, got {actual_stages[i]}")
                            if i == 0:
                                logger.error(f"      Issue: DATA_IN AND PUSH -> OR -> D logic chain")
                            else:
                                logger.error(f"      Issue: Stage {i-1} Q AND PUSH -> OR -> D logic chain")
            
            results.append({
                "step": step,
                "inputs": {"data": data, "push": push, "pop": pop},
                "expected_fifo": fifo_contents.copy(),
                "actual_fifo": actual_stages.copy(),
                "correct": correct,
                "description": description
            })
        
        # Final analysis
        accuracy = (correct_count / len(test_sequence)) * 100
        logger.info(f"\n🎯 COMPLEX FIFO DEBUG RESULTS:")
        logger.info(f"   Correct operations: {correct_count}/{len(test_sequence)}")
        logger.info(f"   Accuracy: {accuracy:.1f}%")
        
        if accuracy >= 80:
            logger.info("✅ Complex FIFO working correctly!")
        elif accuracy >= 40:
            logger.warning("⚠️ Complex FIFO partially working - multi-gate logic issues")
        else:
            logger.error("❌ Complex FIFO failed - fundamental complex logic chain problem")
        
        # Save circuit for detailed inspection
        self.save_circuit("debug_fifo_complex_implementation.panda")
        logger.info("💾 Debug circuit saved as: debug_fifo_complex_implementation.panda")
        
        return accuracy >= 50

def main():
    debugger = FifoComplexImplementationDebugger()
    
    try:
        success = debugger.debug_complex_fifo_implementation()
        
        if success:
            print("🎉 Complex FIFO implementation debug PASSED!")
        else:
            print("💥 Complex FIFO implementation debug FAILED!")
            print("   The issue is in the complex multi-gate logic chains")
            print("   Check logs for detailed analysis of each logic stage")
            
    except Exception as e:
        logger.error(f"Exception during complex FIFO debug: {e}")
        print(f"💥 Exception: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()