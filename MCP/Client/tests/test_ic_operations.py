#!/usr/bin/env python3
"""
IC Operations Tests

Tests for Integrated Circuit (IC) functionality including:
- IC creation workflow
- IC listing and enumeration
- IC instantiation and integration
- IC file management

MCP test implementation
"""

from collections.abc import Awaitable, Callable

from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class ICOperationTests(MCPTestBase):
    """Tests for IC operations and workflows"""

    CATEGORY_NAME = "IC OPERATIONS"

    def tests(self) -> list[Callable[[], Awaitable[bool]]]:
        return [
            self.test_ic_workflow_commands,
        ]

    @beartype
    async def test_ic_workflow_commands(self) -> bool:
        """Test IC workflow commands: create_ic, list_ics, instantiate_ic"""
        print("\n=== IC Workflow Commands Test ===")
        self.set_test_context("test_ic_workflow_commands")

        all_passed: bool = True

        # Test IC workflow commands
        print("Testing IC workflow commands...")

        try:
            # Build the InputButton -> Not -> Led chain that IC creation expects.
            if await self.setup_basic_ic_circuit() is None:
                print("❌ Failed to create required elements for IC workflow test")
                return False

            # Stop simulation before creating IC (may be required for proper serialization)
            sim_stop_resp = await self.send_command("simulation_control", {"action": "stop"})
            if sim_stop_resp.success:
                self.infrastructure.output.success("✅ Simulation stopped for IC creation")
            else:
                print(f"⚠️  Failed to stop simulation: {sim_stop_resp.error}")

            # Test create_ic command with properly connected circuit
            await self.create_test_ic()

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
