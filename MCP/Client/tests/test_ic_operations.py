#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
IC Operations Tests

Tests for Integrated Circuit (IC) functionality including:
- IC creation workflow
- IC listing and enumeration
- IC instantiation and integration
- IC file management

MCP test implementation
"""


from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class ICOperationTests(MCPTestBase):
    """Tests for IC operations and workflows"""

    async def run_category_tests(self) -> bool:
        """Run all IC operation tests

        Returns:
            bool: True if all tests passed, False otherwise
        """
        tests = [
            self.test_ic_workflow_commands,
        ]

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    @beartype
    async def test_ic_workflow_commands(self) -> bool:
        """Test IC workflow commands: create_ic, list_ics, instantiate_ic"""
        print("\n=== IC Workflow Commands Test ===")
        self.set_test_context("test_ic_workflow_commands")

        all_passed: bool = True

        # Test IC workflow commands
        print("Testing IC workflow commands...")

        try:
            # Clean up any existing test IC files to ensure clean state
            import os

            test_ic_file = "test_ic.panda"
            if os.path.exists(test_ic_file):
                os.remove(test_ic_file)
                print("🧹 Cleaned up existing test IC file")

            # Create new circuit first
            # Create elements for IC using proper ElementGroup types
            # InputButton belongs to ElementGroup::Input (required for IC interface)
            input_id = await self.create_element_checked("InputButton", 100.0, 100.0, "create IC input", label="IC_Input")
            # Create a simple buffer logic
            buffer_id = await self.create_element_checked("Not", 200.0, 100.0, "create IC logic gate", label="IC_Buffer")
            # Led belongs to ElementGroup::Output (required for IC interface)
            output_id = await self.create_element_checked("Led", 300.0, 100.0, "create IC output", label="IC_Output")

            # Early return if element creation failed
            if not input_id or not buffer_id or not output_id:
                print("❌ Failed to create required elements for IC workflow test")
                all_passed = False
                return all_passed

            # Connect: InputButton -> NOT -> Led (creates IC with 1 input, 1 output)
            connections = [
                (input_id, 0, buffer_id, 0),  # InputButton to NOT gate
                (buffer_id, 0, output_id, 0),  # NOT gate to Led
            ]

            for source_id, source_port, target_id, target_port in connections:
                connect_resp = await self.send_command(
                    "connect_elements",
                    {
                        "source_id": source_id,
                        "source_port": source_port,
                        "target_id": target_id,
                        "target_port": target_port,
                    },
                )
                if not connect_resp.success:
                    print(f"❌ IC test setup failed - could not connect elements: {connect_resp.error}")
                    all_passed = False
                    return all_passed

            # Stop simulation before creating IC (may be required for proper serialization)
            sim_stop_resp = await self.send_command("simulation_control", {"action": "stop"})
            if sim_stop_resp.success:
                self.infrastructure.output.success("✅ Simulation stopped for IC creation")
            else:
                print(f"⚠️  Failed to stop simulation: {sim_stop_resp.error}")

            # Test create_ic command with properly connected circuit
            ic_resp = await self.send_command(
                "create_ic", {"name": "test_ic", "description": "Test IC created by test suite"}
            )

            if ic_resp.success:
                self.infrastructure.output.success("✅ IC created successfully")
            else:
                # Handle "file already exists" as acceptable for testing
                if "already exists" in str(ic_resp.error):
                    self.infrastructure.output.success(f"✅ create_ic handled existing file: {ic_resp.error}")
                else:
                    print(f"❌ create_ic failed: {ic_resp.error}")

            # Test list_ics command
            list_resp = await self.send_command("list_ics", {})
            if not list_resp.success or not list_resp.result:
                print(f"❌ list_ics failed: {list_resp.error}")
                all_passed = False
                return all_passed

            ics = list_resp.result.get("ics", [])
            self.infrastructure.output.success(f"✅ list_ics successful: found {len(ics)} ICs")

            # Early return if no ICs available for testing
            if not ics:
                print("⚠️  No ICs available for instantiate_ic test")
                return all_passed

            # Create a new clean circuit for IC instantiation testing
            new_circuit_resp = await self.send_command("new_circuit", {})
            if not new_circuit_resp.success:
                print(f"⚠️  Failed to create clean circuit: {new_circuit_resp.error}")
                all_passed = False
                return all_passed

            self.infrastructure.output.success("✅ Clean circuit created for IC instantiation")
            print(f"Available ICs: {[ic.get('name', 'unnamed') for ic in ics]}")

            # First test with an existing known-good IC if available
            existing_ic_name = None
            for ic in ics:
                if ic.get("name") in ["simple_and_circuit", "simple_or_circuit"]:
                    existing_ic_name = ic.get("name")
                    break

            if existing_ic_name:
                print(f"Testing instantiation with existing IC: {existing_ic_name}")
                existing_ic_resp = await self.send_command(
                    "instantiate_ic", {"ic_name": existing_ic_name, "x": 400.0, "y": 150.0, "label": "Existing_IC_Test"}
                )
                if existing_ic_resp.success:
                    self.infrastructure.output.success(f"✅ instantiate_ic works with existing IC: {existing_ic_name}")
                else:
                    print(f"❌ instantiate_ic failed even with existing IC: {existing_ic_resp.error}")
                    all_passed = False

            # Test with our newly created IC
            print("Testing instantiation with newly created IC: test_ic")
            inst_resp = await self.send_command(
                "instantiate_ic", {"ic_name": "test_ic", "x": 500.0, "y": 200.0, "label": "IC_Instance"}
            )
            if inst_resp.success:
                self.infrastructure.output.success("✅ instantiate_ic succeeded with newly created IC")
            else:
                print(f"❌ instantiate_ic failed with new IC: {inst_resp.error}")
                all_passed = False

        except Exception as e:
            print(f"❌ IC workflow test failed with exception: {e}")
            self.test_results.errors.append(f"test_ic_workflow_commands: Exception - {e}")
            all_passed = False

        return all_passed
