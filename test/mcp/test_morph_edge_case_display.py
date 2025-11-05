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
        self.mcp = MCPInfrastructure(enable_validation=False, verbose=False)
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

            # Now use the actual morph_element command to morph Display14 to Display7
            # This tests the real morph behavior with connection cleanup
            resp = await self.mcp.send_command("morph_element", {
                "element_ids": [display14_id],
                "target_type": "Display7"
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to morph Display14 to Display7")
                return

            # Verify the morph was successful
            morphed_ids = resp.result.get("morphed_elements", [])
            if not morphed_ids:
                self.failed.append(f"{test_name}: Morphed elements list is empty")
                return
            # Note: The morphed element has a new ID (new object created with unique ID)

            if resp.result.get("target_type") != "Display7":
                self.failed.append(f"{test_name}: Target type mismatch")
                return

            # Use the morphed ID from the response
            morphed_display_id = morphed_ids[0]

            # Verify connection handling after morph:
            # The connection to port 13 should be REMOVED because Display7 doesn't have port 13
            resp = await self.mcp.send_command("list_connections", {})
            connections_after_morph = len(resp.result.get("connections", []))

            # This is the critical part of the bug test:
            # - If crash occurs: test fails (but we're past the morph, so it didn't crash)
            # - If connection remains orphaned: connections_after_morph would be 1 (BUG)
            # - If connection properly removed: connections_after_morph would be 0 (CORRECT)
            if connections_after_morph != 0:
                self.failed.append(
                    f"{test_name}: Expected 0 connections after morph (connection should be removed for invalid port), "
                    f"got {connections_after_morph}. This indicates the orphaned connection bug."
                )
                return

            # Verify we can still connect to a valid port on the morphed Display7
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": button_id,
                "source_port": 0,
                "target_id": morphed_display_id,  # Use the morphed element ID
                "target_port": 6  # Highest valid port for Display7
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to connect button to morphed Display7.in(6)")
                return

            # Verify the new connection exists
            resp = await self.mcp.send_command("list_connections", {})
            connections_final = len(resp.result.get("connections", []))

            if connections_final != 1:
                self.failed.append(f"{test_name}: Expected 1 connection after creating new connection, got {connections_final}")
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
