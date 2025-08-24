#!/usr/bin/env python3
"""Debug script for Level 5 issues"""

import logging
import sys
import os
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from cpu_level_5_integrated_cpu_components import IntegratedCPUValidator

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

def test_bridge_basic():
    """Test basic bridge functionality"""
    validator = IntegratedCPUValidator()
    
    try:
        logger.info("Testing bridge initialization...")
        validator._ensure_bridge()
        logger.info("✅ Bridge initialized successfully")
        
        logger.info("Testing create new circuit...")
        result = validator.create_new_circuit()
        logger.info(f"✅ Create circuit result: {result}")
        
        logger.info("Testing create simple element...")
        element_id = validator.create_element("InputButton", 100.0, 100.0, "TEST_INPUT")
        logger.info(f"✅ Created element ID: {element_id}")
        
        if element_id:
            logger.info("Testing start simulation...")
            sim_result = validator.start_simulation()
            logger.info(f"✅ Start simulation result: {sim_result}")
            
            if sim_result:
                logger.info("Testing set input value...")
                set_result = validator.bridge.set_input_value(element_id, True)
                logger.info(f"✅ Set input result: {set_result}")
                
                logger.info("Testing update simulation...")
                update_result = validator.bridge.update_simulation()
                logger.info(f"✅ Update simulation result: {update_result}")
        
        logger.info("✅ Basic bridge functionality works!")
        return True
        
    except Exception as e:
        logger.error(f"❌ Bridge test failed: {e}")
        import traceback
        traceback.print_exc()
        return False
    finally:
        validator.cleanup_bridge()

def test_alu_circuit_creation():
    """Test ALU circuit creation step by step"""
    validator = IntegratedCPUValidator()
    
    try:
        logger.info("Testing ALU circuit creation...")
        validator._ensure_bridge()
        
        if not validator.create_new_circuit():
            logger.error("❌ Failed to create new circuit")
            return False
            
        logger.info("Creating A0 input...")
        a0_id = validator.create_element("InputButton", 100.0, 100.0, "A0")
        logger.info(f"A0 ID: {a0_id}")
        
        if not a0_id:
            logger.error("❌ Failed to create A0 input")
            return False
            
        logger.info("Creating B0 input...")
        b0_id = validator.create_element("InputButton", 200.0, 100.0, "B0")
        logger.info(f"B0 ID: {b0_id}")
        
        if not b0_id:
            logger.error("❌ Failed to create B0 input")
            return False
        
        logger.info("Testing different element types...")
        
        # Test basic logic gates first
        logger.info("Creating And gate...")
        and_id = validator.create_element("And", 300.0, 150.0, "AND0")
        logger.info(f"AND0 ID: {and_id}")
        
        logger.info("Creating Or gate...")
        or_id = validator.create_element("Or", 300.0, 200.0, "OR0")  
        logger.info(f"OR0 ID: {or_id}")
        
        logger.info("Creating Not gate...")
        not_id = validator.create_element("Not", 300.0, 250.0, "NOT0")
        logger.info(f"NOT0 ID: {not_id}")
        
        logger.info("Creating Xor gate...")
        xor_id = validator.create_element("Xor", 300.0, 300.0, "XOR0")
        logger.info(f"XOR0 ID: {xor_id}")
        
        logger.info("Now testing FullAdder...")
        fa_id = validator.create_element("FullAdder", 300.0, 350.0, "FA0")
        logger.info(f"FA0 ID: {fa_id}")
        
        if not fa_id:
            logger.error("❌ FullAdder not supported - trying other complex elements...")
            
            # Try DFlipFlop 
            logger.info("Creating DFlipFlop...")
            ff_id = validator.create_element("DFlipFlop", 400.0, 150.0, "FF0")
            logger.info(f"FF0 ID: {ff_id}")
            
            # Try Led
            logger.info("Creating Led...")
            led_id = validator.create_element("Led", 500.0, 150.0, "LED0")
            logger.info(f"LED0 ID: {led_id}")
            
            if not ff_id and not led_id:
                logger.error("❌ Complex elements not working")
                return False
        
        logger.info("✅ ALU circuit creation basics work!")
        return True
        
    except Exception as e:
        logger.error(f"❌ ALU circuit creation failed: {e}")
        import traceback
        traceback.print_exc()
        return False
    finally:
        validator.cleanup_bridge()

if __name__ == "__main__":
    logger.info("=== Debug Level 5 Issues ===")
    
    logger.info("\n1. Testing basic bridge functionality...")
    if test_bridge_basic():
        logger.info("\n2. Testing ALU circuit creation...")
        test_alu_circuit_creation()
    else:
        logger.error("Basic bridge test failed - stopping here")
    
    logger.info("\n=== Debug Complete ===")