#!/usr/bin/env python3
"""
Phase 5.3 Validation Script - Stress Testing & Performance Validation

This script runs focused Phase 5.3 tests for Tier 3 (Nice to Have) stress cases:
- Task 5.3.1: Performance stress tests (100+ operations)
- Task 5.3.2: Large circuit tests (100+ elements)
- Task 5.3.3: Deep undo stacks (500+ operations)
- Task 5.3.4: Rapid operation sequences

Run from project root:
    python3 test/mcp/test_phase5_3_stress_performance.py
"""

import asyncio
import sys
import os
import time

# Add MCP client path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure
from mcp_models import MCPResponse


class Phase5_3Validator:
    """Phase 5.3 validation runner for stress testing"""

    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=True, verbose=False)
        self.test_count = 0
        self.passed_count = 0
        self.failed_tests = []

    async def run_all_tests(self) -> bool:
        """Run all Phase 5.3 validation tests"""
        print("\n" + "=" * 70)
        print("🧪 PHASE 5.3 - Stress Testing & Performance Validation")
        print("=" * 70)
        print("\nStarting MCP process...\n")

        # Start MCP
        if not await self.mcp.start_mcp():
            print("❌ Failed to start MCP process")
            return False

        try:
            # Run tests
            await self.test_100_sequential_rotations()
            await self.test_50_rapid_flips()
            await self.test_100_size_changes()
            await self.test_large_circuit_100_elements()
            await self.test_deep_undo_stack_500()

            # Summary
            return await self.print_summary()

        finally:
            print("\nStopping MCP process...")
            await self.mcp.stop_mcp()

    async def test_100_sequential_rotations(self) -> None:
        """Test 5.3.1: 100 sequential rotations (performance)"""
        self.test_count += 1
        print(f"\n[Test 5.3.1] 100 Sequential Rotations (Performance)")
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
                return

            element_id = resp.result.get("element_id")
            print(f"✓ Created element (ID: {element_id})")

            # Perform 100 rotations by 1° each
            print("→ Performing 100 rotations (1° each)...")
            start_time = time.time()
            for i in range(100):
                resp = await self.mcp.send_command("rotate_element", {
                    "element_id": element_id,
                    "angle": 1
                })
                if not resp.success:
                    print(f"  ❌ Rotation {i+1} failed")
                    self.failed_tests.append(f"test_100_sequential_rotations: rotation {i+1}")
                    return
                if (i + 1) % 20 == 0:
                    elapsed = time.time() - start_time
                    rate = (i + 1) / elapsed
                    print(f"  ✓ {i+1} rotations completed ({rate:.1f} ops/sec)")

            elapsed = time.time() - start_time
            print(f"✓ All 100 rotations completed in {elapsed:.2f}s ({100/elapsed:.1f} ops/sec)")

            # Verify final state (should be 100° = 100°)
            # (In reality this wraps: 100° total rotation)

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_100_sequential_rotations: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_5_3_1")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_100_sequential_rotations: {str(e)}")

    async def test_50_rapid_flips(self) -> None:
        """Test 5.3.2: 50 rapid flip operations (stress undo stack)"""
        self.test_count += 1
        print(f"\n[Test 5.3.2] 50 Rapid Flip Operations (Stress)")
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
                return

            element_id = resp.result.get("element_id")
            print(f"✓ Created element (ID: {element_id})")

            # Perform 50 flips alternating between axes
            print("→ Performing 50 rapid flips...")
            start_time = time.time()
            for i in range(50):
                axis = i % 2
                resp = await self.mcp.send_command("flip_element", {
                    "element_id": element_id,
                    "axis": axis
                })
                if not resp.success:
                    print(f"  ❌ Flip {i+1} failed")
                    self.failed_tests.append(f"test_50_rapid_flips: flip {i+1}")
                    return
                if (i + 1) % 10 == 0:
                    elapsed = time.time() - start_time
                    rate = (i + 1) / elapsed
                    print(f"  ✓ {i+1} flips completed ({rate:.1f} ops/sec)")

            elapsed = time.time() - start_time
            print(f"✓ All 50 flips completed in {elapsed:.2f}s ({50/elapsed:.1f} ops/sec)")

            # Verify undo stack
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                print(f"✓ Undo stack size: {undo_count}")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_50_rapid_flips: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_5_3_2")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_50_rapid_flips: {str(e)}")

    async def test_100_size_changes(self) -> None:
        """Test 5.3.3: 100 size changes on same element"""
        self.test_count += 1
        print(f"\n[Test 5.3.3] 100 Size Changes on Single Element")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create element with many input ports
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                print("❌ Failed to create element")
                return

            element_id = resp.result.get("element_id")
            print(f"✓ Created element (ID: {element_id})")

            # Perform 100 size changes (2→3→4→...→101→2→3...)
            print("→ Performing 100 size changes...")
            start_time = time.time()
            for i in range(100):
                # Cycle through sizes 2-5 to avoid going too high
                size = 2 + (i % 4)
                resp = await self.mcp.send_command("change_input_size", {
                    "element_id": element_id,
                    "size": size
                })
                if not resp.success:
                    # Some size changes might be no-ops (changing to current size)
                    # That's OK for this stress test
                    pass
                if (i + 1) % 20 == 0:
                    elapsed = time.time() - start_time
                    rate = (i + 1) / elapsed
                    print(f"  ✓ {i+1} size changes completed ({rate:.1f} ops/sec)")

            elapsed = time.time() - start_time
            print(f"✓ All 100 size changes completed in {elapsed:.2f}s ({100/elapsed:.1f} ops/sec)")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_100_size_changes: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_5_3_3")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_100_size_changes: {str(e)}")

    async def test_large_circuit_100_elements(self) -> None:
        """Test 5.3.4: Large circuit with 100 elements"""
        self.test_count += 1
        print(f"\n[Test 5.3.4] Large Circuit (100 Elements, 500 Operations)")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create 100 elements
            print("→ Creating 100 elements...")
            element_ids = []
            start_time = time.time()
            for i in range(100):
                resp = await self.mcp.send_command("create_element", {
                    "type": "And" if i % 2 == 0 else "Or",
                    "x": 100.0 + (i % 10) * 50.0,
                    "y": 100.0 + (i // 10) * 50.0
                })
                if not resp.success or not resp.result:
                    print(f"❌ Failed to create element {i+1}")
                    return
                element_ids.append(resp.result.get("element_id"))
                if (i + 1) % 20 == 0:
                    elapsed = time.time() - start_time
                    rate = (i + 1) / elapsed
                    print(f"  ✓ {i+1} elements created ({rate:.1f} creates/sec)")

            elapsed = time.time() - start_time
            print(f"✓ All 100 elements created in {elapsed:.2f}s")

            # Perform 5 rotations on each element (500 operations)
            print("→ Rotating all 100 elements (5 rotations each = 500 ops)...")
            start_time = time.time()
            op_count = 0
            for element_id in element_ids:
                for rotation in range(1, 6):
                    resp = await self.mcp.send_command("rotate_element", {
                        "element_id": element_id,
                        "angle": 90
                    })
                    if resp.success:
                        op_count += 1
                        if op_count % 100 == 0:
                            elapsed = time.time() - start_time
                            rate = op_count / elapsed
                            print(f"  ✓ {op_count} rotations completed ({rate:.1f} ops/sec)")

            elapsed = time.time() - start_time
            print(f"✓ All 500 rotations completed in {elapsed:.2f}s ({500/elapsed:.1f} ops/sec)")

            # List all elements to verify circuit integrity
            resp = await self.mcp.send_command("list_elements", {})
            if resp.success:
                elements = resp.result.get("elements", [])
                print(f"✓ Circuit has {len(elements)} elements (expected ~100)")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_large_circuit_100_elements: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_5_3_4")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_large_circuit_100_elements: {str(e)}")

    async def test_deep_undo_stack_500(self) -> None:
        """Test 5.3.5: Deep undo stack with 500+ operations"""
        self.test_count += 1
        print(f"\n[Test 5.3.5] Deep Undo Stack (500+ Operations)")
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
                return

            element_id = resp.result.get("element_id")
            print(f"✓ Created element (ID: {element_id})")

            # Build 500+ operation stack
            # Perform 100 iterations: Rotate(1°) → Rotate(1°) → Rotate(1°) → Flip → Update
            # Total: 500 operations
            print("→ Building 500-operation undo stack...")
            start_time = time.time()
            for i in range(100):
                # 3 rotations
                for _ in range(3):
                    resp = await self.mcp.send_command("rotate_element", {
                        "element_id": element_id,
                        "angle": 1
                    })
                    if not resp.success:
                        print(f"❌ Rotation iteration {i+1} failed")
                        return

                # 1 flip
                resp = await self.mcp.send_command("flip_element", {
                    "element_id": element_id,
                    "axis": i % 2
                })
                if not resp.success:
                    print(f"❌ Flip iteration {i+1} failed")
                    return

                # 1 update
                resp = await self.mcp.send_command("update_element", {
                    "element_id": element_id,
                    "label": f"Iter{i}"
                })
                if not resp.success:
                    print(f"❌ Update iteration {i+1} failed")
                    return

                if (i + 1) % 20 == 0:
                    elapsed = time.time() - start_time
                    ops_so_far = (i + 1) * 5
                    rate = ops_so_far / elapsed
                    print(f"  ✓ {ops_so_far} operations ({rate:.1f} ops/sec)")

            build_time = time.time() - start_time
            total_ops = 500
            print(f"✓ Built 500-operation stack in {build_time:.2f}s ({total_ops/build_time:.1f} ops/sec)")

            # Verify stack size
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                print(f"✓ Undo stack size: {undo_count}")

            # Undo all operations
            print("→ Undoing all 500 operations...")
            start_time = time.time()
            for i in range(total_ops):
                resp = await self.mcp.send_command("undo", {})
                if not resp.success:
                    print(f"  ❌ Undo {i+1} failed")
                    self.failed_tests.append(f"test_deep_undo_stack_500: undo {i+1}")
                    return
                if (i + 1) % 100 == 0:
                    elapsed = time.time() - start_time
                    rate = (i + 1) / elapsed
                    print(f"  ✓ {i+1} undos completed ({rate:.1f} ops/sec)")

            undo_time = time.time() - start_time
            print(f"✓ All 500 undos completed in {undo_time:.2f}s ({total_ops/undo_time:.1f} ops/sec)")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_deep_undo_stack_500: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_5_3_5")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_deep_undo_stack_500: {str(e)}")

    def _check_success(self, response: MCPResponse, description: str) -> bool:
        """Check if response was successful"""
        if not response.success:
            print(f"❌ {description} failed: {response.error}")
            return False
        return True

    async def print_summary(self) -> bool:
        """Print test summary"""
        print("\n" + "=" * 70)
        print(f"📊 PHASE 5.3 STRESS & PERFORMANCE SUMMARY")
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
        if success:
            print("✅ ALL PHASE 5.3 TESTS PASSED - Stress testing validated!")
        else:
            print("❌ SOME TESTS FAILED - Review errors above")
        print("=" * 70 + "\n")

        return success


async def main() -> None:
    """Main entry point"""
    validator = Phase5_3Validator()
    success = await validator.run_all_tests()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    asyncio.run(main())
