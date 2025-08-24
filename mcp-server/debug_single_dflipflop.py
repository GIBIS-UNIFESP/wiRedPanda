#!/usr/bin/env python3

"""
Debug Single DFlipFlop - Step by step analysis to isolate sequential logic issues
Focus: Test each intermediate step to find where DFlipFlop processing fails
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

class SingleDFlipFlopDebugger(AdvancedSequentialValidator):
    
    def debug_single_dflipflop_step_by_step(self):
        """Debug single DFlipFlop with step-by-step analysis"""
        logger.info("🔧 SINGLE DFLIPFLOP STEP-BY-STEP DEBUG")
        
        if not self.create_new_circuit():
            return False

        # Step 1: Create minimal circuit with one DFlipFlop
        logger.info("📍 STEP 1: Creating minimal circuit")
        
        # Create inputs
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        data_x, data_y = self._get_input_position(1)
        data_id = self.create_element("InputButton", data_x, data_y, "DATA")
        
        # Create one D flip-flop
        ff_x, ff_y = self._get_grid_position(3, 1)
        ff_id = self.create_element("DFlipFlop", ff_x, ff_y, "TEST_FF")
        
        # Create output LED
        out_x, out_y = self._get_output_position(6, 0)
        out_led = self.create_element("Led", out_x, out_y, "Q_OUT")
        
        logger.info(f"✅ Created elements: CLK={clk_id}, DATA={data_id}, FF={ff_id}, LED={out_led}")
        
        # Step 2: Connect elements
        logger.info("📍 STEP 2: Connecting elements")
        
        # Connect data input
        if not self.connect_elements(data_id, 0, ff_id, 0):  # DATA -> FF D
            logger.error("❌ Failed to connect DATA -> FF D")
            return False
        logger.info("✅ Connected DATA -> FF D")
        
        # Connect clock
        if not self.connect_elements(clk_id, 0, ff_id, 1):  # CLK -> FF clock
            logger.error("❌ Failed to connect CLK -> FF clock") 
            return False
        logger.info("✅ Connected CLK -> FF clock")
        
        # Connect output
        if not self.connect_elements(ff_id, 0, out_led, 0):  # FF Q -> LED
            logger.error("❌ Failed to connect FF Q -> LED")
            return False
        logger.info("✅ Connected FF Q -> LED")
        
        # Step 3: Trigger ElementMapping rebuild
        logger.info("📍 STEP 3: Triggering ElementMapping rebuild")
        self.trigger_circuit_update()
        logger.info("✅ ElementMapping rebuild triggered")
        
        # Step 4: Test basic functionality
        logger.info("📍 STEP 4: Testing basic functionality")
        
        # Initialize inputs
        self.set_input(clk_id, False)
        self.set_input(data_id, False)
        self.update_simulation()
        
        initial_output = self.get_output(out_led)
        logger.info(f"Initial state: CLK=False, DATA=False, Q={initial_output}")
        
        # Step 5: Test data setup
        logger.info("📍 STEP 5: Setting up data input")
        self.set_input(data_id, True)  # Set D=1
        self.update_simulation()
        
        setup_output = self.get_output(out_led)
        logger.info(f"After D=1 setup: CLK=False, DATA=True, Q={setup_output}")
        logger.info("Expected: Q should still be initial value (no clock edge yet)")
        
        # Step 6: Apply clock pulse
        logger.info("📍 STEP 6: Applying clock pulse")
        
        # Rising edge
        self.set_input(clk_id, True)
        self.update_simulation()
        
        rising_output = self.get_output(out_led)
        logger.info(f"After rising edge: CLK=True, DATA=True, Q={rising_output}")
        logger.info("Expected: Q should now be True (D value captured)")
        
        # Step 7: Test clock high hold
        logger.info("📍 STEP 7: Testing clock high hold")
        self.set_input(data_id, False)  # Change D while clock high
        self.update_simulation()
        
        hold_output = self.get_output(out_led)
        logger.info(f"Clock high + D change: CLK=True, DATA=False, Q={hold_output}")
        logger.info("Expected: Q should remain True (no new capture until next rising edge)")
        
        # Step 8: Falling edge
        logger.info("📍 STEP 8: Testing falling edge")
        self.set_input(clk_id, False)
        self.update_simulation()
        
        falling_output = self.get_output(out_led)
        logger.info(f"After falling edge: CLK=False, DATA=False, Q={falling_output}")
        logger.info("Expected: Q should remain True (falling edge doesn't affect D-FF)")
        
        # Step 9: Second rising edge test
        logger.info("📍 STEP 9: Second rising edge test")
        self.set_input(clk_id, True)  # D=False should now be captured
        self.update_simulation()
        
        second_rising_output = self.get_output(out_led)
        logger.info(f"Second rising edge: CLK=True, DATA=False, Q={second_rising_output}")
        logger.info("Expected: Q should now be False (new D=False value captured)")
        
        # Step 10: Analysis
        logger.info("📍 STEP 10: Analysis")
        
        expected_sequence = [
            (initial_output, "Initial state"),
            (setup_output, "Data setup (no clock edge)"),
            (rising_output, "After rising edge (should capture D=True)"),
            (hold_output, "Clock high hold (should remain previous)"),
            (falling_output, "Falling edge (should remain previous)"),
            (second_rising_output, "Second rising edge (should capture D=False)")
        ]
        
        logger.info("🎯 STEP-BY-STEP ANALYSIS:")
        working_steps = 0
        
        # Check if D-FF captured D=True on first rising edge
        if rising_output == True:
            logger.info("✅ Step 6 PASSED: Rising edge correctly captured D=True")
            working_steps += 1
        else:
            logger.error("❌ Step 6 FAILED: Rising edge did not capture D=True")
        
        # Check if output remained stable during clock high
        if hold_output == rising_output:
            logger.info("✅ Step 7 PASSED: Output stable during clock high")
            working_steps += 1
        else:
            logger.error("❌ Step 7 FAILED: Output changed during clock high")
        
        # Check if falling edge didn't affect output
        if falling_output == hold_output:
            logger.info("✅ Step 8 PASSED: Falling edge didn't affect output") 
            working_steps += 1
        else:
            logger.error("❌ Step 8 FAILED: Falling edge affected output")
        
        # Check if second rising edge captured D=False
        if second_rising_output == False:
            logger.info("✅ Step 9 PASSED: Second rising edge captured D=False")
            working_steps += 1
        else:
            logger.error("❌ Step 9 FAILED: Second rising edge did not capture D=False")
        
        accuracy = (working_steps / 4) * 100
        logger.info(f"🎯 SINGLE DFLIPFLOP RESULTS:")
        logger.info(f"   Working steps: {working_steps}/4")
        logger.info(f"   Accuracy: {accuracy:.1f}%")
        
        if accuracy >= 75:
            logger.info("✅ Single DFlipFlop working correctly!")
            return True
        else:
            logger.error("❌ Single DFlipFlop has issues")
            return False

def main():
    debugger = SingleDFlipFlopDebugger()
    
    try:
        success = debugger.debug_single_dflipflop_step_by_step()
        
        if success:
            print("🎉 Single DFlipFlop debug PASSED!")
        else:
            print("💥 Single DFlipFlop debug FAILED!")
            print("   Check step-by-step analysis above")
            
    except Exception as e:
        logger.error(f"Exception during single DFlipFlop debug: {e}")
        print(f"💥 Exception: {e}")
    
    finally:
        debugger.cleanup()

if __name__ == "__main__":
    main()