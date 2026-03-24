#!/usr/bin/env python3
"""Migrate all .panda files to the current format via MCP load/save commands.

Usage:
    python3 Scripts/migrate_panda_files.py [path/to/wiredpanda]

Starts the app in headless MCP mode, loads each .panda file, saves it back
(which writes it in the current format), and closes the tab.
"""

import asyncio
import json
import os
import sys
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parent.parent
BINARY = sys.argv[1] if len(sys.argv) > 1 else str(PROJECT_ROOT / "build" / "wiredpanda")

# Directories containing .panda files to migrate
PANDA_DIRS = [
    PROJECT_ROOT / "Examples",
    PROJECT_ROOT / "Tests" / "Fixtures",
    PROJECT_ROOT / "Tests" / "Integration" / "IC" / "Components",
]


EXAMPLES_DIR = PROJECT_ROOT / "Examples"
COMPAT_DIR = PROJECT_ROOT / "Tests" / "BackwardCompatibility"


def read_panda_version(filepath):
    """Read the version from a .panda file header (QDataStream format)."""
    import struct
    with open(filepath, "rb") as fh:
        data = fh.read(50)
    # Magic (4 bytes), then QVersionNumber: segment count (int32) + segments (int32 each)
    count = struct.unpack(">i", data[4:8])[0]
    segments = []
    off = 8
    for _ in range(count):
        seg = struct.unpack(">i", data[off : off + 4])[0]
        segments.append(seg)
        off += 4
    return ".".join(str(s) for s in segments)


def snapshot_examples():
    """Copy Examples/*.panda to Tests/BackwardCompatibility/vX.Y.Z/ based on current file version."""
    examples = sorted(EXAMPLES_DIR.glob("*.panda"))
    if not examples:
        print("No example files found to snapshot.")
        return

    version = read_panda_version(examples[0])
    dest = COMPAT_DIR / f"v{version}.0"

    if dest.exists():
        print(f"Backward compatibility snapshot {dest.name}/ already exists, skipping.")
        return

    import shutil
    dest.mkdir(parents=True)
    for f in examples:
        shutil.copy2(f, dest / f.name)
    print(f"Snapshot {len(examples)} example files to {dest.name}/")


def collect_panda_files():
    files = []
    for d in PANDA_DIRS:
        if d.is_dir():
            files.extend(sorted(d.glob("*.panda")))
    return files


def remove_versioned_backups():
    """Remove .v*.panda backup files created by auto-migration."""
    import re
    pattern = re.compile(r"\.v\d+\.\d+\.panda$")
    removed = 0
    for d in PANDA_DIRS:
        if not d.is_dir():
            continue
        for f in d.iterdir():
            if pattern.search(f.name):
                f.unlink()
                removed += 1
    if removed:
        print(f"Removed {removed} versioned backup file(s).")


async def send_command(proc, command, params, request_id):
    request = {"jsonrpc": "2.0", "method": command, "params": params, "id": request_id}
    line = json.dumps(request) + "\n"
    proc.stdin.write(line.encode("utf-8"))
    await proc.stdin.drain()

    raw = await asyncio.wait_for(proc.stdout.readline(), timeout=30.0)
    return json.loads(raw.decode("utf-8"))


async def main():
    snapshot_examples()

    files = collect_panda_files()
    if not files:
        print("No .panda files found to migrate.")
        return

    print(f"Starting MCP server: {BINARY} --mcp")
    env = os.environ.copy()
    env["QT_QPA_PLATFORM"] = "offscreen"
    proc = await asyncio.create_subprocess_exec(
        BINARY, "--mcp",
        stdin=asyncio.subprocess.PIPE,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE,
        env=env,
    )

    req_id = 1
    migrated = 0
    errors = []

    try:
        for filepath in files:
            name = filepath.name
            abs_path = str(filepath.resolve())

            # Load
            resp = await send_command(proc, "load_circuit", {"filename": abs_path}, req_id)
            req_id += 1
            if "error" in resp:
                errors.append(f"  LOAD FAIL {name}: {resp['error'].get('message', '?')}")
                continue

            # Save back to the same path
            resp = await send_command(proc, "save_circuit", {"filename": abs_path}, req_id)
            req_id += 1
            if "error" in resp:
                errors.append(f"  SAVE FAIL {name}: {resp['error'].get('message', '?')}")
                continue

            # Close tab
            resp = await send_command(proc, "close_circuit", {}, req_id)
            req_id += 1

            migrated += 1
            print(f"  OK  {name}")

    finally:
        proc.stdin.close()
        try:
            await asyncio.wait_for(proc.wait(), timeout=5.0)
        except asyncio.TimeoutError:
            proc.kill()

    remove_versioned_backups()

    print(f"\nMigrated {migrated}/{len(files)} files.")
    if errors:
        print("Errors:")
        for e in errors:
            print(e)
        sys.exit(1)


if __name__ == "__main__":
    asyncio.run(main())
