#!/usr/bin/env python3
"""
Phase 1 Validation Script - Quick Undo/Redo Safety Validation

This script runs focused Phase 1 tests to validate the immediate safety fixes:
- Task 1.1: Null pointer guards in findElements() and findItems()
- Task 1.2: SplitCommand exception safety
- Task 1.3: Scene destruction guard

Run from project root:
    python test/mcp/run_phase1_validation.py
"""

import asyncio
import sys
import os

# Add MCP client path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure
from mcp_models import MCPResponse


class Phase1Validator:
    """Quick Phase 1 validation runner"""

    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=True, verbose=False)
        self.test_count = 0
        self.passed_count = 0
        self.failed_tests = []

    async def run_all_tests(self) -> bool:
        """Run all Phase 1 validation tests"""
        print("\n" + "=" * 70)
        print("🧪 PHASE 1 VALIDATION - Immediate Safety Fixes")
        print("=" * 70)
        print("\nStarting MCP process...\n")

        # Start MCP
        if not await self.mcp.start_mcp():
            print("❌ Failed to start MCP process")
            return False

        try:
            # Run tests
            await self.test_undo_after_deletion()
            await self.test_split_safety()
            await self.test_scene_destruction()
            await self.test_move_with_deletion()

            # Summary
            return await self.print_summary()

        finally:
            print("\nStopping MCP process...")
            await self.mcp.stop_mcp()

    async def test_undo_after_deletion(self) -> None:
        """Test 1.1: Undo after element externally deleted"""
        self.test_count += 1
        print(f"\n[Test 1.1] Undo After Element Deleted")
        print("-" * 50)

        try:
            # Create circuit
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create AND gate
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                print("❌ Failed to create AND gate")
                self.failed_tests.append("test_undo_after_deletion: create element")
                return

            element_id = resp.result.get("element_id")
            print(f"✓ Created AND gate (ID: {element_id})")

            # Move element (creates MoveCommand)
            resp = await self.mcp.send_command("move_element", {
                "element_id": element_id,
                "x": 200.0,
                "y": 200.0
            })
            self._check_success(resp, "Move element")

            # Delete element
            resp = await self.mcp.send_command("delete_element", {
                "element_id": element_id
            })
            if not self._check_success(resp, "Delete element"):
                return

            # Undo - should handle gracefully
            print("→ Attempting undo with deleted element...")
            resp = await self.mcp.send_command("undo", {})

            # Either success or graceful error is acceptable
            if resp.success:
                print("✓ Undo succeeded (element recreated)")
            elif resp.error:
                print(f"✓ Undo failed gracefully: {resp.error[:80]}")
            else:
                print("❌ Unexpected undo result")
                self.failed_tests.append("test_undo_after_deletion: undo result")
                return

            # Check for process crash
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed with code {self.mcp.process.returncode}")
                self.failed_tests.append("test_undo_after_deletion: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_1_1")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_undo_after_deletion: {str(e)}")

    async def test_split_safety(self) -> None:
        """Test 1.2: SplitCommand exception safety"""
        self.test_count += 1
        print(f"\n[Test 1.2] SplitCommand Exception Safety")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create two gates
            resp1 = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            if not resp1.success:
                print("❌ Failed to create first gate")
                self.failed_tests.append("test_split_safety: create gate 1")
                return

            resp2 = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 300.0,
                "y": 100.0
            })
            if not resp2.success:
                print("❌ Failed to create second gate")
                self.failed_tests.append("test_split_safety: create gate 2")
                return

            gate1_id = resp1.result.get("element_id")
            gate2_id = resp2.result.get("element_id")
            print(f"✓ Created gates: AND({gate1_id}), OR({gate2_id})")

            # Connect
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": gate1_id,
                "source_port": 0,
                "target_id": gate2_id,
                "target_port": 0
            })
            if not self._check_success(resp, "Connect elements"):
                return

            # Perform 3 split/undo cycles to test exception safety
            print("→ Performing 3 split/undo cycles...")
            for i in range(3):
                resp = await self.mcp.send_command("split_connection", {
                    "source_id": gate1_id,
                    "source_port": 0,
                    "target_id": gate2_id,
                    "target_port": 0,
                    "x": 200.0,
                    "y": 100.0 + i*20.0
                })

                if resp.success:
                    print(f"  ✓ Split {i+1} succeeded")
                else:
                    error_str = resp.error if isinstance(resp.error, str) else str(resp.error)
                    print(f"  ⚠ Split {i+1} failed: {error_str[:60]}")

                # Undo
                resp = await self.mcp.send_command("undo", {})
                if resp.success:
                    print(f"  ✓ Undo {i+1} succeeded")
                else:
                    print(f"  ⚠ Undo {i+1} failed")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed with code {self.mcp.process.returncode}")
                self.failed_tests.append("test_split_safety: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_1_2")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_split_safety: {str(e)}")

    async def test_scene_destruction(self) -> None:
        """Test 1.3: Scene destruction guard"""
        self.test_count += 1
        print(f"\n[Test 1.3] Scene Destruction Guard")
        print("-" * 50)

        try:
            # Create circuit
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create element
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            if not resp.success:
                print("❌ Failed to create element")
                self.failed_tests.append("test_scene_destruction: create element")
                return

            element_id = resp.result.get("element_id")
            print(f"✓ Created element (ID: {element_id})")

            # Populate undo stack
            for i in range(3):
                resp = await self.mcp.send_command("move_element", {
                    "element_id": element_id,
                    "x": 100.0 + (i+1)*50.0,
                    "y": 100.0 + (i+1)*50.0
                })
                if resp.success:
                    print(f"✓ Move {i+1} added to undo stack")

            # Close circuit (destroys Scene)
            print("→ Closing circuit (destroying Scene)...")
            resp = await self.mcp.send_command("close_circuit", {})
            if not self._check_success(resp, "Close circuit"):
                return

            # Try undo on destroyed Scene
            print("→ Attempting undo after scene destruction...")
            resp = await self.mcp.send_command("undo", {})

            if resp.error:
                error_msg = str(resp.error) if isinstance(resp.error, dict) else resp.error
                print(f"✓ Undo correctly failed: {error_msg[:60]}")
            elif resp.success:
                print("✓ Undo handled gracefully")
            else:
                print("❌ Unexpected undo result")
                self.failed_tests.append("test_scene_destruction: undo result")
                return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed with code {self.mcp.process.returncode}")
                self.failed_tests.append("test_scene_destruction: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_scene_destruction: {str(e)}")

    async def test_move_with_deletion(self) -> None:
        """Test: MoveCommand with deleted element"""
        self.test_count += 1
        print(f"\n[Test 1.1.1] MoveCommand with Deleted Element")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            if not resp.success:
                print("❌ Failed to create element")
                self.failed_tests.append("test_move_with_deletion: create")
                return

            element_id = resp.result.get("element_id")
            print(f"✓ Created element (ID: {element_id})")

            # Move, undo, delete
            resp = await self.mcp.send_command("move_element", {
                "element_id": element_id,
                "x": 200.0,
                "y": 200.0
            })
            self._check_success(resp, "Move element")

            resp = await self.mcp.send_command("undo", {})
            self._check_success(resp, "Undo move")

            resp = await self.mcp.send_command("delete_element", {
                "element_id": element_id
            })
            self._check_success(resp, "Delete element")

            # Redo with deleted element
            print("→ Attempting redo with deleted element...")
            resp = await self.mcp.send_command("redo", {})

            if resp.success or resp.error:
                print("✓ Redo handled gracefully")
            else:
                print("❌ Unexpected redo result")
                self.failed_tests.append("test_move_with_deletion: redo")
                return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_move_with_deletion: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_1_1_1")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_move_with_deletion: {str(e)}")

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
        print(f"📊 PHASE 1 VALIDATION SUMMARY")
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
            print("✅ ALL PHASE 1 TESTS PASSED - Safety fixes validated!")
        else:
            print("❌ SOME TESTS FAILED - Review errors above")
        print("=" * 70 + "\n")

        return success


async def main() -> None:
    """Main entry point"""
    validator = Phase1Validator()
    success = await validator.run_all_tests()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    asyncio.run(main())
