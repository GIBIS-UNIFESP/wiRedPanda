#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Wireless Node Tests

Tests for the WirelessMode property on Node elements:
- Setting None / Tx / Rx mode via set_element_properties
- Rejecting wireless_mode on non-Node elements
- Rejecting out-of-range wireless_mode values
- Signal propagation: Rx node receives signal from matching Tx node label
"""

import asyncio

from beartype import beartype

from tests.mcp_test_base import MCPTestBase


class WirelessNodeTests(MCPTestBase):
    """Tests for wireless Node Tx/Rx mode via MCP"""

    async def run_category_tests(self) -> bool:
        tests = [
            self.test_set_wireless_mode_none,
            self.test_set_wireless_mode_tx,
            self.test_set_wireless_mode_rx,
            self.test_wireless_mode_on_non_node_ignored,
            self.test_wireless_mode_invalid_value,
            self.test_wireless_signal_propagation,
        ]

        print("\n" + "=" * 60)
        print("RUNNING WIRELESS NODE TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    @beartype
    async def test_set_wireless_mode_none(self) -> bool:
        """Setting wireless_mode=0 (None) on a Node succeeds and reports correct old/new values"""
        print("\n=== Wireless Mode None Test ===")

        node_id = await self.create_element_checked("Node", 100, 100, "Create Node")
        if node_id is None:
            return False

        # Set Tx first, then reset to None
        resp = await self.send_command(
            "set_element_properties", {"element_id": node_id, "wireless_mode": 1}
        )
        if not await self.assert_success(resp, "Set Tx mode"):
            return False

        resp = await self.send_command(
            "set_element_properties", {"element_id": node_id, "wireless_mode": 0}
        )
        result = await self.assert_success_and_get_result(resp, "Reset to None mode")
        if result is None:
            return False

        new_props = result.get("new_properties", {})
        if new_props.get("wireless_mode") != 0:
            print(f"  FAIL: expected wireless_mode=0, got {new_props.get('wireless_mode')}")
            return False

        print("  PASS: wireless_mode reset to None")
        return True

    @beartype
    async def test_set_wireless_mode_tx(self) -> bool:
        """Setting wireless_mode=1 (Tx) on a Node succeeds"""
        print("\n=== Wireless Mode Tx Test ===")

        node_id = await self.create_element_checked("Node", 200, 100, "Create Node", label="CLK")
        if node_id is None:
            return False

        resp = await self.send_command(
            "set_element_properties", {"element_id": node_id, "wireless_mode": 1}
        )
        result = await self.assert_success_and_get_result(resp, "Set Tx mode")
        if result is None:
            return False

        new_props = result.get("new_properties", {})
        old_props = result.get("old_properties", {})
        if new_props.get("wireless_mode") != 1:
            print(f"  FAIL: expected new wireless_mode=1, got {new_props.get('wireless_mode')}")
            return False
        if old_props.get("wireless_mode") != 0:
            print(f"  FAIL: expected old wireless_mode=0, got {old_props.get('wireless_mode')}")
            return False

        print("  PASS: wireless_mode set to Tx, old=0 new=1")
        return True

    @beartype
    async def test_set_wireless_mode_rx(self) -> bool:
        """Setting wireless_mode=2 (Rx) on a Node succeeds"""
        print("\n=== Wireless Mode Rx Test ===")

        node_id = await self.create_element_checked("Node", 300, 100, "Create Node", label="CLK")
        if node_id is None:
            return False

        resp = await self.send_command(
            "set_element_properties", {"element_id": node_id, "wireless_mode": 2}
        )
        result = await self.assert_success_and_get_result(resp, "Set Rx mode")
        if result is None:
            return False

        new_props = result.get("new_properties", {})
        if new_props.get("wireless_mode") != 2:
            print(f"  FAIL: expected wireless_mode=2, got {new_props.get('wireless_mode')}")
            return False

        print("  PASS: wireless_mode set to Rx")
        return True

    @beartype
    async def test_wireless_mode_on_non_node_ignored(self) -> bool:
        """wireless_mode on a non-Node element (e.g. And gate) is silently ignored"""
        print("\n=== Wireless Mode Ignored on Non-Node Test ===")

        and_id = await self.create_element_checked("And", 400, 100, "Create And gate")
        if and_id is None:
            return False

        # The server silently skips wireless_mode for elements where hasWirelessMode() == false
        resp = await self.send_command(
            "set_element_properties", {"element_id": and_id, "wireless_mode": 1}
        )
        result = await self.assert_success_and_get_result(resp, "Set wireless_mode on And (should be ignored)")
        if result is None:
            return False

        # wireless_mode should NOT appear in the response properties
        new_props = result.get("new_properties", {})
        if "wireless_mode" in new_props:
            print(f"  FAIL: wireless_mode appeared in And gate response: {new_props}")
            return False

        print("  PASS: wireless_mode silently ignored on non-Node element")
        return True

    @beartype
    async def test_wireless_mode_invalid_value(self) -> bool:
        """wireless_mode values outside 0-2 are rejected with an error"""
        print("\n=== Wireless Mode Invalid Value Test ===")

        node_id = await self.create_element_checked("Node", 500, 100, "Create Node")
        if node_id is None:
            return False

        resp = await self.send_command(
            "set_element_properties", {"element_id": node_id, "wireless_mode": 3}
        )
        if not await self.assert_failure(resp, "wireless_mode=3 should fail"):
            return False

        print("  PASS: wireless_mode=3 correctly rejected")
        return True

    @beartype
    async def test_wireless_signal_propagation(self) -> bool:
        """A Tx Node drives an Rx Node with the same label; output LED reflects input switch state"""
        print("\n=== Wireless Signal Propagation Test ===")

        # InputSwitch → Tx Node  (label "DATA")
        sw_id = await self.create_element_checked("InputSwitch", 48, 200, "Create InputSwitch")
        if sw_id is None:
            return False
        tx_id = await self.create_element_checked("Node", 128, 200, "Create Tx Node", label="DATA")
        if tx_id is None:
            return False

        resp = await self.send_command(
            "set_element_properties", {"element_id": tx_id, "wireless_mode": 1}
        )
        if not await self.assert_success(resp, "Set Tx mode on Tx node"):
            return False

        resp = await self.send_command(
            "connect_elements", {"source_id": sw_id, "target_id": tx_id, "source_port": 0, "target_port": 0}
        )
        if not await self.assert_success(resp, "Connect switch → Tx node"):
            return False

        # Rx Node (label "DATA") → LED
        rx_id = await self.create_element_checked("Node", 256, 200, "Create Rx Node", label="DATA")
        if rx_id is None:
            return False

        resp = await self.send_command(
            "set_element_properties", {"element_id": rx_id, "wireless_mode": 2}
        )
        if not await self.assert_success(resp, "Set Rx mode on Rx node"):
            return False

        led_id = await self.create_element_checked("Led", 336, 200, "Create LED")
        if led_id is None:
            return False

        resp = await self.send_command(
            "connect_elements", {"source_id": rx_id, "target_id": led_id, "source_port": 0, "target_port": 0}
        )
        if not await self.assert_success(resp, "Connect Rx node → LED"):
            return False

        # Allow the simulation to rebuild the ElementMapping with the wireless link
        # now that both Tx and Rx nodes are present with their modes set.
        await asyncio.sleep(0.05)

        # Switch OFF → LED should be OFF
        resp = await self.send_command("set_input_value", {"element_id": sw_id, "value": False})
        if not await self.assert_success(resp, "Set switch OFF"):
            return False

        await asyncio.sleep(0.05)  # wait for simulation tick
        if not await self.verify_output_value(led_id, False, "LED OFF when switch OFF"):
            return False

        # Switch ON → LED should be ON
        resp = await self.send_command("set_input_value", {"element_id": sw_id, "value": True})
        if not await self.assert_success(resp, "Set switch ON"):
            return False

        await asyncio.sleep(0.05)  # wait for simulation tick
        if not await self.verify_output_value(led_id, True, "LED ON when switch ON"):
            return False

        print("  PASS: wireless signal propagates Tx → Rx correctly")
        return True
