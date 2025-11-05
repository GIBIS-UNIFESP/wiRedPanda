#!/usr/bin/env python3

"""
Phase 7.3: Configuration Edge Cases - Element configuration changes with undo/redo

Tests for:
1. Port Reduction on Connected Element (removing ports with connections)
2. Port Expansion on Connected Element (adding ports)
3. Multiple Configuration Changes (sequential port changes)
4. Configuration with Feedback Loops (port changes in circular circuits)
5. Configuration-Undo-Configuration Pattern (repetitive config changes)

Since morph/type-change isn't in MCP, we test via input/output size changes
which have similar effects: changing port count and requiring connection management.

Run: python3 test/mcp/test_phase7_3_configuration_edge_cases.py
"""

import asyncio
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure

class Phase73ConfigurationEdgeCases:
    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=True, verbose=False)
        self.test_count = 0
        self.passed = 0
        self.failed = []

    async def run_all(self):
        print("\n" + "="*70)
        print("🔬 PHASE 7.3 - CONFIGURATION EDGE CASES")
        print("="*70)

        if not await self.mcp.start_mcp():
            print("❌ MCP startup failed")
            return False

        try:
            # Configuration edge cases
            await self.test_port_reduction()
            await self.test_port_expansion()
            await self.test_multiple_reconfigurations()
            await self.test_configuration_with_feedback()
            await self.test_configuration_undo_pattern()

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

    async def test_port_reduction(self):
        """Test: Reduce input ports on element with connected inputs"""
        self.test_count += 1
        print("\n[7.3.1] Port Reduction with Connections")

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create element with 4 inputs and connect them all
            resp = await self.mcp.send_command("create_element", {"type": "Or", "x": 100, "y": 100})
            sink_id = resp.result["element_id"]

            # Expand to 4 inputs
            await self.mcp.send_command("change_input_size", {"element_id": sink_id, "size": 4})

            # Create 4 sources and connect to all ports
            sources = []
            for i in range(4):
                resp = await self.mcp.send_command("create_element", {"type": "And", "x": 100+i*50, "y": 100})
                sources.append(resp.result["element_id"])

            for i, src_id in enumerate(sources):
                await self.mcp.send_command("connect_elements", {
                    "source_id": src_id, "source_port": 0,
                    "target_id": sink_id, "target_port": i
                })

            # Verify 4 connections
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 4:
                self.failed.append(f"7.3.1: Expected 4 connections before reduction, got {len(resp.result.get('connections', []))}")
                return

            # Reduce to 2 inputs (should remove connections to ports 2 and 3)
            await self.mcp.send_command("change_input_size", {"element_id": sink_id, "size": 2})

            # Verify only 2 connections remain
            resp = await self.mcp.send_command("list_connections", {})
            conns = resp.result.get("connections", [])
            if len(conns) != 2:
                self.failed.append(f"7.3.1: Expected 2 connections after reduction, got {len(conns)}")
                return

            # Verify remaining connections are to ports 0 and 1
            ports = sorted([c["target_port"] for c in conns])
            if ports != [0, 1]:
                self.failed.append(f"7.3.1: Expected connections on ports [0, 1], got {ports}")
                return

            # Undo reduction (expand back to 4)
            await self.mcp.send_command("undo", {})

            # Verify all 4 connections restored with correct port mapping
            resp = await self.mcp.send_command("list_connections", {})
            conns = resp.result.get("connections", [])
            if len(conns) != 4:
                self.failed.append(f"7.3.1: Expected 4 connections after undo, got {len(conns)}")
                return

            # Verify all ports restored correctly
            ports = sorted([c["target_port"] for c in conns])
            if ports != [0, 1, 2, 3]:
                self.failed.append(f"7.3.1: Expected connections on ports [0, 1, 2, 3] after undo, got {ports}")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.3.1: {e}")

    async def test_port_expansion(self):
        """Test: Expand input ports on element and add new connections"""
        self.test_count += 1
        print("\n[7.3.2] Port Expansion with New Connections")

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create element with 2 inputs and connect them
            resp = await self.mcp.send_command("create_element", {"type": "Or", "x": 100, "y": 100})
            sink_id = resp.result["element_id"]

            # Create 2 sources and connect
            resp = await self.mcp.send_command("create_element", {"type": "And", "x": 50, "y": 100})
            src1_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {"type": "And", "x": 100, "y": 100})
            src2_id = resp.result["element_id"]

            await self.mcp.send_command("connect_elements", {
                "source_id": src1_id, "source_port": 0,
                "target_id": sink_id, "target_port": 0
            })

            await self.mcp.send_command("connect_elements", {
                "source_id": src2_id, "source_port": 0,
                "target_id": sink_id, "target_port": 1
            })

            # Verify 2 connections
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 2:
                self.failed.append(f"7.3.2: Expected 2 initial connections, got {len(resp.result.get('connections', []))}")
                return

            # Expand to 4 inputs
            await self.mcp.send_command("change_input_size", {"element_id": sink_id, "size": 4})

            # Connections should still be there
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 2:
                self.failed.append(f"7.3.2: Expected 2 connections after expansion, got {len(resp.result.get('connections', []))}")
                return

            # Create 2 more sources and connect to new ports
            resp = await self.mcp.send_command("create_element", {"type": "And", "x": 150, "y": 100})
            src3_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {"type": "And", "x": 200, "y": 100})
            src4_id = resp.result["element_id"]

            await self.mcp.send_command("connect_elements", {
                "source_id": src3_id, "source_port": 0,
                "target_id": sink_id, "target_port": 2
            })

            await self.mcp.send_command("connect_elements", {
                "source_id": src4_id, "source_port": 0,
                "target_id": sink_id, "target_port": 3
            })

            # Verify 4 connections now
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 4:
                self.failed.append(f"7.3.2: Expected 4 connections after expansion and new connections, got {len(resp.result.get('connections', []))}")
                return

            # Undo new connections (2 undos)
            await self.mcp.send_command("undo", {})
            await self.mcp.send_command("undo", {})

            # Should have 2 connections again
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 2:
                self.failed.append(f"7.3.2: Expected 2 connections after undoing new connections, got {len(resp.result.get('connections', []))}")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.3.2: {e}")

    async def test_multiple_reconfigurations(self):
        """Test: Multiple sequential port size changes"""
        self.test_count += 1
        print("\n[7.3.3] Multiple Sequential Reconfigurations")

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create element and source for testing
            resp = await self.mcp.send_command("create_element", {"type": "Or", "x": 100, "y": 100})
            elem_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {"type": "And", "x": 50, "y": 100})
            src_id = resp.result["element_id"]

            # Sequence: 2 → 4 → 3 → 5 → 2 → 4
            await self.mcp.send_command("change_input_size", {"element_id": elem_id, "size": 4})

            # Test can connect to port 3 (should fail if still at 2)
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": src_id, "source_port": 0,
                "target_id": elem_id, "target_port": 3
            })
            if not resp.success:
                self.failed.append(f"7.3.3: Should be able to connect to port 3 after expanding to 4")
                return

            await self.mcp.send_command("change_input_size", {"element_id": elem_id, "size": 3})
            await self.mcp.send_command("change_input_size", {"element_id": elem_id, "size": 5})
            await self.mcp.send_command("change_input_size", {"element_id": elem_id, "size": 2})

            # Now at 2 inputs, should NOT be able to connect to port 3
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": src_id, "source_port": 0,
                "target_id": elem_id, "target_port": 3
            })
            if resp.success:
                self.failed.append(f"7.3.3: Should NOT be able to connect to port 3 when only 2 inputs")
                return

            await self.mcp.send_command("change_input_size", {"element_id": elem_id, "size": 4})

            # Should be able to connect to port 3 again
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": src_id, "source_port": 0,
                "target_id": elem_id, "target_port": 3
            })
            if not resp.success:
                self.failed.append(f"7.3.3: Should be able to connect to port 3 after expanding to 4 again")
                return

            # Undo all 7 changes (6 size changes + 1 connection)
            for _ in range(7):
                await self.mcp.send_command("undo", {})

            # Should be back to 2 inputs (default) and original connection gone
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 0:
                self.failed.append(f"7.3.3: Expected 0 connections after all undos")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.3.3: {e}")

    async def test_configuration_with_feedback(self):
        """Test: Change ports on element in feedback loop"""
        self.test_count += 1
        print("\n[7.3.4] Configuration with Feedback Loop")

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create feedback loop
            resp = await self.mcp.send_command("create_element", {"type": "Or", "x": 100, "y": 100})
            a_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {"type": "Or", "x": 200, "y": 100})
            b_id = resp.result["element_id"]

            # Configure for feedback
            await self.mcp.send_command("change_input_size", {"element_id": a_id, "size": 2})
            await self.mcp.send_command("change_input_size", {"element_id": b_id, "size": 2})

            # Create feedback: A.out → B.in(0), B.out → A.in(1)
            await self.mcp.send_command("connect_elements", {
                "source_id": a_id, "source_port": 0,
                "target_id": b_id, "target_port": 0
            })

            await self.mcp.send_command("connect_elements", {
                "source_id": b_id, "source_port": 0,
                "target_id": a_id, "target_port": 1
            })

            # Verify 2 connections
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 2:
                self.failed.append(f"7.3.4: Expected 2 connections in feedback loop, got {len(resp.result.get('connections', []))}")
                return

            # Expand A to 4 inputs (should preserve feedback connection to port 1)
            await self.mcp.send_command("change_input_size", {"element_id": a_id, "size": 4})

            # Verify feedback still intact
            resp = await self.mcp.send_command("list_connections", {})
            conns = resp.result.get("connections", [])
            if len(conns) != 2:
                self.failed.append(f"7.3.4: Expected 2 connections after expansion, got {len(conns)}")
                return

            # Verify feedback connection still on port 1
            feedback_conn = [c for c in conns if c["source_id"] == b_id and c["target_id"] == a_id]
            if not feedback_conn or feedback_conn[0]["target_port"] != 1:
                self.failed.append(f"7.3.4: Feedback connection not preserved to port 1")
                return

            # Undo expansion
            await self.mcp.send_command("undo", {})

            # Verify back to 2 inputs with feedback intact
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 2:
                self.failed.append(f"7.3.4: Expected 2 connections after undo, got {len(resp.result.get('connections', []))}")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.3.4: {e}")

    async def test_configuration_undo_pattern(self):
        """Test: Configuration-Undo-Configuration pattern"""
        self.test_count += 1
        print("\n[7.3.5] Configuration-Undo-Configuration Pattern")

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create element with connections
            resp = await self.mcp.send_command("create_element", {"type": "Or", "x": 100, "y": 100})
            sink_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {"type": "And", "x": 50, "y": 100})
            src_id = resp.result["element_id"]

            await self.mcp.send_command("connect_elements", {
                "source_id": src_id, "source_port": 0,
                "target_id": sink_id, "target_port": 0
            })

            # First configuration change: 2 → 4
            await self.mcp.send_command("change_input_size", {"element_id": sink_id, "size": 4})

            # Verify expanded by testing port access
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": src_id, "source_port": 0,
                "target_id": sink_id, "target_port": 3
            })
            if not resp.success:
                self.failed.append(f"7.3.5: Should be able to connect to port 3 after expanding")
                return

            # Undo configuration change and the new connection
            await self.mcp.send_command("undo", {})
            await self.mcp.send_command("undo", {})

            # Verify back to 2 and connection still exists (the original one)
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 1:
                self.failed.append(f"7.3.5: Expected 1 connection after undo")
                return

            # Second configuration change: 2 → 3
            await self.mcp.send_command("change_input_size", {"element_id": sink_id, "size": 3})

            # Verify at 3 by testing port access
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": src_id, "source_port": 0,
                "target_id": sink_id, "target_port": 2
            })
            if not resp.success:
                self.failed.append(f"7.3.5: Should be able to connect to port 2 after expanding to 3")
                return

            # Second undo (undo the new connection and config change)
            await self.mcp.send_command("undo", {})
            await self.mcp.send_command("undo", {})

            # Verify back to 2 with original connection
            resp = await self.mcp.send_command("list_connections", {})
            if len(resp.result.get("connections", [])) != 1:
                self.failed.append(f"7.3.5: Expected 1 connection after second undo")
                return

            print("✅ PASSED")
            self.passed += 1
        except Exception as e:
            print(f"❌ ERROR: {e}")
            self.failed.append(f"7.3.5: {e}")


async def main():
    tester = Phase73ConfigurationEdgeCases()
    return 0 if await tester.run_all() else 1


if __name__ == "__main__":
    exit_code = asyncio.run(main())
    sys.exit(exit_code)
