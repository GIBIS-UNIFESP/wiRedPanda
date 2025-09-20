#!/usr/bin/env python3
"""
Capture debug output from MCP server during Arduino export
"""

import asyncio
import sys
import os
import subprocess
import time

# Add the MCP client to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure


async def capture_debug():
    print("🔍 Capturing debug output...")

    # Start server with output to file
    with open("debug_output.txt", "w") as debug_file:
        server_process = subprocess.Popen([
            './build/wiredpanda', '--mcp'
        ], stdout=debug_file, stderr=subprocess.STDOUT,
           universal_newlines=True, bufsize=1)

        # Give server time to start
        time.sleep(2)

        if server_process.poll() is not None:
            print("❌ Server exited immediately")
            return False

        mcp = MCPInfrastructure(enable_validation=False, verbose=False)
        mcp.process = server_process

        try:
            # Load and export
            await mcp.send_command("load_circuit", {"filename": "examples/a.panda"})
            await mcp.send_command("export_arduino", {"filename": "examples/a.ino"})

            print("✅ Export completed - debug saved to debug_output.txt")
            return True

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
    success = asyncio.run(capture_debug())

    # Show relevant debug lines
    if os.path.exists("debug_output.txt"):
        print("\n🔍 Relevant debug output:")
        print("=" * 50)
        with open("debug_output.txt", "r") as f:
            for line_num, line in enumerate(f, 1):
                if any(keyword in line for keyword in [
                    "IC output", "connected to port", "aux_node",
                    "MAPPING ISSUE", "internal value", "Found aux_node"
                ]):
                    print(f"{line_num}: {line.strip()}")

    sys.exit(0 if success else 1)