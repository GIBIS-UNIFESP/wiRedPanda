#!/usr/bin/env python3
"""
Sequential Logic Tests

Tests for advanced sequential logic functionality including:
- Multi-clock domain interactions and cross-domain signal transfer
- Clock enable and gating control functionality
- Power-on reset circuit with initial state validation
- Reset timing and synchronization behavior

Each test drives its flip-flops' clocks itself and checks every output it reads against the value the
circuit must hold. set_input_value runs a simulation update before it answers, so one read after it sees
the result.

MCP test implementation
"""

from collections.abc import Awaitable, Callable
from typing import Any

from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase

# D flip-flop ports: D, Clock, ~Preset, ~Clear
D_PORT = 0
CLOCK_PORT = 1
CLEAR_PORT = 3

# Two D flip-flops in a shift register, both cleared by an active-low reset
SHIFT_REGISTER_SPEC: dict[str, Any] = {
    "elements": [
        {"id": 1, "type": "InputButton", "label": "POR_RESET", "x": 0, "y": 0},
        {"id": 2, "type": "InputButton", "label": "SYS_CLK", "x": 0, "y": 100},
        {"id": 3, "type": "DFlipFlop", "label": "STATE_FF1", "x": 200, "y": 0},
        {"id": 4, "type": "DFlipFlop", "label": "STATE_FF2", "x": 200, "y": 100},
        {"id": 5, "type": "InputButton", "label": "DATA_IN", "x": 0, "y": 200},
        {"id": 6, "type": "Led", "label": "STATE1", "x": 400, "y": 0},
        {"id": 7, "type": "Led", "label": "STATE2", "x": 400, "y": 100},
    ],
    "connections": [
        {"source": 2, "target": 3, "source_port": 0, "target_port": CLOCK_PORT},
        {"source": 2, "target": 4, "source_port": 0, "target_port": CLOCK_PORT},
        {"source": 5, "target": 3, "source_port": 0, "target_port": D_PORT},
        {"source": 3, "target": 4, "source_port": 0, "target_port": D_PORT},
        {"source": 3, "target": 6, "source_port": 0, "target_port": 0},
        {"source": 4, "target": 7, "source_port": 0, "target_port": 0},
        {"source": 1, "target": 3, "source_port": 0, "target_port": CLEAR_PORT},
        {"source": 1, "target": 4, "source_port": 0, "target_port": CLEAR_PORT},
    ],
}


class SequentialTests(MCPTestBase):
    """Tests for advanced sequential logic behavior"""

    CATEGORY_NAME = "SEQUENTIAL LOGIC"

    def tests(self) -> list[Callable[[], Awaitable[bool]]]:
        return [
            self.test_multi_clock_dual_domain_systems,
            self.test_multi_clock_gating_control,
            self.test_power_on_reset_circuit_validation,
            self.test_reset_timing_synchronization,
        ]

    # ==================== HELPERS ====================

    async def _build_and_start(self, spec: dict[str, Any], name: str) -> dict[int, int] | None:
        """Build the circuit in the test's fresh tab and start its simulation, or report why not"""
        mapping = await self.create_circuit_from_spec(spec)
        if len(mapping) != len(spec["elements"]):
            self.report_failure(f"The {name} circuit was not built")
            return None
        resp = await self.send_command("simulation_control", {"action": "start"})
        if not await self.assert_success(resp, f"Start the {name} simulation"):
            return None
        return mapping

    async def _set(self, element_id: int, value: bool, what: str) -> bool:
        resp = await self.send_command("set_input_value", {"element_id": element_id, "value": value})
        return await self.assert_success(resp, f"Set {what} to {value}")

    async def _pulse(self, clock_id: int, what: str) -> bool:
        """A rising edge on the clock, which is left low"""
        return await self._set(clock_id, True, what) and await self._set(clock_id, False, what)

    async def _read(self, element_id: int, what: str) -> bool | None:
        """The level an output element shows, or None (reported) when it is not a level"""
        resp = await self.send_command("get_output_value", {"element_id": element_id})
        if not await self.assert_success(resp, f"Read {what}"):
            return None
        status = resp.result.get("status") if resp.result else None
        if status not in ("low", "high"):
            self.report_failure(f"{what} reads {status}, not a level")
            return None
        return status == "high"

    async def _expect(self, element_id: int, expected: bool, what: str) -> bool:
        actual = await self._read(element_id, what)
        if actual is None:
            return False
        if actual != expected:
            self.report_failure(f"{what} is {actual}, expected {expected}")
            return False
        return True

    # ==================== TEST METHODS ====================

    @beartype
    async def test_multi_clock_dual_domain_systems(self) -> bool:
        """Two flip-flops on two clocks, ANDed: each captures its data on its own clock's edges only"""
        print("\n=== Dual Clock Domain Systems Test ===")
        self.set_test_context("test_multi_clock_dual_domain_systems")

        dual_clock_spec: dict[str, Any] = {
            "elements": [
                {"id": 1, "type": "InputButton", "label": "CLK1", "x": 0, "y": 0},
                {"id": 2, "type": "InputButton", "label": "CLK2", "x": 0, "y": 100},
                {"id": 3, "type": "DFlipFlop", "label": "FF1", "x": 200, "y": 0},
                {"id": 4, "type": "DFlipFlop", "label": "FF2", "x": 200, "y": 100},
                {"id": 5, "type": "InputButton", "label": "DATA1", "x": -200, "y": 0},
                {"id": 6, "type": "InputButton", "label": "DATA2", "x": -200, "y": 100},
                {"id": 7, "type": "Led", "label": "Q1", "x": 400, "y": 0},
                {"id": 8, "type": "Led", "label": "Q2", "x": 400, "y": 100},
                {"id": 9, "type": "And", "label": "SYNC", "x": 600, "y": 50},
                {"id": 10, "type": "Led", "label": "SYNC_OUT", "x": 800, "y": 50},
            ],
            "connections": [
                {"source": 5, "target": 3, "source_port": 0, "target_port": D_PORT},
                {"source": 6, "target": 4, "source_port": 0, "target_port": D_PORT},
                {"source": 1, "target": 3, "source_port": 0, "target_port": CLOCK_PORT},
                {"source": 2, "target": 4, "source_port": 0, "target_port": CLOCK_PORT},
                {"source": 3, "target": 7, "source_port": 0, "target_port": 0},
                {"source": 4, "target": 8, "source_port": 0, "target_port": 0},
                {"source": 3, "target": 9, "source_port": 0, "target_port": 0},
                {"source": 4, "target": 9, "source_port": 0, "target_port": 1},
                {"source": 9, "target": 10, "source_port": 0, "target_port": 0},
            ],
        }
        ids = await self._build_and_start(dual_clock_spec, "dual clock domain")
        if ids is None:
            return False

        all_passed = True
        q1 = q2 = False
        for step in range(12):
            data1 = step % 4 < 2
            data2 = step % 3 < 2
            all_passed &= await self._set(ids[5], data1, "DATA1")
            all_passed &= await self._set(ids[6], data2, "DATA2")
            # CLK1 ticks every second step, CLK2 every third, so the domains capture at different times
            if step % 2 == 0:
                all_passed &= await self._pulse(ids[1], "CLK1")
                q1 = data1
            if step % 3 == 0:
                all_passed &= await self._pulse(ids[2], "CLK2")
                q2 = data2
            all_passed &= await self._expect(ids[7], q1, f"Q1 at step {step}")
            all_passed &= await self._expect(ids[8], q2, f"Q2 at step {step}")
            all_passed &= await self._expect(ids[10], q1 and q2, f"SYNC_OUT at step {step}")

        if all_passed:
            self.infrastructure.output.success("✅ Each domain captures on its own clock; SYNC follows both")
        return all_passed

    @beartype
    async def test_multi_clock_gating_control(self) -> bool:
        """A clock gated by an enable: the flip-flop captures only while the gate is open"""
        print("\n=== Clock Gating and Enable Control Test ===")
        self.set_test_context("test_multi_clock_gating_control")

        clock_enable_spec: dict[str, Any] = {
            "elements": [
                {"id": 1, "type": "InputButton", "label": "MASTER_CLK", "x": 0, "y": 0},
                {"id": 2, "type": "InputButton", "label": "CLK_ENABLE", "x": 0, "y": 100},
                {"id": 3, "type": "And", "label": "CLK_GATE", "x": 200, "y": 50},
                {"id": 4, "type": "DFlipFlop", "label": "GATED_FF", "x": 400, "y": 50},
                {"id": 5, "type": "InputButton", "label": "DATA", "x": 200, "y": -50},
                {"id": 6, "type": "Led", "label": "GATED_Q", "x": 600, "y": 50},
            ],
            "connections": [
                {"source": 1, "target": 3, "source_port": 0, "target_port": 0},
                {"source": 2, "target": 3, "source_port": 0, "target_port": 1},
                {"source": 3, "target": 4, "source_port": 0, "target_port": CLOCK_PORT},
                {"source": 5, "target": 4, "source_port": 0, "target_port": D_PORT},
                {"source": 4, "target": 6, "source_port": 0, "target_port": 0},
            ],
        }
        ids = await self._build_and_start(clock_enable_spec, "clock gating")
        if ids is None:
            return False

        all_passed = True
        # Enabled: an edge captures the data
        all_passed &= await self._set(ids[2], True, "CLK_ENABLE")
        all_passed &= await self._set(ids[5], True, "DATA")
        all_passed &= await self._pulse(ids[1], "MASTER_CLK")
        all_passed &= await self._expect(ids[6], True, "GATED_Q after an enabled edge with DATA high")

        # Disabled: the gate holds the flip-flop's clock low, so an edge changes nothing
        all_passed &= await self._set(ids[2], False, "CLK_ENABLE")
        all_passed &= await self._set(ids[5], False, "DATA")
        all_passed &= await self._pulse(ids[1], "MASTER_CLK")
        all_passed &= await self._expect(ids[6], True, "GATED_Q after a disabled edge with DATA low")

        # Enabled again: the next edge captures the new data
        all_passed &= await self._set(ids[2], True, "CLK_ENABLE")
        all_passed &= await self._expect(ids[6], True, "GATED_Q after enabling with the clock low")
        all_passed &= await self._pulse(ids[1], "MASTER_CLK")
        all_passed &= await self._expect(ids[6], False, "GATED_Q after an enabled edge with DATA low")

        if all_passed:
            self.infrastructure.output.success("✅ The gated flip-flop captures only while enabled")
        return all_passed

    @beartype
    async def test_power_on_reset_circuit_validation(self) -> bool:
        """A shift register held clear by its reset, which then shifts on each edge"""
        print("\n=== Power-On Reset Circuit Validation Test ===")
        self.set_test_context("test_power_on_reset_circuit_validation")

        ids = await self._build_and_start(SHIFT_REGISTER_SPEC, "power-on reset")
        if ids is None:
            return False

        all_passed = True
        # Reset asserted (active low): both stages clear
        all_passed &= await self._set(ids[1], False, "POR_RESET")
        all_passed &= await self._set(ids[5], False, "DATA_IN")
        all_passed &= await self._expect(ids[6], False, "STATE1 with reset asserted")
        all_passed &= await self._expect(ids[7], False, "STATE2 with reset asserted")

        # Reset released: nothing changes until an edge
        all_passed &= await self._set(ids[1], True, "POR_RESET")
        all_passed &= await self._expect(ids[6], False, "STATE1 after reset release")
        all_passed &= await self._expect(ids[7], False, "STATE2 after reset release")

        # First edge: the first stage captures DATA_IN
        all_passed &= await self._set(ids[5], True, "DATA_IN")
        all_passed &= await self._pulse(ids[2], "SYS_CLK")
        all_passed &= await self._expect(ids[6], True, "STATE1 after the first edge")
        all_passed &= await self._expect(ids[7], False, "STATE2 after the first edge")

        # Second edge: the first stage's value shifts into the second
        all_passed &= await self._pulse(ids[2], "SYS_CLK")
        all_passed &= await self._expect(ids[6], True, "STATE1 after the second edge")
        all_passed &= await self._expect(ids[7], True, "STATE2 after the second edge")

        if all_passed:
            self.infrastructure.output.success("✅ Reset clears both stages; the register shifts after release")
        return all_passed

    @beartype
    async def test_reset_timing_synchronization(self) -> bool:
        """The reset clears loaded stages at once, without an edge, and holds them clear against edges"""
        print("\n=== Reset Timing and Synchronization Test ===")
        self.set_test_context("test_reset_timing_synchronization")

        ids = await self._build_and_start(SHIFT_REGISTER_SPEC, "reset timing")
        if ids is None:
            return False

        all_passed = True
        for cycle in range(3):
            # Load both stages with a one
            all_passed &= await self._set(ids[1], True, "POR_RESET")
            all_passed &= await self._set(ids[5], True, "DATA_IN")
            all_passed &= await self._pulse(ids[2], "SYS_CLK")
            all_passed &= await self._pulse(ids[2], "SYS_CLK")
            all_passed &= await self._expect(ids[6], True, f"STATE1 loaded, cycle {cycle}")
            all_passed &= await self._expect(ids[7], True, f"STATE2 loaded, cycle {cycle}")

            # The reset clears asynchronously, with no clock edge
            all_passed &= await self._set(ids[1], False, "POR_RESET")
            all_passed &= await self._expect(ids[6], False, f"STATE1 on reset, cycle {cycle}")
            all_passed &= await self._expect(ids[7], False, f"STATE2 on reset, cycle {cycle}")

            # Edges while the reset is held change nothing
            all_passed &= await self._pulse(ids[2], "SYS_CLK")
            all_passed &= await self._expect(ids[6], False, f"STATE1 clocked under reset, cycle {cycle}")
            all_passed &= await self._expect(ids[7], False, f"STATE2 clocked under reset, cycle {cycle}")

            # After release, one edge loads only the first stage
            all_passed &= await self._set(ids[1], True, "POR_RESET")
            all_passed &= await self._pulse(ids[2], "SYS_CLK")
            all_passed &= await self._expect(ids[6], True, f"STATE1 one edge after release, cycle {cycle}")
            all_passed &= await self._expect(ids[7], False, f"STATE2 one edge after release, cycle {cycle}")

        if all_passed:
            self.infrastructure.output.success("✅ The reset clears at once and holds against edges, every cycle")
        return all_passed
