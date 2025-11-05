#!/usr/bin/env python3

"""
Morph Edge Case Test: Display Type Morphing with Invalid Connection

This test reproduces the bug where:
1. Add a 14-segment display and a button
2. Connect button to the highest index port of the display (port 13 for 14-segment)
3. Morph display to 7-segment (which has ports 0-6, so port 13 is invalid)

Expected behavior (newer versions): Connection should be removed when port becomes invalid
Buggy behavior (older versions): Crash or orphaned connection with invalid port reference

Since morph is not exposed in MCP, we simulate by:
1. Creating Display14 and Button
2. Connecting to high-index port
3. Deleting Display14 and creating Display7
4. Verifying connection state management
"""

import asyncio
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure

class MorphEdgeCaseTest:
    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=True, verbose=False)
        self.test_count = 0
        self.passed = 0
        self.failed = []

    async def run_all(self):
        print("\n" + "="*70)
        print("🔬 MORPH EDGE CASE - Display Type Changes with Invalid Connections")
        print("="*70)

        if not await self.mcp.start_mcp():
            print("❌ MCP startup failed")
            return False

        try:
            # Test the display morphing edge case
            await self.test_display_port_reduction_with_connection()

            # Print summary
            print("\n" + "="*70)
            print(f"RESULTS: {self.passed}/{self.test_count} tests passed")
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

    async def test_display_port_reduction_with_connection(self):
        """Test edge case: Connection to port that no longer exists after morph

        This tests the scenario where a connection exists to a high-numbered port
        (e.g., port 13 on Display14) and then the display is morphed to a type
        with fewer ports (e.g., Display7 with only ports 0-6).

        The connection should either:
        1. Be removed (correct behavior)
        2. Not be in orphaned/invalid state (crash would be bad)
        """
        self.test_count += 1
        test_name = "Display Morph: Port Reduction with Connection"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create a Display14 (14 input ports: 0-13)
            resp = await self.mcp.send_command("create_element", {
                "type": "Display14",
                "x": 100,
                "y": 100
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to create Display14")
                return

            display14_id = resp.result["element_id"]

            # Create an input button (output/source element)
            resp = await self.mcp.send_command("create_element", {
                "type": "InputButton",
                "x": 0,
                "y": 100
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to create InputButton")
                return

            button_id = resp.result["element_id"]

            # Connect button to the HIGHEST port of Display14 (port 13)
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": button_id,
                "source_port": 0,
                "target_id": display14_id,
                "target_port": 13  # Highest valid port for Display14
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to connect button to Display14.in(13)")
                return

            # Verify connection exists
            resp = await self.mcp.send_command("list_connections", {})
            connections_before = len(resp.result.get("connections", []))

            if connections_before != 1:
                self.failed.append(f"{test_name}: Expected 1 connection before morph, got {connections_before}")
                return

            # Now simulate morphing Display14 to Display7 by:
            # 1. Delete the Display14
            # 2. Create a Display7 in its place
            # (This simulates what would happen if morph changed port count)

            await self.mcp.send_command("delete_element", {
                "element_id": display14_id
            })

            # Verify connection was removed when Display14 was deleted
            resp = await self.mcp.send_command("list_connections", {})
            connections_after_delete = len(resp.result.get("connections", []))

            if connections_after_delete != 0:
                self.failed.append(f"{test_name}: Expected 0 connections after deleting Display14, got {connections_after_delete}")
                return

            # Create a Display7 (7 input ports: 0-6, so port 13 doesn't exist)
            resp = await self.mcp.send_command("create_element", {
                "type": "Display7",
                "x": 100,
                "y": 100
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to create Display7")
                return

            display7_id = resp.result["element_id"]

            # Try to connect button to Display7
            # First try the high port (should fail - port doesn't exist)
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": button_id,
                "source_port": 0,
                "target_id": display7_id,
                "target_port": 13  # This port doesn't exist on Display7!
            })

            # This should fail because port 13 doesn't exist on Display7
            if resp.success:
                self.failed.append(f"{test_name}: Should not be able to connect to non-existent port 13 on Display7")
                return

            # Try connecting to a valid port on Display7
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": button_id,
                "source_port": 0,
                "target_id": display7_id,
                "target_port": 6  # Highest valid port for Display7
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to connect button to Display7.in(6)")
                return

            # Verify the new connection exists
            resp = await self.mcp.send_command("list_connections", {})
            connections_final = len(resp.result.get("connections", []))

            if connections_final != 1:
                self.failed.append(f"{test_name}: Expected 1 connection after creating Display7, got {connections_final}")
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

async def main():
    test_suite = MorphEdgeCaseTest()
    success = await test_suite.run_all()
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    asyncio.run(main())
