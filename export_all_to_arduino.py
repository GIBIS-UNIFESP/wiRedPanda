#!/usr/bin/env python3
"""
Export All Examples to Arduino - Automation Script
Re-exports all .panda files in examples/ with Phase 1/2/3 optimizations
"""

import asyncio
import sys
import os
import glob

# Add the MCP client to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'mcp', 'client'))

from mcp_infrastructure import MCPInfrastructure


async def export_all_examples():
    print("🚀 Re-export ALL Examples with Phase 1/2/3 Arduino Optimizations")
    print("=" * 60)

    # Find all .panda files in examples folder
    panda_files = glob.glob("examples/*.panda")
    panda_files.sort()  # Sort for consistent output

    print(f"Found {len(panda_files)} .panda files to export:")
    for file in panda_files:
        print(f"  - {file}")
    print()

    mcp = MCPInfrastructure(enable_validation=True, verbose=True)  # Enable verbose to see errors

    successful_exports = 0
    failed_exports = 0
    export_results = []

    try:
        print("Starting MCP...")
        if not await mcp.start_mcp():
            print("❌ Failed to start MCP")
            return False

        for panda_file in panda_files:
            filename_base = os.path.splitext(os.path.basename(panda_file))[0]
            ino_file = f"examples/{filename_base}.ino"

            print(f"📁 Processing {panda_file} → {ino_file}")

            # Load the .panda file
            response = await mcp.send_command("load_circuit", {
                "filename": panda_file
            })
            if not (response and response.success):
                print(f"  ❌ Load failed: {response.error if response else 'No response'}")
                failed_exports += 1
                export_results.append((panda_file, False, f"Load failed: {response.error if response else 'No response'}"))
                continue

            # Export to .ino
            response = await mcp.send_command("export_arduino", {
                "filename": ino_file
            })

            if response and response.success:
                # Verify file was created and check size
                if os.path.exists(ino_file):
                    with open(ino_file, "r") as f:
                        lines = f.readlines()
                        file_size = os.path.getsize(ino_file)
                    print(f"  ✅ Success! Generated {len(lines)} lines ({file_size} bytes)")
                    successful_exports += 1
                    export_results.append((panda_file, True, f"{len(lines)} lines, {file_size} bytes"))
                else:
                    print(f"  ❌ File not created at {ino_file}")
                    failed_exports += 1
                    export_results.append((panda_file, False, "File not created"))
            else:
                print(f"  ❌ Export failed: {response.error if response else 'No response'}")
                failed_exports += 1
                export_results.append((panda_file, False, f"Export failed: {response.error if response else 'No response'}"))

    except Exception as e:
        print(f"❌ Exception: {e}")
        return False

    finally:
        await mcp.stop_mcp()

    # Summary report
    print("\n" + "=" * 60)
    print("📊 EXPORT SUMMARY REPORT")
    print("=" * 60)
    print(f"Total files processed: {len(panda_files)}")
    print(f"✅ Successful exports: {successful_exports}")
    print(f"❌ Failed exports: {failed_exports}")
    if len(panda_files) > 0:
        print(f"Success rate: {(successful_exports/len(panda_files)*100):.1f}%")
    else:
        print("Success rate: 0.0% (no files found)")

    print("\n📋 Detailed Results:")
    for panda_file, success, details in export_results:
        status = "✅" if success else "❌"
        filename = os.path.basename(panda_file)
        print(f"  {status} {filename:<25} - {details}")

    if successful_exports > 0:
        print(f"\n🎯 SUCCESS: {successful_exports} Arduino files updated with Phase 1/2/3 features:")
        print("  - Phase 1: LED PWM, Audio tone(), InputRotary encoder")
        print("  - Phase 2: SevSeg/SevSegShift display optimization (62% pin reduction)")
        print("  - Phase 3: Mux/Demux support + intelligent library selection")

    return successful_exports > 0

if __name__ == "__main__":
    success = asyncio.run(export_all_examples())
    print(f"\n{'🚀 MISSION ACCOMPLISHED' if success else '❌ MISSION FAILED'}")
    sys.exit(0 if success else 1)
