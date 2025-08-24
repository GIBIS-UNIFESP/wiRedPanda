#!/usr/bin/env python3
"""Test MCP commands directly"""

import json
import subprocess
import sys
import threading
import time

def test_fulladder_mcp():
    """Test FullAdder creation directly through MCP"""
    
    # Start wiRedPanda MCP process
    process = subprocess.Popen(
        ["/workspace/build/wiredpanda", "--headless", "--mcp"],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    
    def read_with_timeout(stream, timeout=5):
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
    
    try:
        print("1. Creating new circuit...")
        cmd = {"command": "new_circuit", "parameters": {}}
        process.stdin.write(json.dumps(cmd) + "\n")
        process.stdin.flush()
        
        response = read_with_timeout(process.stdout, 5)
        print(f"New circuit response: {response}")
        
        if not response:
            print("❌ No response for new_circuit - something is wrong")
            return
            
        print("2. Testing And gate (known working)...")
        cmd = {"command": "create_element", "parameters": {"type": "And", "x": 100.0, "y": 100.0}}
        process.stdin.write(json.dumps(cmd) + "\n")
        process.stdin.flush()
        
        response = read_with_timeout(process.stdout, 5)
        print(f"And gate response: {response}")
        
        print("3. Testing FullAdder (suspected hanging)...")
        cmd = {"command": "create_element", "parameters": {"type": "FullAdder", "x": 200.0, "y": 100.0}}
        process.stdin.write(json.dumps(cmd) + "\n")
        process.stdin.flush()
        
        print("Waiting for FullAdder response (will timeout if hanging)...")
        response = read_with_timeout(process.stdout, 10)
        
        if response:
            print(f"FullAdder response: {response}")
        else:
            print("❌ FullAdder command timed out - this is the source of the hang!")
            
    except Exception as e:
        print(f"Test failed: {e}")
    finally:
        process.terminate()
        process.wait()

if __name__ == "__main__":
    test_fulladder_mcp()