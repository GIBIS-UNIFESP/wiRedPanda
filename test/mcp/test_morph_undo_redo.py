#!/usr/bin/env python3

"""
Test: Morph Undo/Redo with Connection Deletion

Steps:
1. Create Display14 and Button
2. Connect button to port 13 (highest port of Display14)
3. Morph Display14 to Display7 (connection should be deleted)
4. UNDO: Display7 should morph back to Display14, connection should be restored
5. UNDO again: Should not crash
"""

import asyncio
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure

class MorphUndoRedoTest:
    def __init__(self):
        self.mcp = MCPInfrastructure(enable_validation=False, verbose=False)
        self.test_count = 0
        self.passed = 0
        self.failed = []

    async def run_all(self):
        print("\n" + "="*70)
        print("🧪 MORPH UNDO/REDO TEST - Connection Restoration")
        print("="*70)

        if not await self.mcp.start_mcp():
            print("❌ MCP startup failed")
            return False

        try:
            await self.test_morph_undo_redo_connection_restoration()

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

    async def test_morph_undo_redo_connection_restoration(self):
        """Test that undo/redo correctly restores deleted connections during morph"""
        self.test_count += 1
        test_name = "Morph Undo/Redo: Connection Restoration"

        try:
            print(f"\n📋 {test_name}")
            
            # Step 1: Create new circuit
            await self.mcp.send_command("new_circuit", {})
            print("  ✓ Created new circuit")

            # Step 2: Create Display14
            resp = await self.mcp.send_command("create_element", {
                "type": "Display14",
                "x": 100,
                "y": 100
            })
            if not resp.success:
                self.failed.append(f"{test_name}: Failed to create Display14")
                return
            display14_id = resp.result["element_id"]
            print(f"  ✓ Created Display14 (id={display14_id})")

            # Step 3: Create Button
            resp = await self.mcp.send_command("create_element", {
                "type": "InputButton",
                "x": 0,
                "y": 100
            })
            if not resp.success:
                self.failed.append(f"{test_name}: Failed to create InputButton")
                return
            button_id = resp.result["element_id"]
            print(f"  ✓ Created InputButton (id={button_id})")

            # Step 4: Connect button to port 13 of Display14
            resp = await self.mcp.send_command("connect_elements", {
                "source_id": button_id,
                "source_port": 0,
                "target_id": display14_id,
                "target_port": 13
            })
            if not resp.success:
                self.failed.append(f"{test_name}: Failed to connect button to Display14.in(13)")
                return
            connection_id = resp.result.get("connection_id")
            print(f"  ✓ Connected button to Display14.in(13) (id={connection_id})")

            # Verify connection exists
            resp = await self.mcp.send_command("list_connections", {})
            connections_before = len(resp.result.get("connections", []))
            print(f"  ✓ Connections before morph: {connections_before}")
            if connections_before != 1:
                self.failed.append(f"{test_name}: Expected 1 connection before morph, got {connections_before}")
                return

            # Step 5: Morph Display14 to Display7
            resp = await self.mcp.send_command("morph_element", {
                "element_ids": [display14_id],
                "target_type": "Display7"
            })
            if not resp.success:
                self.failed.append(f"{test_name}: Failed to morph Display14 to Display7")
                return
            morphed_display7_id = resp.result.get("morphed_elements", [])[0]
            print(f"  ✓ Morphed Display14 to Display7 (new id={morphed_display7_id})")

            # Verify connection deleted
            resp = await self.mcp.send_command("list_connections", {})
            connections_after_morph = len(resp.result.get("connections", []))
            print(f"  ✓ Connections after morph: {connections_after_morph} (deleted as expected)")
            if connections_after_morph != 0:
                self.failed.append(f"{test_name}: Expected 0 connections after morph, got {connections_after_morph}")
                return

            # Step 6: UNDO - morph back to Display14, connection should restore
            resp = await self.mcp.send_command("undo", {})
            if not resp.success:
                self.failed.append(f"{test_name}: First undo failed")
                return
            print(f"  ✓ First undo executed (morph reverted)")

            # Verify Display14 is restored
            try:
                resp = await self.mcp.send_command("list_elements", {})
            except Exception as e:
                self.failed.append(f"{test_name}: Exception calling list_elements after undo: {str(e)}")
                import traceback
                traceback.print_exc()
                return

            if not resp.success:
                self.failed.append(f"{test_name}: list_elements failed: {resp.error if hasattr(resp, 'error') else 'unknown'}")
                return
            elements = resp.result.get("elements", [])
            display14_restored = False
            for elem in elements:
                if elem.get("type") == "Display14":
                    display14_restored = True
                    print(f"  ✓ Display14 restored (id={elem.get('element_id')})")
                    break
            if not display14_restored:
                self.failed.append(f"{test_name}: Display14 not restored after undo")
                return

            # CRITICAL: Check if connection is restored
            resp = await self.mcp.send_command("list_connections", {})
            connections_after_undo = len(resp.result.get("connections", []))
            print(f"  ✓ Connections after undo: {connections_after_undo}")
            
            if connections_after_undo != 1:
                self.failed.append(
                    f"{test_name}: Expected 1 connection after undo (should be restored), "
                    f"got {connections_after_undo}. BUG: Connection not restored!"
                )
                return
            print(f"  ✓✓ CONNECTION RESTORED SUCCESSFULLY!")

            # Step 7: UNDO again - should not crash
            try:
                resp = await self.mcp.send_command("undo", {})
                if not resp.success:
                    # This might fail if undo stack is empty, which is OK
                    print(f"  ✓ Second undo returned success=false (expected if undo stack exhausted)")
                else:
                    print(f"  ✓ Second undo executed without crashing")
            except Exception as e:
                self.failed.append(f"{test_name}: Second undo crashed with: {str(e)}")
                return

            print(f"\n✅ {test_name} PASSED")
            self.passed += 1

        except Exception as e:
            self.failed.append(f"{test_name}: Exception: {str(e)}")

async def main():
    test_suite = MorphUndoRedoTest()
    success = await test_suite.run_all()

    sys.exit(0 if success else 1)

if __name__ == "__main__":
    asyncio.run(main())
