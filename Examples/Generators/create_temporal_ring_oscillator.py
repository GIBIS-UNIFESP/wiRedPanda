#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create the Temporal Ring Oscillator example.

Three inverters in a loop (NOT -> NOT -> NOT -> back to the first), with the first
inverter's stage tapped by an LED labelled "Q".

A three-inverter ring has no stable logic value. In functional (zero-delay) mode the
engine therefore canonicalises it to Unknown -- which is the teaching point. Switch
the toolbar selector to Temporal and the simulation re-settles under propagation
delays: with the 5 ns per-inverter default the loop oscillates with a ~30 ns period,
visible in the Temporal Waveform dock. No per-element delay overrides: the defaults
drive it.

Usage:
    python3 create_temporal_ring_oscillator.py
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


class RingOscillatorBuilder(ICBuilderBase):
    """Builder for the temporal ring-oscillator example."""

    async def create(self) -> bool:
        """Build the ring-oscillator circuit and save it to Examples/."""
        await self.begin_build("Temporal Ring Oscillator")

        if not await self.create_new_circuit():
            return False

        y = 120.0
        not1 = await self.create_element("Not", 150.0, y, "n1")
        not2 = await self.create_element("Not", 270.0, y, "n2")
        not3 = await self.create_element("Not", 390.0, y, "n3")
        led = await self.create_element("Led", 510.0, y, "Q")
        if None in (not1, not2, not3, led):
            return False

        # Close the inverter loop: n1 -> n2 -> n3 -> n1; tap n1 to the LED.
        if not await self.connect(not1, not2):
            return False
        if not await self.connect(not2, not3):
            return False
        if not await self.connect(not3, not1):
            return False
        if not await self.connect(not1, led):
            return False

        output_file = str(EXAMPLES_DIR / "temporal_ring_oscillator.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Ring oscillator ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point invoked by run_ic_builder()."""
    builder = RingOscillatorBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "Temporal Ring Oscillator"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
