#!/usr/bin/env python3
import asyncio
import subprocess
import json
import sys

async def test_fan_in():
    proc = subprocess.Popen([
        r"C:\Users\Torres\Documents\GitHub\wiRedPanda\build\wiredpanda.exe",
        "--mcp"
    ], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    async def send_cmd(method, params):
        cmd = json.dumps({"jsonrpc": "2.0", "id": 1, "method": method, "params": params})
        print(f">>> {method}")
        proc.stdin.write(cmd + "\n")
        proc.stdin.flush()

        response = proc.stdout.readline()
        data = json.loads(response)
        if "result" in data:
            return data["result"]
        elif "error" in data:
            print(f"ERROR: {data['error']}")
            return None

    # New circuit
    await send_cmd("new_circuit", {})

    # Create sources A, B, C (3 sources for 3-input Or gate)
    sources = []
    for i in range(3):
        result = await send_cmd("create_element", {"type": "And", "x": 100+i*50, "y": 100})
        if result:
            sources.append(result["element_id"])
            print(f"Created source {i}: {sources[-1]}")

    # Create sink Z and set it to have 3 inputs
    result = await send_cmd("create_element", {"type": "Or", "x": 300, "y": 100})
    if result:
        z_id = result["element_id"]
        print(f"Created sink Z: {z_id}")

        # Change Or gate to have 3 inputs instead of 2
        result = await send_cmd("change_input_size", {"element_id": z_id, "size": 3})
        if result:
            print(f"Changed Z to have 3 inputs")

    # Connect all sources to Z
    for i, s_id in enumerate(sources):
        result = await send_cmd("connect_elements", {
            "source_id": s_id, "source_port": 0,
            "target_id": z_id, "target_port": i
        })
        if result:
            print(f"Connected {s_id} -> {z_id}.in({i})")

    # List connections before delete
    result = await send_cmd("list_connections", {})
    if result:
        conns = result.get("connections", [])
        print(f"\nBefore delete: {len(conns)} connections")
        for conn in conns:
            print(f"  {conn}")

    # Delete Z
    await send_cmd("delete_element", {"element_id": z_id})
    print(f"\nDeleted Z")

    # List connections after delete
    result = await send_cmd("list_connections", {})
    if result:
        conns = result.get("connections", [])
        print(f"After delete: {len(conns)} connections")

    # Undo
    await send_cmd("undo", {})
    print(f"\nUndone delete")

    # List connections after undo
    result = await send_cmd("list_connections", {})
    if result:
        conns = result.get("connections", [])
        print(f"After undo: {len(conns)} connections")
        for conn in conns:
            print(f"  {conn}")
        if len(conns) == 3:
            print("\n✅ SUCCESS: All 3 connections restored!")
        else:
            print(f"\n❌ FAILURE: Expected 3 connections, got {len(conns)}")

    proc.terminate()

asyncio.run(test_fan_in())
