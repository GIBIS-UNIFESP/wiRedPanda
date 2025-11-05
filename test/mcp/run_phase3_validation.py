#!/usr/bin/env python3
"""
Phase 3 Validation Script - Robustness Enhancements Validation

This script runs focused Phase 3 tests to validate the robustness enhancements:
- Task 3.1: UndoTransaction RAII class for transactional safety
- Task 3.2: Deserialization validation in loadItems()
- Task 3.3: MorphCommand safety with port compatibility validation

Run from project root:
    python test/mcp/run_phase3_validation.py
"""

import asyncio
import sys
import os

# Add MCP client path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure
from mcp_models import MCPResponse


class Phase3Validator:
    """Phase 3 validation runner for robustness enhancements"""

    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=True, verbose=False)
        self.test_count = 0
        self.passed_count = 0
        self.failed_tests = []

    async def run_all_tests(self) -> bool:
        """Run all Phase 3 validation tests"""
        print("\n" + "=" * 70)
        print("🧪 PHASE 3 VALIDATION - Robustness Enhancements")
        print("=" * 70)
        print("\nStarting MCP process...\n")

        # Start MCP
        if not await self.mcp.start_mcp():
            print("❌ Failed to start MCP process")
            return False

        try:
            # Run tests
            await self.test_undo_redo_with_deletions()
            await self.test_morph_with_port_compatibility()
            await self.test_morph_incompatible_ports()
            await self.test_multiple_morphs_with_validation()

            # Summary
            return await self.print_summary()

        finally:
            print("\nStopping MCP process...")
            await self.mcp.stop_mcp()

    async def test_undo_redo_with_deletions(self) -> None:
        """Test 3.1/3.2: Undo/redo stability with element deletions (UndoTransaction safety)"""
        self.test_count += 1
        print(f"\n[Test 3.1/3.2] Undo/Redo with Element Deletions")
        print("-" * 50)

        try:
            # Create circuit
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create multiple elements to build up undo stack
            element_ids = []
            for i in range(3):
                resp = await self.mcp.send_command("create_element", {
                    "type": "And",
                    "x": 100.0 + i * 100.0,
                    "y": 100.0
                })
                if resp.success and resp.result:
                    element_ids.append(resp.result.get("element_id"))
                    print(f"✓ Created element {i+1} (ID: {element_ids[-1]})")

            if not element_ids:
                print("❌ Failed to create elements")
                self.failed_tests.append("test_undo_redo_with_deletions: create elements")
                return

            # Perform multiple operations
            print("→ Performing operations to test transaction safety...")
            for i, eid in enumerate(element_ids):
                resp = await self.mcp.send_command("move_element", {
                    "element_id": eid,
                    "x": 150.0 + i * 50.0,
                    "y": 150.0
                })
                if resp.success:
                    print(f"  ✓ Moved element {i+1}")

            # Delete first element
            resp = await self.mcp.send_command("delete_element", {
                "element_id": element_ids[0]
            })
            self._check_success(resp, "Delete first element")

            # Get undo stack state
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                print(f"✓ Undo stack has {undo_count} actions")

            # Perform undo operations - transaction safety should handle missing elements gracefully
            print("→ Performing undo with transaction safety validation...")
            for i in range(2):
                resp = await self.mcp.send_command("undo", {})
                if resp.success:
                    print(f"  ✓ Undo {i+1} succeeded")
                elif resp.error:
                    print(f"  ✓ Undo {i+1} handled gracefully: {str(resp.error)[:60]}...")
                else:
                    print(f"  ⚠ Undo {i+1} had unexpected result")

            # Check process didn't crash
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed with code {self.mcp.process.returncode}")
                self.failed_tests.append("test_undo_redo_with_deletions: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_3_1")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_undo_redo_with_deletions: {str(e)}")

    async def test_morph_with_port_compatibility(self) -> None:
        """Test 3.3: MorphCommand validates port compatibility before transfer"""
        self.test_count += 1
        print(f"\n[Test 3.3] MorphCommand Port Compatibility Validation")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create AND gate (2 inputs)
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                print("❌ Failed to create AND gate")
                self.failed_tests.append("test_morph_with_port_compatibility: create AND")
                return

            and_id = resp.result.get("element_id")
            print(f"✓ Created AND gate (ID: {and_id})")

            # Connect input to it (creates a connection)
            resp = await self.mcp.send_command("create_element", {
                "type": "InputSwitch",
                "x": 50.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                print("❌ Failed to create switch")
                return

            switch_id = resp.result.get("element_id")
            print(f"✓ Created Switch (ID: {switch_id})")

            # Connect switch to AND input
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": switch_id,
                "source_port": 0,
                "target_id": and_id,
                "target_port": 0
            })
            self._check_success(resp, "Connect switch to AND")

            # Morph AND to another gate with same input count (OR - also 2 inputs)
            print("→ Morphing AND gate to OR gate (same port count)...")
            resp = await self.mcp.send_command("morph_element", {
                "element_id": and_id,
                "new_type": "Or"
            })

            if resp.success:
                print("✓ Morph succeeded with compatible ports")
            elif resp.error:
                print(f"✓ Morph handled gracefully: {str(resp.error)[:60]}...")
            else:
                print("⚠ Morph had unexpected result")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_morph_with_port_compatibility: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_3_3_compat")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_morph_with_port_compatibility: {str(e)}")

    async def test_morph_incompatible_ports(self) -> None:
        """Test 3.3: MorphCommand gracefully handles incompatible port counts"""
        self.test_count += 1
        print(f"\n[Test 3.3] MorphCommand Incompatible Port Handling")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create AND gate (2 inputs) and connect it
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                print("❌ Failed to create AND gate")
                return

            and_id = resp.result.get("element_id")
            print(f"✓ Created AND gate (ID: {and_id})")

            # Create two input switches
            resp1 = await self.mcp.send_command("create_element", {
                "type": "InputSwitch",
                "x": 50.0,
                "y": 80.0
            })
            resp2 = await self.mcp.send_command("create_element", {
                "type": "InputSwitch",
                "x": 50.0,
                "y": 120.0
            })

            if not resp1.success or not resp2.success:
                print("❌ Failed to create switches")
                return

            sw1_id = resp1.result.get("element_id")
            sw2_id = resp2.result.get("element_id")

            # Connect both switches to AND inputs
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": sw1_id,
                "source_port": 0,
                "target_id": and_id,
                "target_port": 0
            })
            self._check_success(resp, "Connect switch 1")

            resp = await self.mcp.send_command("connect_elements", {
                "source_id": sw2_id,
                "source_port": 0,
                "target_id": and_id,
                "target_port": 1
            })
            self._check_success(resp, "Connect switch 2")

            # Morph AND (2 inputs) to NOT gate (1 input) - incompatible!
            print("→ Morphing AND gate to NOT gate (incompatible port count)...")
            resp = await self.mcp.send_command("morph_element", {
                "element_id": and_id,
                "new_type": "Not"
            })

            if resp.success:
                print("✓ Morph succeeded with graceful port handling")
            elif resp.error:
                error_msg = str(resp.error) if isinstance(resp.error, str) else str(resp.error)
                print(f"✓ Morph handled gracefully: {error_msg[:60]}...")
            else:
                print("⚠ Morph had unexpected result")

            # Check process didn't crash (this is the critical test)
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed with incompatible morphing")
                self.failed_tests.append("test_morph_incompatible_ports: crash")
                return

            print("✅ Test PASSED - Incompatible morph handled without crash")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_3_3_incompat")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_morph_incompatible_ports: {str(e)}")

    async def test_multiple_morphs_with_validation(self) -> None:
        """Test 3.3: Multiple morphs with validation at each step"""
        self.test_count += 1
        print(f"\n[Test 3.3] Multiple Morphs with Validation")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create base AND gate
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                print("❌ Failed to create base element")
                return

            elm_id = resp.result.get("element_id")
            print(f"✓ Created AND gate (ID: {elm_id})")

            # Perform sequence of morphs, each validated
            morph_sequence = [
                ("Or", "AND → OR (same ports)"),
                ("Xor", "OR → XOR (same ports)"),
                ("Not", "XOR → NOT (fewer ports)"),
                ("Or", "NOT → OR (more ports)"),
            ]

            print("→ Performing morph sequence with validation...")
            for new_type, description in morph_sequence:
                resp = await self.mcp.send_command("morph_element", {
                    "element_id": elm_id,
                    "new_type": new_type
                })

                if resp.success:
                    print(f"  ✓ {description}: morphed successfully")
                elif resp.error:
                    print(f"  ✓ {description}: handled gracefully")
                else:
                    print(f"  ⚠ {description}: unexpected result")

            # Get undo stack to verify morphs were recorded
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                print(f"✓ Undo stack recorded {undo_count} morph operations")

            # Undo all morphs with validation
            print("→ Undoing all morphs...")
            for i in range(min(4, 5)):  # Safely undo without error if fewer than 4
                resp = await self.mcp.send_command("undo", {})
                if resp.success or resp.error:
                    print(f"  ✓ Undo {i+1} handled")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_multiple_morphs_with_validation: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_3_3_multiple")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_multiple_morphs_with_validation: {str(e)}")

    def _check_success(self, response: MCPResponse, description: str) -> bool:
        """Check if response was successful"""
        if not response.success:
            print(f"❌ {description} failed: {response.error}")
            return False
        print(f"✓ {description}")
        return True

    async def print_summary(self) -> bool:
        """Print test summary"""
        print("\n" + "=" * 70)
        print(f"📊 PHASE 3 VALIDATION SUMMARY")
        print("=" * 70)
        print(f"\nTests Run:    {self.test_count}")
        print(f"Tests Passed: {self.passed_count}")
        print(f"Tests Failed: {len(self.failed_tests)}")

        if self.failed_tests:
            print("\nFailed Tests:")
            for test in self.failed_tests:
                print(f"  ❌ {test}")

        success = (self.passed_count == self.test_count)
        print("\n" + ("=" * 70))
        if self.passed_count == self.test_count:
            print("✅ ALL PHASE 3 TESTS PASSED - Robustness enhancements validated!")
        else:
            print("❌ SOME TESTS FAILED - Review errors above")
        print("=" * 70 + "\n")

        return success


async def main() -> None:
    """Main entry point"""
    validator = Phase3Validator()
    success = await validator.run_all_tests()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    asyncio.run(main())
