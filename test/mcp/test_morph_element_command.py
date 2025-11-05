#!/usr/bin/env python3

"""
Comprehensive Morph Element Command Tests

This test suite validates the morph_element command functionality,
specifically testing the bug scenario where:
1. Create a Display14 element (14 input ports)
2. Connect a button to the highest port (port 13)
3. Morph Display14 to Display7 (only 7 ports: 0-6)
4. Verify connection handling (should be removed since port 13 doesn't exist)

This tests the actual morph_element MCP command implementation.
"""

import asyncio
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure

class MorphElementCommandTest:
    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=True, verbose=False)
        self.test_count = 0
        self.passed = 0
        self.failed = []

    async def run_all(self):
        print("\n" + "="*70)
        print("🧬 MORPH ELEMENT COMMAND - Comprehensive MCP Integration Tests")
        print("="*70)

        if not await self.mcp.start_mcp():
            print("❌ MCP startup failed")
            return False

        try:
            await self.test_morph_basic()
            await self.test_morph_with_connection()
            await self.test_morph_display14_to_display7()
            await self.test_morph_multiple_elements()
            await self.test_morph_preserves_valid_connections()

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

    async def test_morph_basic(self):
        """Test basic morph operation: And gate to Or gate"""
        self.test_count += 1
        test_name = "Morph Basic: And to Or"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create an And gate
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100,
                "y": 100
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to create And gate")
                return

            and_id = resp.result["element_id"]

            # Morph And to Or
            resp = await self.mcp.send_command("morph_element", {
                "element_ids": [and_id],
                "target_type": "Or"
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to morph And to Or")
                return

            morphed_ids = resp.result.get("morphed_elements", [])
            if not morphed_ids or and_id not in morphed_ids:
                self.failed.append(f"{test_name}: Morphed element ID mismatch")
                return

            if resp.result.get("target_type") != "Or":
                self.failed.append(f"{test_name}: Target type mismatch")
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

    async def test_morph_with_connection(self):
        """Test morphing element with incoming connection"""
        self.test_count += 1
        test_name = "Morph with Connection: Preserve valid port"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create source button
            resp = await self.mcp.send_command("create_element", {
                "type": "InputButton",
                "x": 0,
                "y": 100
            })
            button_id = resp.result["element_id"]

            # Create And gate
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100,
                "y": 100
            })
            and_id = resp.result["element_id"]

            # Connect button to And (port 0)
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": button_id,
                "source_port": 0,
                "target_id": and_id,
                "target_port": 0
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to create connection")
                return

            # Verify connection exists
            resp = await self.mcp.send_command("list_connections", {})
            connections_before = len(resp.result.get("connections", []))

            if connections_before != 1:
                self.failed.append(f"{test_name}: Expected 1 connection before morph, got {connections_before}")
                return

            # Morph And to Or (both have port 0, so connection should survive)
            resp = await self.mcp.send_command("morph_element", {
                "element_ids": [and_id],
                "target_type": "Or"
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to morph And to Or")
                return

            # Verify connection still exists
            resp = await self.mcp.send_command("list_connections", {})
            connections_after = len(resp.result.get("connections", []))

            if connections_after != 1:
                self.failed.append(f"{test_name}: Expected 1 connection after morph, got {connections_after}")
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

    async def test_morph_display14_to_display7(self):
        """
        Test the exact bug scenario: Display14 with connection to port 13, morphed to Display7
        This is the core bug that was reported.
        """
        self.test_count += 1
        test_name = "Morph Display14→Display7: High Port Connection"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create Display14 (14 input ports: 0-13)
            resp = await self.mcp.send_command("create_element", {
                "type": "Display14",
                "x": 100,
                "y": 100
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to create Display14")
                return

            display14_id = resp.result["element_id"]

            # Create input button
            resp = await self.mcp.send_command("create_element", {
                "type": "InputButton",
                "x": 0,
                "y": 100
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to create InputButton")
                return

            button_id = resp.result["element_id"]

            # Connect button to HIGHEST port of Display14 (port 13)
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

            # NOW: Morph Display14 to Display7 (only has ports 0-6)
            resp = await self.mcp.send_command("morph_element", {
                "element_ids": [display14_id],
                "target_type": "Display7"
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to morph Display14 to Display7")
                return

            # Verify the morph was successful
            morphed_ids = resp.result.get("morphed_elements", [])
            if not morphed_ids or display14_id not in morphed_ids:
                self.failed.append(f"{test_name}: Display14 not in morphed_elements list")
                return

            # Verify connection handling:
            # The connection to port 13 should be REMOVED because Display7 doesn't have port 13
            resp = await self.mcp.send_command("list_connections", {})
            connections_after = len(resp.result.get("connections", []))

            # This is the critical part of the bug test:
            # - If crash occurs: test fails (but we're past the morph, so it didn't crash)
            # - If connection remains orphaned: connections_after would be 1 (BUG)
            # - If connection properly removed: connections_after would be 0 (CORRECT)

            if connections_after != 0:
                self.failed.append(
                    f"{test_name}: Expected 0 connections after morph (connection should be removed for invalid port), "
                    f"got {connections_after}. This indicates the orphaned connection bug."
                )
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

    async def test_morph_multiple_elements(self):
        """Test morphing multiple elements at once"""
        self.test_count += 1
        test_name = "Morph Multiple Elements"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create three And gates
            and_ids = []
            for i in range(3):
                resp = await self.mcp.send_command("create_element", {
                    "type": "And",
                    "x": 100 + i*50,
                    "y": 100
                })
                and_ids.append(resp.result["element_id"])

            # Morph all three to Or gates
            resp = await self.mcp.send_command("morph_element", {
                "element_ids": and_ids,
                "target_type": "Or"
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to morph multiple elements")
                return

            morphed_ids = resp.result.get("morphed_elements", [])
            if len(morphed_ids) != 3:
                self.failed.append(f"{test_name}: Expected 3 morphed elements, got {len(morphed_ids)}")
                return

            # Verify all IDs are in morphed list
            for and_id in and_ids:
                if and_id not in morphed_ids:
                    self.failed.append(f"{test_name}: Element {and_id} not in morphed list")
                    return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

    async def test_morph_preserves_valid_connections(self):
        """Test that morphing preserves connections to port indices that exist in target type"""
        self.test_count += 1
        test_name = "Morph Preserves Valid Connections"

        try:
            await self.mcp.send_command("new_circuit", {})

            # Create two buttons
            resp = await self.mcp.send_command("create_element", {
                "type": "InputButton",
                "x": 0,
                "y": 100
            })
            button1_id = resp.result["element_id"]

            resp = await self.mcp.send_command("create_element", {
                "type": "InputButton",
                "x": 0,
                "y": 150
            })
            button2_id = resp.result["element_id"]

            # Create And gate
            resp = await self.mcp.send_command("create_element", {
                "type": "And",
                "x": 100,
                "y": 100
            })
            and_id = resp.result["element_id"]

            # Connect both buttons to And gates (ports 0 and 1)
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": button1_id,
                "source_port": 0,
                "target_id": and_id,
                "target_port": 0
            })

            resp = await self.mcp.send_command("connect_elements", {
                "source_id": button2_id,
                "source_port": 0,
                "target_id": and_id,
                "target_port": 1
            })

            # Verify 2 connections exist
            resp = await self.mcp.send_command("list_connections", {})
            connections_before = len(resp.result.get("connections", []))

            if connections_before != 2:
                self.failed.append(f"{test_name}: Expected 2 connections before morph, got {connections_before}")
                return

            # Morph And to Or (both have ports 0 and 1, so connections should survive)
            resp = await self.mcp.send_command("morph_element", {
                "element_ids": [and_id],
                "target_type": "Or"
            })

            if not resp.success:
                self.failed.append(f"{test_name}: Failed to morph And to Or")
                return

            # Verify connections still exist
            resp = await self.mcp.send_command("list_connections", {})
            connections_after = len(resp.result.get("connections", []))

            if connections_after != 2:
                self.failed.append(f"{test_name}: Expected 2 connections after morph, got {connections_after}")
                return

            print(f"✅ {test_name}")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: {str(e)}")

async def main():
    test_suite = MorphElementCommandTest()
    success = await test_suite.run_all()
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    asyncio.run(main())
