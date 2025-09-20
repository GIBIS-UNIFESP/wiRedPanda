#!/usr/bin/env python3
"""
Export a.panda to Arduino - Automation Script
Automates the workflow: load a.panda and export to examples/a.ino
"""

import asyncio
import sys
import os

# Add the MCP client to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure


async def export_a_panda():
    print("🔬 Export a.panda to Arduino")
    print("=" * 40)

    mcp = MCPInfrastructure(enable_validation=False, verbose=False)

    try:
        print("Starting MCP...")
        if not await mcp.start_mcp():
            print("❌ Failed to start MCP")
            return False

        print("Loading examples/a.panda...")
        response = await mcp.send_command("load_circuit", {
            "filename": "examples/a.panda"
        })
        if not (response and response.success):
            print(f"❌ Load failed: {response.error if response else 'No response'}")
            return False

        print("Exporting to examples/a.ino...")
        response = await mcp.send_command("export_arduino", {
            "filename": "examples/a.ino"
        })

        if response and response.success:
            print("✅ Arduino export succeeded!")

            # Verify file was created
            if os.path.exists("examples/a.ino"):
                with open("examples/a.ino", "r") as f:
                    lines = f.readlines()
                    print(f"✅ Generated {len(lines)} lines of Arduino code")
                return True
            else:
                print("❌ Arduino file not found at examples/a.ino")
                return False

        else:
            print(f"❌ Arduino export failed: {response.error if response else 'No response'}")
            return False

    except Exception as e:
        print(f"❌ Exception: {e}")
        return False

    finally:
        await mcp.stop_mcp()

if __name__ == "__main__":
    success = asyncio.run(export_a_panda())
    print(f"\n{'✅ SUCCESS' if success else '❌ FAILED'}")
    sys.exit(0 if success else 1)