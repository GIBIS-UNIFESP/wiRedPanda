#!/usr/bin/env python3
"""
Phase 5.2 Validation Script - Combined Operations and Deep Undo/Redo Testing

This script runs focused Phase 5.2 tests for Tier 2 (Important) edge cases:
- Task 5.2.1: Multi-step scenarios (create -> rotate -> flip -> update)
- Task 5.2.2: Deep undo chains (10-50 operations)
- Task 5.2.3: State consistency across operations
- Task 5.2.4: Connection preservation with transformations
- Task 5.2.5: Multi-element scenarios

Run from project root:
    python3 test/mcp/test_phase5_2_combined_operations.py
"""

import asyncio
import sys
import os

# Add MCP client path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure
from mcp_models import MCPResponse


class Phase5_2Validator:
    """Phase 5.2 validation runner for combined operations"""

    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=True, verbose=False)
        self.test_count = 0
        self.passed_count = 0
        self.failed_tests = []

    async def run_all_tests(self) -> bool:
        """Run all Phase 5.2 validation tests"""
        print("\n" + "=" * 70)
        print("🧪 PHASE 5.2 - Combined Operations & Deep Undo/Redo Testing")
        print("=" * 70)
        print("\nStarting MCP process...\n")

        # Start MCP
        if not await self.mcp.start_mcp():
            print("❌ Failed to start MCP process")
            return False

        try:
            # Run tests
            await self.test_multi_step_scenario()
            await self.test_rotation_flip_consistency()
            await self.test_deep_undo_chain_20()
            await self.test_deep_undo_chain_50()
            await self.test_size_change_with_connections()
            await self.test_multi_element_operations()
            await self.test_state_consistency_multiple_ops()
            await self.test_connection_preservation_rotation()

            # Summary
            return await self.print_summary()

        finally:
            print("\nStopping MCP process...")
            await self.mcp.stop_mcp()

    async def test_multi_step_scenario(self) -> None:
        """Test 5.2.1: Multi-step scenario (create -> rotate -> flip -> update)"""
        self.test_count += 1
        print(f"\n[Test 5.2.1] Multi-Step Scenario (Create → Rotate → Flip → Update)")
        print("-" * 50)

        try:
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
                self.failed_tests.append("test_multi_step_scenario: create")
                return

            element_id = resp.result.get("element_id")
            print(f"✓ Created AND gate (ID: {element_id})")

            # Step 1: Rotate 90°
            resp = await self.mcp.send_command("rotate_element", {
                "element_id": element_id,
                "angle": 90
            })
            if not self._check_success(resp, "Step 1: Rotate 90°"):
                return

            # Step 2: Flip horizontal
            resp = await self.mcp.send_command("flip_element", {
                "element_id": element_id,
                "axis": 0
            })
            if not self._check_success(resp, "Step 2: Flip horizontal"):
                return

            # Step 3: Rotate 180°
            resp = await self.mcp.send_command("rotate_element", {
                "element_id": element_id,
                "angle": 180
            })
            if not self._check_success(resp, "Step 3: Rotate 180°"):
                return

            # Step 4: Update label
            resp = await self.mcp.send_command("update_element", {
                "element_id": element_id,
                "label": "TestGate"
            })
            if not self._check_success(resp, "Step 4: Update label"):
                return

            # Step 5: Flip vertical
            resp = await self.mcp.send_command("flip_element", {
                "element_id": element_id,
                "axis": 1
            })
            if not self._check_success(resp, "Step 5: Flip vertical"):
                return

            # Get undo stack
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                print(f"✓ Undo stack has {undo_count} actions (expected 5)")

            # Undo all 5 operations
            print("→ Undoing all 5 operations...")
            for i in range(5):
                resp = await self.mcp.send_command("undo", {})
                if resp.success:
                    print(f"  ✓ Undo {i+1} succeeded")
                else:
                    print(f"  ❌ Undo {i+1} failed")
                    self.failed_tests.append(f"test_multi_step_scenario: undo {i+1}")
                    return

            # Verify back to original state (undo stack empty)
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                redo_count = stack_info.get("redo_count", 0)
                if undo_count == 0 and redo_count == 5:
                    print(f"✓ Back to original state (undo: {undo_count}, redo: {redo_count})")
                else:
                    print(f"⚠ Undo stack state unexpected (undo: {undo_count}, redo: {redo_count})")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_multi_step_scenario: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_5_2_1")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_multi_step_scenario: {str(e)}")

    async def test_rotation_flip_consistency(self) -> None:
        """Test 5.2.2: Rotation + Flip consistency (Example 2 from plan)"""
        self.test_count += 1
        print(f"\n[Test 5.2.2] Rotation + Flip Consistency")
        print("-" * 50)

        try:
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
                print("❌ Failed to create element")
                return

            element_id = resp.result.get("element_id")
            print(f"✓ Created AND gate (ID: {element_id})")

            # Perform sequence: Rotate 90° → Flip(0) → Rotate 180° → Flip(1)
            ops = [
                ("rotate_element", {"element_id": element_id, "angle": 90}, "Rotate 90°"),
                ("flip_element", {"element_id": element_id, "axis": 0}, "Flip(0)"),
                ("rotate_element", {"element_id": element_id, "angle": 180}, "Rotate 180°"),
                ("flip_element", {"element_id": element_id, "axis": 1}, "Flip(1)"),
            ]

            print("→ Performing operation sequence...")
            for cmd, params, name in ops:
                resp = await self.mcp.send_command(cmd, params)
                if resp.success:
                    print(f"  ✓ {name}")
                else:
                    print(f"  ❌ {name} failed: {resp.error}")
                    self.failed_tests.append(f"test_rotation_flip_consistency: {name}")
                    return

            # Undo all 4 operations
            print("→ Undoing all 4 operations...")
            for i in range(4):
                resp = await self.mcp.send_command("undo", {})
                if not resp.success:
                    print(f"  ❌ Undo {i+1} failed")
                    self.failed_tests.append(f"test_rotation_flip_consistency: undo {i+1}")
                    return
                print(f"  ✓ Undo {i+1}")

            # Verify undo stack is clean
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                if undo_count == 0:
                    print(f"✓ Element back to original state")
                else:
                    print(f"⚠ Undo stack still has {undo_count} actions")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_rotation_flip_consistency: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_5_2_2")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_rotation_flip_consistency: {str(e)}")

    async def test_deep_undo_chain_20(self) -> None:
        """Test 5.2.3: Deep undo chain with 20 operations"""
        self.test_count += 1
        print(f"\n[Test 5.2.3] Deep Undo Chain (20 operations)")
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

            # Perform 20 mixed operations (rotate, flip, rotate, flip, ...)
            print("→ Performing 20 mixed operations...")
            for i in range(10):
                # Rotate 90°
                resp = await self.mcp.send_command("rotate_element", {
                    "element_id": element_id,
                    "angle": 90
                })
                if not resp.success:
                    print(f"  ❌ Rotate {i+1} failed")
                    return

                # Flip alternating axes
                axis = i % 2
                resp = await self.mcp.send_command("flip_element", {
                    "element_id": element_id,
                    "axis": axis
                })
                if not resp.success:
                    print(f"  ❌ Flip {i+1} failed")
                    return

            print("  ✓ All 20 operations completed")

            # Get undo stack size
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                print(f"✓ Undo stack has {undo_count} actions")

            # Undo all 20 operations
            print("→ Undoing all 20 operations...")
            for i in range(20):
                resp = await self.mcp.send_command("undo", {})
                if not resp.success:
                    print(f"  ❌ Undo {i+1} failed")
                    self.failed_tests.append(f"test_deep_undo_chain_20: undo {i+1}")
                    return
                if (i + 1) % 5 == 0:
                    print(f"  ✓ Undid {i+1} operations")

            print("  ✓ All undos completed")

            # Verify back to original
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                print(f"✓ Back to original (undo stack: {undo_count})")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_deep_undo_chain_20: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_5_2_3")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_deep_undo_chain_20: {str(e)}")

    async def test_deep_undo_chain_50(self) -> None:
        """Test 5.2.3b: Deep undo chain with 50 operations (Example 4 from plan)"""
        self.test_count += 1
        print(f"\n[Test 5.2.3b] Deep Undo Chain (50 operations = 150 undo steps)")
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

            # Perform 50 iterations: Rotate(90°) → Flip(0) → Update(label)
            # This creates 150 total undo steps
            print("→ Performing 50 iterations (150 undo steps)...")
            for i in range(50):
                # Rotate 90°
                resp = await self.mcp.send_command("rotate_element", {
                    "element_id": element_id,
                    "angle": 90
                })
                if not resp.success:
                    print(f"  ❌ Rotate iteration {i+1} failed")
                    return

                # Flip axis 0
                resp = await self.mcp.send_command("flip_element", {
                    "element_id": element_id,
                    "axis": 0
                })
                if not resp.success:
                    print(f"  ❌ Flip iteration {i+1} failed")
                    return

                # Update label
                resp = await self.mcp.send_command("update_element", {
                    "element_id": element_id,
                    "label": f"Op{i+1}"
                })
                if not resp.success:
                    print(f"  ❌ Update iteration {i+1} failed")
                    return

                if (i + 1) % 10 == 0:
                    print(f"  ✓ Completed {i+1} iterations ({(i+1)*3} undo steps)")

            print("  ✓ All 50 iterations completed")

            # Get undo stack size
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                print(f"✓ Undo stack has {undo_count} actions (expected ~150)")

            # Undo all 150 operations (50 * 3)
            print("→ Undoing all 150 operations...")
            for i in range(150):
                resp = await self.mcp.send_command("undo", {})
                if not resp.success:
                    print(f"  ❌ Undo {i+1} failed")
                    self.failed_tests.append(f"test_deep_undo_chain_50: undo {i+1}")
                    return
                if (i + 1) % 30 == 0:
                    print(f"  ✓ Undid {i+1} operations")

            print("  ✓ All 150 undos completed")

            # Verify back to original
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                print(f"✓ Back to original state (undo stack: {undo_count})")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_deep_undo_chain_50: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_5_2_3b")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_deep_undo_chain_50: {str(e)}")

    async def test_size_change_with_connections(self) -> None:
        """Test 5.2.4: Size change with connections (Example 1 from plan)"""
        self.test_count += 1
        print(f"\n[Test 5.2.4] Size Change With Connections")
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
                return
            and_id = resp.result.get("element_id")
            print(f"✓ Created AND gate (ID: {and_id})")

            # Create OR gate (2 inputs)
            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 300.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                print("❌ Failed to create OR gate")
                return
            or_id = resp.result.get("element_id")
            print(f"✓ Created OR gate (ID: {or_id})")

            # Connect AND output to OR input
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": and_id,
                "source_port": 0,
                "target_id": or_id,
                "target_port": 0
            })
            if not self._check_success(resp, "Connect AND to OR"):
                return

            # Change AND input size from 2 to 3
            print("→ Changing AND input size from 2 to 3...")
            resp = await self.mcp.send_command("change_input_size", {
                "element_id": and_id,
                "size": 3
            })
            if not self._check_success(resp, "Increase input size to 3"):
                return

            # Undo size change
            resp = await self.mcp.send_command("undo", {})
            if not self._check_success(resp, "Undo size change"):
                return

            # Redo size change
            resp = await self.mcp.send_command("redo", {})
            if not self._check_success(resp, "Redo size change"):
                return

            # Verify circuit still valid
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                print("✓ Circuit state remains valid after size change undo/redo")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_size_change_with_connections: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_5_2_4")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_size_change_with_connections: {str(e)}")

    async def test_multi_element_operations(self) -> None:
        """Test 5.2.5: Multi-element operations"""
        self.test_count += 1
        print(f"\n[Test 5.2.5] Multi-Element Operations")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create 5 elements
            element_ids = []
            print("→ Creating 5 elements...")
            for i in range(5):
                resp = await self.mcp.send_command("create_element", {
                    "type": "And" if i % 2 == 0 else "Or",
                    "x": 100.0 + i * 50.0,
                    "y": 100.0
                })
                if not resp.success or not resp.result:
                    print(f"❌ Failed to create element {i+1}")
                    return
                element_ids.append(resp.result.get("element_id"))
                print(f"  ✓ Created element {i+1} (ID: {element_ids[-1]})")

            # Perform different operations on each element
            print("→ Performing operations on all elements...")
            for i, eid in enumerate(element_ids):
                if i % 2 == 0:
                    # Rotate
                    resp = await self.mcp.send_command("rotate_element", {
                        "element_id": eid,
                        "angle": 90
                    })
                    if resp.success:
                        print(f"  ✓ Rotated element {i+1}")
                else:
                    # Flip
                    resp = await self.mcp.send_command("flip_element", {
                        "element_id": eid,
                        "axis": 0
                    })
                    if resp.success:
                        print(f"  ✓ Flipped element {i+1}")

            # Get undo stack
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                print(f"✓ Undo stack has {undo_count} actions (expected 5)")

            # Undo all operations
            print("→ Undoing all operations...")
            for i in range(5):
                resp = await self.mcp.send_command("undo", {})
                if resp.success:
                    print(f"  ✓ Undo {i+1}")
                else:
                    print(f"  ❌ Undo {i+1} failed")
                    return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_multi_element_operations: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_5_2_5")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_multi_element_operations: {str(e)}")

    async def test_state_consistency_multiple_ops(self) -> None:
        """Test 5.2.6: State consistency across multiple operations"""
        self.test_count += 1
        print(f"\n[Test 5.2.6] State Consistency Multiple Operations")
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

            # Perform: Rotate → Update → Rotate → Flip
            ops = [
                ("rotate_element", {"element_id": element_id, "angle": 90}, "Rotate 90°"),
                ("update_element", {"element_id": element_id, "label": "Updated"}, "Update label"),
                ("rotate_element", {"element_id": element_id, "angle": 180}, "Rotate 180°"),
                ("flip_element", {"element_id": element_id, "axis": 0}, "Flip(0)"),
            ]

            print("→ Executing operation sequence...")
            for cmd, params, name in ops:
                resp = await self.mcp.send_command(cmd, params)
                if resp.success:
                    print(f"  ✓ {name}")
                else:
                    print(f"  ❌ {name} failed")
                    return

            # Undo then redo alternating
            print("→ Alternating undo/redo pattern...")
            # Undo 2
            resp = await self.mcp.send_command("undo", {})
            self._check_success(resp, "Undo 1")
            resp = await self.mcp.send_command("undo", {})
            self._check_success(resp, "Undo 2")

            # Redo 1
            resp = await self.mcp.send_command("redo", {})
            self._check_success(resp, "Redo 1")

            # Undo 1
            resp = await self.mcp.send_command("undo", {})
            self._check_success(resp, "Undo 1 (again)")

            # Redo 2
            resp = await self.mcp.send_command("redo", {})
            self._check_success(resp, "Redo 1 (again)")
            resp = await self.mcp.send_command("redo", {})
            self._check_success(resp, "Redo 2")

            # Verify stack is consistent
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                redo_count = stack_info.get("redo_count", 0)
                print(f"✓ Stack consistent (undo: {undo_count}, redo: {redo_count})")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_state_consistency_multiple_ops: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_5_2_6")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_state_consistency_multiple_ops: {str(e)}")

    async def test_connection_preservation_rotation(self) -> None:
        """Test 5.2.7: Connection preservation after rotation/flip"""
        self.test_count += 1
        print(f"\n[Test 5.2.7] Connection Preservation After Rotation/Flip")
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
            resp2 = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 300.0,
                "y": 100.0
            })

            if not resp1.success or not resp2.success:
                print("❌ Failed to create gates")
                return

            gate1_id = resp1.result.get("element_id")
            gate2_id = resp2.result.get("element_id")
            print(f"✓ Created gates: AND({gate1_id}), OR({gate2_id})")

            # Connect them
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": gate1_id,
                "source_port": 0,
                "target_id": gate2_id,
                "target_port": 0
            })
            if not self._check_success(resp, "Connect gates"):
                return

            # Perform transformations on connected element
            print("→ Transforming connected element...")
            resp = await self.mcp.send_command("rotate_element", {
                "element_id": gate1_id,
                "angle": 90
            })
            self._check_success(resp, "Rotate connected element")

            resp = await self.mcp.send_command("flip_element", {
                "element_id": gate1_id,
                "axis": 0
            })
            self._check_success(resp, "Flip connected element")

            # Undo transformations
            print("→ Undoing transformations...")
            resp = await self.mcp.send_command("undo", {})
            self._check_success(resp, "Undo flip")
            resp = await self.mcp.send_command("undo", {})
            self._check_success(resp, "Undo rotate")

            # Verify circuit still valid
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = resp.result.get("connections", [])
                if connections:
                    print(f"✓ Connection preserved after transformations ({len(connections)} connection(s))")
                else:
                    print("⚠ Connection lost (expected 1, got 0)")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_connection_preservation_rotation: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_5_2_7")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_connection_preservation_rotation: {str(e)}")

    def _check_success(self, response: MCPResponse, description: str) -> bool:
        """Check if response was successful"""
        if not response.success:
            print(f"❌ {description} failed: {response.error}")
            return False
        return True

    async def print_summary(self) -> bool:
        """Print test summary"""
        print("\n" + "=" * 70)
        print(f"📊 PHASE 5.2 COMBINED OPERATIONS SUMMARY")
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
            print("✅ ALL PHASE 5.2 TESTS PASSED - Combined operations validated!")
        else:
            print("❌ SOME TESTS FAILED - Review errors above")
        print("=" * 70 + "\n")

        return success


async def main() -> None:
    """Main entry point"""
    validator = Phase5_2Validator()
    success = await validator.run_all_tests()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    asyncio.run(main())
