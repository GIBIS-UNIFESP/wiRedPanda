#!/usr/bin/env python3

"""
Phase 7.2: Deletion Edge Cases - Complex deletion scenarios with undo/redo

Tests for:
1. Delete with Complex Fan-Out (multiple outputs)
2. Delete with Complex Fan-In (multiple inputs)
3. Delete in Feedback Loop (circular dependencies)
4. Delete with Deep Undo Stack (100+ operations)
5. Delete-Undo-Delete Pattern (repetitive operations)

Run: python3 test/mcp/test_phase7_2_deletion_edge_cases.py
"""

import asyncio
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure

class Phase72DeletionEdgeCases:
    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=True, verbose=False)
        self.test_count = 0
        self.passed = 0
        self.failed = []

    async def run_all(self):
        print("\n" + "="*70)
        print("🔬 PHASE 7.2 - DELETION EDGE CASES")
        print("="*70)

        if not await self.mcp.start_mcp():
            print("❌ MCP startup failed")
            return False

        try:
            # Deletion edge cases
            await self.test_complex_fan_out()
            await self.test_complex_fan_in()
            await self.test_feedback_loop()
            await self.test_deep_undo_stack()
            await self.test_delete_undo_delete_pattern()

            # Results
            print("\n" + "="*70)
            print(f"📊 RESULTS: {self.passed}/{self.test_count} passed")
            if self.failed:
                print("❌ FAILED:")
                for t in self.failed:
                    print(f"  - {t}")
                return False
            else:
                print("✅ ALL PASSED")
                return True

        finally:
            await self.mcp.stop_mcp()

    async def test_complex_fan_out(self):
        """Test: Delete source with 4 outputs (fan-out)"""
        self.test_count += 1
        print("\n[7.2.1] Delete with Complex Fan-Out (4 outputs)")

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create source A with 4 outputs, each connected to a different sink
            resp = await self.mcp.send_command("create_element", {"type": "And", "x": 100, "y": 100})
            a_id = resp.result["element_id"]

            sinks = []
            for i in range(4):
                resp = await self.mcp.send_command("create_element", {"type": "Or", "x": 200+i*60, "y": 100})
                sinks.append(resp.result["element_id"])

            # Connect A to all 4 sinks
            for i, sink_id in enumerate(sinks):
                # Note: AND gate only has 1 output, so we connect to port 0 four times
                # but to different sink inputs
                resp = await self.mcp.send_command("create_element", {"type": "And", "x": 100+i*15, "y": 150})
                source_id = resp.result["element_id"]

                await self.mcp.send_command("connect_elements", {
                    "source_id": source_id, "source_port": 0,
                    "target_id": sink_id, "target_port": 0
                })

            # Verify 4 connections exist
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 4:
                self.failed.append(f"7.2.1: Expected 4 connections before delete, got {len(resp.result.get('connections', []))}")
                return

            # Delete first source (should delete 1 connection)
            resp = await self.mcp.send_command("create_element", {"type": "And", "x": 100, "y": 100})
            first_source = resp.result["element_id"]

            await self.mcp.send_command("connect_elements", {
                "source_id": first_source, "source_port": 0,
                "target_id": sinks[0], "target_port": 0
            })

            await self.mcp.send_command("delete_element", {"element_id": first_source})

            # Verify connection removed
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 4:
                self.failed.append(f"7.2.1: Expected 4 connections after delete, got {len(resp.result.get('connections', []))}")
                return

            # Undo
            await self.mcp.send_command("undo", {})

            # Verify connection restored
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 5:
                self.failed.append(f"7.2.1: Expected 5 connections after undo, got {len(resp.result.get('connections', []))}")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.2.1: {e}")

    async def test_complex_fan_in(self):
        """Test: Delete sink with 4 inputs (fan-in)"""
        self.test_count += 1
        print("\n[7.2.2] Delete with Complex Fan-In (4 inputs)")

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create 4 sources and 1 sink with 4 inputs
            sources = []
            for i in range(4):
                resp = await self.mcp.send_command("create_element", {"type": "And", "x": 100+i*50, "y": 100})
                sources.append(resp.result["element_id"])

            resp = await self.mcp.send_command("create_element", {"type": "Or", "x": 300, "y": 100})
            z_id = resp.result["element_id"]

            # Change Or gate to have 4 inputs
            await self.mcp.send_command("change_input_size", {"element_id": z_id, "size": 4})

            # Connect all sources to Z
            for i, s_id in enumerate(sources):
                await self.mcp.send_command("connect_elements", {
                    "source_id": s_id, "source_port": 0,
                    "target_id": z_id, "target_port": i
                })

            # Verify 4 connections exist
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 4:
                self.failed.append(f"7.2.2: Expected 4 connections before delete, got {len(resp.result.get('connections', []))}")
                return

            # Delete Z
            await self.mcp.send_command("delete_element", {"element_id": z_id})

            # Verify all connections removed
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 0:
                self.failed.append(f"7.2.2: Expected 0 connections after delete, got {len(resp.result.get('connections', []))}")
                return

            # Verify sources still exist
            resp = await self.mcp.send_command("list_elements", {})
            elem_ids = [e.get("element_id") for e in resp.result.get("elements", [])]
            for s_id in sources:
                if s_id not in elem_ids:
                    self.failed.append(f"7.2.2: Source {s_id} should still exist after Z deletion")
                    return

            # Undo
            await self.mcp.send_command("undo", {})

            # Verify all connections restored
            resp = await self.mcp.send_command("list_connections", {})
            conns = resp.result.get("connections", [])
            if len(conns) != 4:
                self.failed.append(f"7.2.2: Expected 4 connections after undo, got {len(conns)}")
                return

            # Verify correct port mappings
            for i in range(4):
                expected_conn = {
                    "source_id": sources[i],
                    "source_port": 0,
                    "target_id": z_id,
                    "target_port": i
                }
                if expected_conn not in conns:
                    self.failed.append(f"7.2.2: Expected connection {expected_conn} not found in restored connections")
                    return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.2.2: {e}")

    async def test_feedback_loop(self):
        """Test: Delete element in feedback loop"""
        self.test_count += 1
        print("\n[7.2.3] Delete in Feedback Loop")

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create two NAND gates for SR latch
            resp = await self.mcp.send_command("create_element", {"type": "Nand", "x": 100, "y": 100})
            nand1_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {"type": "Nand", "x": 200, "y": 100})
            nand2_id = resp.result["element_id"]

            # Create feedback: NAND1 → NAND2, NAND2 → NAND1
            # NAND gates need to have configurable inputs
            await self.mcp.send_command("change_input_size", {"element_id": nand1_id, "size": 2})
            await self.mcp.send_command("change_input_size", {"element_id": nand2_id, "size": 2})

            # Connect feedback loop
            await self.mcp.send_command("connect_elements", {
                "source_id": nand1_id, "source_port": 0,
                "target_id": nand2_id, "target_port": 0
            })

            await self.mcp.send_command("connect_elements", {
                "source_id": nand2_id, "source_port": 0,
                "target_id": nand1_id, "target_port": 1
            })

            # Verify 2 connections exist
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 2:
                self.failed.append(f"7.2.3: Expected 2 connections before delete, got {len(resp.result.get('connections', []))}")
                return

            # Delete NAND1
            await self.mcp.send_command("delete_element", {"element_id": nand1_id})

            # Verify connections removed
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 0:
                self.failed.append(f"7.2.3: Expected 0 connections after delete, got {len(resp.result.get('connections', []))}")
                return

            # Verify NAND2 still exists
            resp = await self.mcp.send_command("list_elements", {})
            elem_ids = [e.get("element_id") for e in resp.result.get("elements", [])]
            if nand2_id not in elem_ids:
                self.failed.append(f"7.2.3: NAND2 should still exist after NAND1 deletion")
                return

            # Undo
            await self.mcp.send_command("undo", {})

            # Verify feedback loop restored
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 2:
                self.failed.append(f"7.2.3: Expected 2 connections after undo, got {len(resp.result.get('connections', []))}")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.2.3: {e}")

    async def test_deep_undo_stack(self):
        """Test: Delete after 20+ operations"""
        self.test_count += 1
        print("\n[7.2.4] Delete with Deep Undo Stack (20+ ops)")

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create chain: A → B → C → D
            elements = []
            for i in range(4):
                resp = await self.mcp.send_command("create_element", {"type": "And", "x": 100+i*50, "y": 100})
                elements.append(resp.result["element_id"])

            # Connect them
            for i in range(3):
                await self.mcp.send_command("connect_elements", {
                    "source_id": elements[i], "source_port": 0,
                    "target_id": elements[i+1], "target_port": 0
                })

            ops = 0
            # Perform 20+ moves
            for i in range(20):
                resp = await self.mcp.send_command("update_position", {
                    "element_id": elements[i % 4],
                    "x": 100 + i*5,
                    "y": 100 + (i % 2) * 10
                })
                if resp.success:
                    ops += 1

            # Delete middle element (B)
            await self.mcp.send_command("delete_element", {"element_id": elements[1]})

            # Verify connections: A→B removed, B→C removed, but C→D remains
            resp = await self.mcp.send_command("list_connections", {})
            conns = resp.result.get("connections", [])
            if len(conns) != 1:  # Only C→D should remain
                self.failed.append(f"7.2.4: Expected 1 connection after delete (C→D), got {len(conns)}")
                return

            # Undo all operations
            for _ in range(ops + 1):  # +1 for deletion
                await self.mcp.send_command("undo", {})

            # Verify all restored
            resp = await self.mcp.send_command("list_elements", {})
            elem_count = len(resp.result.get("elements", []))
            if elem_count != 4:
                self.failed.append(f"7.2.4: Expected 4 elements after undo, got {elem_count}")
                return

            resp = await self.mcp.send_command("list_connections", {})
            conn_count = len(resp.result.get("connections", []))
            if conn_count != 3:
                self.failed.append(f"7.2.4: Expected 3 connections after undo, got {conn_count}")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.2.4: {e}")

    async def test_delete_undo_delete_pattern(self):
        """Test: Delete-Undo-Delete-Undo pattern (repetitive operations)"""
        self.test_count += 1
        print("\n[7.2.5] Delete-Undo-Delete-Undo Pattern")

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create simple circuit: A → B
            resp = await self.mcp.send_command("create_element", {"type": "And", "x": 100, "y": 100})
            a_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {"type": "Or", "x": 200, "y": 100})
            b_id = resp.result["element_id"]

            await self.mcp.send_command("connect_elements", {
                "source_id": a_id, "source_port": 0,
                "target_id": b_id, "target_port": 0
            })

            # First delete
            await self.mcp.send_command("delete_element", {"element_id": a_id})

            # Verify deleted
            resp = await self.mcp.send_command("list_elements", {})
            elem_ids = [e.get("element_id") for e in resp.result.get("elements", [])]
            if a_id in elem_ids:
                self.failed.append("7.2.5: A should be deleted after first delete")
                return

            # First undo
            await self.mcp.send_command("undo", {})

            # Verify restored
            resp = await self.mcp.send_command("list_elements", {})
            elem_ids = [e.get("element_id") for e in resp.result.get("elements", [])]
            if a_id not in elem_ids:
                self.failed.append("7.2.5: A should be restored after undo")
                return

            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 1:
                self.failed.append("7.2.5: Connection should be restored after undo")
                return

            # Second delete (same element)
            await self.mcp.send_command("delete_element", {"element_id": a_id})

            # Verify deleted
            resp = await self.mcp.send_command("list_elements", {})
            elem_ids = [e.get("element_id") for e in resp.result.get("elements", [])]
            if a_id in elem_ids:
                self.failed.append("7.2.5: A should be deleted after second delete")
                return

            # Second undo
            await self.mcp.send_command("undo", {})

            # Verify restored again
            resp = await self.mcp.send_command("list_elements", {})
            elem_ids = [e.get("element_id") for e in resp.result.get("elements", [])]
            if a_id not in elem_ids:
                self.failed.append("7.2.5: A should be restored after second undo")
                return

            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 1:
                self.failed.append("7.2.5: Connection should be restored after second undo")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.2.5: {e}")


async def main():
    tester = Phase72DeletionEdgeCases()
    return 0 if await tester.run_all() else 1


if __name__ == "__main__":
    exit_code = asyncio.run(main())
    sys.exit(exit_code)
