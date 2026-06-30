#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create the Temporal Static-Hazard example.

Reconvergent fanout F = A OR (NOT A) -- logically a tautology (always 1). But the
two paths to the OR have different delays, so when A falls the direct path reaches
the OR before the inverted path does; for a moment the OR sees (0, 0) and F dips
low: a static-1 hazard.

This is tuned with per-element propagation delays so the hazard is visible: the NOT
is made slow (10 ns) and the OR fast (3 ns), so the OR (faster than the path skew)
re-evaluates while both inputs are momentarily 0. In functional (zero-delay) mode F
stays high and the glitch never appears -- it exists only under temporal simulation.

Usage:
    python3 create_temporal_static_hazard.py
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

NOT_DELAY_NS = 10  # slow inverted path -> the skew that creates the hazard
OR_DELAY_NS = 3    # fast reconverging gate -> faster than the skew, so the glitch survives


class StaticHazardBuilder(ICBuilderBase):
    """Builder for the temporal static-hazard example."""

    async def create(self) -> bool:
        """Build the static-hazard circuit and save it to Examples/."""
        await self.begin_build("Temporal Static Hazard")

        if not await self.create_new_circuit():
            return False

        switch_a = await self.create_element("InputSwitch", 50.0, 150.0, "A")
        not_gate = await self.create_element("Not", 180.0, 230.0, "inv")
        or_gate = await self.create_element("Or", 320.0, 150.0, "reconv")
        led_f = await self.create_element("Led", 450.0, 150.0, "F")
        if None in (switch_a, not_gate, or_gate, led_f):
            return False

        # Direct path A -> OR.in0; inverted path A -> NOT -> OR.in1; OR -> F.
        if not await self.connect(switch_a, or_gate, target_port=0):
            return False
        if not await self.connect(switch_a, not_gate):
            return False
        if not await self.connect(not_gate, or_gate, target_port=1):
            return False
        if not await self.connect(or_gate, led_f):
            return False

        # Tune the delays so the hazard is observable (OR faster than the NOT-path skew).
        assert not_gate is not None and or_gate is not None
        if not await self.set_propagation_delay(not_gate, NOT_DELAY_NS):
            return False
        if not await self.set_propagation_delay(or_gate, OR_DELAY_NS):
            return False

        output_file = str(EXAMPLES_DIR / "temporal_static_hazard.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"✅ Static hazard ({self.element_count} elements, {self.connection_count} connections)")
        await self.log(f"   Saved to: {output_file}")
        return True


async def build(mcp) -> bool:
    """Entry point invoked by run_ic_builder()."""
    builder = StaticHazardBuilder(mcp, verbose=True)
    return await builder.create()


if __name__ == "__main__":
    import traceback

    try:
        exit_code = asyncio.run(run_ic_builder(build, "Temporal Static Hazard"))
        sys.exit(exit_code)
    except Exception as e:
        print(f"Error: {e}")
        traceback.print_exc()
        sys.exit(1)
