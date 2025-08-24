#!/usr/bin/env python3

"""
Cycle Detection Test Suite - Comprehensive testing of feedback loop detection
Focus: Systematically test all cycle types to isolate algorithm failures
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

class CycleDetectionTester(AdvancedSequentialValidator):
    
    def test_single_element_cycle(self):
        """Test 1: Single D flip-flop self-loop (A -> A)"""
        logger.info("🔧 TEST 1: Single Element Cycle (A -> A)")
        logger.info("Expected: 1 feedback group with 1 element")
        
        if not self.create_new_circuit():
            return False

        # Create minimal setup
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        # Create single D flip-flop
        ff_x, ff_y = self._get_grid_position(3, 1)
        ff_id = self.create_element("DFlipFlop", ff_x, ff_y, "FF_A")
        
        # Connect clock
        self.connect_elements(clk_id, 0, ff_id, 1)  # CLK -> FF clock
        
        # Create self-loop: ~Q -> D
        logger.info("Creating self-loop: FF_A(~Q) -> FF_A(D)")
        if not self.connect_elements(ff_id, 1, ff_id, 0):  # ~Q -> D
            logger.error("Failed to create self-loop")
            return False
        
        # Save for inspection
        self.save_circuit("test1_single_cycle.panda")
        logger.info("💾 Test 1 circuit saved as: test1_single_cycle.panda")
        
        # Expected: 1 feedback group detected
        logger.info("✅ Test 1 complete - check debug logs for cycle detection")
        return True
    
    def test_two_element_cycle(self):
        """Test 2: Two-element cycle (A -> B -> A)"""
        logger.info("🔧 TEST 2: Two Element Cycle (A -> B -> A)")
        logger.info("Expected: 1 feedback group with 2 elements")
        
        if not self.create_new_circuit():
            return False

        # Create inputs
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        # Create two D flip-flops
        ff_a_x, ff_a_y = self._get_grid_position(3, 1)
        ff_a_id = self.create_element("DFlipFlop", ff_a_x, ff_a_y, "FF_A")
        
        ff_b_x, ff_b_y = self._get_grid_position(3, 3)
        ff_b_id = self.create_element("DFlipFlop", ff_b_x, ff_b_y, "FF_B")
        
        # Connect clocks
        self.connect_elements(clk_id, 0, ff_a_id, 1)  # CLK -> FF_A
        self.connect_elements(clk_id, 0, ff_b_id, 1)  # CLK -> FF_B
        
        # Create cycle: A -> B -> A
        logger.info("Creating cycle: FF_A(Q) -> FF_B(D) -> FF_A(D)")
        if not self.connect_elements(ff_a_id, 0, ff_b_id, 0):  # FF_A Q -> FF_B D
            logger.error("Failed to connect A -> B")
            return False
        if not self.connect_elements(ff_b_id, 0, ff_a_id, 0):  # FF_B Q -> FF_A D
            logger.error("Failed to connect B -> A")
            return False
        
        # Save for inspection
        self.save_circuit("test2_two_cycle.panda")
        logger.info("💾 Test 2 circuit saved as: test2_two_cycle.panda")
        
        # Expected: 1 feedback group with 2 elements
        logger.info("✅ Test 2 complete - check debug logs for cycle detection")
        return True
    
    def test_three_element_cycle(self):
        """Test 3: Three-element cycle (A -> B -> C -> A)"""
        logger.info("🔧 TEST 3: Three Element Cycle (A -> B -> C -> A)")
        logger.info("Expected: 1 feedback group with 3 elements (Johnson Counter pattern)")
        
        if not self.create_new_circuit():
            return False

        # Create inputs
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        # Create three D flip-flops
        ff_a_x, ff_a_y = self._get_grid_position(3, 1)
        ff_a_id = self.create_element("DFlipFlop", ff_a_x, ff_a_y, "FF_A")
        
        ff_b_x, ff_b_y = self._get_grid_position(3, 3)
        ff_b_id = self.create_element("DFlipFlop", ff_b_x, ff_b_y, "FF_B")
        
        ff_c_x, ff_c_y = self._get_grid_position(3, 5)
        ff_c_id = self.create_element("DFlipFlop", ff_c_x, ff_c_y, "FF_C")
        
        # Connect clocks
        self.connect_elements(clk_id, 0, ff_a_id, 1)  # CLK -> FF_A
        self.connect_elements(clk_id, 0, ff_b_id, 1)  # CLK -> FF_B
        self.connect_elements(clk_id, 0, ff_c_id, 1)  # CLK -> FF_C
        
        # Create cycle: A -> B -> C -> A (Johnson Counter style)
        logger.info("Creating cycle: FF_A(Q) -> FF_B(D), FF_B(Q) -> FF_C(D), FF_C(~Q) -> FF_A(D)")
        if not self.connect_elements(ff_a_id, 0, ff_b_id, 0):  # FF_A Q -> FF_B D
            logger.error("Failed to connect A -> B")
            return False
        if not self.connect_elements(ff_b_id, 0, ff_c_id, 0):  # FF_B Q -> FF_C D
            logger.error("Failed to connect B -> C")
            return False
        if not self.connect_elements(ff_c_id, 1, ff_a_id, 0):  # FF_C ~Q -> FF_A D (inverted feedback)
            logger.error("Failed to connect C -> A (inverted)")
            return False
        
        # Save for inspection
        self.save_circuit("test3_three_cycle.panda")
        logger.info("💾 Test 3 circuit saved as: test3_three_cycle.panda")
        
        # Expected: 1 feedback group with 3 elements
        logger.info("✅ Test 3 complete - check debug logs for cycle detection")
        return True
    
    def test_four_element_cycle(self):
        """Test 4: Four-element cycle (A -> B -> C -> D -> A)"""
        logger.info("🔧 TEST 4: Four Element Cycle (A -> B -> C -> D -> A)")
        logger.info("Expected: 1 feedback group with 4 elements")
        
        if not self.create_new_circuit():
            return False

        # Create inputs
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        # Create four D flip-flops
        ff_a_x, ff_a_y = self._get_grid_position(3, 1)
        ff_a_id = self.create_element("DFlipFlop", ff_a_x, ff_a_y, "FF_A")
        
        ff_b_x, ff_b_y = self._get_grid_position(3, 3)
        ff_b_id = self.create_element("DFlipFlop", ff_b_x, ff_b_y, "FF_B")
        
        ff_c_x, ff_c_y = self._get_grid_position(3, 5)
        ff_c_id = self.create_element("DFlipFlop", ff_c_x, ff_c_y, "FF_C")
        
        ff_d_x, ff_d_y = self._get_grid_position(3, 7)
        ff_d_id = self.create_element("DFlipFlop", ff_d_x, ff_d_y, "FF_D")
        
        # Connect clocks
        self.connect_elements(clk_id, 0, ff_a_id, 1)  # CLK -> FF_A
        self.connect_elements(clk_id, 0, ff_b_id, 1)  # CLK -> FF_B
        self.connect_elements(clk_id, 0, ff_c_id, 1)  # CLK -> FF_C
        self.connect_elements(clk_id, 0, ff_d_id, 1)  # CLK -> FF_D
        
        # Create cycle: A -> B -> C -> D -> A
        logger.info("Creating cycle: FF_A -> FF_B -> FF_C -> FF_D -> FF_A")
        if not self.connect_elements(ff_a_id, 0, ff_b_id, 0):  # FF_A Q -> FF_B D
            logger.error("Failed to connect A -> B")
            return False
        if not self.connect_elements(ff_b_id, 0, ff_c_id, 0):  # FF_B Q -> FF_C D
            logger.error("Failed to connect B -> C")
            return False
        if not self.connect_elements(ff_c_id, 0, ff_d_id, 0):  # FF_C Q -> FF_D D
            logger.error("Failed to connect C -> D")
            return False
        if not self.connect_elements(ff_d_id, 0, ff_a_id, 0):  # FF_D Q -> FF_A D
            logger.error("Failed to connect D -> A")
            return False
        
        # Save for inspection
        self.save_circuit("test4_four_cycle.panda")
        logger.info("💾 Test 4 circuit saved as: test4_four_cycle.panda")
        
        # Expected: 1 feedback group with 4 elements
        logger.info("✅ Test 4 complete - check debug logs for cycle detection")
        return True
    
    def test_two_separate_cycles(self):
        """Test 5: Two independent cycles in same circuit"""
        logger.info("🔧 TEST 5: Two Separate Cycles")
        logger.info("Expected: 2 feedback groups, each with 2 elements")
        
        if not self.create_new_circuit():
            return False

        # Create inputs
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        # Create first cycle: A1 -> B1 -> A1
        ff_a1_x, ff_a1_y = self._get_grid_position(3, 1)
        ff_a1_id = self.create_element("DFlipFlop", ff_a1_x, ff_a1_y, "FF_A1")
        
        ff_b1_x, ff_b1_y = self._get_grid_position(3, 3)
        ff_b1_id = self.create_element("DFlipFlop", ff_b1_x, ff_b1_y, "FF_B1")
        
        # Create second cycle: A2 -> B2 -> A2  
        ff_a2_x, ff_a2_y = self._get_grid_position(7, 1)
        ff_a2_id = self.create_element("DFlipFlop", ff_a2_x, ff_a2_y, "FF_A2")
        
        ff_b2_x, ff_b2_y = self._get_grid_position(7, 3)
        ff_b2_id = self.create_element("DFlipFlop", ff_b2_x, ff_b2_y, "FF_B2")
        
        # Connect clocks
        self.connect_elements(clk_id, 0, ff_a1_id, 1)  # CLK -> FF_A1
        self.connect_elements(clk_id, 0, ff_b1_id, 1)  # CLK -> FF_B1
        self.connect_elements(clk_id, 0, ff_a2_id, 1)  # CLK -> FF_A2
        self.connect_elements(clk_id, 0, ff_b2_id, 1)  # CLK -> FF_B2
        
        # Create first cycle: A1 -> B1 -> A1
        logger.info("Creating cycle 1: FF_A1 -> FF_B1 -> FF_A1")
        if not self.connect_elements(ff_a1_id, 0, ff_b1_id, 0):  # FF_A1 Q -> FF_B1 D
            logger.error("Failed to connect A1 -> B1")
            return False
        if not self.connect_elements(ff_b1_id, 0, ff_a1_id, 0):  # FF_B1 Q -> FF_A1 D
            logger.error("Failed to connect B1 -> A1")
            return False
        
        # Create second cycle: A2 -> B2 -> A2
        logger.info("Creating cycle 2: FF_A2 -> FF_B2 -> FF_A2")
        if not self.connect_elements(ff_a2_id, 0, ff_b2_id, 0):  # FF_A2 Q -> FF_B2 D
            logger.error("Failed to connect A2 -> B2")
            return False
        if not self.connect_elements(ff_b2_id, 0, ff_a2_id, 0):  # FF_B2 Q -> FF_A2 D
            logger.error("Failed to connect B2 -> A2")
            return False
        
        # Save for inspection
        self.save_circuit("test5_two_cycles.panda")
        logger.info("💾 Test 5 circuit saved as: test5_two_cycles.panda")
        
        # Expected: 2 feedback groups, each with 2 elements
        logger.info("✅ Test 5 complete - check debug logs for cycle detection")
        return True
    
    def test_mixed_cycle(self):
        """Test 6: Mixed sequential/combinational cycle"""
        logger.info("🔧 TEST 6: Mixed Sequential/Combinational Cycle")
        logger.info("Expected: 1 feedback group with 3 elements (2 sequential, 1 combinational)")
        
        if not self.create_new_circuit():
            return False

        # Create inputs
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        # Create two D flip-flops
        ff_a_x, ff_a_y = self._get_grid_position(3, 1)
        ff_a_id = self.create_element("DFlipFlop", ff_a_x, ff_a_y, "FF_A")
        
        ff_b_x, ff_b_y = self._get_grid_position(3, 3)
        ff_b_id = self.create_element("DFlipFlop", ff_b_x, ff_b_y, "FF_B")
        
        # Create XOR gate (combinational)
        xor_x, xor_y = self._get_grid_position(5, 2)
        xor_id = self.create_element("Xor", xor_x, xor_y, "XOR_LOGIC")
        
        # Connect clocks
        self.connect_elements(clk_id, 0, ff_a_id, 1)  # CLK -> FF_A
        self.connect_elements(clk_id, 0, ff_b_id, 1)  # CLK -> FF_B
        
        # Create mixed cycle: FF_A -> XOR -> FF_B -> FF_A
        logger.info("Creating mixed cycle: FF_A(Q) -> XOR -> FF_B(D), FF_B(Q) -> FF_A(D)")
        if not self.connect_elements(ff_a_id, 0, xor_id, 0):  # FF_A Q -> XOR input 0
            logger.error("Failed to connect A -> XOR")
            return False
        if not self.connect_elements(ff_b_id, 0, xor_id, 1):  # FF_B Q -> XOR input 1
            logger.error("Failed to connect B -> XOR")
            return False
        if not self.connect_elements(xor_id, 0, ff_b_id, 0):  # XOR -> FF_B D
            logger.error("Failed to connect XOR -> B")
            return False
        if not self.connect_elements(ff_b_id, 0, ff_a_id, 0):  # FF_B Q -> FF_A D
            logger.error("Failed to connect B -> A")
            return False
        
        # Save for inspection
        self.save_circuit("test6_mixed_cycle.panda")
        logger.info("💾 Test 6 circuit saved as: test6_mixed_cycle.panda")
        
        # Expected: 1 feedback group with 3 elements (mixed)
        logger.info("✅ Test 6 complete - check debug logs for cycle detection")
        return True
    
    def test_complex_topology(self):
        """Test 7: Complex topology with multiple interconnections"""
        logger.info("🔧 TEST 7: Complex Topology")
        logger.info("Expected: Multiple feedback groups with complex dependencies")
        
        if not self.create_new_circuit():
            return False

        # Create inputs
        clk_x, clk_y = self._get_input_position(0)
        clk_id = self.create_element("InputButton", clk_x, clk_y, "CLK")
        
        # Create five flip-flops for complex topology
        ffs = []
        for i in range(5):
            ff_x, ff_y = self._get_grid_position(3, i*2 + 1)
            ff_id = self.create_element("DFlipFlop", ff_x, ff_y, f"FF_{chr(65+i)}")
            ffs.append(ff_id)
            # Connect clock
            self.connect_elements(clk_id, 0, ff_id, 1)
        
        # Create complex interconnection pattern
        # A -> B -> C (3-element cycle)
        # D -> E -> D (2-element cycle)
        # A -> D (cross-connection)
        logger.info("Creating complex topology with multiple cycles")
        
        # Main 3-element cycle: A -> B -> C -> A
        self.connect_elements(ffs[0], 0, ffs[1], 0)  # A -> B
        self.connect_elements(ffs[1], 0, ffs[2], 0)  # B -> C  
        self.connect_elements(ffs[2], 0, ffs[0], 0)  # C -> A
        
        # Secondary 2-element cycle: D -> E -> D
        self.connect_elements(ffs[3], 0, ffs[4], 0)  # D -> E
        self.connect_elements(ffs[4], 0, ffs[3], 0)  # E -> D
        
        # Cross-connection: A -> D (links the cycles)
        self.connect_elements(ffs[0], 0, ffs[3], 0)  # A -> D
        
        # Save for inspection
        self.save_circuit("test7_complex_topology.panda")
        logger.info("💾 Test 7 circuit saved as: test7_complex_topology.panda")
        
        # Expected: Complex feedback detection
        logger.info("✅ Test 7 complete - check debug logs for cycle detection")
        return True

def main():
    tester = CycleDetectionTester()
    
    test_results = []
    tests = [
        ("Single Element Cycle", tester.test_single_element_cycle),
        ("Two Element Cycle", tester.test_two_element_cycle),
        ("Three Element Cycle", tester.test_three_element_cycle),
        ("Four Element Cycle", tester.test_four_element_cycle),
        ("Two Separate Cycles", tester.test_two_separate_cycles),
        ("Mixed Cycle", tester.test_mixed_cycle),
        ("Complex Topology", tester.test_complex_topology),
    ]
    
    logger.info("🚀 STARTING COMPREHENSIVE CYCLE DETECTION TEST SUITE")
    logger.info("=" * 80)
    
    for test_name, test_func in tests:
        logger.info(f"\n{'='*20} {test_name} {'='*20}")
        try:
            result = test_func()
            test_results.append((test_name, result, "PASS" if result else "FAIL"))
            logger.info(f"✅ {test_name}: {'PASS' if result else 'FAIL'}")
        except Exception as e:
            logger.error(f"❌ {test_name}: EXCEPTION - {e}")
            test_results.append((test_name, False, f"EXCEPTION: {e}"))
        finally:
            tester.cleanup()
    
    # Summary
    logger.info("\n" + "="*80)
    logger.info("🎯 CYCLE DETECTION TEST SUITE RESULTS:")
    logger.info("=" * 80)
    
    passed = 0
    for test_name, result, status in test_results:
        logger.info(f"  {status:12} - {test_name}")
        if result:
            passed += 1
    
    logger.info("=" * 80)
    logger.info(f"📊 SUMMARY: {passed}/{len(tests)} tests created circuits successfully")
    logger.info("📝 Check debug logs above for detailed cycle detection analysis")
    logger.info("💾 All test circuits saved as test*.panda files for manual inspection")
    
    if passed == len(tests):
        print("🎉 All cycle detection tests PASSED circuit creation!")
        print("   Check debug logs for cycle detection results")
    else:
        print(f"💥 {len(tests)-passed}/{len(tests)} tests FAILED circuit creation!")
    
    return passed == len(tests)

if __name__ == "__main__":
    main()