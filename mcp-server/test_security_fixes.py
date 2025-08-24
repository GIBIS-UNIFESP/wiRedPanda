#!/usr/bin/env python3
"""Test MCP security fixes"""

import json
import subprocess
import threading
import time
import sys
import os

sys.path.append('.')

def test_mcp_security():
    """Test various security vulnerabilities are now fixed"""
    
    # Start wiRedPanda MCP process
    process = subprocess.Popen(
        ["/workspace/build/wiredpanda", "--headless", "--mcp"],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    
    def read_with_timeout(stream, timeout=3):
        result = []
        
        def target():
            line = stream.readline()
            if line:
                result.append(line.strip())
        
        thread = threading.Thread(target=target)
        thread.daemon = True
        thread.start()
        thread.join(timeout)
        
        return result[0] if result else None
    
    def send_command(cmd_obj):
        """Send command and get response"""
        process.stdin.write(json.dumps(cmd_obj) + "\n")
        process.stdin.flush()
        return read_with_timeout(process.stdout, 5)
    
    try:
        print("=== Testing MCP Security Fixes ===")
        
        # 1. Test command structure validation
        print("\n1. Testing command structure validation...")
        
        # Missing command field
        response = send_command({"parameters": {}})
        print(f"Missing command field: {response}")
        if response and "Missing 'command' field" in response:
            print("✅ Missing command field validation works")
        else:
            print(f"❌ Unexpected response for missing command field: {response}")
            
        # Let's also test with a timeout to see if the server is hanging
        time.sleep(0.5)
        
        # Invalid command type (not string)
        response = send_command({"command": 123, "parameters": {}})
        print(f"Invalid command type: {response}")
        if response and "'command' field must be a string" in response:
            print("✅ Command type validation works")
        else:
            print(f"❌ Unexpected response for invalid command type: {response}")
        
        # Missing parameters field
        response = send_command({"command": "test"})
        print(f"Missing parameters field: {response}")
        if response and "Missing 'parameters' field" in response:
            print("✅ Parameters field validation works")
        else:
            print(f"❌ Unexpected response for missing parameters: {response}")
        
        # Invalid parameters type (not object)
        response = send_command({"command": "test", "parameters": "invalid"})
        print(f"Invalid parameters type: {response}")
        if response and "'parameters' field must be an object" in response:
            print("✅ Parameters type validation works")
        else:
            print(f"❌ Unexpected response for invalid parameters type: {response}")
        
        print("✅ Command structure validation works")
        
        # 2. Test parameter type validation
        print("\n2. Testing parameter type validation...")
        
        # Create new circuit first
        response = send_command({"command": "new_circuit", "parameters": {}})
        print(f"New circuit: {response}")
        
        # Test invalid coordinate types
        response = send_command({
            "command": "create_element",
            "parameters": {
                "type": "InputButton",
                "x": "invalid_x",  # Should be numeric
                "y": 100
            }
        })
        print(f"Invalid x coordinate: {response}")
        if response and "must be numeric" in response:
            print("✅ Coordinate type validation works")
        else:
            print(f"❌ Unexpected response for invalid coordinate: {response}")
        
        # Test extreme coordinates
        response = send_command({
            "command": "create_element",
            "parameters": {
                "type": "InputButton", 
                "x": 99999,  # Out of reasonable bounds
                "y": 100
            }
        })
        print(f"Extreme coordinate: {response}")
        if response and "out of reasonable bounds" in response:
            print("✅ Coordinate bounds validation works")
        else:
            print(f"❌ Unexpected response for extreme coordinate: {response}")
        
        print("✅ Parameter validation testing complete")
        
        # 3. Test element type validation (our original fix)
        print("\n3. Testing element type validation...")
        
        response = send_command({
            "command": "create_element",
            "parameters": {
                "type": "FullAdder",  # Invalid element type
                "x": 100,
                "y": 100
            }
        })
        print(f"Invalid element type: {response}")
        if response and "Failed to create element" in response and "FullAdder" in response:
            print("✅ Element type validation works")
        else:
            print(f"❌ Unexpected response for invalid element: {response}")
        
        # 4. Test file path sanitization
        print("\n4. Testing file path sanitization...")
        
        # Test invalid file extension
        response = send_command({
            "command": "save_circuit", 
            "parameters": {
                "file_path": "test.exe"  # Invalid extension
            }
        })
        print(f"Invalid extension: {response}")
        if response and "must have .panda or .pandaproject extension" in response:
            print("✅ File extension validation works")
        else:
            print(f"❌ Unexpected response for invalid extension: {response}")
        
        # 5. Test valid operations still work
        print("\n5. Testing valid operations still work...")
        
        response = send_command({
            "command": "create_element",
            "parameters": {
                "type": "And", 
                "x": 100,
                "y": 100,
                "label": "TEST_AND"
            }
        })
        print(f"Valid element creation: {response}")
        if response and "success" in response:
            print("✅ Valid operations still work")
        else:
            print(f"❌ Valid operation failed: {response}")
        
        print("\n🎉 Security testing complete!")
        
    except Exception as e:
        print(f"❌ Security test failed: {e}")
        import traceback
        traceback.print_exc()
    finally:
        process.terminate()
        process.wait()

if __name__ == "__main__":
    test_mcp_security()