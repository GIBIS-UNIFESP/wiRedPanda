#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Create the Temporal Static Hazard example (written to Examples/).

F = A OR (NOT A). Boolean algebra says F is always 1, and in functional (zero-delay) mode
that is exactly what the simulation shows. Real gates disagree: the inverted branch arrives
late, so a falling edge on A leaves both OR inputs momentarily low and F dips -- a static-1
hazard.

The delays are tuned per element (NOT 10 ns, OR 3 ns) so the glitch is wide enough to land
in its own column. Open BeWavedDolphin, switch on the Temporal toolbar toggle, and hold A
high for several columns before dropping it: the dip appears a few columns after the fall.

Usage:
    python3 create_example_temporal_static_hazard.py
"""

import asyncio
import sys
from pathlib import Path

from ic_builder_base import ICBuilderBase, run_ic_builder

EXAMPLES_DIR = Path(__file__).resolve().parents[4] / "Examples"


class StaticHazardBuilder(ICBuilderBase):
    """Builder for the temporal static-hazard example."""

    async def create(self) -> bool:
        """Build the static-hazard circuit and save it to Examples/."""
        await self.begin_build("Temporal Static Hazard")

        if not await self.create_new_circuit():
            return False

        switch = await self.create_element("InputSwitch", 120.0, 160.0, "A")
        not_gate = await self.create_element("Not", 260.0, 240.0, "NOT_A")
        or_gate = await self.create_element("Or", 400.0, 190.0, "F")
        led = await self.create_element("Led", 540.0, 190.0, "F_OUT")
        if None in (switch, not_gate, or_gate, led):
            return False
        assert switch is not None and not_gate is not None and or_gate is not None and led is not None

        # A feeds the OR directly and through the inverter: the two paths race.
        if not await self.connect(switch, not_gate):
            return False
        if not await self.connect(switch, or_gate, target_port=0):
            return False
        if not await self.connect(not_gate, or_gate, target_port=1):
            return False
        if not await self.connect(or_gate, led):
            return False

        # Tuned so the inverted branch is clearly the late one.
        if not await self.set_propagation_delay(not_gate, 10):
            return False
        if not await self.set_propagation_delay(or_gate, 3):
            return False

        output_file = str(EXAMPLES_DIR / "temporal_static_hazard.panda")
        if not await self.save_circuit(output_file):
            return False

        await self.log(f"Static hazard ({self.element_count} elements, {self.connection_count} connections)")
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
