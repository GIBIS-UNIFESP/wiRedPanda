#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create the Temporal Gate-Delay-Chain example.

A chain of four inverters A -> n1 -> n2 -> n3 -> n4 -> F, with every inverter
output tapped by an LED (T1..T4) so a transition can be watched marching down the
chain one stage at a time.

Temporal mode: each NOT adds its 5 ns delay, so an edge on A reaches T1 after ~5 ns,
T2 after ~10 ns, ... and F after ~20 ns -- the cumulative propagation delay is
visible as a staircase in the Temporal Waveform dock, or as column-lag in
BeWavedDolphin's temporal sweep. Functional mode: every tap changes in the same
instant. Default delays; no overrides.

Usage:
    python3 create_temporal_gate_delay_chain.py
"""

import asyncio
import sys
from pathlib import Path

try:
    from ic_builder_base import ICBuilderBase, run_ic_builder
except ImportError:
    _root = Path(__file__).resolve().parents[2]
    sys.path.insert(0, str(_root / "MCP" / "Client"))
    sys.path.insert(0, str(_root / "Tests" / "Integration" / "IC" / "Generators"))
    del _root
    from ic_builder_base import ICBuilderBase, run_ic_builder

EXAMPLES_DIR = Path(__file__).resolve().parents[2] / "Examples"

STAGES = 4


class GateDelayChainBuilder(ICBuilderBase):
    """Builder for the temporal gate-delay-chain example."""

    async def create(self) -> bool:
        """Build the gate-delay-chain circuit and save it to Examples/."""
        await self.begin_build("Temporal Gate Delay Chain")

        if not await self.create_new_circuit():
            return False

        chain_y = 100.0
        tap_y = 220.0

        switch_a = await self.create_element("InputSwitch", 50.0, chain_y, "A")
        if switch_a is None:
            return False

        prev = switch_a
        for stage in range(1, STAGES + 1):
            x = 50.0 + stage * 110.0
            inv = await self.create_element("Not", x, chain_y, f"n{stage}")
            if inv is None:
                return False
            if not await self.connect(prev, inv):
                return False
            # Tap this stage with an LED so its (delayed) transition is observable.
            tap = await self.create_element("Led", x, tap_y, f"T{stage}")
            if tap is None:
                return False
            if not await self.connect(inv, tap):
                return False
            prev = inv

        # Final output LED on the last stage.
        led_f = await self.create_element("Led", 50.0 + (STAGES + 1) * 110.0, chain_y, "F")
        if led_f is None:
            return False
        if not await self.connect(prev, led_f):
            return False

        output_file = str(EXAMPLES_DIR / "temporal_gate_delay_chain.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Gate-delay chain ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point invoked by run_ic_builder()."""
    builder = GateDelayChainBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "Temporal Gate Delay Chain"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
