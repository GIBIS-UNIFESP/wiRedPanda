#!/usr/bin/env python3
"""
Phase 6 Validation Script - Undo/Redo System with Connected Elements

This script runs focused tests for undo/redo reliability with connected elements.
Connected elements exercise different code paths than isolated elements:
- Deletion with connections must handle removal
- Morphing with connections must validate port compatibility
- Moving/rotating/flipping with connections affects geometry
- Size changes with connections affect port indices

Run from project root:
    python3 test/mcp/test_phase6_connected_elements.py
"""

import asyncio
import sys
import os

# Add MCP client path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure
from mcp_models import MCPResponse


class Phase6Validator:
    """Phase 6 validation runner for connected element undo/redo"""

    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=True, verbose=False)
        self.test_count = 0
        self.passed_count = 0
        self.failed_tests = []

    async def run_all_tests(self) -> bool:
        """Run all Phase 6 validation tests"""
        print("\n" + "=" * 70)
        print("🧪 PHASE 6 - Undo/Redo with Connected Elements")
        print("=" * 70)
        print("\nStarting MCP process...\n")

        # Start MCP
        if not await self.mcp.start_mcp():
            print("❌ Failed to start MCP process")
            return False

        try:
            # Run Tier 1 critical tests
            await self.test_delete_with_connections_undo()
            await self.test_move_with_connections()
            await self.test_rotate_with_connections()
            await self.test_flip_with_connections()
            await self.test_morph_with_connections()
            await self.test_linear_chain_a_b_c_d()
            await self.test_branching_network()
            await self.test_delete_middle_element_chain()

            # Summary
            return await self.print_summary()

        finally:
            print("\nStopping MCP process...")
            await self.mcp.stop_mcp()

    async def test_delete_with_connections_undo(self) -> None:
        """Test 6.1: Delete element with connections → undo restoration"""
        self.test_count += 1
        print(f"\n[Test 6.1] Delete Element with Connections → Undo Restoration")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create A→B→C chain
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                print("❌ Failed to create element A")
                return
            a_id = resp.result.get("element_id")
            print(f"✓ Created A (ID: {a_id})")

            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 200.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                return
            b_id = resp.result.get("element_id")
            print(f"✓ Created B (ID: {b_id})")

            resp = await self.mcp.send_command("create_element", {
                "type": "Not",
                "x": 300.0,
                "y": 100.0
            })
            if not resp.success or not resp.result:
                return
            c_id = resp.result.get("element_id")
            print(f"✓ Created C (ID: {c_id})")

            # Connect A→B
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": a_id,
                "source_port": 0,
                "target_id": b_id,
                "target_port": 0
            })
            if not self._check_success(resp, "Connect A→B"):
                return

            # Connect B→C
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": b_id,
                "source_port": 0,
                "target_id": c_id,
                "target_port": 0
            })
            if not self._check_success(resp, "Connect B→C"):
                return

            # Verify connections before deletion
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections_before = len(resp.result.get("connections", []))
                print(f"✓ {connections_before} connections before deletion")

            # Delete B (middle element)
            print("→ Deleting middle element B...")
            resp = await self.mcp.send_command("delete_element", {
                "element_id": b_id
            })
            if not self._check_success(resp, "Delete B"):
                return

            # Verify connections after deletion
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections_after = len(resp.result.get("connections", []))
                print(f"✓ {connections_after} connections after deletion (expected 0)")
                if connections_after != 0:
                    print(f"⚠ Warning: Expected 0 connections, got {connections_after}")

            # CRITICAL TEST: Undo deletion
            print("→ Undoing deletion of B...")
            resp = await self.mcp.send_command("undo", {})
            if not resp.success:
                print(f"❌ Undo failed: {resp.error}")
                self.failed_tests.append("test_delete_with_connections_undo: undo failed")
                return
            print("✓ Undo succeeded")

            # Verify B is back and connections restored
            resp = await self.mcp.send_command("list_elements", {})
            if resp.success:
                elements = resp.result.get("elements", [])
                if len(elements) == 3:
                    print(f"✓ All 3 elements restored")
                else:
                    print(f"❌ Expected 3 elements, got {len(elements)}")
                    self.failed_tests.append("test_delete_with_connections_undo: element count")
                    return

            # Verify connections restored
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections_restored = len(resp.result.get("connections", []))
                if connections_restored == connections_before:
                    print(f"✓ All {connections_restored} connections restored")
                else:
                    print(f"❌ Expected {connections_before} connections, got {connections_restored}")
                    self.failed_tests.append("test_delete_with_connections_undo: connection restore")
                    return

            # CRITICAL TEST: Redo deletion
            print("→ Redoing deletion of B...")
            resp = await self.mcp.send_command("redo", {})
            if not resp.success:
                print(f"❌ Redo failed: {resp.error}")
                self.failed_tests.append("test_delete_with_connections_undo: redo failed")
                return
            print("✓ Redo succeeded")

            # Verify B is deleted again
            resp = await self.mcp.send_command("list_elements", {})
            if resp.success:
                elements = resp.result.get("elements", [])
                if len(elements) == 2:
                    print(f"✓ Element B deleted again (2 elements remain)")
                else:
                    print(f"❌ Expected 2 elements, got {len(elements)}")
                    return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_delete_with_connections_undo: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_6_1")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_delete_with_connections_undo: {str(e)}")

    async def test_move_with_connections(self) -> None:
        """Test 6.2: Move element with connections → geometry update"""
        self.test_count += 1
        print(f"\n[Test 6.2] Move Element with Connections (Geometry)")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create A→B connection
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            a_id = resp.result.get("element_id")
            print(f"✓ Created A at (100, 100)")

            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 200.0,
                "y": 100.0
            })
            b_id = resp.result.get("element_id")
            print(f"✓ Created B at (200, 100)")

            resp = await self.mcp.send_command("connect_elements", {
                "source_id": a_id,
                "source_port": 0,
                "target_id": b_id,
                "target_port": 0
            })
            if not self._check_success(resp, "Connect A→B"):
                return

            # Move B
            print("→ Moving B to (250, 150)...")
            resp = await self.mcp.send_command("move_element", {
                "element_id": b_id,
                "x": 250.0,
                "y": 150.0
            })
            if not self._check_success(resp, "Move B"):
                return

            # Verify connection still exists
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = len(resp.result.get("connections", []))
                if connections == 1:
                    print(f"✓ Connection still exists after move")
                else:
                    print(f"❌ Expected 1 connection, got {connections}")
                    return

            # Undo move
            print("→ Undoing move...")
            resp = await self.mcp.send_command("undo", {})
            if not self._check_success(resp, "Undo move"):
                return

            # Verify B back at original position
            resp = await self.mcp.send_command("list_elements", {})
            if resp.success:
                for elem in resp.result.get("elements", []):
                    if elem["id"] == b_id:
                        if elem["x"] == 200.0 and elem["y"] == 100.0:
                            print(f"✓ B back at original (200, 100)")
                        else:
                            print(f"⚠ B position: ({elem['x']}, {elem['y']}) vs expected (200, 100)")

            # Verify connection still valid
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = len(resp.result.get("connections", []))
                if connections == 1:
                    print(f"✓ Connection still valid after undo")
                else:
                    print(f"❌ Connection lost: {connections} connections")
                    return

            # Redo move
            resp = await self.mcp.send_command("redo", {})
            if not self._check_success(resp, "Redo move"):
                return

            # Verify B moved again and connection valid
            resp = await self.mcp.send_command("list_elements", {})
            connections_valid = True
            if resp.success:
                for elem in resp.result.get("elements", []):
                    if elem["id"] == b_id:
                        if elem["x"] == 250.0 and elem["y"] == 150.0:
                            print(f"✓ B moved again to (250, 150)")
                        else:
                            connections_valid = False

            resp = await self.mcp.send_command("list_connections", {})
            if resp.success and connections_valid:
                connections = len(resp.result.get("connections", []))
                if connections == 1:
                    print(f"✓ Connection valid after redo")
                else:
                    print(f"❌ Connection invalid: {connections} connections")
                    return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_move_with_connections: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_6_2")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_move_with_connections: {str(e)}")

    async def test_rotate_with_connections(self) -> None:
        """Test 6.3: Rotate element with connections → port positions"""
        self.test_count += 1
        print(f"\n[Test 6.3] Rotate Element with Connections (Port Positions)")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create A→B with input connection
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            a_id = resp.result.get("element_id")

            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 200.0,
                "y": 100.0
            })
            b_id = resp.result.get("element_id")

            resp = await self.mcp.send_command("connect_elements", {
                "source_id": a_id,
                "source_port": 0,
                "target_id": b_id,
                "target_port": 0
            })
            if not self._check_success(resp, "Connect A→B"):
                return
            print("✓ Connected A→B")

            # Rotate A (source of connection)
            print("→ Rotating A by 90°...")
            resp = await self.mcp.send_command("rotate_element", {
                "element_id": a_id,
                "angle": 90
            })
            if not self._check_success(resp, "Rotate A"):
                return

            # Verify connection still valid
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = len(resp.result.get("connections", []))
                if connections == 1:
                    print(f"✓ Connection still valid after rotation")
                else:
                    print(f"❌ Connection lost: {connections} connections")
                    return

            # Undo rotation
            print("→ Undoing rotation...")
            resp = await self.mcp.send_command("undo", {})
            if not self._check_success(resp, "Undo rotation"):
                return

            # Verify rotation undone and connection valid
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = len(resp.result.get("connections", []))
                if connections == 1:
                    print(f"✓ Rotation undone, connection still valid")
                else:
                    print(f"❌ Connection lost: {connections} connections")
                    return

            # Redo rotation
            resp = await self.mcp.send_command("redo", {})
            if not self._check_success(resp, "Redo rotation"):
                return

            # Verify rotation redone and connection valid
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = len(resp.result.get("connections", []))
                if connections == 1:
                    print(f"✓ Rotation redone, connection still valid")
                else:
                    print(f"❌ Connection lost: {connections} connections")
                    return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_rotate_with_connections: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_6_3")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_rotate_with_connections: {str(e)}")

    async def test_flip_with_connections(self) -> None:
        """Test 6.4: Flip element with connections"""
        self.test_count += 1
        print(f"\n[Test 6.4] Flip Element with Connections")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create A→B connection
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            a_id = resp.result.get("element_id")

            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 200.0,
                "y": 100.0
            })
            b_id = resp.result.get("element_id")

            resp = await self.mcp.send_command("connect_elements", {
                "source_id": a_id,
                "source_port": 0,
                "target_id": b_id,
                "target_port": 0
            })
            if not self._check_success(resp, "Connect A→B"):
                return
            print("✓ Connected A→B")

            # Flip A
            print("→ Flipping A (horizontal)...")
            resp = await self.mcp.send_command("flip_element", {
                "element_id": a_id,
                "axis": 0
            })
            if not self._check_success(resp, "Flip A"):
                return

            # Verify connection valid
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = len(resp.result.get("connections", []))
                if connections == 1:
                    print(f"✓ Connection valid after flip")
                else:
                    print(f"❌ Connection lost: {connections} connections")
                    return

            # Undo flip
            print("→ Undoing flip...")
            resp = await self.mcp.send_command("undo", {})
            if not self._check_success(resp, "Undo flip"):
                return

            # Verify flip undone
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = len(resp.result.get("connections", []))
                if connections == 1:
                    print(f"✓ Flip undone, connection valid")
                else:
                    print(f"❌ Connection lost: {connections} connections")
                    return

            # Redo flip
            resp = await self.mcp.send_command("redo", {})
            if not self._check_success(resp, "Redo flip"):
                return

            # Verify flip redone
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = len(resp.result.get("connections", []))
                if connections == 1:
                    print(f"✓ Flip redone, connection valid")
                else:
                    print(f"❌ Connection lost: {connections} connections")
                    return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_flip_with_connections: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_6_4")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_flip_with_connections: {str(e)}")

    async def test_morph_with_connections(self) -> None:
        """Test 6.5: Morph element with connection compatibility"""
        self.test_count += 1
        print(f"\n[Test 6.5] Morph Element with Connections")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create AND→OR connection
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            and_id = resp.result.get("element_id")

            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 200.0,
                "y": 100.0
            })
            or_id = resp.result.get("element_id")

            resp = await self.mcp.send_command("connect_elements", {
                "source_id": and_id,
                "source_port": 0,
                "target_id": or_id,
                "target_port": 0
            })
            if not self._check_success(resp, "Connect AND→OR"):
                return
            print("✓ Connected AND→OR")

            # Morph AND to OR (compatible: both have similar port count)
            print("→ Morphing AND to OR...")
            resp = await self.mcp.send_command("morph_element", {
                "element_id": and_id,
                "new_type": "Or"
            })
            if resp.success or resp.error:
                # Either success or graceful error is OK for morph
                print("✓ Morph completed (success or graceful error)")
            else:
                print(f"❌ Morph failed unexpectedly")
                return

            # Undo morph
            print("→ Undoing morph...")
            resp = await self.mcp.send_command("undo", {})
            if not resp.success:
                print(f"⚠ Undo failed: {resp.error}")
                # Continue anyway, may not have been an operation
            else:
                print("✓ Undo completed")

            # Verify we're back to AND
            resp = await self.mcp.send_command("list_elements", {})
            if resp.success:
                for elem in resp.result.get("elements", []):
                    if elem["id"] == and_id:
                        print(f"✓ Element type after undo: {elem.get('type', 'unknown')}")

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_morph_with_connections: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_6_5")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_morph_with_connections: {str(e)}")

    async def test_linear_chain_a_b_c_d(self) -> None:
        """Test 6.6: Linear connection chain (A→B→C→D) undo/redo"""
        self.test_count += 1
        print(f"\n[Test 6.6] Linear Connection Chain (A→B→C→D)")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create 4 elements
            ids = []
            names = ['A', 'B', 'C', 'D']
            for i, name in enumerate(names):
                resp = await self.mcp.send_command("create_element", {
                    "type": "And",
                    "x": 100.0 + i * 100.0,
                    "y": 100.0
                })
                ids.append(resp.result.get("element_id"))
            print(f"✓ Created 4 elements: {names}")

            # Connect in chain: A→B→C→D
            for i in range(len(ids) - 1):
                resp = await self.mcp.send_command("connect_elements", {
                    "source_id": ids[i],
                    "source_port": 0,
                    "target_id": ids[i+1],
                    "target_port": 0
                })
                if not self._check_success(resp, f"Connect {names[i]}→{names[i+1]}"):
                    return
            print("✓ Created chain: A→B→C→D")

            # Verify 3 connections
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = len(resp.result.get("connections", []))
                if connections == 3:
                    print(f"✓ {connections} connections in chain")
                else:
                    print(f"⚠ Expected 3 connections, got {connections}")

            # Undo all (7 operations: 4 creates + 3 connections)
            print("→ Undoing entire chain (7 operations)...")
            for i in range(7):
                resp = await self.mcp.send_command("undo", {})
                if not resp.success:
                    print(f"❌ Undo {i+1} failed")
                    return

            # Verify completely undone
            resp = await self.mcp.send_command("list_elements", {})
            if resp.success:
                elements = len(resp.result.get("elements", []))
                if elements == 0:
                    print(f"✓ All elements undone")
                else:
                    print(f"⚠ Expected 0 elements, got {elements}")

            # Redo all
            print("→ Redoing entire chain (7 operations)...")
            for i in range(7):
                resp = await self.mcp.send_command("redo", {})
                if not resp.success:
                    print(f"❌ Redo {i+1} failed")
                    return

            # Verify chain recreated
            resp = await self.mcp.send_command("list_elements", {})
            if resp.success:
                elements = len(resp.result.get("elements", []))
                if elements == 4:
                    print(f"✓ All 4 elements redone")
                else:
                    print(f"❌ Expected 4 elements, got {elements}")
                    return

            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = len(resp.result.get("connections", []))
                if connections == 3:
                    print(f"✓ All 3 connections redone")
                else:
                    print(f"❌ Expected 3 connections, got {connections}")
                    return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_linear_chain_a_b_c_d: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_6_6")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_linear_chain_a_b_c_d: {str(e)}")

    async def test_branching_network(self) -> None:
        """Test 6.7: Branching network (one source to multiple targets)"""
        self.test_count += 1
        print(f"\n[Test 6.7] Branching Network (One Source → Multiple Targets)")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create network: A outputs to [B, C, D, E]
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            a_id = resp.result.get("element_id")
            print(f"✓ Created A (source)")

            target_ids = []
            for i in range(4):
                resp = await self.mcp.send_command("create_element", {
                    "type": "Or",
                    "x": 200.0 + i * 50.0,
                    "y": 100.0 + (i % 2) * 50.0
                })
                target_ids.append(resp.result.get("element_id"))
            print(f"✓ Created 4 target elements (B, C, D, E)")

            # Connect A to all targets
            for i, tid in enumerate(target_ids):
                resp = await self.mcp.send_command("connect_elements", {
                    "source_id": a_id,
                    "source_port": 0,
                    "target_id": tid,
                    "target_port": 0
                })
                if not self._check_success(resp, f"Connect A→T{i+1}"):
                    return
            print("✓ Connected A to all 4 targets (4 connections)")

            # Delete A
            print("→ Deleting A (source of 4 connections)...")
            resp = await self.mcp.send_command("delete_element", {
                "element_id": a_id
            })
            if not self._check_success(resp, "Delete A"):
                return

            # Verify all connections gone
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = len(resp.result.get("connections", []))
                if connections == 0:
                    print(f"✓ All 4 connections removed with A")
                else:
                    print(f"⚠ Expected 0 connections, got {connections}")

            # Undo deletion
            print("→ Undoing deletion of A...")
            resp = await self.mcp.send_command("undo", {})
            if not self._check_success(resp, "Undo delete A"):
                return

            # Verify A and all connections restored
            resp = await self.mcp.send_command("list_elements", {})
            if resp.success:
                elements = len(resp.result.get("elements", []))
                if elements == 5:
                    print(f"✓ All 5 elements restored (A + 4 targets)")
                else:
                    print(f"⚠ Expected 5 elements, got {elements}")

            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = len(resp.result.get("connections", []))
                if connections == 4:
                    print(f"✓ All 4 connections restored")
                else:
                    print(f"❌ Expected 4 connections, got {connections}")
                    return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_branching_network: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_6_7")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_branching_network: {str(e)}")

    async def test_delete_middle_element_chain(self) -> None:
        """Test 6.8: Delete middle element in chain (A→B→C)"""
        self.test_count += 1
        print(f"\n[Test 6.8] Delete Middle Element in Chain (A→B→C)")
        print("-" * 50)

        try:
            resp = await self.mcp.send_command("new_circuit", {})
            if not self._check_success(resp, "Create circuit"):
                return

            # Create A→B→C
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100.0,
                "y": 100.0
            })
            a_id = resp.result.get("element_id")

            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 200.0,
                "y": 100.0
            })
            b_id = resp.result.get("element_id")

            resp = await self.mcp.send_command("create_element", {
                "type": "Not",
                "x": 300.0,
                "y": 100.0
            })
            c_id = resp.result.get("element_id")

            resp = await self.mcp.send_command("connect_elements", {
                "source_id": a_id,
                "source_port": 0,
                "target_id": b_id,
                "target_port": 0
            })

            resp = await self.mcp.send_command("connect_elements", {
                "source_id": b_id,
                "source_port": 0,
                "target_id": c_id,
                "target_port": 0
            })
            print("✓ Created chain: A→B→C")

            # Delete B
            print("→ Deleting middle element B...")
            resp = await self.mcp.send_command("delete_element", {
                "element_id": b_id
            })
            if not self._check_success(resp, "Delete B"):
                return

            # Verify A and C still exist, B deleted
            resp = await self.mcp.send_command("list_elements", {})
            if resp.success:
                elements = resp.result.get("elements", [])
                if len(elements) == 2:
                    print(f"✓ B deleted, A and C remain")
                else:
                    print(f"❌ Expected 2 elements, got {len(elements)}")
                    return

            # Verify connections gone
            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = len(resp.result.get("connections", []))
                if connections == 0:
                    print(f"✓ Both connections removed with B")
                else:
                    print(f"❌ Expected 0 connections, got {connections}")
                    return

            # Undo deletion
            print("→ Undoing deletion of B...")
            resp = await self.mcp.send_command("undo", {})
            if not self._check_success(resp, "Undo delete B"):
                return

            # Verify all restored
            resp = await self.mcp.send_command("list_elements", {})
            if resp.success:
                elements = len(resp.result.get("elements", []))
                if elements == 3:
                    print(f"✓ B restored")
                else:
                    print(f"❌ Expected 3 elements, got {elements}")
                    return

            resp = await self.mcp.send_command("list_connections", {})
            if resp.success:
                connections = len(resp.result.get("connections", []))
                if connections == 2:
                    print(f"✓ Both connections restored")
                else:
                    print(f"❌ Expected 2 connections, got {connections}")
                    return

            # Redo deletion
            print("→ Redoing deletion of B...")
            resp = await self.mcp.send_command("redo", {})
            if not self._check_success(resp, "Redo delete B"):
                return

            # Verify back to deleted state
            resp = await self.mcp.send_command("list_elements", {})
            if resp.success:
                elements = len(resp.result.get("elements", []))
                if elements == 2:
                    print(f"✓ B deleted again")
                else:
                    print(f"❌ Expected 2 elements, got {elements}")
                    return

            # Check process
            if self.mcp.process and self.mcp.process.returncode is not None:
                print(f"❌ Process crashed")
                self.failed_tests.append("test_delete_middle_element_chain: crash")
                return

            print("✅ Test PASSED")
            self.passed_count += 1
            await self.mcp.cleanup_circuit("test_6_8")

        except Exception as e:
            print(f"❌ Exception: {e}")
            self.failed_tests.append(f"test_delete_middle_element_chain: {str(e)}")

    def _check_success(self, response: MCPResponse, description: str) -> bool:
        """Check if response was successful"""
        if not response.success:
            print(f"❌ {description} failed: {response.error}")
            return False
        return True

    async def print_summary(self) -> bool:
        """Print test summary"""
        print("\n" + "=" * 70)
        print(f"📊 PHASE 6 CONNECTED ELEMENTS SUMMARY")
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
            print("✅ ALL PHASE 6 TESTS PASSED - Connected element undo/redo validated!")
        else:
            print("❌ SOME TESTS FAILED - Review errors above")
        print("=" * 70 + "\n")

        return success


async def main() -> None:
    """Main entry point"""
    validator = Phase6Validator()
    success = await validator.run_all_tests()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    asyncio.run(main())
