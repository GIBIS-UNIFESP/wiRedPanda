#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create the Temporal Gate Delay Chain example (written to Examples/).

A switch drives four inverters in series, with an LED tapping every stage. In functional
(zero-delay) mode all four LEDs change in the same instant, so the chain teaches nothing.
Open BeWavedDolphin, switch on the Temporal toolbar toggle, and drive a wide pulse: the
edge marches stage by stage, each tap changing ~5 ns (the NOT default) after the one before
it, so cumulative propagation delay is visible as a staircase across the columns.

Usage:
    python3 create_example_temporal_gate_delay_chain.py
"""

import asyncio
import sys
from pathlib import Path

from ic_builder_base import ICBuilderBase, run_ic_builder

EXAMPLES_DIR = Path(__file__).resolve().parents[4] / "Examples"

STAGES = 4


class GateDelayChainBuilder(ICBuilderBase):
    """Builder for the temporal gate-delay-chain example."""

    async def create(self) -> bool:
        """Build the inverter chain with per-stage taps and save it to Examples/."""
        await self.begin_build("Temporal Gate Delay Chain")

        if not await self.create_new_circuit():
            return False

        switch = await self.create_element("InputSwitch", 80.0, 160.0, "A")
        if switch is None:
            return False

        previous = switch
        for stage in range(1, STAGES + 1):
            x = 80.0 + stage * 140.0
            inverter = await self.create_element("Not", x, 160.0, f"n{stage}")
            tap = await self.create_element("Led", x, 300.0, f"Q{stage}")
            if inverter is None or tap is None:
                return False
            if not await self.connect(previous, inverter):
                return False
            if not await self.connect(inverter, tap):
                return False
            previous = inverter

        output_file = str(EXAMPLES_DIR / "temporal_gate_delay_chain.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"Gate delay chain ({self.element_count} elements, {self.connection_count} connections)")
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
