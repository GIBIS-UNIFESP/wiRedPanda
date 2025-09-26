#!/usr/bin/env python3

import os
import sys
import subprocess
from pathlib import Path

def export_panda_to_verilog(panda_file, output_dir):
    """Export a .panda file to Verilog using wiRedPanda."""

    # Get base filename without extension
    base_name = Path(panda_file).stem
    verilog_file = os.path.join(output_dir, f"{base_name}.v")

    # Build the wiRedPanda command
    cmd = [
        "./build/wiredpanda",
        "--verilog-file", verilog_file,
        panda_file
    ]

    print(f"Exporting {base_name}...")
    print(f"  Input:  {panda_file}")
    print(f"  Output: {verilog_file}")

    try:
        # Run wiRedPanda export command
        result = subprocess.run(cmd,
                              capture_output=True,
                              text=True,
                              timeout=30)

        if result.returncode == 0:
            # Check if output file was created
            if os.path.exists(verilog_file):
                file_size = os.path.getsize(verilog_file)
                print(f"  ✅ Success: {file_size} bytes")
                return True
            else:
                print(f"  ❌ Failed: No output file created")
                if result.stdout:
                    print(f"     STDOUT: {result.stdout.strip()}")
                if result.stderr:
                    print(f"     STDERR: {result.stderr.strip()}")
                return False
        else:
            print(f"  ❌ Failed: Exit code {result.returncode}")
            if result.stdout:
                print(f"     STDOUT: {result.stdout.strip()}")
            if result.stderr:
                print(f"     STDERR: {result.stderr.strip()}")
            return False

    except subprocess.TimeoutExpired:
        print(f"  ❌ Failed: Timeout after 30 seconds")
        return False
    except Exception as e:
        print(f"  ❌ Failed: Exception {e}")
        return False

def main():
    print("wiRedPanda: Exporting sequential.panda to Verilog")
    print("=" * 50)

    # Check if wiRedPanda binary exists
    if not os.path.exists("./build/wiredpanda"):
        print("❌ Error: wiRedPanda binary not found at ./build/wiredpanda")
        print("Please build wiRedPanda first with: cmake --build --preset release --target wiredpanda")
        sys.exit(1)

    # Create output directory
    output_dir = "test/verilog"
    os.makedirs(output_dir, exist_ok=True)

    # Export sequential.panda specifically
    panda_file = "test/arduino/sequential.panda"

    if not os.path.exists(panda_file):
        print(f"❌ Error: {panda_file} not found")
        sys.exit(1)

    print(f"Exporting {panda_file}...")
    print()

    if export_panda_to_verilog(panda_file, output_dir):
        print("=" * 50)
        print("🎉 Export successful!")
        sys.exit(0)
    else:
        print("=" * 50)
        print("⚠️ Export failed. Check the output above for details.")
        sys.exit(1)

if __name__ == "__main__":
    main()