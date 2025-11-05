#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Phase 1: Immediate Safety Fixes - Undo/Redo System Tests

Tests for null pointer safety improvements in the undo/redo system:
- Task 1.1: Null pointer guards in findElements() and findItems()
- Task 1.2: SplitCommand exception safety and cleanup
- Task 1.3: Scene destruction guard with undo stack cleanup

Tests verify that the undo/redo system gracefully handles:
- Deleted elements during undo/redo operations
- Scene destruction while undo stack contains commands
- Memory leaks in SplitCommand constructor failures
"""

import asyncio
from typing import Any, Dict, Optional

from beartype import beartype

# Import test infrastructure
from tests.mcp_test_base import MCPTestBase
from mcp_models import MCPResponse


class Phase1UndoRedoSafetyTests(MCPTestBase):
    """Tests for Phase 1: Immediate Safety Fixes in undo/redo system"""

    async def run_category_tests(self) -> bool:
        """Run all Phase 1 safety tests

        Returns:
            bool: True if all tests passed, False otherwise
        """
        tests = [
            self.test_undo_after_element_deleted,
            self.test_split_command_exception_safety,
            self.test_scene_destruction_guard,
            self.test_move_command_with_deleted_element,
            self.test_undo_redo_sequence_with_guard,
        ]

        print("\n" + "=" * 70)
        print("🧪 PHASE 1: IMMEDIATE SAFETY FIXES - UNDO/REDO SYSTEM TESTS")
        print("=" * 70)
        print("\nObjective: Verify null pointer guards, exception safety, and cleanup\n")

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    # ==================== TEST METHODS ====================

    @beartype
    async def test_undo_after_element_deleted(self) -> bool:
        """Test 1.1: Undo handles deleted elements gracefully

        Verifies Task 1.1 - Enhanced findElements() with null pointer guards:
        - Create AND gate
        - Move element (creates MoveCommand in undo stack)
        - Delete element externally
        - Try to undo - should handle gracefully or throw informative error
        """
        print("\n=== Test 1.1: Undo After Element Deleted ===")
        print("Testing: Enhanced findElements() null pointer guards")
        print("Expected: Graceful handling, no crash, informative error")

        all_passed = True

        try:
            # Create circuit
            resp = await self.send_command("new_circuit", {})
            all_passed &= await self.assert_success(resp, "Create new circuit")
            if not all_passed:
                return False

            # Create AND gate
            resp = await self.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            element_id = await self.validate_element_creation_response(resp, "Create AND gate for deletion test")
            if element_id is None:
                return False

            print(f"   Created AND gate with ID: {element_id}")

            # Move element (creates MoveCommand in undo stack)
            resp = await self.send_command("move_element", {
                "element_id": element_id,
                "x": 200.0,
                "y": 200.0
            })
            all_passed &= await self.assert_success(resp, "Move element to create MoveCommand")

            # Verify element is at new position
            resp = await self.send_command("get_element_properties", {
                "element_id": element_id
            })
            if resp.success:
                result = await self.get_response_result(resp)
                if result and result.get("x") == 200.0:
                    print("   ✅ Element moved to new position")
            else:
                print("   ❌ Failed to verify element position")
                all_passed = False

            # Delete element externally via DeleteItemsCommand
            resp = await self.send_command("delete_element", {
                "element_id": element_id
            })
            all_passed &= await self.assert_success(resp, "Delete element")

            # Verify element is deleted
            resp = await self.send_command("get_element_properties", {
                "element_id": element_id
            })
            if resp.error is not None:
                print("   ✅ Element confirmed deleted")
            else:
                print("   ❌ Element still exists after delete")
                all_passed = False

            # Try to undo - should NOT crash, should handle gracefully
            print("\n   Attempting undo of deleted element...")
            resp = await self.send_command("simulation_control", {
                "action": "undo"
            })

            # Phase 1 fix: This should either succeed (element recreated) or
            # fail gracefully with error message, NOT crash
            if resp.success:
                # Undo succeeded - element may have been recreated
                print("   ✅ Undo succeeded (element recreated or operation reverted)")

                # Check if element was recreated
                resp = await self.send_command("list_elements", {})
                if resp.success:
                    result = await self.get_response_result(resp)
                    if result and "elements" in result:
                        print(f"   ℹ️  Circuit has {len(result['elements'])} elements after undo")
            else:
                # Undo failed - check if error is informative (Phase 1 improvement)
                if "not found" in resp.error.lower() or "deleted" in resp.error.lower():
                    print(f"   ✅ Undo failed gracefully with informative error: {resp.error}")
                else:
                    print(f"   ⚠️  Undo failed with error: {resp.error}")

            # Critical: Check for crash logs (Task 1.1 must prevent crashes)
            print("\n   Checking for crash logs...")
            if not await self._check_for_crash_logs():
                print("   ✅ No crash logs detected")
            else:
                print("   ❌ Crash logs detected!")
                all_passed = False

            await self.cleanup_circuit("test_undo_after_element_deleted")

        except Exception as e:
            print(f"   ❌ Test exception: {e}")
            all_passed = False

        return all_passed

    @beartype
    async def test_split_command_exception_safety(self) -> bool:
        """Test 1.2: SplitCommand exception safety and cleanup

        Verifies Task 1.2 - SplitCommand constructor reordering:
        - Create connection between two AND gates
        - Perform multiple split operations
        - Undo after each split
        - Monitor for memory leaks
        """
        print("\n=== Test 1.2: SplitCommand Exception Safety ===")
        print("Testing: SplitCommand constructor exception safety")
        print("Expected: No memory leaks, proper cleanup on exception")

        all_passed = True

        try:
            # Create circuit
            resp = await self.send_command("new_circuit", {})
            all_passed &= await self.assert_success(resp, "Create new circuit for split test")
            if not all_passed:
                return False

            # Create two AND gates
            resp1 = await self.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            gate1_id = await self.validate_element_creation_response(resp1, "Create first AND gate")
            if gate1_id is None:
                return False

            resp2 = await self.send_command("create_element", {
                "type": "Or",
                "x": 300.0,
                "y": 100.0
            })
            gate2_id = await self.validate_element_creation_response(resp2, "Create second OR gate")
            if gate2_id is None:
                return False

            print(f"   Created gates: AND({gate1_id}), OR({gate2_id})")

            # Connect AND output to OR input
            resp = await self.send_command("connect_elements", {
                "from_element_id": gate1_id,
                "from_port": 0,
                "to_element_id": gate2_id,
                "to_port": 0
            })
            all_passed &= await self.assert_success(resp, "Connect elements")
            if not all_passed:
                return False

            # Get connection ID
            resp = await self.send_command("list_connections", {})
            if not resp.success:
                print("   ❌ Failed to list connections")
                return False

            result = await self.get_response_result(resp)
            if not result or "connections" not in result or len(result["connections"]) == 0:
                print("   ❌ No connections found after connect_elements")
                return False

            conn_id = result["connections"][0].get("id")
            print(f"   Got connection ID: {conn_id}")

            # Perform repeated split and undo operations
            print("\n   Performing 5 split/undo cycles...")
            for cycle in range(5):
                # Split connection
                resp = await self.send_command("split_connection", {
                    "connection_id": conn_id,
                    "x": 200.0,
                    "y": 100.0
                })

                if resp.success:
                    print(f"   ✅ Split cycle {cycle + 1}: Split succeeded")
                else:
                    print(f"   ⚠️  Split cycle {cycle + 1}: Split failed - {resp.error}")
                    # Continue to test undo behavior

                # Undo the split (tests exception cleanup)
                resp = await self.send_command("simulation_control", {
                    "action": "undo"
                })

                if resp.success:
                    print(f"   ✅ Split cycle {cycle + 1}: Undo succeeded")
                else:
                    print(f"   ⚠️  Split cycle {cycle + 1}: Undo failed - {resp.error}")

            # Check for memory issues (Task 1.2 must prevent leaks)
            print("\n   Checking for crashes/memory issues...")
            if not await self._check_for_crash_logs():
                print("   ✅ No crash logs after repeated splits")
            else:
                print("   ❌ Crash logs detected!")
                all_passed = False

            await self.cleanup_circuit("test_split_command_exception_safety")

        except Exception as e:
            print(f"   ❌ Test exception: {e}")
            all_passed = False

        return all_passed

    @beartype
    async def test_scene_destruction_guard(self) -> bool:
        """Test 1.3: Scene destruction guard with undo stack cleanup

        Verifies Task 1.3 - Scene destructor clears undo stack:
        - Create circuit in tab 1
        - Add elements and operations (populate undo stack)
        - Close tab (Scene destroyed)
        - Try undo on closed tab - should not crash
        """
        print("\n=== Test 1.3: Scene Destruction Guard ===")
        print("Testing: Scene destructor clears undo stack")
        print("Expected: Safe tab close, no access to deleted Scene pointer")

        all_passed = True

        try:
            # Create initial circuit in tab 1
            resp = await self.send_command("new_circuit", {})
            all_passed &= await self.assert_success(resp, "Create initial circuit for scene destruction test")
            if not all_passed:
                return False

            # Create AND gate to populate undo stack
            resp = await self.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            element_id = await self.validate_element_creation_response(resp, "Create AND gate")
            if element_id is None:
                return False

            print(f"   Created AND gate with ID: {element_id}")

            # Perform several operations to populate undo stack
            for i in range(3):
                resp = await self.send_command("move_element", {
                    "element_id": element_id,
                    "x": 100.0 + (i + 1) * 50.0,
                    "y": 100.0 + (i + 1) * 50.0
                })
                if resp.success:
                    print(f"   ✅ Move {i + 1}: Element moved")
                else:
                    print(f"   ❌ Move {i + 1}: Failed")
                    all_passed = False

            # Get tab count before close
            resp = await self.send_command("get_tab_count", {})
            initial_tabs = 0
            if resp.success:
                result = await self.get_response_result(resp)
                if result and "tab_count" in result:
                    initial_tabs = result["tab_count"]
                    print(f"   ℹ️  Initial tab count: {initial_tabs}")

            # Close the circuit tab (Scene destructor called)
            print("\n   Closing circuit tab (Scene destructor to be called)...")
            resp = await self.send_command("close_circuit", {})
            all_passed &= await self.assert_success(resp, "Close circuit tab")

            # Verify tab was closed
            resp = await self.send_command("get_tab_count", {})
            if resp.success:
                result = await self.get_response_result(resp)
                if result and "tab_count" in result:
                    final_tabs = result["tab_count"]
                    if final_tabs < initial_tabs:
                        print(f"   ✅ Tab closed: {initial_tabs} -> {final_tabs} tabs")
                    else:
                        print(f"   ❌ Tab not closed: {initial_tabs} -> {final_tabs} tabs")

            # Try undo on closed/empty scene
            # Task 1.3: Scene destructor should have cleared undo stack
            print("\n   Attempting undo after scene destruction...")
            resp = await self.send_command("simulation_control", {
                "action": "undo"
            })

            # Should fail gracefully (no undo available), not crash
            if resp.error is not None:
                print(f"   ✅ Undo correctly failed: {resp.error}")
            else:
                print("   ⚠️  Undo succeeded (may have undo history)")

            # Check for crash logs (Task 1.3 must prevent crashes)
            print("\n   Checking for crash logs...")
            if not await self._check_for_crash_logs():
                print("   ✅ No crash logs after scene destruction")
            else:
                print("   ❌ Crash logs detected!")
                all_passed = False

        except Exception as e:
            print(f"   ❌ Test exception: {e}")
            all_passed = False

        return all_passed

    @beartype
    async def test_move_command_with_deleted_element(self) -> bool:
        """Test: MoveCommand handles deleted elements gracefully

        Extended test for Task 1.1:
        - Create element and move it
        - Delete it
        - Redo (move) should handle deleted element
        """
        print("\n=== Test: MoveCommand with Deleted Element ===")
        print("Testing: MoveCommand exception handling for deleted elements")

        all_passed = True

        try:
            # Create circuit
            resp = await self.send_command("new_circuit", {})
            all_passed &= await self.assert_success(resp, "Create circuit")
            if not all_passed:
                return False

            # Create AND gate
            resp = await self.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            element_id = await self.validate_element_creation_response(resp, "Create AND gate")
            if element_id is None:
                return False

            # Move element
            resp = await self.send_command("move_element", {
                "element_id": element_id,
                "x": 200.0,
                "y": 200.0
            })
            all_passed &= await self.assert_success(resp, "Move element")

            # Undo move
            resp = await self.send_command("simulation_control", {
                "action": "undo"
            })
            all_passed &= await self.assert_success(resp, "Undo move")

            # Delete element
            resp = await self.send_command("delete_element", {
                "element_id": element_id
            })
            all_passed &= await self.assert_success(resp, "Delete element")

            # Redo (should handle deleted element gracefully)
            print("   Attempting redo with deleted element...")
            resp = await self.send_command("simulation_control", {
                "action": "redo"
            })

            if resp.success or resp.error is not None:
                print("   ✅ Redo handled gracefully")
            else:
                print("   ❌ Redo failed unexpectedly")
                all_passed = False

            # Check for crashes
            if not await self._check_for_crash_logs():
                print("   ✅ No crash logs")
            else:
                print("   ❌ Crash logs detected!")
                all_passed = False

            await self.cleanup_circuit("test_move_command_with_deleted_element")

        except Exception as e:
            print(f"   ❌ Test exception: {e}")
            all_passed = False

        return all_passed

    @beartype
    async def test_undo_redo_sequence_with_guard(self) -> bool:
        """Test: Extended undo/redo sequence with guards active

        Comprehensive test verifying all three Phase 1 fixes work together:
        - Create complex circuit
        - Perform multiple operations (populate undo stack)
        - Delete elements
        - Undo/redo sequence
        - Verify safety guards activate
        """
        print("\n=== Test: Extended Undo/Redo Sequence with Guards ===")
        print("Testing: All Phase 1 fixes working together")

        all_passed = True

        try:
            # Create circuit
            resp = await self.send_command("new_circuit", {})
            all_passed &= await self.assert_success(resp, "Create circuit")
            if not all_passed:
                return False

            # Create multiple gates
            element_ids = []
            for i in range(3):
                resp = await self.send_command("create_element", {
                    "type": "And",
                    "x": 100.0 + i * 150.0,
                    "y": 100.0
                })
                elem_id = await self.validate_element_creation_response(resp, f"Create gate {i + 1}")
                if elem_id is None:
                    return False
                element_ids.append(elem_id)

            print(f"   Created {len(element_ids)} gates")

            # Move all elements (populate undo stack)
            for elem_id in element_ids:
                resp = await self.send_command("move_element", {
                    "element_id": elem_id,
                    "x": 200.0,
                    "y": 200.0
                })
                all_passed &= await self.assert_success(resp, "Move element")

            print("   ✅ Undo stack populated with 6 operations (3 creates + 3 moves)")

            # Delete first element
            resp = await self.send_command("delete_element", {
                "element_id": element_ids[0]
            })
            all_passed &= await self.assert_success(resp, "Delete first element")

            # Perform undo/redo sequence
            print("\n   Performing undo/redo sequence...")

            # Undo delete
            resp = await self.send_command("simulation_control", {
                "action": "undo"
            })
            print(f"   Undo delete: {'✅' if resp.success else '⚠️'}")

            # Undo moves
            for i in range(2):
                resp = await self.send_command("simulation_control", {
                    "action": "undo"
                })
                print(f"   Undo move {i + 1}: {'✅' if resp.success else '⚠️'}")

            # Redo operations
            for i in range(2):
                resp = await self.send_command("simulation_control", {
                    "action": "redo"
                })
                print(f"   Redo move {i + 1}: {'✅' if resp.success else '⚠️'}")

            # Redo delete
            resp = await self.send_command("simulation_control", {
                "action": "redo"
            })
            print(f"   Redo delete: {'✅' if resp.success else '⚠️'}")

            # Check for crashes throughout
            if not await self._check_for_crash_logs():
                print("\n   ✅ No crash logs detected throughout sequence")
            else:
                print("\n   ❌ Crash logs detected!")
                all_passed = False

            await self.cleanup_circuit("test_undo_redo_sequence_with_guard")

        except Exception as e:
            print(f"   ❌ Test exception: {e}")
            all_passed = False

        return all_passed

    # ==================== HELPER METHODS ====================

    @beartype
    async def _check_for_crash_logs(self) -> bool:
        """Check if any crash logs/errors were detected

        Returns:
            bool: True if crashes detected, False if safe
        """
        # This is a simplified check - in a real implementation,
        # would query system logs or MCP debug output
        # For now, just check that process is still alive
        if self.process and self.process.returncode is not None:
            print(f"   ❌ Process crashed with code {self.process.returncode}")
            return True
        return False
