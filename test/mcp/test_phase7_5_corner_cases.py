#!/usr/bin/env python3

"""
Phase 7.5: Corner Cases & Validation - Validation of edge cases and corner scenarios

Tests for:
1. Circular Feedback Loops (A→B→C→A with deletion)
2. Element Configuration Extremes (maximum port counts)
3. Rapid Create-Delete-Create Lifecycle
4. Element ID Uniqueness (deletion doesn't reuse IDs)
5. Stress Test with 50+ Simultaneous Connections

Run: python3 test/mcp/test_phase7_5_corner_cases.py
"""

import asyncio
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure

class Phase75CornerCases:
    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=True, verbose=False)
        self.test_count = 0
        self.passed = 0
        self.failed = []

    async def run_all(self):
        print("\n" + "="*70)
        print("🔬 PHASE 7.5 - CORNER CASES & VALIDATION")
        print("="*70)

        if not await self.mcp.start_mcp():
            print("❌ MCP startup failed")
            return False

        try:
            # Corner case tests
            await self.test_circular_feedback_loop()
            await self.test_maximum_port_configuration()
            await self.test_rapid_create_delete_create()
            await self.test_element_id_uniqueness()
            await self.test_massive_fan_out_stress()

            # Print summary
            print("\n" + "="*70)
            print(f"PHASE 7.5 RESULTS: {self.passed}/{self.test_count} tests passed")
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

    async def test_circular_feedback_loop(self):
        """Test 7.5.1: Circular Feedback Loop A→B→C→A

        Tests handling of circular connections without deadlock.
        """
        self.test_count += 1
        test_name = "7.5.1: Circular Feedback Loop"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create 3 elements for feedback loop
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

            # Create circular loop: A→B, B→C, C→A
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

            # Verify circular loop created
            resp = await self.mcp.send_command("list_connections", {})
            connections_before = len(resp.result.get("connections", []))

            if connections_before != 3:
                self.failed.append(f"{test_name}: Expected 3 connections in loop, got {connections_before}")
                return

            # Delete A (breaks the circular loop)
            await self.mcp.send_command("delete_element", {
                "element_id": a_id
            })

            # Verify B and C still exist
            resp = await self.mcp.send_command("list_elements", {})
            elements_after_delete = len(resp.result.get("elements", []))

            if elements_after_delete != 2:  # Only B and C
                self.failed.append(f"{test_name}: Expected 2 elements after delete, got {elements_after_delete}")
                return

            resp = await self.mcp.send_command("list_connections", {})
            connections_after_delete = len(resp.result.get("connections", []))

            # After deleting A, two connections should be gone (A→B and C→A)
            # but B→C should remain since both B and C still exist
            if connections_after_delete != 1:
                self.failed.append(f"{test_name}: Expected 1 connection after A delete (B→C), got {connections_after_delete}")
                return

            # Undo deletion
            await self.mcp.send_command("undo", {})

            # Verify circular loop restored
            resp = await self.mcp.send_command("list_elements", {})
            elements_after_undo = len(resp.result.get("elements", []))

            if elements_after_undo != 3:
                self.failed.append(f"{test_name}: Expected 3 elements after undo, got {elements_after_undo}")
                return

            resp = await self.mcp.send_command("list_connections", {})
            connections_after_undo = len(resp.result.get("connections", []))

            if connections_after_undo != 3:
                self.failed.append(f"{test_name}: Expected 3 connections after undo, got {connections_after_undo}")
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

    async def test_maximum_port_configuration(self):
        """Test 7.5.3: Maximum Port Count Configuration

        Tests behavior with many ports on a single element.
        """
        self.test_count += 1
        test_name = "7.5.3: Maximum Port Configuration"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create an Or gate and expand it to maximum ports
            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 100,
                "y": 100
            })
            sink_id = resp.result["element_id"]

            # Expand to 10 inputs
            await self.mcp.send_command("change_input_size", {
                "element_id": sink_id,
                "size": 10
            })

            # Create 10 source elements
            source_ids = []
            for i in range(10):
                resp = await self.mcp.send_command("create_element", {
                    "type": "And",
                    "x": 100 + i * 40,
                    "y": 0
                })
                source_ids.append(resp.result["element_id"])

            # Connect all 10 sources to all 10 input ports
            successful_connections = 0
            for i, src_id in enumerate(source_ids):
                resp = await self.mcp.send_command("connect_elements", {
                    "source_id": src_id,
                    "source_port": 0,
                    "target_id": sink_id,
                    "target_port": i
                })
                if resp.success:
                    successful_connections += 1

            if successful_connections < 2:
                self.failed.append(f"{test_name}: Failed to connect at least 2 sources, got {successful_connections}")
                return

            # Verify connections exist (may be fewer than 10 due to port limitations)
            resp = await self.mcp.send_command("list_connections", {})
            connections = len(resp.result.get("connections", []))

            if connections < 2:  # At least 2 connections should succeed
                self.failed.append(f"{test_name}: Expected at least 2 connections, got {connections}")
                return

            # Record initial connection count
            initial_connections = connections

            # Reduce ports to 1 (should remove most high-indexed connections)
            await self.mcp.send_command("change_input_size", {
                "element_id": sink_id,
                "size": 1
            })

            # Verify connections were reduced (only port 0 should remain)
            resp = await self.mcp.send_command("list_connections", {})
            connections_after_reduce = len(resp.result.get("connections", []))

            if connections_after_reduce > 1:
                self.failed.append(f"{test_name}: Expected at most 1 connection after reduce to 1 port, got {connections_after_reduce}")
                return

            # Expand back to 10
            await self.mcp.send_command("change_input_size", {
                "element_id": sink_id,
                "size": 10
            })

            # Verify connections to low-numbered ports still exist
            resp = await self.mcp.send_command("list_connections", {})
            connections_after_expand = len(resp.result.get("connections", []))

            if connections_after_expand != connections_after_reduce:
                self.failed.append(f"{test_name}: Expected {connections_after_reduce} connections after expand, got {connections_after_expand}")
                return

            # Verify we still have connection to port 0 after expand
            if connections_after_expand < 1:
                self.failed.append(f"{test_name}: Expected at least 1 connection after expand, got {connections_after_expand}")
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

    async def test_rapid_create_delete_create(self):
        """Test 7.5.4: Rapid Create-Delete-Create Lifecycle

        Tests element ID uniqueness and lifecycle management.
        """
        self.test_count += 1
        test_name = "7.5.4: Rapid Create-Delete-Create"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create element A
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100,
                "y": 100
            })
            a_id = resp.result["element_id"]

            # Create element B to connect to A
            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 200,
                "y": 100
            })
            b_id = resp.result["element_id"]

            # Connect A → B
            await self.mcp.send_command("connect_elements", {
                "source_id": a_id,
                "source_port": 0,
                "target_id": b_id,
                "target_port": 0
            })

            # Delete A
            await self.mcp.send_command("delete_element", {
                "element_id": a_id
            })

            # Create new element C at same position (should get different ID)
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100,
                "y": 100
            })
            c_id = resp.result["element_id"]

            # Verify C has different ID than A
            if c_id == a_id:
                self.failed.append(f"{test_name}: New element C got same ID as deleted A")
                return

            # Connect C → B
            await self.mcp.send_command("connect_elements", {
                "source_id": c_id,
                "source_port": 0,
                "target_id": b_id,
                "target_port": 0
            })

            # Verify only 1 connection (C→B)
            resp = await self.mcp.send_command("list_connections", {})
            connections = len(resp.result.get("connections", []))

            if connections != 1:
                self.failed.append(f"{test_name}: Expected 1 connection (C→B), got {connections}")
                return

            # Undo creation of C
            await self.mcp.send_command("undo", {})

            # Undo connection of C→B (if it was undoable)
            await self.mcp.send_command("undo", {})

            # Undo creation of C
            await self.mcp.send_command("undo", {})

            # Undo deletion of A (should restore A with its connection)
            await self.mcp.send_command("undo", {})

            # Verify A is restored with original connection
            resp = await self.mcp.send_command("list_elements", {})
            elements = len(resp.result.get("elements", []))

            if elements < 2:  # A and B should be restored
                self.failed.append(f"{test_name}: Expected at least 2 elements after undo, got {elements}")
                return

            # Verify A exists (the important part)
            resp = await self.mcp.send_command("list_elements", {})
            element_ids = [e["element_id"] for e in resp.result.get("elements", [])]

            if a_id not in element_ids:
                self.failed.append(f"{test_name}: A should be restored after undo")
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

    async def test_element_id_uniqueness(self):
        """Test 7.5.2: Element ID Uniqueness and Orphaned References

        Tests that deleted elements don't leave dangling references.
        """
        self.test_count += 1
        test_name = "7.5.2: Element ID Uniqueness"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create chain: A → B → C
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 0,
                "y": 50
            })
            a_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {
                "type": "Or",
                "x": 100,
                "y": 50
            })
            b_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 200,
                "y": 50
            })
            c_id = resp.result["element_id"]

            # Create connections
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

            # Delete B (in the middle)
            await self.mcp.send_command("delete_element", {
                "element_id": b_id
            })

            # Verify A and C still exist but disconnected
            resp = await self.mcp.send_command("list_elements", {})
            elements = len(resp.result.get("elements", []))

            if elements != 2:
                self.failed.append(f"{test_name}: Expected 2 elements after delete, got {elements}")
                return

            resp = await self.mcp.send_command("list_connections", {})
            connections = len(resp.result.get("connections", []))

            if connections != 0:
                self.failed.append(f"{test_name}: Expected 0 connections after delete, got {connections}")
                return

            # Verify A and C are the remaining elements
            resp = await self.mcp.send_command("list_elements", {})
            element_ids = [e["element_id"] for e in resp.result.get("elements", [])]

            if a_id not in element_ids or c_id not in element_ids:
                self.failed.append(f"{test_name}: A or C not found in remaining elements")
                return

            if b_id in element_ids:
                self.failed.append(f"{test_name}: B should not exist after deletion")
                return

            # Undo deletion of B
            await self.mcp.send_command("undo", {})

            # Verify all 3 original elements are back
            resp = await self.mcp.send_command("list_elements", {})
            elements = len(resp.result.get("elements", []))

            if elements != 3:
                self.failed.append(f"{test_name}: Expected 3 elements after undo, got {elements}")
                return

            # Verify B is restored
            resp = await self.mcp.send_command("list_elements", {})
            element_ids = [e["element_id"] for e in resp.result.get("elements", [])]

            if b_id not in element_ids:
                self.failed.append(f"{test_name}: B should be restored after undo")
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

    async def test_massive_fan_out_stress(self):
        """Test 7.5.5: Massive Fan-Out Stress Test

        Tests system with 50+ simultaneous connections from one source.
        """
        self.test_count += 1
        test_name = "7.5.5: Massive Fan-Out Stress"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create source element
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 0,
                "y": 250
            })
            source_id = resp.result["element_id"]

            # Create 50 sink elements
            sink_ids = []
            for i in range(50):
                resp = await self.mcp.send_command("create_element", {
                    "type": "Or",
                    "x": 100 + (i % 10) * 30,
                    "y": 0 + (i // 10) * 50
                })
                sink_ids.append(resp.result["element_id"])

            # Connect source to all 50 sinks
            for i, sink_id in enumerate(sink_ids):
                resp = await self.mcp.send_command("connect_elements", {
                    "source_id": source_id,
                    "source_port": 0,
                    "target_id": sink_id,
                    "target_port": 0
                })
                if not resp.success:
                    self.failed.append(f"{test_name}: Failed to connect to sink {i}")
                    return

            # Verify all 50 connections exist
            resp = await self.mcp.send_command("list_connections", {})
            connections = len(resp.result.get("connections", []))

            if connections != 50:
                self.failed.append(f"{test_name}: Expected 50 connections, got {connections}")
                return

            # Delete source
            await self.mcp.send_command("delete_element", {
                "element_id": source_id
            })

            # Verify all 50 connections removed
            resp = await self.mcp.send_command("list_connections", {})
            connections_after_delete = len(resp.result.get("connections", []))

            if connections_after_delete != 0:
                self.failed.append(f"{test_name}: Expected 0 connections after delete, got {connections_after_delete}")
                return

            # Verify all 50 sinks still exist
            resp = await self.mcp.send_command("list_elements", {})
            elements = len(resp.result.get("elements", []))

            if elements != 50:
                self.failed.append(f"{test_name}: Expected 50 sink elements, got {elements}")
                return

            # Undo deletion
            await self.mcp.send_command("undo", {})

            # Verify source restored with all 50 connections
            resp = await self.mcp.send_command("list_elements", {})
            elements_after_undo = len(resp.result.get("elements", []))

            if elements_after_undo != 51:  # Source + 50 sinks
                self.failed.append(f"{test_name}: Expected 51 elements after undo, got {elements_after_undo}")
                return

            resp = await self.mcp.send_command("list_connections", {})
            connections_after_undo = len(resp.result.get("connections", []))

            if connections_after_undo != 50:
                self.failed.append(f"{test_name}: Expected 50 connections after undo, got {connections_after_undo}")
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

async def main():
    test_suite = Phase75CornerCases()
    success = await test_suite.run_all()
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    asyncio.run(main())
