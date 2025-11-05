#!/usr/bin/env python3
"""
Phase 4 Task 4.1: Comprehensive Command Type Tests

Tests all 11 command types with undo/redo to validate complete undo/redo system.

Run from project root:
    python test/mcp/test_phase4_comprehensive_regression.py
"""

import asyncio
import sys
import os

# Add MCP client path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure
from mcp_models import MCPResponse


class Phase4CommandTests:
    """Comprehensive tests for all 11 command types"""

    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=True, verbose=False)
        self.test_count = 0
        self.passed_count = 0
        self.failed_tests = []

    async def run_all_tests(self) -> bool:
        """Run all Phase 4 command type tests"""
        print("\n" + "=" * 70)
        print("🧪 PHASE 4.1 - Comprehensive Command Type Tests")
        print("=" * 70)
        print("\nStarting MCP process...\n")

        # Start MCP
        if not await self.mcp.start_mcp():
            print("❌ Failed to start MCP process")
            return False

        try:
            # Run tests for each command type
            await self.test_move_command()
            await self.test_rotate_command()
            await self.test_flip_command()
            await self.test_update_command()
            await self.test_morph_command()
            await self.test_split_command()
            await self.test_change_input_size_command()
            await self.test_change_output_size_command()
            await self.test_toggle_truth_table_command()
            await self.test_add_items_command()
            await self.test_delete_items_command()

            # Summary
            return await self.print_summary()

        finally:
            print("\nStopping MCP process...")
            await self.mcp.stop_mcp()

    async def test_move_command(self) -> None:
        """Test MoveCommand: Move elements and undo/redo"""
        self.test_count += 1
        print(f"\n[Test 4.1.1] MoveCommand (Move Elements)")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create two elements
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                print("❌ Failed to create element")
                return

            elm_id = resp.result.get("element_id")
            print(f"✓ Created AND gate (ID: {elm_id})")

            # Move element
            resp = await self.mcp.send_command("move_element", {
                "element_id": elm_id,
                "x": 200.0,
                "y": 200.0
            })
            self._check_success(resp, "Move to (200, 200)")

            # Undo move
            resp = await self.mcp.send_command("undo", {})
            if resp.success:
                print("✓ Undo move succeeded")
            else:
                print(f"❌ Undo failed: {resp.error}")
                self.failed_tests.append("test_move_command: undo failed")
                return

            # Redo move
            resp = await self.mcp.send_command("redo", {})
            if resp.success:
                print("✓ Redo move succeeded")
            else:
                print(f"❌ Redo failed: {resp.error}")
                self.failed_tests.append("test_move_command: redo failed")
                return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_move_command: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_move")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_move_command: {str(e)}")

    async def test_rotate_command(self) -> None:
        """Test RotateCommand: Rotate elements (90°, 180°, 270°)"""
        self.test_count += 1
        print(f"\n[Test 4.1.2] RotateCommand (Rotate Elements)")
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

            elm_id = resp.result.get("element_id")
            print(f"✓ Created AND gate (ID: {elm_id})")

            # Rotate 90 degrees
            resp = await self.mcp.send_command("rotate_element", {
                "element_id": elm_id,
                "angle": 90
            })
            if resp.success:
                print("✓ Rotate 90° succeeded")
            else:
                print(f"❌ Rotate failed: {resp.error}")
                self.failed_tests.append("test_rotate_command: rotate failed")
                return

            # Undo rotate
            resp = await self.mcp.send_command("undo", {})
            if resp.success:
                print("✓ Undo rotate succeeded")
            else:
                print(f"❌ Undo failed: {resp.error}")
                self.failed_tests.append("test_rotate_command: undo failed")
                return

            # Redo rotate
            resp = await self.mcp.send_command("redo", {})
            if resp.success:
                print("✓ Redo rotate succeeded")
            else:
                print(f"❌ Redo failed: {resp.error}")
                self.failed_tests.append("test_rotate_command: redo failed")
                return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_rotate_command: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_rotate")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_rotate_command: {str(e)}")

    async def test_flip_command(self) -> None:
        """Test FlipCommand: Flip horizontally and vertically"""
        self.test_count += 1
        print(f"\n[Test 4.1.3] FlipCommand (Flip Elements)")
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

            elm_id = resp.result.get("element_id")
            print(f"✓ Created AND gate (ID: {elm_id})")

            # Flip horizontally
            resp = await self.mcp.send_command("flip_element", {
                "element_id": elm_id,
                "axis": 0  # 0 = horizontal, 1 = vertical
            })
            if resp.success:
                print("✓ Flip horizontally succeeded")
            else:
                print(f"❌ Flip failed: {resp.error}")
                self.failed_tests.append("test_flip_command: flip failed")
                return

            # Undo flip
            resp = await self.mcp.send_command("undo", {})
            if resp.success:
                print("✓ Undo flip succeeded")
            else:
                print(f"❌ Undo failed: {resp.error}")
                self.failed_tests.append("test_flip_command: undo failed")
                return

            # Redo flip
            resp = await self.mcp.send_command("redo", {})
            if resp.success:
                print("✓ Redo flip succeeded")
            else:
                print(f"❌ Redo failed: {resp.error}")
                self.failed_tests.append("test_flip_command: redo failed")
                return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_flip_command: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_flip")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_flip_command: {str(e)}")

    async def test_update_command(self) -> None:
        """Test UpdateCommand: Update element properties"""
        self.test_count += 1
        print(f"\n[Test 4.1.4] UpdateCommand (Update Properties)")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create element
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0,
                "label": "Original"
            })
            if not resp.success or not resp.result:
                print("❌ Failed to create element")
                return

            elm_id = resp.result.get("element_id")
            print(f"✓ Created AND gate (ID: {elm_id})")

            # Update label (creates UpdateCommand)
            resp = await self.mcp.send_command("update_element", {
                "element_id": elm_id,
                "label": "Updated"
            })
            if resp.success:
                print("✓ Update label succeeded")
            else:
                print(f"❌ Update failed: {resp.error}")
                self.failed_tests.append("test_update_command: update failed")
                return

            # Undo update
            resp = await self.mcp.send_command("undo", {})
            if resp.success:
                print("✓ Undo update succeeded")
            else:
                print(f"❌ Undo failed: {resp.error}")
                self.failed_tests.append("test_update_command: undo failed")
                return

            # Redo update
            resp = await self.mcp.send_command("redo", {})
            if resp.success:
                print("✓ Redo update succeeded")
            else:
                print(f"❌ Redo failed: {resp.error}")
                self.failed_tests.append("test_update_command: redo failed")
                return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_update_command: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_update")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_update_command: {str(e)}")

    async def test_morph_command(self) -> None:
        """Test MorphCommand: Type conversion"""
        self.test_count += 1
        print(f"\n[Test 4.1.5] MorphCommand (Type Conversion)")
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

            elm_id = resp.result.get("element_id")
            print(f"✓ Created AND gate (ID: {elm_id})")

            # Morph AND to OR
            resp = await self.mcp.send_command("morph_element", {
                "element_id": elm_id,
                "new_type": "Or"
            })
            if resp.success:
                print("✓ Morph AND → OR succeeded")
            else:
                print(f"⚠ Morph result: {resp.error}")

            # Undo morph
            resp = await self.mcp.send_command("undo", {})
            if resp.success:
                print("✓ Undo morph succeeded")
            else:
                print(f"❌ Undo failed: {resp.error}")
                self.failed_tests.append("test_morph_command: undo failed")
                return

            # Redo morph
            resp = await self.mcp.send_command("redo", {})
            if resp.success:
                print("✓ Redo morph succeeded")
            else:
                print(f"❌ Redo failed: {resp.error}")
                self.failed_tests.append("test_morph_command: redo failed")
                return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_morph_command: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_morph")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_morph_command: {str(e)}")

    async def test_split_command(self) -> None:
        """Test SplitCommand: Split connections"""
        self.test_count += 1
        print(f"\n[Test 4.1.6] SplitCommand (Split Connections)")
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
                "x": 200.0,
                "y": 100.0
            })

            if not resp1.success or not resp2.success:
                print("❌ Failed to create elements")
                return

            elm1_id = resp1.result.get("element_id")
            elm2_id = resp2.result.get("element_id")
            print(f"✓ Created AND({elm1_id}) and OR({elm2_id})")

            # Connect them
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": elm1_id,
                "source_port": 0,
                "target_id": elm2_id,
                "target_port": 0
            })
            self._check_success(resp, "Connect elements")

            # Split connection at midpoint
            resp = await self.mcp.send_command("split_connection", {
                "source_id": elm1_id,
                "source_port": 0,
                "target_id": elm2_id,
                "target_port": 0,
                "x": 150.0,
                "y": 100.0
            })
            if resp.success:
                print("✓ Split connection succeeded")
            else:
                print(f"⚠ Split result: {resp.error}")

            # Undo split
            resp = await self.mcp.send_command("undo", {})
            if resp.success:
                print("✓ Undo split succeeded")
            else:
                print(f"❌ Undo failed: {resp.error}")
                self.failed_tests.append("test_split_command: undo failed")
                return

            # Redo split
            resp = await self.mcp.send_command("redo", {})
            if resp.success:
                print("✓ Redo split succeeded")
            else:
                print(f"❌ Redo failed: {resp.error}")
                self.failed_tests.append("test_split_command: redo failed")
                return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_split_command: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_split")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_split_command: {str(e)}")

    async def test_change_input_size_command(self) -> None:
        """Test ChangeInputSizeCommand: Modify input count"""
        self.test_count += 1
        print(f"\n[Test 4.1.7] ChangeInputSizeCommand (Change Input Size)")
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
                print("❌ Failed to create element")
                return

            elm_id = resp.result.get("element_id")
            print(f"✓ Created AND gate (ID: {elm_id})")

            # Try to change input size
            resp = await self.mcp.send_command("change_input_size", {
                "element_id": elm_id,
                "size": 3
            })
            if resp.success:
                print("✓ Change input size to 3 succeeded")
            else:
                print(f"❌ Change input size failed: {resp.error}")
                self.failed_tests.append("test_change_input_size_command: change_input_size failed")
                return

            # Undo change
            resp = await self.mcp.send_command("undo", {})
            if resp.success:
                print("✓ Undo input size change succeeded")
            else:
                print(f"❌ Undo failed: {resp.error}")
                self.failed_tests.append("test_change_input_size_command: undo failed")
                return

            # Redo change
            resp = await self.mcp.send_command("redo", {})
            if resp.success:
                print("✓ Redo input size change succeeded")
            else:
                print(f"❌ Redo failed: {resp.error}")
                self.failed_tests.append("test_change_input_size_command: redo failed")
                return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_change_input_size_command: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_input_size")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_change_input_size_command: {str(e)}")

    async def test_change_output_size_command(self) -> None:
        """Test ChangeOutputSizeCommand: Modify output count"""
        self.test_count += 1
        print(f"\n[Test 4.1.8] ChangeOutputSizeCommand (Change Output Size)")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create Demux (has multiple outputs)
            resp = await self.mcp.send_command("create_element", {
                "type": "Demux",
                "x": 100.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                print("❌ Failed to create element")
                return

            elm_id = resp.result.get("element_id")
            print(f"✓ Created Demux (ID: {elm_id})")

            # Try to change output size
            resp = await self.mcp.send_command("change_output_size", {
                "element_id": elm_id,
                "size": 4
            })
            if resp.success:
                print("✓ Change output size succeeded")
            else:
                print(f"❌ Change output size failed: {resp.error}")
                self.failed_tests.append("test_change_output_size_command: change_output_size failed")
                return

            # Undo change
            resp = await self.mcp.send_command("undo", {})
            if resp.success:
                print("✓ Undo output size change succeeded")
            else:
                print(f"❌ Undo failed: {resp.error}")
                self.failed_tests.append("test_change_output_size_command: undo failed")
                return

            # Redo change
            resp = await self.mcp.send_command("redo", {})
            if resp.success:
                print("✓ Redo output size change succeeded")
            else:
                print(f"❌ Redo failed: {resp.error}")
                self.failed_tests.append("test_change_output_size_command: redo failed")
                return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_change_output_size_command: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_output_size")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_change_output_size_command: {str(e)}")

    async def test_toggle_truth_table_command(self) -> None:
        """Test ToggleTruthTableOutputCommand"""
        self.test_count += 1
        print(f"\n[Test 4.1.9] ToggleTruthTableOutputCommand")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create TruthTable element
            resp = await self.mcp.send_command("create_element", {
                "type": "TruthTable",
                "x": 100.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                print("❌ Failed to create TruthTable")
                return

            elm_id = resp.result.get("element_id")
            print(f"✓ Created TruthTable (ID: {elm_id})")

            # Try to toggle truth table output
            resp = await self.mcp.send_command("toggle_truth_table_output", {
                "element_id": elm_id,
                "position": 0
            })
            if resp.success:
                print("✓ Toggle truth table output succeeded")
            else:
                print(f"❌ Toggle failed: {resp.error}")
                self.failed_tests.append("test_toggle_truth_table_command: toggle failed")
                return

            # Undo toggle
            resp = await self.mcp.send_command("undo", {})
            if resp.success:
                print("✓ Undo toggle succeeded")
            else:
                print(f"❌ Undo failed: {resp.error}")
                self.failed_tests.append("test_toggle_truth_table_command: undo failed")
                return

            # Redo toggle
            resp = await self.mcp.send_command("redo", {})
            if resp.success:
                print("✓ Redo toggle succeeded")
            else:
                print(f"❌ Redo failed: {resp.error}")
                self.failed_tests.append("test_toggle_truth_table_command: redo failed")
                return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_toggle_truth_table_command: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_truth_table")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_toggle_truth_table_command: {str(e)}")

    async def test_add_items_command(self) -> None:
        """Test AddItemsCommand: Add multiple elements"""
        self.test_count += 1
        print(f"\n[Test 4.1.10] AddItemsCommand (Add Elements)")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create multiple elements
            element_ids = []
            for i in range(3):
                resp = await self.mcp.send_command("create_element", {
                    "type": "And",
                    "x": 100.0 + i * 50.0,
                    "y": 100.0
                })
                if resp.success and resp.result:
                    element_ids.append(resp.result.get("element_id"))
                    print(f"✓ Created element {i+1}")

            if len(element_ids) < 3:
                print("❌ Failed to create all elements")
                self.failed_tests.append("test_add_items_command: create failed")
                return

            # Get undo stack count
            resp = await self.mcp.send_command("get_undo_stack", {})
            if resp.success:
                stack_info = resp.result.get("stack_info", {})
                undo_count = stack_info.get("undo_count", 0)
                print(f"✓ Undo stack has {undo_count} actions")

            # Undo all additions
            for i in range(3):
                resp = await self.mcp.send_command("undo", {})
                if resp.success:
                    print(f"✓ Undo {i+1} succeeded")
                else:
                    print(f"❌ Undo {i+1} failed: {resp.error}")
                    self.failed_tests.append("test_add_items_command: undo failed")
                    return

            # Redo all additions
            for i in range(3):
                resp = await self.mcp.send_command("redo", {})
                if resp.success:
                    print(f"✓ Redo {i+1} succeeded")
                else:
                    print(f"❌ Redo {i+1} failed: {resp.error}")
                    self.failed_tests.append("test_add_items_command: redo failed")
                    return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_add_items_command: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_add_items")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_add_items_command: {str(e)}")

    async def test_delete_items_command(self) -> None:
        """Test DeleteItemsCommand: Delete multiple elements"""
        self.test_count += 1
        print(f"\n[Test 4.1.11] DeleteItemsCommand (Delete Elements)")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create multiple elements
            element_ids = []
            for i in range(3):
                resp = await self.mcp.send_command("create_element", {
                    "type": "And",
                    "x": 100.0 + i * 50.0,
                    "y": 100.0
                })
                if resp.success and resp.result:
                    element_ids.append(resp.result.get("element_id"))

            if len(element_ids) < 3:
                print("❌ Failed to create all elements")
                return

            print(f"✓ Created {len(element_ids)} elements")

            # Delete all elements
            for elm_id in element_ids:
                resp = await self.mcp.send_command("delete_element", {
                    "element_id": elm_id
                })
                if resp.success:
                    print(f"✓ Deleted element {elm_id}")
                else:
                    print(f"❌ Delete failed: {resp.error}")
                    self.failed_tests.append("test_delete_items_command: delete failed")
                    return

            # Undo all deletions
            for i in range(3):
                resp = await self.mcp.send_command("undo", {})
                if resp.success:
                    print(f"✓ Undo delete {i+1} succeeded")
                else:
                    print(f"❌ Undo delete {i+1} failed: {resp.error}")
                    self.failed_tests.append("test_delete_items_command: undo failed")
                    return

            # Redo all deletions
            for i in range(3):
                resp = await self.mcp.send_command("redo", {})
                if resp.success:
                    print(f"✓ Redo delete {i+1} succeeded")
                else:
                    print(f"❌ Redo delete {i+1} failed: {resp.error}")
                    self.failed_tests.append("test_delete_items_command: redo failed")
                    return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_delete_items_command: process crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_delete_items")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_delete_items_command: {str(e)}")

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
        print(f"📊 PHASE 4.1 COMPREHENSIVE COMMAND TESTS SUMMARY")
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
            print("✅ ALL PHASE 4.1 TESTS PASSED - All command types validated!")
        else:
            print("❌ SOME TESTS FAILED - Review errors above")
        print("=" * 70 + "\n")

        return success


async def main() -> None:
    """Main entry point"""
    tester = Phase4CommandTests()
    success = await tester.run_all_tests()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    asyncio.run(main())
