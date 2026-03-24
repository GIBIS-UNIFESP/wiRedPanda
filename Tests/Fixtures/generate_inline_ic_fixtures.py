#!/usr/bin/env python3
"""
Generate .panda test fixtures for embedded IC (inline) tests.

Usage:
    cd /path/to/wiredpanda
    python3 Tests/Fixtures/generate_inline_ic_fixtures.py

Requires a built wiredpanda executable in build/.
Creates:
    Tests/Fixtures/simple_and.panda        — 2 InputSwitches → AND → LED
    Tests/Fixtures/nested_and.panda        — IC(simple_and) + 2 InputSwitches → LED
    Tests/Fixtures/parent_inline_ic.panda  — embedded IC(simple_and) + wiring
    Tests/Fixtures/chain_c.panda           — NOT gate (leaf in A→B→C chain)
    Tests/Fixtures/chain_b.panda           — AND → IC(chain_c) (middle layer)
    Tests/Fixtures/chain_a.panda           — IC(chain_b) (top, references B→C)
    Tests/Fixtures/circular_a.panda        — IC(circular_b) (circular reference pair)
    Tests/Fixtures/circular_b.panda        — IC(circular_a) (circular reference pair)
"""

import asyncio
import sys
from pathlib import Path

# Add MCP client to path
project_root = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(project_root / "MCP" / "Client"))

from mcp_infrastructure import MCPInfrastructure  # noqa: E402

FIXTURES_DIR = project_root / "Tests" / "Fixtures"


async def create_simple_and(mcp: MCPInfrastructure) -> bool:
    """Create simple_and.panda: 2 InputSwitches → AND → LED"""
    print("\n=== Creating simple_and.panda ===")

    resp = await mcp.send_command("new_circuit", {})
    if not resp.success:
        print(f"  FAIL new_circuit: {resp.message}")
        return False

    # Create elements on grid-snapped positions (multiples of 8)
    a = await mcp.send_command("create_element", {
        "type": "InputSwitch", "x": 96, "y": 96, "label": "A"
    })
    b = await mcp.send_command("create_element", {
        "type": "InputSwitch", "x": 96, "y": 192, "label": "B"
    })
    gate = await mcp.send_command("create_element", {
        "type": "And", "x": 256, "y": 144
    })
    led = await mcp.send_command("create_element", {
        "type": "Led", "x": 400, "y": 144, "label": "OUT"
    })

    for r in [a, b, gate, led]:
        if not r.success:
            print(f"  FAIL create_element: {r.message}")
            return False

    a_id = a.result["element_id"]
    b_id = b.result["element_id"]
    gate_id = gate.result["element_id"]
    led_id = led.result["element_id"]

    # Connect: A→AND.0, B→AND.1, AND→LED
    conns = [
        {"source_id": a_id, "source_port": 0, "target_id": gate_id, "target_port": 0},
        {"source_id": b_id, "source_port": 0, "target_id": gate_id, "target_port": 1},
        {"source_id": gate_id, "source_port": 0, "target_id": led_id, "target_port": 0},
    ]
    for c in conns:
        r = await mcp.send_command("connect_elements", c)
        if not r.success:
            print(f"  FAIL connect: {r.message}")
            return False

    # Save
    path = str(FIXTURES_DIR / "simple_and.panda")
    r = await mcp.send_command("save_circuit", {"filename": path})
    if not r.success:
        print(f"  FAIL save: {r.message}")
        return False

    print(f"  OK → {path}")
    return True


async def create_nested_and(mcp: MCPInfrastructure) -> bool:
    """Create nested_and.panda: IC(simple_and) + 2 InputSwitches → LED

    This circuit uses simple_and.panda as a file-backed IC, then wires
    2 InputSwitches through the IC to an output LED.
    """
    print("\n=== Creating nested_and.panda ===")

    # simple_and.panda must exist first
    simple_and_path = str(FIXTURES_DIR / "simple_and.panda")
    if not Path(simple_and_path).exists():
        print("  FAIL: simple_and.panda not found (must be created first)")
        return False

    # Save a temporary circuit to the fixtures dir to set currentDir() context.
    # instantiate_ic resolves IC files relative to currentDir().
    resp = await mcp.send_command("new_circuit", {})
    if not resp.success:
        print(f"  FAIL new_circuit: {resp.message}")
        return False

    nested_path = str(FIXTURES_DIR / "nested_and.panda")
    resp = await mcp.send_command("save_circuit", {"filename": nested_path})
    if not resp.success:
        print(f"  FAIL initial save: {resp.message}")
        return False

    # Create input switches
    a = await mcp.send_command("create_element", {
        "type": "InputSwitch", "x": 96, "y": 96, "label": "X"
    })
    b = await mcp.send_command("create_element", {
        "type": "InputSwitch", "x": 96, "y": 192, "label": "Y"
    })

    # Instantiate the simple_and IC
    ic = await mcp.send_command("instantiate_ic", {
        "ic_name": "simple_and", "x": 256, "y": 144
    })

    # Create LED output
    led = await mcp.send_command("create_element", {
        "type": "Led", "x": 400, "y": 144, "label": "Z"
    })

    for r in [a, b, ic, led]:
        if not r.success:
            print(f"  FAIL create: {r.message}")
            return False

    a_id = a.result["element_id"]
    b_id = b.result["element_id"]
    ic_id = ic.result["element_id"]
    led_id = led.result["element_id"]

    # Connect: X→IC.0, Y→IC.1, IC→LED
    conns = [
        {"source_id": a_id, "source_port": 0, "target_id": ic_id, "target_port": 0},
        {"source_id": b_id, "source_port": 0, "target_id": ic_id, "target_port": 1},
        {"source_id": ic_id, "source_port": 0, "target_id": led_id, "target_port": 0},
    ]
    for c in conns:
        r = await mcp.send_command("connect_elements", c)
        if not r.success:
            print(f"  FAIL connect: {r.message}")
            return False

    # Save to the fixtures dir (same dir as simple_and so IC resolves)
    path = str(FIXTURES_DIR / "nested_and.panda")
    r = await mcp.send_command("save_circuit", {"filename": path})
    if not r.success:
        print(f"  FAIL save: {r.message}")
        return False

    print(f"  OK → {path}")
    return True


async def create_parent_inline_ic(mcp: MCPInfrastructure) -> bool:
    """Create parent_inline_ic.panda: embedded IC (simple_and as inline) + wiring.

    Uses instantiate_ic with inline=true to create an embedded IC.
    """
    print("\n=== Creating parent_inline_ic.panda ===")

    simple_and_path = str(FIXTURES_DIR / "simple_and.panda")
    if not Path(simple_and_path).exists():
        print("  FAIL: simple_and.panda not found")
        return False

    # Save temporary circuit to fixtures dir to set context
    resp = await mcp.send_command("new_circuit", {})
    if not resp.success:
        print(f"  FAIL new_circuit: {resp.message}")
        return False

    parent_path = str(FIXTURES_DIR / "parent_inline_ic.panda")
    resp = await mcp.send_command("save_circuit", {"filename": parent_path})
    if not resp.success:
        print(f"  FAIL initial save: {resp.message}")
        return False

    # Create input switches
    a = await mcp.send_command("create_element", {
        "type": "InputSwitch", "x": 96, "y": 96, "label": "IN_A"
    })
    b = await mcp.send_command("create_element", {
        "type": "InputSwitch", "x": 96, "y": 192, "label": "IN_B"
    })

    # Instantiate as embedded IC
    ic = await mcp.send_command("instantiate_ic", {
        "ic_name": "simple_and", "x": 256, "y": 144, "inline": True
    })

    # Create LED output
    led = await mcp.send_command("create_element", {
        "type": "Led", "x": 400, "y": 144, "label": "RESULT"
    })

    for r in [a, b, ic, led]:
        if not r.success:
            print(f"  FAIL create: {r.message}")
            return False

    a_id = a.result["element_id"]
    b_id = b.result["element_id"]
    ic_id = ic.result["element_id"]
    led_id = led.result["element_id"]

    # Verify it was embedded
    if ic.result.get("inline"):
        print(f"  Embedded IC created: blobName={ic.result.get('blobName')}")
    else:
        print("  WARNING: IC was not created as inline")

    # Connect: IN_A→IC.0, IN_B→IC.1, IC→LED
    conns = [
        {"source_id": a_id, "source_port": 0, "target_id": ic_id, "target_port": 0},
        {"source_id": b_id, "source_port": 0, "target_id": ic_id, "target_port": 1},
        {"source_id": ic_id, "source_port": 0, "target_id": led_id, "target_port": 0},
    ]
    for c in conns:
        r = await mcp.send_command("connect_elements", c)
        if not r.success:
            print(f"  FAIL connect: {r.message}")
            return False

    # Save
    path = str(FIXTURES_DIR / "parent_inline_ic.panda")
    r = await mcp.send_command("save_circuit", {"filename": path})
    if not r.success:
        print(f"  FAIL save: {r.message}")
        return False

    print(f"  OK → {path}")
    return True


async def set_fixtures_context(mcp: MCPInfrastructure, filename: str) -> bool:
    """Save a blank circuit to the fixtures dir to set currentDir() context."""
    resp = await mcp.send_command("new_circuit", {})
    if not resp.success:
        return False
    resp = await mcp.send_command("save_circuit", {"filename": str(FIXTURES_DIR / filename)})
    return resp.success


async def create_chain_c(mcp: MCPInfrastructure) -> bool:
    """Create chain_c.panda: NOT gate (1 InputSwitch → NOT → LED)

    Leaf circuit in the A→B→C nesting chain.
    """
    print("\n=== Creating chain_c.panda ===")

    resp = await mcp.send_command("new_circuit", {})
    if not resp.success:
        print(f"  FAIL new_circuit: {resp.message}")
        return False

    inp = await mcp.send_command("create_element", {
        "type": "InputSwitch", "x": 96, "y": 144, "label": "C_IN"
    })
    gate = await mcp.send_command("create_element", {
        "type": "Not", "x": 256, "y": 144
    })
    led = await mcp.send_command("create_element", {
        "type": "Led", "x": 400, "y": 144, "label": "C_OUT"
    })

    for r in [inp, gate, led]:
        if not r.success:
            print(f"  FAIL create_element: {r.message}")
            return False

    conns = [
        {"source_id": inp.result["element_id"], "source_port": 0,
         "target_id": gate.result["element_id"], "target_port": 0},
        {"source_id": gate.result["element_id"], "source_port": 0,
         "target_id": led.result["element_id"], "target_port": 0},
    ]
    for c in conns:
        r = await mcp.send_command("connect_elements", c)
        if not r.success:
            print(f"  FAIL connect: {r.message}")
            return False

    path = str(FIXTURES_DIR / "chain_c.panda")
    r = await mcp.send_command("save_circuit", {"filename": path})
    if not r.success:
        print(f"  FAIL save: {r.message}")
        return False

    print(f"  OK → {path}")
    return True


async def create_chain_b(mcp: MCPInfrastructure) -> bool:
    """Create chain_b.panda: IC(chain_c) + AND gate

    Middle circuit: 2 InputSwitches → AND → IC(chain_c) → LED.
    The IC(chain_c) inverts the AND result.
    """
    print("\n=== Creating chain_b.panda ===")

    if not await set_fixtures_context(mcp, "chain_b.panda"):
        print("  FAIL: could not set context dir")
        return False

    inp_a = await mcp.send_command("create_element", {
        "type": "InputSwitch", "x": 96, "y": 96, "label": "B_A"
    })
    inp_b = await mcp.send_command("create_element", {
        "type": "InputSwitch", "x": 96, "y": 192, "label": "B_B"
    })
    gate = await mcp.send_command("create_element", {
        "type": "And", "x": 224, "y": 144
    })
    ic_c = await mcp.send_command("instantiate_ic", {
        "ic_name": "chain_c", "x": 368, "y": 144
    })
    led = await mcp.send_command("create_element", {
        "type": "Led", "x": 512, "y": 144, "label": "B_OUT"
    })

    for r in [inp_a, inp_b, gate, ic_c, led]:
        if not r.success:
            print(f"  FAIL create: {r.message}")
            return False

    conns = [
        {"source_id": inp_a.result["element_id"], "source_port": 0,
         "target_id": gate.result["element_id"], "target_port": 0},
        {"source_id": inp_b.result["element_id"], "source_port": 0,
         "target_id": gate.result["element_id"], "target_port": 1},
        {"source_id": gate.result["element_id"], "source_port": 0,
         "target_id": ic_c.result["element_id"], "target_port": 0},
        {"source_id": ic_c.result["element_id"], "source_port": 0,
         "target_id": led.result["element_id"], "target_port": 0},
    ]
    for c in conns:
        r = await mcp.send_command("connect_elements", c)
        if not r.success:
            print(f"  FAIL connect: {r.message}")
            return False

    path = str(FIXTURES_DIR / "chain_b.panda")
    r = await mcp.send_command("save_circuit", {"filename": path})
    if not r.success:
        print(f"  FAIL save: {r.message}")
        return False

    print(f"  OK → {path}  (file-backed IC: chain_c)")
    return True


async def create_chain_a(mcp: MCPInfrastructure) -> bool:
    """Create chain_a.panda: IC(chain_b) wired up

    Top-level circuit: 2 InputSwitches → IC(chain_b) → LED.
    chain_b itself contains IC(chain_c), so embedding chain_b should
    recursively flatten chain_c inside the blob.
    """
    print("\n=== Creating chain_a.panda ===")

    if not await set_fixtures_context(mcp, "chain_a.panda"):
        print("  FAIL: could not set context dir")
        return False

    inp_a = await mcp.send_command("create_element", {
        "type": "InputSwitch", "x": 96, "y": 96, "label": "A_X"
    })
    inp_b = await mcp.send_command("create_element", {
        "type": "InputSwitch", "x": 96, "y": 192, "label": "A_Y"
    })
    ic_b = await mcp.send_command("instantiate_ic", {
        "ic_name": "chain_b", "x": 304, "y": 144
    })
    led = await mcp.send_command("create_element", {
        "type": "Led", "x": 512, "y": 144, "label": "A_OUT"
    })

    for r in [inp_a, inp_b, ic_b, led]:
        if not r.success:
            print(f"  FAIL create: {r.message}")
            return False

    conns = [
        {"source_id": inp_a.result["element_id"], "source_port": 0,
         "target_id": ic_b.result["element_id"], "target_port": 0},
        {"source_id": inp_b.result["element_id"], "source_port": 0,
         "target_id": ic_b.result["element_id"], "target_port": 1},
        {"source_id": ic_b.result["element_id"], "source_port": 0,
         "target_id": led.result["element_id"], "target_port": 0},
    ]
    for c in conns:
        r = await mcp.send_command("connect_elements", c)
        if not r.success:
            print(f"  FAIL connect: {r.message}")
            return False

    path = str(FIXTURES_DIR / "chain_a.panda")
    r = await mcp.send_command("save_circuit", {"filename": path})
    if not r.success:
        print(f"  FAIL save: {r.message}")
        return False

    print(f"  OK → {path}  (file-backed IC: chain_b → chain_c)")
    return True


async def create_example_nested(mcp: MCPInfrastructure) -> bool:
    """Create Examples/nested.panda: truly nested embedded IC.

    Embeds chain_b (which itself contains file-backed chain_c) as an inline IC.
    flattenBlob recursively converts chain_c to inline inside chain_b's blob,
    so the result is: nested.panda → embedded "chain_b" → embedded "chain_c".

    Circuit: 2 InputSwitches → embedded NAND(chain_b) → LED
    chain_b = AND gate → IC(chain_c)
    chain_c = NOT gate
    So chain_b computes NAND.
    """
    print("\n=== Creating Examples/nested.panda ===")

    examples_dir = project_root / "Examples"
    examples_dir.mkdir(parents=True, exist_ok=True)

    # Set context to fixtures dir where chain_b/chain_c live
    if not await set_fixtures_context(mcp, "dummy_ctx.panda"):
        print("  FAIL: could not set context dir")
        return False

    # Embed chain_b as inline IC — flattenBlob will recursively embed chain_c
    ic_nand = await mcp.send_command("instantiate_ic", {
        "ic_name": "chain_b", "x": 304, "y": 144, "inline": True
    })
    if not ic_nand.success:
        print(f"  FAIL instantiate chain_b inline: {ic_nand.message}")
        return False
    print(f"  Embedded NAND: blobName={ic_nand.result.get('blobName')}")
    print(f"    (chain_b blob contains chain_c, recursively flattened)")

    # Create input switches and LED
    inp_a = await mcp.send_command("create_element", {
        "type": "InputSwitch", "x": 96, "y": 96, "label": "A"
    })
    inp_b = await mcp.send_command("create_element", {
        "type": "InputSwitch", "x": 96, "y": 192, "label": "B"
    })
    led = await mcp.send_command("create_element", {
        "type": "Led", "x": 512, "y": 144, "label": "NAND"
    })

    for r in [inp_a, inp_b, led]:
        if not r.success:
            print(f"  FAIL create: {r.message}")
            return False

    # Wire: A→chain_b.0, B→chain_b.1, chain_b→LED
    conns = [
        {"source_id": inp_a.result["element_id"], "source_port": 0,
         "target_id": ic_nand.result["element_id"], "target_port": 0},
        {"source_id": inp_b.result["element_id"], "source_port": 0,
         "target_id": ic_nand.result["element_id"], "target_port": 1},
        {"source_id": ic_nand.result["element_id"], "source_port": 0,
         "target_id": led.result["element_id"], "target_port": 0},
    ]
    for c in conns:
        r = await mcp.send_command("connect_elements", c)
        if not r.success:
            print(f"  FAIL connect: {r.message}")
            return False

    path = str(examples_dir / "nested.panda")
    r = await mcp.send_command("save_circuit", {"filename": path})
    if not r.success:
        print(f"  FAIL save: {r.message}")
        return False

    # Clean up the dummy context file
    dummy = FIXTURES_DIR / "dummy_ctx.panda"
    if dummy.exists():
        dummy.unlink()

    print(f"  OK → {path}  (2 embedded ICs: simple_and + chain_c)")
    return True


async def main() -> int:
    FIXTURES_DIR.mkdir(parents=True, exist_ok=True)

    mcp = MCPInfrastructure(enable_validation=True, verbose=False)

    try:
        print("Starting wiRedPanda MCP server...")
        if not await mcp.start_mcp():
            print("FAIL: Could not start MCP server")
            return 1

        results = []
        results.append(await create_simple_and(mcp))
        results.append(await create_nested_and(mcp))
        results.append(await create_parent_inline_ic(mcp))
        results.append(await create_chain_c(mcp))
        results.append(await create_chain_b(mcp))
        results.append(await create_chain_a(mcp))
        # circular_a/circular_b are created directly in TestICInline.cpp
        # via C++ Serialization API to avoid the loadFile circular crash.
        results.append(await create_example_nested(mcp))

        ok = all(results)
        print(f"\n{'All fixtures generated successfully!' if ok else 'Some fixtures failed.'}")
        return 0 if ok else 1

    finally:
        await mcp.stop_mcp()


if __name__ == "__main__":
    sys.exit(asyncio.run(main()))
