#!/usr/bin/env python3

import os
import sys
import subprocess
import glob
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
    print("wiRedPanda: Exporting all Arduino test circuits to Verilog")
    print("=" * 60)

    # Check if wiRedPanda binary exists
    if not os.path.exists("./build/wiredpanda"):
        print("❌ Error: wiRedPanda binary not found at ./build/wiredpanda")
        print("Please build wiRedPanda first with: cmake --build --preset release --target wiredpanda")
        sys.exit(1)

    # Create output directory
    output_dir = "test/verilog"
    os.makedirs(output_dir, exist_ok=True)

    # Find all .panda files in test/arduino
    panda_files = sorted(glob.glob("test/arduino/*.panda"))

    if not panda_files:
        print("❌ No .panda files found in test/arduino/")
        sys.exit(1)

    print(f"Found {len(panda_files)} circuit files to export:")
    for f in panda_files:
        print(f"  - {Path(f).stem}")
    print()

    # Export each file
    success_count = 0
    failure_count = 0

    for panda_file in panda_files:
        if export_panda_to_verilog(panda_file, output_dir):
            success_count += 1
        else:
            failure_count += 1
        print()

    # Summary
    print("=" * 60)
    print(f"Export completed:")
    print(f"  ✅ Successful: {success_count}")
    print(f"  ❌ Failed:     {failure_count}")
    print(f"  📁 Output:     {output_dir}/")

    if failure_count == 0:
        print("\n🎉 All exports successful! Verilog baseline test suite created.")
        sys.exit(0)
    else:
        print(f"\n⚠️  {failure_count} exports failed. Check the output above for details.")
        sys.exit(1)

if __name__ == "__main__":
    main()