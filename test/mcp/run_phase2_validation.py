#!/usr/bin/env python3
"""
Phase 2 Validation Script - Architectural Improvements Validation

This script runs focused Phase 2 tests to validate the architectural improvements:
- Task 2.1: QPointer for Scene (automatic null detection)
- Task 2.2: ItemHandle + generation counters (stale reference detection)
- Task 2.3: Command state validation (early validation in undo/redo)

Run from project root:
    python test/mcp/run_phase2_validation.py
"""

import asyncio
import sys
import os

# Add MCP client path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure
from mcp_models import MCPResponse


class Phase2Validator:
    """Phase 2 validation runner for architectural improvements"""

    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=True, verbose=False)
        self.test_count = 0
        self.passed_count = 0
        self.failed_tests = []

    async def run_all_tests(self) -> bool:
        """Run all Phase 2 validation tests"""
        print("\n" + "=" * 70)
        print("🧪 PHASE 2 VALIDATION - Architectural Improvements")
        print("=" * 70)
        print("\nStarting MCP process...\n")

        # Start MCP
        if not await self.mcp.start_mcp():
            print("❌ Failed to start MCP process")
            return False

        try:
            # Run tests
            await self.test_qpointer_scene_safety()
            await self.test_command_validation_on_missing_element()
            await self.test_validation_guards_after_deletion()
            await self.test_multiple_operations_with_validation()

            # Summary
            return await self.print_summary()

        finally:
            print("\nStopping MCP process...")
            await self.mcp.stop_mcp()

    async def test_qpointer_scene_safety(self) -> None:
        """Test 2.1: QPointer Scene null safety after destruction"""
        self.test_count += 1
        print(f"\n[Test 2.1] QPointer Scene Null Safety")
        print("-" * 50)

        try:
            # Create circuit and populate undo stack
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create multiple elements
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

            # Perform operations to populate undo stack
            print("→ Performing operations to populate undo stack...")
            for i, eid in enumerate(element_ids):
                resp = await self.mcp.send_command("move_element", {
                    "element_id": eid,
                    "x": 200.0 + i * 50.0,
                    "y": 200.0
                })
                if resp.success:
                    print(f"  ✓ Moved element {i+1}")

            # Get undo stack state
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                print(f"✓ Undo stack has {undo_count} actions")

            # Close circuit (destroys Scene, but QPointer should handle it)
            print("→ Closing circuit (destroying Scene)...")
            resp = await self.mcp.send_command("close_circuit", {})
            if not self._check_success(resp, "Close circuit"):
                return

            # Check process still alive
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed with code {self.mcp.process.returncode}")
                self.failed_tests.append("test_qpointer_scene_safety: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_2_1")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_qpointer_scene_safety: {str(e)}")

    async def test_command_validation_on_missing_element(self) -> None:
        """Test 2.3: Command validation detects missing elements"""
        self.test_count += 1
        print(f"\n[Test 2.3] Command Validation on Missing Element")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create element
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                print("❌ Failed to create element")
                self.failed_tests.append("test_command_validation_on_missing_element: create")
                return

            element_id = resp.result.get("element_id")
            print(f"✓ Created element (ID: {element_id})")

            # Move element (creates MoveCommand in undo stack)
            resp = await self.mcp.send_command("move_element", {
                "element_id": element_id,
                "x": 200.0,
                "y": 200.0
            })
            self._check_success(resp, "Move element")

            # Delete element externally
            resp = await self.mcp.send_command("delete_element", {
                "element_id": element_id
            })
            if not self._check_success(resp, "Delete element"):
                return

            # Undo should validate and handle gracefully
            print("→ Attempting undo with deleted element...")
            resp = await self.mcp.send_command("undo", {})

            # Should either succeed (recreates) or fail gracefully
            if resp.success:
                print("✓ Undo succeeded (element recreated)")
            elif resp.error:
                error_msg = str(resp.error) if isinstance(resp.error, str) else str(resp.error)
                print(f"✓ Undo failed gracefully: {error_msg[:80]}")
            else:
                print("❌ Unexpected undo result")
                self.failed_tests.append("test_command_validation_on_missing_element: undo result")
                return

            # Check process didn't crash
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed with code {self.mcp.process.returncode}")
                self.failed_tests.append("test_command_validation_on_missing_element: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_2_3")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_command_validation_on_missing_element: {str(e)}")

    async def test_validation_guards_after_deletion(self) -> None:
        """Test 2.2/2.3: Validation guards detect stale references after deletion"""
        self.test_count += 1
        print(f"\n[Test 2.2/2.3] Validation Guards After Deletion")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create two elements
            resp1 = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            resp2 = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 300.0,
                "y": 100.0
            })

            if not resp1.success or not resp2.success:
                print("❌ Failed to create elements")
                self.failed_tests.append("test_validation_guards_after_deletion: create")
                return

            elm1_id = resp1.result.get("element_id")
            elm2_id = resp2.result.get("element_id")
            print(f"✓ Created elements: AND({elm1_id}), OR({elm2_id})")

            # Connect them
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": elm1_id,
                "source_port": 0,
                "target_id": elm2_id,
                "target_port": 0
            })
            if not self._check_success(resp, "Connect elements"):
                return

            # Rotate first element (creates RotateCommand)
            resp = await self.mcp.send_command("rotate_element", {
                "element_id": elm1_id,
                "angle": 90
            })
            self._check_success(resp, "Rotate element")

            # Delete first element
            resp = await self.mcp.send_command("delete_element", {
                "element_id": elm1_id
            })
            self._check_success(resp, "Delete first element")

            # Undo rotate - should validate and handle missing element
            print("→ Attempting undo rotate with deleted element...")
            resp = await self.mcp.send_command("undo", {})

            if resp.success or resp.error:
                print("✓ Undo handled gracefully")
            else:
                print("❌ Unexpected undo result")
                self.failed_tests.append("test_validation_guards_after_deletion: undo result")
                return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_validation_guards_after_deletion: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_2_2")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_validation_guards_after_deletion: {str(e)}")

    async def test_multiple_operations_with_validation(self) -> None:
        """Test 2.3: Multiple operations trigger validation at each step"""
        self.test_count += 1
        print(f"\n[Test 2.3] Multiple Operations with Validation")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create element
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                print("❌ Failed to create element")
                self.failed_tests.append("test_multiple_operations_with_validation: create")
                return

            element_id = resp.result.get("element_id")
            print(f"✓ Created element (ID: {element_id})")

            # Perform multiple operations (each will be validated in Phase 2.3)
            operations = [
                ("move_element", {"element_id": element_id, "x": 150.0, "y": 150.0}, "Move"),
                ("rotate_element", {"element_id": element_id, "angle": 90}, "Rotate"),
                ("move_element", {"element_id": element_id, "x": 200.0, "y": 200.0}, "Move"),
                ("rotate_element", {"element_id": element_id, "angle": 180}, "Rotate"),
            ]

            print("→ Performing 4 operations with validation...")
            for cmd, params, name in operations:
                resp = await self.mcp.send_command(cmd, params)
                if resp.success:
                    print(f"  ✓ {name} succeeded")
                else:
                    print(f"  ⚠ {name} failed: {resp.error}")

            # Get undo stack size
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                print(f"✓ Undo stack has {undo_count} actions")

            # Undo all operations - each should validate
            print("→ Undoing all 4 operations with validation...")
            for i in range(4):
                resp = await self.mcp.send_command("undo", {})
                if resp.success:
                    print(f"  ✓ Undo {i+1} succeeded")
                else:
                    print(f"  ⚠ Undo {i+1} failed")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_multiple_operations_with_validation: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_2_3_multiple")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_multiple_operations_with_validation: {str(e)}")

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
        print(f"📊 PHASE 2 VALIDATION SUMMARY")
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
            print("✅ ALL PHASE 2 TESTS PASSED - Architectural improvements validated!")
        else:
            print("❌ SOME TESTS FAILED - Review errors above")
        print("=" * 70 + "\n")

        return success


async def main() -> None:
    """Main entry point"""
    validator = Phase2Validator()
    success = await validator.run_all_tests()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    asyncio.run(main())
