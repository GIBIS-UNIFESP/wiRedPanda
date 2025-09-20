#!/usr/bin/env python3
"""
Debug IC Mapping - Run export with visible debug output
"""

import asyncio
import sys
import os
import subprocess
import time

# Add the MCP client to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure


async def debug_ic_mapping():
    print("🔍 Debug IC Mapping")
    print("=" * 40)

    # Start server manually to see debug output
    print("Starting MCP server manually...")
    server_process = subprocess.Popen([
        './build/wiredpanda', '--mcp'
    ], stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
       universal_newlines=True, bufsize=1)

    # Give server time to start
    time.sleep(2)

    if server_process.poll() is not None:
        print("❌ Server exited immediately")
        output, _ = server_process.communicate()
        print("Output:")
        print(output)
        return False

    mcp = MCPInfrastructure(enable_validation=False, verbose=False)
    mcp.process = server_process  # Use our manual process

    try:
        print("Loading a.panda...")
        response = await mcp.send_command("load_circuit", {
            "filename": "examples/a.panda"
        })
        if not (response and response.success):
            print("❌ Load failed")
            return False

        print("Exporting to Arduino (watch debug output)...")
        response = await mcp.send_command("export_arduino", {
            "filename": "examples/a.ino"
        }, timeout=20.0)

        if response and response.success:
            print("✅ Export completed - check debug output above")
            return True
        else:
            print("❌ Export failed")
            return False

    except Exception as e:
        print(f"❌ Exception: {e}")
        return False

    finally:
        if server_process.poll() is None:
            server_process.terminate()
            try:
                server_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                server_process.kill()
                server_process.wait()

if __name__ == "__main__":
    success = asyncio.run(debug_ic_mapping())
    sys.exit(0 if success else 1)
