#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Generate the performance fixtures through the MCP server.

Shapes, with the size as a parameter (every position is on the 8 px grid MCP snaps to):
    clocked_<N>        N/20 rows, each an InputSwitch, 18 DFlipFlops chained Q -> Data and an Led, every
                       flip-flop clocked by one shared Clock: N + 1 elements, 37 N / 20 wires.
    combinational_<N>  N/20 rows, each an InputSwitch, 18 gates cycling And, Or, Xor with both inputs on the
                       previous output, and an Led: N elements, 37 N / 20 wires.
    ic_heavy           one Clock and one InputSwitch driving the two inputs of N file-backed instances of
                       perf_ic_4gate (A AND B and A OR B into an XOR, inverted), outputs open: N + 2 elements,
                       2 N wires. The instances name the IC by file, so perf_ic_4gate.panda stays beside
                       ic_heavy.panda.

Usage (from the repository root):
    python3 Tests/Fixtures/Perf/generate_perf_fixtures.py
        perf_ic_4gate, clocked_2000, combinational_1000 and ic_heavy (2,000 instances) into this directory.
    python3 Tests/Fixtures/Perf/generate_perf_fixtures.py --large
        clocked_8000 and combinational_20000 into ../wiredpanda-profiling-circuits, beside the repository,
        never committed.
    python3 Tests/Fixtures/Perf/generate_perf_fixtures.py --shape clocked --size 4000 --out DIR

Needs a built wiredpanda executable; WIREDPANDA_EXE chooses one. The output is deterministic: the same generator and
application write the same bytes, from an unoptimized build or an optimized one. Any failure ends the run with a
message and status 1.
"""

import argparse
import asyncio
import sys
from pathlib import Path
from typing import Any, NoReturn

try:
    from mcp_client.mcp_infrastructure import MCPInfrastructure
except ImportError:
    _root = Path(__file__).resolve().parents[3]
    sys.path.insert(0, str(_root / "MCP" / "Client"))
    del _root
    from mcp_client.mcp_infrastructure import MCPInfrastructure

PERF_DIR = Path(__file__).resolve().parent
LARGE_DIR = Path(__file__).resolve().parents[4] / "wiredpanda-profiling-circuits"
IC_4GATE = "perf_ic_4gate.panda"

ROW_LENGTH = 18  # flip-flops or gates per row
ROW_SIZE = ROW_LENGTH + 2  # elements per row, with its InputSwitch and Led
PITCH = 104  # between neighbouring elements and between rows: a 64 px element and a 40 px gap
GATE_CYCLE = ("And", "Or", "Xor")

IC_COLUMNS = 50
IC_PITCH_X = 128  # an IC's body is 76 px wide
IC_PITCH_Y = 112

COMMAND_TIMEOUT = 10.0  # seconds
SAVE_TIMEOUT = 600.0  # seconds; one save writes the whole circuit
# Seconds for the command after a close: closing returns at once, and the closed circuit is destroyed when the next
# command reaches the event loop (about 32 s for 8,000 elements).
TEARDOWN_TIMEOUT = 600.0


class FixtureBuilder:
    """Builds one circuit at a time through MCP commands, counting what it creates."""

    def __init__(self, mcp: MCPInfrastructure) -> None:
        self.mcp = mcp
        self.elements = 0
        self.wires = 0

    async def command(self, name: str, params: dict[str, Any], timeout: float = COMMAND_TIMEOUT) -> dict[str, Any]:
        """Send one MCP command and return its result; a failure ends the generation."""
        response = await self.mcp.send_command(name, params, timeout)
        if not response.success:
            raise RuntimeError(f"{name} {params} failed: {response.message}")
        return response.result or {}

    async def new_circuit(self) -> None:
        """Open an empty circuit in a new tab, once any circuit closed before it is destroyed."""
        await self.command("new_circuit", {}, TEARDOWN_TIMEOUT)
        self.elements = 0
        self.wires = 0

    async def element(self, kind: str, x: int, y: int, label: str) -> int:
        """Create an element and return its id."""
        result = await self.command("create_element", {"type": kind, "x": x, "y": y, "label": label})
        self.elements += 1
        return int(result["element_id"])

    async def ic(self, ic_file: Path, x: int, y: int, label: str) -> int:
        """Place a file-backed instance of the IC in ic_file, an absolute path, and return its id."""
        params = {"ic_name": str(ic_file.with_suffix("")), "x": x, "y": y, "label": label}
        result = await self.command("instantiate_ic", params)
        self.elements += 1
        return int(result["element_id"])

    async def wire(self, source: int, target: int, target_port: int = 0) -> None:
        """Connect the first output of source to input target_port of target."""
        params = {"source_id": source, "source_port": 0, "target_id": target, "target_port": target_port}
        await self.command("connect_elements", params)
        self.wires += 1

    async def save_and_close(self, path: Path) -> None:
        """Save the circuit to path, report what it holds and close its tab."""
        await self.command("save_circuit", {"filename": str(path)}, SAVE_TIMEOUT)
        print(f"  {path}: {self.elements} elements, {self.wires} wires")
        await self.command("close_circuit", {})


async def build_ic_4gate(builder: FixtureBuilder, out_dir: Path) -> None:
    """perf_ic_4gate: inputs A and B, output Out = NOT((A AND B) XOR (A OR B))."""
    await builder.new_circuit()
    a = await builder.element("InputSwitch", 48, 48, "A")
    b = await builder.element("InputSwitch", 48, 152, "B")
    g1 = await builder.element("And", 200, 48, "g1")
    g2 = await builder.element("Or", 200, 152, "g2")
    g3 = await builder.element("Xor", 352, 104, "g3")
    g4 = await builder.element("Not", 504, 104, "g4")
    out = await builder.element("Led", 656, 104, "Out")
    await builder.wire(a, g1, 0)
    await builder.wire(a, g2, 0)
    await builder.wire(b, g1, 1)
    await builder.wire(b, g2, 1)
    await builder.wire(g1, g3, 0)
    await builder.wire(g2, g3, 1)
    await builder.wire(g3, g4)
    await builder.wire(g4, out)
    await builder.save_and_close(out_dir / IC_4GATE)


async def build_clocked(builder: FixtureBuilder, size: int, out_dir: Path) -> None:
    """clocked_<size>: rows of 18 DFlipFlops (Data on input 0, Clock on input 1) behind one Clock."""
    await builder.new_circuit()
    clock = await builder.element("Clock", 48, 48, "clk")
    for row in range(size // ROW_SIZE):
        y = 152 + row * PITCH
        previous = await builder.element("InputSwitch", 48, y, f"d{row}")
        for col in range(ROW_LENGTH):
            flip_flop = await builder.element("DFlipFlop", 152 + col * PITCH, y, f"ff{row}_{col}")
            await builder.wire(previous, flip_flop, 0)
            await builder.wire(clock, flip_flop, 1)
            previous = flip_flop
        led = await builder.element("Led", 152 + ROW_LENGTH * PITCH, y, f"q{row}")
        await builder.wire(previous, led)
    await builder.save_and_close(out_dir / f"clocked_{size}.panda")


async def build_combinational(builder: FixtureBuilder, size: int, out_dir: Path) -> None:
    """combinational_<size>: rows of 18 two-input gates, each fed twice by the one before it."""
    await builder.new_circuit()
    for row in range(size // ROW_SIZE):
        y = 104 + row * PITCH
        previous = await builder.element("InputSwitch", 48, y, f"in{row}")
        for col in range(ROW_LENGTH):
            gate = await builder.element(GATE_CYCLE[col % len(GATE_CYCLE)], 152 + col * PITCH, y, f"g{row}_{col}")
            await builder.wire(previous, gate, 0)
            await builder.wire(previous, gate, 1)
            previous = gate
        led = await builder.element("Led", 152 + ROW_LENGTH * PITCH, y, f"out{row}")
        await builder.wire(previous, led)
    await builder.save_and_close(out_dir / f"combinational_{size}.panda")


async def build_ic_heavy(builder: FixtureBuilder, size: int, out_dir: Path) -> None:
    """ic_heavy: size instances of perf_ic_4gate, the Clock on every input A and the InputSwitch on every B."""
    ic_file = out_dir / IC_4GATE
    if not ic_file.is_file():
        raise RuntimeError(f"{ic_file} is missing: perf_ic_4gate is generated first")
    await builder.new_circuit()
    clock = await builder.element("Clock", 0, -200, "clk")
    select = await builder.element("InputSwitch", 0, -104, "sel")
    for index in range(size):
        x = 200 + (index % IC_COLUMNS) * IC_PITCH_X
        y = (index // IC_COLUMNS) * IC_PITCH_Y
        ic = await builder.ic(ic_file, x, y, f"ic{index}")
        await builder.wire(clock, ic, 0)
        await builder.wire(select, ic, 1)
    await builder.save_and_close(out_dir / "ic_heavy.panda")


async def build_shape(builder: FixtureBuilder, shape: str, size: int, out_dir: Path) -> None:
    """Build one shape at one size into out_dir; ic_heavy builds its IC first."""
    if shape == "clocked":
        await build_clocked(builder, size, out_dir)
    elif shape == "combinational":
        await build_combinational(builder, size, out_dir)
    else:
        await build_ic_4gate(builder, out_dir)
        await build_ic_heavy(builder, size, out_dir)


class ArgumentParser(argparse.ArgumentParser):
    """An argument parser whose errors end the run like any other failure: a message and status 1, not argparse's 2."""

    def error(self, message: str) -> NoReturn:
        self.print_usage(sys.stderr)
        print(f"FAIL: {message}", file=sys.stderr)
        sys.exit(1)


def parse_args() -> argparse.Namespace:
    """Parse and check the command line."""
    parser = ArgumentParser(description="Generate the performance fixtures through the MCP server.")
    parser.add_argument("--large", action="store_true", help=f"clocked_8000 and combinational_20000 into {LARGE_DIR}")
    parser.add_argument("--shape", choices=("clocked", "combinational", "ic_heavy"), help="build one shape only")
    parser.add_argument("--size", type=int, help="elements (clocked, combinational) or IC instances (ic_heavy)")
    parser.add_argument("--out", type=Path, help="output directory (default: this directory, or the large one)")
    args = parser.parse_args()
    if (args.shape is None) != (args.size is None):
        parser.error("--shape and --size go together")
    if args.shape is not None and args.large:
        parser.error("--large builds a fixed set; use --shape with --out instead")
    if args.shape in ("clocked", "combinational") and (args.size <= 0 or args.size % ROW_SIZE != 0):
        parser.error(f"{args.shape} sizes are positive multiples of {ROW_SIZE}")
    if args.shape == "ic_heavy" and args.size <= 0:
        parser.error("ic_heavy needs at least one instance")
    return args


async def generate(args: argparse.Namespace) -> None:
    """Start the MCP server and build the requested fixtures."""
    out_dir = (args.out or (LARGE_DIR if args.large else PERF_DIR)).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    mcp = MCPInfrastructure(enable_validation=True, verbose=False)
    try:
        if not await mcp.start_mcp():
            raise RuntimeError("could not start the MCP server")
        builder = FixtureBuilder(mcp)
        if args.shape is not None:
            await build_shape(builder, args.shape, args.size, out_dir)
        elif args.large:
            await build_clocked(builder, 8000, out_dir)
            await build_combinational(builder, 20000, out_dir)
        else:
            await build_ic_4gate(builder, out_dir)
            await build_clocked(builder, 2000, out_dir)
            await build_combinational(builder, 1000, out_dir)
            await build_ic_heavy(builder, 2000, out_dir)
    finally:
        await mcp.stop_mcp()


def main() -> int:
    """Entry point; returns the process's exit status."""
    args = parse_args()
    try:
        asyncio.run(generate(args))
    except Exception as error:  # pylint: disable=broad-exception-caught  # every failure is a message and status 1
        print(f"FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
