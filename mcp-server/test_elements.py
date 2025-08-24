#!/usr/bin/env python3
"""Test what elements are supported"""

import logging
import sys
import os
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from cpu_level_5_integrated_cpu_components import IntegratedCPUValidator

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

def test_element_types():
    validator = IntegratedCPUValidator()
    
    # Elements to test
    test_elements = [
        ("InputButton", "Input"),
        ("Led", "LED"),  
        ("And", "AND"),
        ("Or", "OR"),
        ("Not", "NOT"),
        ("Xor", "XOR"),
        ("Nand", "NAND"), 
        ("Nor", "NOR"),
        ("DFlipFlop", "D Flip-Flop"),
        ("FullAdder", "Full Adder"),
        ("HalfAdder", "Half Adder"),
        ("Mux", "Multiplexer"),
        ("Clock", "Clock"),
    ]
    
    working_elements = []
    hanging_elements = []
    
    try:
        validator._ensure_bridge()
        validator.create_new_circuit()
        
        for element_type, description in test_elements:
            try:
                logger.info(f"Testing {element_type} ({description})...")
                element_id = validator.create_element(element_type, 100.0, 100.0, f"TEST_{element_type}")
                
                if element_id:
                    logger.info(f"✅ {element_type} works - ID: {element_id}")
                    working_elements.append(element_type)
                else:
                    logger.warning(f"❌ {element_type} returned None")
                    
            except Exception as e:
                logger.error(f"❌ {element_type} failed: {e}")
                hanging_elements.append(element_type)
                
    except Exception as e:
        logger.error(f"Test failed: {e}")
    finally:
        validator.cleanup_bridge()
    
    logger.info(f"\n✅ Working elements: {working_elements}")
    logger.info(f"❌ Problem elements: {hanging_elements}")

if __name__ == "__main__":
    test_element_types()