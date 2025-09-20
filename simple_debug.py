#!/usr/bin/env python3
import subprocess
import time
import asyncio
import sys
import os

# Add the MCP client to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'mcp', 'client'))
from mcp_infrastructure import MCPInfrastructure

async def simple_debug():
    # Start server with output redirection
    with open("simple_debug.log", "w") as log_file:
        server = subprocess.Popen([
            './build/wiredpanda', '--mcp'
        ], stdout=log_file, stderr=subprocess.STDOUT, bufsize=1)

        time.sleep(1)

        mcp = MCPInfrastructure(enable_validation=False, verbose=False)
        mcp.process = server

        try:
            await mcp.send_command("load_circuit", {"filename": "examples/a.panda"})
            await mcp.send_command("export_arduino", {"filename": "examples/a.ino"})
        except:
            pass
        finally:
            server.terminate()
            server.wait()

asyncio.run(simple_debug())

# Show debug lines
if os.path.exists("simple_debug.log"):
    with open("simple_debug.log") as f:
        lines = f.readlines()
        for i, line in enumerate(lines):
            if any(x in line for x in ["IC output", "mapping", "Direct", "aux_node", "MAPPING"]):
                print(f"{i}: {line.strip()}")
