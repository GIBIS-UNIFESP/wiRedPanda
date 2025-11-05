#!/usr/bin/env python3
"""
Phase 7.1: Critical Edge Case Testing - Simplified Focus

Core edge cases from ultrathink analysis:
1. Dangling connections (source and sink deletion)
2. Self-loops and circular connections
3. Fan-out/fan-in deletion cascades
4. Deep undo stacks with connections

Run: python3 test/mcp/test_phase7_1_edge_cases.py
"""

import asyncio
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure

class Phase71EdgeCases:
    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=True, verbose=False)
        self.test_count = 0
        self.passed = 0
        self.failed = []

    async def run_all(self):
        print("\n" + "="*70)
        print("🔬 PHASE 7.1 - CRITICAL EDGE CASES")
        print("="*70)

        if not await self.mcp.start_mcp():
            print("❌ MCP startup failed")
            return False

        try:
            # Edge cases
            await self.test_dangling_source()
            await self.test_dangling_sink()
            await self.test_self_loop()
            await self.test_fan_out_delete()
            await self.test_fan_in_delete()
            await self.test_deep_undo()

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

    async def test_dangling_source(self):
        """Edge case: Delete source, verify connection removed, undo restores"""
        self.test_count += 1
        print("\n[7.1.1] Dangling Connection - Source Delete")

        try:
            await self.mcp.send_command("new_circuit", {})

            # A → B
            resp = await self.mcp.send_command("create_element", {"type": "And", "x": 100, "y": 100})
            a_id = resp.result["element_id"]
            resp = await self.mcp.send_command("create_element", {"type": "Or", "x": 200, "y": 100})
            b_id = resp.result["element_id"]

            # Connect
            await self.mcp.send_command("connect_elements", {
                "source_id": a_id, "source_port": 0,
                "target_id": b_id, "target_port": 0
            })

            # Delete A
            await self.mcp.send_command("delete_element", {"element_id": a_id})

            # Verify connection gone
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 0:
                self.failed.append("7.1.1: connection not removed")
                return

            # Undo
            await self.mcp.send_command("undo", {})

            # Verify connection restored
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 1:
                self.failed.append("7.1.1: connection not restored")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.1.1: {e}")

    async def test_dangling_sink(self):
        """Edge case: Delete sink, verify connection removed, undo restores"""
        self.test_count += 1
        print("\n[7.1.2] Dangling Connection - Sink Delete")

        try:
            await self.mcp.send_command("new_circuit", {})

            # A → B
            resp = await self.mcp.send_command("create_element", {"type": "And", "x": 100, "y": 100})
            a_id = resp.result["element_id"]
            resp = await self.mcp.send_command("create_element", {"type": "Or", "x": 200, "y": 100})
            b_id = resp.result["element_id"]

            # Connect
            await self.mcp.send_command("connect_elements", {
                "source_id": a_id, "source_port": 0,
                "target_id": b_id, "target_port": 0
            })

            # Delete B (sink)
            await self.mcp.send_command("delete_element", {"element_id": b_id})

            # Verify connection gone
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 0:
                self.failed.append("7.1.2: connection not removed")
                return

            # Verify A still exists
            resp = await self.mcp.send_command("list_elements", {})
            if not any(e.get("element_id") == a_id for e in resp.result.get("elements", [])):
                self.failed.append("7.1.2: source deleted with sink")
                return

            # Undo
            await self.mcp.send_command("undo", {})

            # Verify connection restored
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 1:
                self.failed.append("7.1.2: connection not restored")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.1.2: {e}")

    async def test_self_loop(self):
        """Edge case: Self-loop (A → A)"""
        self.test_count += 1
        print("\n[7.1.3] Self-Loop Connection")

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create A
            resp = await self.mcp.send_command("create_element", {"type": "And", "x": 100, "y": 100})
            a_id = resp.result["element_id"]

            # Try self-loop
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": a_id, "source_port": 0,
                "target_id": a_id, "target_port": 0
            })

            if not resp.success:
                print("ℹ Self-loops not supported")
                self.passed += 1
                return

            # Delete A
            await self.mcp.send_command("delete_element", {"element_id": a_id})

            # Verify self-loop gone
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 0:
                self.failed.append("7.1.3: self-loop not removed")
                return

            # Undo
            await self.mcp.send_command("undo", {})

            # Verify self-loop restored
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 1:
                self.failed.append("7.1.3: self-loop not restored")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.1.3: {e}")

    async def test_fan_out_delete(self):
        """Edge case: Delete source with fan-out (A → [B,C,D])"""
        self.test_count += 1
        print("\n[7.2.1] Delete with Fan-Out")

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create A and targets
            resp = await self.mcp.send_command("create_element", {"type": "And", "x": 100, "y": 100})
            a_id = resp.result["element_id"]

            targets = []
            for i in range(3):
                resp = await self.mcp.send_command("create_element", {"type": "Or", "x": 200+i*50, "y": 100})
                targets.append(resp.result["element_id"])

            # Connect A to all
            for t_id in targets:
                await self.mcp.send_command("connect_elements", {
                    "source_id": a_id, "source_port": 0,
                    "target_id": t_id, "target_port": 0
                })

            # Delete A
            await self.mcp.send_command("delete_element", {"element_id": a_id})

            # Verify all connections gone
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 0:
                self.failed.append(f"7.2.1: {len(resp.result.get('connections', []))} connections remain")
                return

            # Undo
            await self.mcp.send_command("undo", {})

            # Verify all connections restored
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 3:
                self.failed.append("7.2.1: not all connections restored")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.2.1: {e}")

    async def test_fan_in_delete(self):
        """Edge case: Delete sink with fan-in ([A,B,C] → Z)"""
        self.test_count += 1
        print("\n[7.2.2] Delete with Fan-In")

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create sources and Z
            sources = []
            for i in range(3):
                resp = await self.mcp.send_command("create_element", {"type": "And", "x": 100+i*50, "y": 100})
                sources.append(resp.result["element_id"])

            resp = await self.mcp.send_command("create_element", {"type": "Or", "x": 300, "y": 100})
            z_id = resp.result["element_id"]

            # Change Or gate to have 3 inputs
            await self.mcp.send_command("change_input_size", {"element_id": z_id, "size": 3})

            # Connect all sources to Z
            for i, s_id in enumerate(sources):
                await self.mcp.send_command("connect_elements", {
                    "source_id": s_id, "source_port": 0,
                    "target_id": z_id, "target_port": i
                })

            # Delete Z
            await self.mcp.send_command("delete_element", {"element_id": z_id})

            # Verify all connections gone
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 0:
                self.failed.append("7.2.2: connections not removed")
                return

            # Undo
            await self.mcp.send_command("undo", {})

            # Verify all connections restored
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 3:
                self.failed.append("7.2.2: not all connections restored")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.2.2: {e}")

    async def test_deep_undo(self):
        """Edge case: Deep undo stack with connections"""
        self.test_count += 1
        print("\n[7.2.4] Deep Undo Stack")

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

            # Perform moves (20+ operations)
            ops = 0
            for i in range(20):
                resp = await self.mcp.send_command("update_position", {
                    "element_id": elements[i % 4],
                    "x": 100 + i*5,
                    "y": 100
                })
                if resp.success:
                    ops += 1

            # Delete middle element
            await self.mcp.send_command("delete_element", {"element_id": elements[1]})

            # Undo everything
            for _ in range(ops + 1):  # +1 for deletion
                await self.mcp.send_command("undo", {})

            # Verify state restored
            resp = await self.mcp.send_command("list_elements", {})
            elem_count = len(resp.result.get("elements", []))
            if elem_count != 4:
                self.failed.append(f"7.2.4: {elem_count} elements (expected 4)")
                return

            resp = await self.mcp.send_command("list_connections", {})
            conn_count = len(resp.result.get("connections", []))
            if conn_count != 3:
                self.failed.append(f"7.2.4: {conn_count} connections (expected 3)")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.2.4: {e}")


async def main():
    validator = Phase71EdgeCases()
    return 0 if await validator.run_all() else 1

if __name__ == "__main__":
    exit_code = asyncio.run(main())
    sys.exit(exit_code)
