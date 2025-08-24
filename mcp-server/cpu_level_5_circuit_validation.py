#!/usr/bin/env python3
"""
CPU Level 5 Circuit Creation Validation
=======================================

Quick validation script to test that all circuit creation methods work
without full simulation timeouts.
"""

import logging
import sys
import os

# Add current directory to path for imports
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from wiredpanda_bridge import WiredPandaBridge
from cpu_level_5_integrated_cpu_components import IntegratedCPUValidator

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

def test_circuit_creation_methods():
    """Test all circuit creation methods without full simulation"""
    
    logger.info("Starting CPU Level 5 Circuit Creation Validation...")
    
    validator = IntegratedCPUValidator()
    
    # Initialize bridge
    validator.bridge = WiredPandaBridge()
    validator.bridge.start()
    logger.info("Bridge initialized successfully")
    
    # Test each circuit creation method
    creation_methods = [
        ("4-bit ALU", validator._create_4bit_alu_circuit),
        ("Dual-port register file", validator._create_dual_port_register_elements),
        ("Register file", validator._create_register_file_elements),
        ("Instruction decoder", validator._create_instruction_decoder_elements),
        ("Control unit", validator._create_control_unit_elements),
        ("Cache controller", validator._create_cache_controller_elements),
        ("MMU", validator._create_mmu_elements),
        ("Pipeline stage", validator._create_pipeline_stage_elements),
        ("Pipeline coordination", validator._create_pipeline_coordination_elements),
    ]
    
    results = {}
    
    for name, method in creation_methods:
        logger.info(f"Testing {name} creation method...")
        try:
            # Create new circuit for each test
            if not validator.create_new_circuit():
                results[name] = {"success": False, "error": "Failed to create new circuit"}
                continue
            
            # Call the creation method
            if name == "4-bit ALU":
                # This method returns a tuple
                result = method()
                success = result is not None and len(result) == 2
            else:
                # These methods return (input_ids, output_ids)
                input_ids, output_ids = method()
                success = len(input_ids) > 0 and len(output_ids) > 0
            
            if success:
                results[name] = {"success": True, "elements_created": True}
                logger.info(f"✅ {name}: Circuit creation successful")
            else:
                results[name] = {"success": False, "error": "No elements created"}
                logger.warning(f"❌ {name}: Circuit creation failed")
                
        except Exception as e:
            results[name] = {"success": False, "error": str(e)}
            logger.error(f"❌ {name}: Exception during creation: {e}")
    
    # Cleanup
    try:
        validator.cleanup_bridge()
        logger.info("Bridge cleanup completed")
    except Exception as e:
        logger.warning(f"Bridge cleanup warning: {e}")
    
    # Report results
    logger.info("\n" + "="*60)
    logger.info("CIRCUIT CREATION VALIDATION RESULTS")
    logger.info("="*60)
    
    successful = 0
    total = len(creation_methods)
    
    for name, result in results.items():
        status = "✅ PASS" if result["success"] else "❌ FAIL"
        error_info = f" - {result['error']}" if not result["success"] else ""
        logger.info(f"{status}: {name}{error_info}")
        if result["success"]:
            successful += 1
    
    logger.info("="*60)
    logger.info(f"Overall: {successful}/{total} circuit creation methods working ({successful/total*100:.1f}%)")
    
    if successful == total:
        logger.info("🎉 ALL circuit creation methods are working!")
        return True
    else:
        logger.warning(f"⚠️  {total-successful} circuit creation methods need attention")
        return False

def main():
    """Main validation function"""
    try:
        success = test_circuit_creation_methods()
        return 0 if success else 1
    except Exception as e:
        logger.error(f"Validation failed with exception: {e}")
        return 1

if __name__ == "__main__":
    exit_code = main()
    sys.exit(exit_code)