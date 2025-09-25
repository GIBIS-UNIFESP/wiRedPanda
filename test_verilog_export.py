#!/usr/bin/env python3

import json
import socket
import time
import os
import tempfile

def send_mcp_command(command, params=None):
    """Send MCP command to wiRedPanda server and return response."""

    # Create MCP request
    request = {
        "jsonrpc": "2.0",
        "method": command,
        "id": 1
    }
    if params:
        request["params"] = params

    # Convert to JSON
    message = json.dumps(request) + "\n"

    try:
        # Connect to MCP server (assuming it's running on localhost:8080)
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5.0)
        sock.connect(('localhost', 8080))

        # Send request
        sock.sendall(message.encode('utf-8'))

        # Receive response
        response_data = b""
        while True:
            chunk = sock.recv(4096)
            if not chunk:
                break
            response_data += chunk
            if b'\n' in chunk:  # Assuming responses end with newline
                break

        sock.close()

        # Parse response
        response_text = response_data.decode('utf-8').strip()
        return json.loads(response_text)

    except Exception as e:
        print(f"Error connecting to MCP server: {e}")
        return None

def test_verilog_export():
    """Test Verilog export functionality via MCP."""

    print("Testing Verilog export via MCP...")

    # Create a simple test circuit first
    print("1. Creating a new circuit...")
    response = send_mcp_command("new_circuit")
    if not response or "error" in response:
        print(f"Failed to create new circuit: {response}")
        return False

    print("2. Adding elements to the circuit...")

    # Add an input button
    response = send_mcp_command("create_element", {
        "type": "InputButton",
        "x": 100,
        "y": 100,
        "label": "Input1"
    })
    if not response or "error" in response:
        print(f"Failed to create input button: {response}")
        return False
    input_id = response["result"]["element_id"]

    # Add an AND gate
    response = send_mcp_command("create_element", {
        "type": "And",
        "x": 200,
        "y": 100,
        "label": "AndGate1"
    })
    if not response or "error" in response:
        print(f"Failed to create AND gate: {response}")
        return False
    and_id = response["result"]["element_id"]

    # Add an LED output
    response = send_mcp_command("create_element", {
        "type": "Led",
        "x": 300,
        "y": 100,
        "label": "Output1"
    })
    if not response or "error" in response:
        print(f"Failed to create LED: {response}")
        return False
    led_id = response["result"]["element_id"]

    # Connect elements
    print("3. Connecting elements...")

    # Connect input to AND gate
    response = send_mcp_command("connect_elements", {
        "source_id": input_id,
        "source_port": 0,
        "target_id": and_id,
        "target_port": 0
    })
    if not response or "error" in response:
        print(f"Failed to connect input to AND gate: {response}")
        return False

    # Connect AND gate to LED
    response = send_mcp_command("connect_elements", {
        "source_id": and_id,
        "source_port": 0,
        "target_id": led_id,
        "target_port": 0
    })
    if not response or "error" in response:
        print(f"Failed to connect AND gate to LED: {response}")
        return False

    print("4. Exporting circuit to Verilog...")

    # Create temporary file
    with tempfile.NamedTemporaryFile(mode='w', suffix='.v', delete=False) as f:
        verilog_file = f.name

    try:
        # Export to Verilog
        response = send_mcp_command("export_verilog", {
            "filename": verilog_file
        })

        if not response:
            print("No response received for Verilog export")
            return False

        if "error" in response:
            print(f"Verilog export failed: {response['error']}")
            return False

        print(f"5. Verilog export successful!")
        print(f"Response: {response}")

        # Check if file was created and has content
        if os.path.exists(verilog_file):
            with open(verilog_file, 'r') as f:
                content = f.read()
            print(f"Verilog file created with {len(content)} characters")
            print("First few lines of the generated Verilog:")
            print("=" * 50)
            for line in content.split('\n')[:10]:
                print(line)
            print("=" * 50)
            return True
        else:
            print("Verilog file was not created")
            return False

    finally:
        # Cleanup
        if os.path.exists(verilog_file):
            os.unlink(verilog_file)

if __name__ == "__main__":
    print("wiRedPanda Verilog Export MCP Test")
    print("=" * 40)

    print("Note: This test requires wiRedPanda MCP server to be running on localhost:8080")
    print("Start the server with: ./wiredpanda --mcp-server --port 8080")
    print()

    success = test_verilog_export()

    if success:
        print("\n✅ Verilog export test PASSED!")
    else:
        print("\n❌ Verilog export test FAILED!")

    exit(0 if success else 1)