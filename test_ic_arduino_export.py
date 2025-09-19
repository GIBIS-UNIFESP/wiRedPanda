#!/usr/bin/env python3
"""
IC Arduino Export Validation Test

This test validates that IC support in Arduino code generation works correctly
through the MCP interface. It creates circuits with ICs and verifies the
generated Arduino code.
"""

import asyncio
import os
import sys
from pathlib import Path

# Add MCP client directory to path
mcp_client_dir = Path(__file__).parent / 'mcp' / 'client'
sys.path.insert(0, str(mcp_client_dir))

from mcp_infrastructure import MCPInfrastructure
from mcp_validation import MCPValidation
from mcp_models import TestResults


class ICArduinoExportTest:
    """Test IC Arduino export functionality"""

    def __init__(self):
        self.test_results = TestResults(total=0, passed=0, failed=0)
        self.infrastructure = MCPInfrastructure(enable_validation=True, verbose=True)
        self.validation = MCPValidation(self.test_results, self.infrastructure)

    async def run_test(self) -> bool:
        """Run the complete IC Arduino export test"""
        print("🔧 IC Arduino Export Validation Test")
        print("=" * 50)

        try:
            # Start the MCP server
            if not await self.infrastructure.start_mcp():
                print("❌ Failed to start MCP server")
                return False

            print("✅ MCP Server started successfully")

            # Run the actual test
            success = await self.test_ic_arduino_export()

            # Cleanup
            await self.infrastructure.stop_mcp()

            return success

        except Exception as e:
            print(f"❌ Test failed with exception: {e}")
            await self.infrastructure.stop_mcp()
            return False

    async def test_ic_arduino_export(self) -> bool:
        """Test IC creation, instantiation, and Arduino export"""
        print("\n🔍 Testing IC Arduino Export...")

        # Step 1: Create a new circuit
        new_circuit_resp = await self.infrastructure.send_command("new_circuit", {})
        if not await self.validation.assert_success(new_circuit_resp, "Create new circuit"):
            return False

        # Step 2: Create elements for an IC (AND gate with input/output)
        print("📍 Creating IC components...")

        # Create input button (IC input interface)
        input_id = await self.validation.validate_element_creation_response(
            await self.infrastructure.send_command("create_element", {
                "type": "InputButton", "x": 100.0, "y": 100.0, "label": "IC_In"
            }), "Create IC input"
        )

        # Create AND gate (IC logic)
        and_id = await self.validation.validate_element_creation_response(
            await self.infrastructure.send_command("create_element", {
                "type": "And", "x": 200.0, "y": 100.0, "label": "IC_Logic"
            }), "Create IC logic gate"
        )

        # Create second input for AND gate
        input2_id = await self.validation.validate_element_creation_response(
            await self.infrastructure.send_command("create_element", {
                "type": "InputButton", "x": 100.0, "y": 150.0, "label": "IC_In2"
            }), "Create IC second input"
        )

        # Create LED (IC output interface)
        output_id = await self.validation.validate_element_creation_response(
            await self.infrastructure.send_command("create_element", {
                "type": "Led", "x": 300.0, "y": 100.0, "label": "IC_Out"
            }), "Create IC output"
        )

        if not all([input_id, and_id, input2_id, output_id]):
            print("❌ Failed to create IC components")
            return False

        # Step 3: Connect the elements
        print("🔗 Connecting IC components...")
        connections = [
            (input_id, 0, and_id, 0),     # Input1 -> AND gate input 0
            (input2_id, 0, and_id, 1),   # Input2 -> AND gate input 1
            (and_id, 0, output_id, 0)    # AND gate output -> LED
        ]

        for source_id, source_port, target_id, target_port in connections:
            connect_resp = await self.infrastructure.send_command("connect_elements", {
                "source_id": source_id, "source_port": source_port,
                "target_id": target_id, "target_port": target_port
            })
            if not await self.validation.assert_success(connect_resp, f"Connect {source_id}->{target_id}"):
                return False

        # Step 4: Create the IC
        print("🔧 Creating IC...")
        ic_resp = await self.infrastructure.send_command("create_ic", {
            "name": "test_and_ic",
            "description": "Test AND gate IC for Arduino export validation"
        })

        # Accept either success or "already exists" as valid
        if ic_resp.success:
            print("✅ IC created successfully")
        elif "already exists" in str(ic_resp.error):
            print("✅ IC already exists (acceptable)")
        else:
            print(f"❌ IC creation failed: {ic_resp.error}")
            return False

        # Step 5: Create a new circuit for testing IC instantiation
        print("📋 Creating test circuit...")
        new_circuit_resp = await self.infrastructure.send_command("new_circuit", {})
        if not await self.validation.assert_success(new_circuit_resp, "Create test circuit"):
            return False

        # Step 6: Create inputs for the IC test
        test_input1_id = await self.validation.validate_element_creation_response(
            await self.infrastructure.send_command("create_element", {
                "type": "InputButton", "x": 50.0, "y": 100.0, "label": "Test_In1"
            }), "Create test input 1"
        )

        test_input2_id = await self.validation.validate_element_creation_response(
            await self.infrastructure.send_command("create_element", {
                "type": "InputButton", "x": 50.0, "y": 150.0, "label": "Test_In2"
            }), "Create test input 2"
        )

        # Step 7: Instantiate the IC
        print("🔌 Instantiating IC...")
        ic_instance_id = await self.validation.validate_element_creation_response(
            await self.infrastructure.send_command("instantiate_ic", {
                "ic_name": "test_and_ic", "x": 200.0, "y": 125.0, "label": "AND_IC"
            }), "Instantiate IC"
        )

        # Step 8: Create output LED
        test_output_id = await self.validation.validate_element_creation_response(
            await self.infrastructure.send_command("create_element", {
                "type": "Led", "x": 350.0, "y": 125.0, "label": "Test_Out"
            }), "Create test output"
        )

        if not all([test_input1_id, test_input2_id, ic_instance_id, test_output_id]):
            print("❌ Failed to create test circuit components")
            return False

        # Step 9: Connect the test circuit
        print("🔗 Connecting test circuit...")
        test_connections = [
            (test_input1_id, 0, ic_instance_id, 0),   # Test input 1 -> IC input 0
            (test_input2_id, 0, ic_instance_id, 1),   # Test input 2 -> IC input 1
            (ic_instance_id, 0, test_output_id, 0)    # IC output -> Test LED
        ]

        for source_id, source_port, target_id, target_port in test_connections:
            connect_resp = await self.infrastructure.send_command("connect_elements", {
                "source_id": source_id, "source_port": source_port,
                "target_id": target_id, "target_port": target_port
            })
            if not await self.validation.assert_success(connect_resp, f"Connect test {source_id}->{target_id}"):
                return False

        # Step 10: Export to Arduino and validate
        print("🚀 Exporting to Arduino...")
        arduino_file = "test_ic_circuit.ino"

        # Clean up any existing test file
        if os.path.exists(arduino_file):
            os.remove(arduino_file)

        export_resp = await self.infrastructure.send_command("export_arduino", {
            "filename": arduino_file
        })

        if not await self.validation.assert_success(export_resp, "Export Arduino code"):
            return False

        # Step 11: Validate the generated Arduino code
        print("🔍 Validating generated Arduino code...")
        return await self.validate_arduino_code(arduino_file)

    async def validate_arduino_code(self, filename: str) -> bool:
        """Validate that the generated Arduino code contains IC handling"""
        if not os.path.exists(filename):
            print(f"❌ Arduino file not found: {filename}")
            return False

        try:
            with open(filename, 'r') as f:
                content = f.read()

            print(f"✅ Arduino file generated ({len(content)} characters)")

            # Check for IC-specific content
            validation_checks = [
                ("// IC:", "IC boundary comments"),
                ("// End IC:", "IC end boundary comments"),
                ("boolean aux_", "Auxiliary variables declaration"),
                ("void setup()", "Arduino setup function"),
                ("void loop()", "Arduino loop function"),
                ("digitalWrite(", "Digital output operations"),
                ("digitalRead(", "Digital input operations")
            ]

            all_checks_passed = True
            for check_text, description in validation_checks:
                if check_text in content:
                    print(f"✅ Found {description}")
                else:
                    print(f"❌ Missing {description}")
                    all_checks_passed = False

            # Additional IC-specific validation
            if "// IC:" in content and "// End IC:" in content:
                print("✅ IC boundaries properly marked in generated code")
            else:
                print("❌ IC boundaries not found in generated code")
                all_checks_passed = False

            # Save the generated code for inspection
            print(f"📄 Arduino code saved to: {filename}")
            print(f"📊 Code preview (first 500 chars):")
            print("-" * 50)
            print(content[:500])
            if len(content) > 500:
                print("... (truncated)")
            print("-" * 50)

            return all_checks_passed

        except Exception as e:
            print(f"❌ Error validating Arduino code: {e}")
            return False


async def main():
    """Main test execution"""
    test = ICArduinoExportTest()
    success = await test.run_test()

    if success:
        print("\n🎉 IC Arduino Export Test PASSED!")
        print("✅ IC support in Arduino code generation is working correctly")
    else:
        print("\n❌ IC Arduino Export Test FAILED!")
        print("❌ Issues found in IC Arduino code generation")

    return 0 if success else 1


if __name__ == "__main__":
    exit_code = asyncio.run(main())
    sys.exit(exit_code)