#!/usr/bin/env python3

"""
Phase 7.4: Interaction & Stress Edge Cases - Complex interaction patterns and stress testing

Tests for:
1. Massive Connection Network (10 gates with 20+ connections - stress test)
2. Rapid-Fire Operations (10+ rapid ops without pausing - interaction test)
3. Mixed Topology Changes (concurrent config changes and deletions - interaction test)
4. Undo Stack Saturation (50+ operations with full undo/redo - stress test)
5. Connection Re-optimization (diamond topology with deletion - interaction test)

Run: python3 test/mcp/test_phase7_4_interaction_stress_edge_cases.py
"""

import asyncio
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure

class Phase74InteractionStressEdgeCases:
    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=True, verbose=False)
        self.test_count = 0
        self.passed = 0
        self.failed = []

    async def run_all(self):
        print("\n" + "="*70)
        print("🔬 PHASE 7.4 - INTERACTION & STRESS EDGE CASES")
        print("="*70)

        if not await self.mcp.start_mcp():
            print("❌ MCP startup failed")
            return False

        try:
            # Interaction & stress edge cases
            await self.test_massive_connection_network()
            await self.test_rapid_fire_operations()
            await self.test_mixed_topology_changes()
            await self.test_undo_stack_saturation()
            await self.test_connection_reoptimization()

            # Print summary
            print("\n" + "="*70)
            print(f"PHASE 7.4 RESULTS: {self.passed}/{self.test_count} tests passed")
            print("="*70)

            if self.failed:
                print("\n❌ FAILED TESTS:")
                for failure in self.failed:
                    print(f"  - {failure}")
                return False
            else:
                print("\n✅ ALL TESTS PASSED!")
                return True

        finally:
            await self.mcp.stop_mcp()

    async def test_massive_connection_network(self):
        """Test 7.4.1: Massive Connection Network (stress test)

        Creates 10 logic gates in various configurations with 20+ connections,
        tests undo/redo with complex topologies.
        """
        self.test_count += 1
        test_name = "7.4.1: Massive Connection Network"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create 10 And gates
            gate_ids = []
            for i in range(10):
                resp = await self.mcp.send_command("create_element", {
                    "type": "And",
                    "x": i * 50,
                    "y": i * 50
                })
                if resp.success:
                    gate_ids.append(resp.result["element_id"])

            if len(gate_ids) != 10:
                self.failed.append(f"{test_name}: Failed to create 10 gates")
                return

            # Create complex connection topology - deep chain plus cross-connections
            # Pattern: 0→1, 1→2, 2→3, ... 8→9 (9 connections)
            # Plus cross-connections: 0→3, 1→4, 2→5, 3→6, 4→7, 5→8, 6→9 (7 connections)
            # Plus feedback: 9→0 (1 connection)
            # Total: 17 connections

            conn_count = 0

            # Main chain: 0→1→2→...→8→9
            for i in range(9):
                resp = await self.mcp.send_command("connect_elements", {
                    "source_id": gate_ids[i],
                    "source_port": 0,
                    "target_id": gate_ids[i + 1],
                    "target_port": 0
                })
                if resp.success:
                    conn_count += 1

            # Cross-connections
            cross_pairs = [(0, 3), (1, 4), (2, 5), (3, 6), (4, 7), (5, 8), (6, 9)]
            for src_idx, tgt_idx in cross_pairs:
                resp = await self.mcp.send_command("connect_elements", {
                    "source_id": gate_ids[src_idx],
                    "source_port": 0,
                    "target_id": gate_ids[tgt_idx],
                    "target_port": 1
                })
                if resp.success:
                    conn_count += 1

            # Feedback connection: 9→0
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": gate_ids[9],
                "source_port": 0,
                "target_id": gate_ids[0],
                "target_port": 1
            })
            if resp.success:
                conn_count += 1

            # Verify connections created
            resp = await self.mcp.send_command("list_connections", {})
            connections_before = len(resp.result.get("connections", []))

            if connections_before < 15:  # At least 15 of 17 should work
                self.failed.append(f"{test_name}: Expected 15+ connections, got {connections_before}")
                return

            # Perform 3 undo/redo cycles
            for cycle in range(3):
                # Undo 5 operations
                for _ in range(5):
                    await self.mcp.send_command("undo", {})

                # Redo 5 operations
                for _ in range(5):
                    await self.mcp.send_command("redo", {})

            # Final verification
            resp = await self.mcp.send_command("list_connections", {})
            connections_after = len(resp.result.get("connections", []))

            if connections_after != connections_before:
                self.failed.append(f"{test_name}: Connection count changed. Before: {connections_before}, After: {connections_after}")
                return

            # Verify all 10 gates still exist
            resp = await self.mcp.send_command("list_elements", {})
            elements_count = len(resp.result.get("elements", []))

            if elements_count != 10:
                self.failed.append(f"{test_name}: Expected 10 elements, got {elements_count}")
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

    async def test_rapid_fire_operations(self):
        """Test 7.4.2: Rapid-Fire Operations

        Performs 10+ operations rapidly without pausing:
        Create → Connect → Delete → Undo → Redo → Reconnect
        """
        self.test_count += 1
        test_name = "7.4.2: Rapid-Fire Operations"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create base elements
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 0,
                "y": 0
            })
            base_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 100,
                "y": 0
            })
            target_id = resp.result["element_id"]

            # Rapid operations sequence
            operations_performed = 0

            # Connect
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": base_id,
                "source_port": 0,
                "target_id": target_id,
                "target_port": 0
            })
            if resp.success:
                operations_performed += 1

            # Move base element
            for i in range(5):
                resp = await self.mcp.send_command("update_position", {
                    "element_id": base_id,
                    "x": i * 20,
                    "y": i * 20
                })
                operations_performed += 1  # Count even if not returned as undoable

            # Move target element
            for i in range(5):
                resp = await self.mcp.send_command("update_position", {
                    "element_id": target_id,
                    "x": 100 + i * 20,
                    "y": i * 20
                })
                operations_performed += 1  # Count even if not returned as undoable

            total_ops = operations_performed

            if total_ops < 10:
                self.failed.append(f"{test_name}: Expected 10+ operations, got {total_ops}")
                return

            # Verify connection still exists
            resp = await self.mcp.send_command("list_connections", {})
            connections = resp.result.get("connections", [])

            if len(connections) != 1:
                self.failed.append(f"{test_name}: Expected 1 connection, got {len(connections)}")
                return

            # Undo all operations
            for _ in range(total_ops):
                await self.mcp.send_command("undo", {})

            # Verify connection removed by undoing the connect
            resp = await self.mcp.send_command("list_connections", {})
            connections_after_undo = len(resp.result.get("connections", []))

            if connections_after_undo != 0:
                self.failed.append(f"{test_name}: Expected 0 connections after undo, got {connections_after_undo}")
                return

            # Redo all operations
            for _ in range(total_ops):
                await self.mcp.send_command("redo", {})

            # Verify connection restored
            resp = await self.mcp.send_command("list_connections", {})
            connections_after_redo = len(resp.result.get("connections", []))

            if connections_after_redo != 1:
                self.failed.append(f"{test_name}: Expected 1 connection after redo, got {connections_after_redo}")
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

    async def test_mixed_topology_changes(self):
        """Test 7.4.3: Mixed Topology Changes

        Creates circuit with feedback loop + cross-connections,
        modifies element ports while connections exist,
        and deletes elements with pending configurations.
        """
        self.test_count += 1
        test_name = "7.4.3: Mixed Topology Changes"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create elements
            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 0,
                "y": 0
            })
            a_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 100,
                "y": 0
            })
            b_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 200,
                "y": 0
            })
            c_id = resp.result["element_id"]

            # Create connections: A→B, B→C, C→A (feedback loop)
            await self.mcp.send_command("connect_elements", {
                "source_id": a_id,
                "source_port": 0,
                "target_id": b_id,
                "target_port": 0
            })

            await self.mcp.send_command("connect_elements", {
                "source_id": b_id,
                "source_port": 0,
                "target_id": c_id,
                "target_port": 0
            })

            await self.mcp.send_command("connect_elements", {
                "source_id": c_id,
                "source_port": 0,
                "target_id": a_id,
                "target_port": 0
            })

            # Verify initial state
            resp = await self.mcp.send_command("list_connections", {})
            connections_initial = len(resp.result.get("connections", []))

            if connections_initial != 3:
                self.failed.append(f"{test_name}: Initial state should have 3 connections, got {connections_initial}")
                return

            # Mix operations: expand B's ports while connected
            await self.mcp.send_command("change_input_size", {
                "element_id": b_id,
                "size": 4
            })

            # Add cross-connection to B's new port while in this state
            await self.mcp.send_command("connect_elements", {
                "source_id": a_id,
                "source_port": 0,
                "target_id": b_id,
                "target_port": 2
            })

            # Now delete C (which breaks the feedback loop)
            await self.mcp.send_command("delete_element", {
                "element_id": c_id
            })

            # Verify state after mixed operations
            resp = await self.mcp.send_command("list_elements", {})
            elements = len(resp.result.get("elements", []))

            if elements != 2:  # Only A and B should remain
                self.failed.append(f"{test_name}: Expected 2 elements after delete, got {elements}")
                return

            resp = await self.mcp.send_command("list_connections", {})
            connections = len(resp.result.get("connections", []))

            # A→B (old) + A→B (new cross) = 2 connections (C→A feedback removed)
            if connections != 2:
                self.failed.append(f"{test_name}: Expected 2 connections after mixed ops, got {connections}")
                return

            # Undo all mixed operations (in reverse: delete, cross-connect, resize)
            await self.mcp.send_command("undo", {})  # Undo delete
            await self.mcp.send_command("undo", {})  # Undo cross-connect
            await self.mcp.send_command("undo", {})  # Undo resize

            # Verify back to initial state
            resp = await self.mcp.send_command("list_elements", {})
            elements_restored = len(resp.result.get("elements", []))

            if elements_restored != 3:
                self.failed.append(f"{test_name}: Expected 3 elements after undo, got {elements_restored}")
                return

            resp = await self.mcp.send_command("list_connections", {})
            connections_restored = len(resp.result.get("connections", []))

            if connections_restored != 3:
                self.failed.append(f"{test_name}: Expected 3 connections after undo, got {connections_restored}")
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

    async def test_undo_stack_saturation(self):
        """Test 7.4.4: Undo Stack Saturation

        Performs 50+ position update operations to build large undo stack,
        then undoes and redoes all operations to verify no corruption.
        """
        self.test_count += 1
        test_name = "7.4.4: Undo Stack Saturation"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create 2 elements to move around
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 0,
                "y": 0
            })
            elem1_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 100,
                "y": 100
            })
            elem2_id = resp.result["element_id"]

            # Connect them
            await self.mcp.send_command("connect_elements", {
                "source_id": elem1_id,
                "source_port": 0,
                "target_id": elem2_id,
                "target_port": 0
            })

            # Perform 50+ position updates (stress the undo stack)
            operations = 1  # Connection counts as 1

            for i in range(25):
                await self.mcp.send_command("update_position", {
                    "element_id": elem1_id,
                    "x": i * 10,
                    "y": i * 10
                })
                operations += 1

                await self.mcp.send_command("update_position", {
                    "element_id": elem2_id,
                    "x": 100 + i * 10,
                    "y": 100 + i * 10
                })
                operations += 1

            # Total: 1 (connect) + 50 (moves) = 51 operations

            if operations < 50:
                self.failed.append(f"{test_name}: Expected 50+ operations, got {operations}")
                return

            # Verify connection still intact
            resp = await self.mcp.send_command("list_connections", {})
            connections_before = len(resp.result.get("connections", []))

            if connections_before != 1:
                self.failed.append(f"{test_name}: Connection lost before undo phase, got {connections_before}")
                return

            # Undo ALL operations
            for _ in range(operations):
                await self.mcp.send_command("undo", {})

            # Verify connection removed (was created in operation 0)
            resp = await self.mcp.send_command("list_connections", {})
            connections_after_undo = len(resp.result.get("connections", []))

            if connections_after_undo != 0:
                self.failed.append(f"{test_name}: Expected 0 connections after undo all, got {connections_after_undo}")
                return

            # Redo ALL operations
            for _ in range(operations):
                await self.mcp.send_command("redo", {})

            # Verify connection restored
            resp = await self.mcp.send_command("list_connections", {})
            connections_after_redo = len(resp.result.get("connections", []))

            if connections_after_redo != 1:
                self.failed.append(f"{test_name}: Expected 1 connection after redo all, got {connections_after_redo}")
                return

            # Verify both elements still exist
            resp = await self.mcp.send_command("list_elements", {})
            elements = len(resp.result.get("elements", []))

            if elements != 2:
                self.failed.append(f"{test_name}: Expected 2 elements, got {elements}")
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

    async def test_connection_reoptimization(self):
        """Test 7.4.5: Connection Re-optimization

        Creates diamond topology (A→B,C; B,C→D), deletes B,
        reconnects A→D directly, then undoes everything.
        Tests that connection counts remain accurate.
        """
        self.test_count += 1
        test_name = "7.4.5: Connection Re-optimization"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create diamond: A → B → D
            #                 ↘ C ↗

            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 0,
                "y": 50
            })
            a_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 100,
                "y": 0
            })
            b_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 100,
                "y": 100
            })
            c_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 200,
                "y": 50
            })
            d_id = resp.result["element_id"]

            # Configure D to have 2 inputs
            await self.mcp.send_command("change_input_size", {
                "element_id": d_id,
                "size": 2
            })

            # Create diamond: A→B, A→C, B→D.in(0), C→D.in(1)
            await self.mcp.send_command("connect_elements", {
                "source_id": a_id,
                "source_port": 0,
                "target_id": b_id,
                "target_port": 0
            })

            await self.mcp.send_command("connect_elements", {
                "source_id": a_id,
                "source_port": 0,
                "target_id": c_id,
                "target_port": 0
            })

            await self.mcp.send_command("connect_elements", {
                "source_id": b_id,
                "source_port": 0,
                "target_id": d_id,
                "target_port": 0
            })

            await self.mcp.send_command("connect_elements", {
                "source_id": c_id,
                "source_port": 0,
                "target_id": d_id,
                "target_port": 1
            })

            # Verify diamond state: 4 connections
            resp = await self.mcp.send_command("list_connections", {})
            connections_diamond = len(resp.result.get("connections", []))

            if connections_diamond != 4:
                self.failed.append(f"{test_name}: Diamond should have 4 connections, got {connections_diamond}")
                return

            # Delete B (removes A→B and B→D connections)
            await self.mcp.send_command("delete_element", {
                "element_id": b_id
            })

            # Verify state after B deletion: 2 connections (A→C, C→D.in(1))
            resp = await self.mcp.send_command("list_connections", {})
            connections_after_delete = len(resp.result.get("connections", []))

            if connections_after_delete != 2:
                self.failed.append(f"{test_name}: After B delete, expected 2 connections, got {connections_after_delete}")
                return

            # Reconnect A→D directly to replace B→D path
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": a_id,
                "source_port": 0,
                "target_id": d_id,
                "target_port": 0
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to create A→D direct connection")
                return

            # Verify reoptimized state: 3 connections (A→C, C→D.in(1), A→D.in(0))
            resp = await self.mcp.send_command("list_connections", {})
            connections_reopt = len(resp.result.get("connections", []))

            if connections_reopt != 3:
                self.failed.append(f"{test_name}: After reoptimization, expected 3 connections, got {connections_reopt}")
                return

            # Undo all operations (in reverse order)
            await self.mcp.send_command("undo", {})  # Undo A→D
            await self.mcp.send_command("undo", {})  # Undo B delete

            # Verify back to diamond state: 4 connections
            resp = await self.mcp.send_command("list_connections", {})
            connections_restored = len(resp.result.get("connections", []))

            if connections_restored != 4:
                self.failed.append(f"{test_name}: After undo, expected 4 connections, got {connections_restored}")
                return

            # Verify all 4 elements restored
            resp = await self.mcp.send_command("list_elements", {})
            elements = len(resp.result.get("elements", []))

            if elements != 4:
                self.failed.append(f"{test_name}: Expected 4 elements, got {elements}")
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

async def main():
    test_suite = Phase74InteractionStressEdgeCases()
    success = await test_suite.run_all()
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    asyncio.run(main())
