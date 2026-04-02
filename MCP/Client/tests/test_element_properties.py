#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Element Properties Tests

Tests for the extended set_element_properties and list_elements commands,
covering properties added in the MCP feature parity update:
- delay (Clock phase)
- trigger (keyboard shortcut)
- locked (input element lock)
- skin (custom appearance)
- Enhanced list_elements response fields
"""

from tests.mcp_test_base import MCPTestBase


class ElementPropertiesTests(MCPTestBase):
    """Tests for extended element property support"""

    async def run_category_tests(self) -> bool:
        tests = [
            self.test_delay_property,
            self.test_trigger_property,
            self.test_volume_property,
            self.test_locked_property,
            self.test_skin_property,
            self.test_skin_index_property,
            self.test_list_elements_extended,
        ]

        print("\n" + "=" * 60)
        print("RUNNING ELEMENT PROPERTIES TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    async def test_delay_property(self) -> bool:
        """Test setting delay on a Clock element."""
        print("\n=== Delay Property Test ===")
        self.set_test_context("test_delay_property")

        await self.send_command("new_circuit", {})

        clk_resp = await self.send_command("create_element", {"type": "Clock", "x": 100.0, "y": 100.0})
        if not clk_resp.success or not clk_resp.result:
            print("Failed to create Clock")
            return False
        clk_id = clk_resp.result["element_id"]

        # Set delay
        resp = await self.send_command("set_element_properties", {"element_id": clk_id, "delay": 0.25})
        if not resp.success:
            print(f"Failed to set delay: {resp.error}")
            return False

        # Verify via new_properties in response
        new_props = resp.result.get("new_properties", {}) if resp.result else {}
        if abs(new_props.get("delay", -1) - 0.25) > 0.001:
            print(f"Delay not set correctly: {new_props}")
            return False

        self.infrastructure.output.success("delay property works")
        return True

    async def test_trigger_property(self) -> bool:
        """Test setting keyboard trigger on an InputSwitch."""
        print("\n=== Trigger Property Test ===")
        self.set_test_context("test_trigger_property")

        await self.send_command("new_circuit", {})

        sw_resp = await self.send_command("create_element", {"type": "InputSwitch", "x": 100.0, "y": 100.0})
        if not sw_resp.success or not sw_resp.result:
            print("Failed to create InputSwitch")
            return False
        sw_id = sw_resp.result["element_id"]

        resp = await self.send_command("set_element_properties", {"element_id": sw_id, "trigger": "A"})
        if not resp.success:
            print(f"Failed to set trigger: {resp.error}")
            return False

        new_props = resp.result.get("new_properties", {}) if resp.result else {}
        if new_props.get("trigger") != "A":
            print(f"Trigger not set correctly: {new_props}")
            return False

        self.infrastructure.output.success("trigger property works")
        return True

    async def test_volume_property(self) -> bool:
        """Test setting volume on a Buzzer element."""
        print("\n=== Volume Property Test ===")
        self.set_test_context("test_volume_property")

        await self.send_command("new_circuit", {})

        buz_resp = await self.send_command("create_element", {"type": "Buzzer", "x": 100.0, "y": 100.0})
        if not buz_resp.success or not buz_resp.result:
            print("Failed to create Buzzer")
            return False
        buz_id = buz_resp.result["element_id"]

        resp = await self.send_command("set_element_properties", {"element_id": buz_id, "volume": 0.75})
        if not resp.success:
            print(f"Failed to set volume: {resp.error}")
            return False

        new_props = resp.result.get("new_properties", {}) if resp.result else {}
        if abs(new_props.get("volume", -1) - 0.75) > 0.01:
            print(f"Volume not set correctly: {new_props}")
            return False

        self.infrastructure.output.success("volume property works")
        return True

    async def test_locked_property(self) -> bool:
        """Test locking an input element."""
        print("\n=== Locked Property Test ===")
        self.set_test_context("test_locked_property")

        await self.send_command("new_circuit", {})

        sw_resp = await self.send_command("create_element", {"type": "InputSwitch", "x": 100.0, "y": 100.0})
        if not sw_resp.success or not sw_resp.result:
            print("Failed to create InputSwitch")
            return False
        sw_id = sw_resp.result["element_id"]

        # Lock
        resp = await self.send_command("set_element_properties", {"element_id": sw_id, "locked": True})
        if not resp.success:
            print(f"Failed to set locked: {resp.error}")
            return False

        new_props = resp.result.get("new_properties", {}) if resp.result else {}
        if new_props.get("locked") is not True:
            print(f"Locked not set correctly: {new_props}")
            return False

        # Unlock
        resp = await self.send_command("set_element_properties", {"element_id": sw_id, "locked": False})
        if not resp.success:
            print(f"Failed to unlock: {resp.error}")
            return False

        new_props = resp.result.get("new_properties", {}) if resp.result else {}
        if new_props.get("locked") is not False:
            print(f"Unlock not set correctly: {new_props}")
            return False

        self.infrastructure.output.success("locked property works")
        return True

    async def test_skin_property(self) -> bool:
        """Test setting and resetting a custom skin on an LED."""
        print("\n=== Skin Property Test ===")
        self.set_test_context("test_skin_property")

        await self.send_command("new_circuit", {})

        led_resp = await self.send_command("create_element", {"type": "Led", "x": 100.0, "y": 100.0})
        if not led_resp.success or not led_resp.result:
            print("Failed to create Led")
            return False
        led_id = led_resp.result["element_id"]

        # Set custom skin (using a built-in resource as test path)
        resp = await self.send_command("set_element_properties", {
            "element_id": led_id, "skin": ":/Components/Output/Led/RedLed.svg"
        })
        if not resp.success:
            print(f"Failed to set skin: {resp.error}")
            return False

        new_props = resp.result.get("new_properties", {}) if resp.result else {}
        if new_props.get("skin_default") is not False:
            print(f"Skin not marked as custom: {new_props}")
            return False

        # Reset to default
        resp = await self.send_command("set_element_properties", {"element_id": led_id, "skin": ""})
        if not resp.success:
            print(f"Failed to reset skin: {resp.error}")
            return False

        new_props = resp.result.get("new_properties", {}) if resp.result else {}
        if new_props.get("skin_default") is not True:
            print(f"Skin not reset to default: {new_props}")
            return False

        self.infrastructure.output.success("skin property works")
        return True

    async def test_skin_index_property(self) -> bool:
        """Test setting a skin at a specific index on a multi-input LED."""
        print("\n=== Skin Index Property Test ===")
        self.set_test_context("test_skin_index_property")

        await self.send_command("new_circuit", {})

        led_resp = await self.send_command("create_element", {"type": "Led", "x": 100.0, "y": 100.0})
        if not led_resp.success or not led_resp.result:
            print("Failed to create Led")
            return False
        led_id = led_resp.result["element_id"]

        # Set skin at index 1 (the "On" state for a 1-input white LED)
        resp = await self.send_command("set_element_properties", {
            "element_id": led_id,
            "skin": ":/Components/Output/Led/GreenLed.svg",
            "skin_index": 1
        })
        if not resp.success:
            print(f"Failed to set skin at index: {resp.error}")
            return False

        new_props = resp.result.get("new_properties", {}) if resp.result else {}
        if new_props.get("skin_index") != 1:
            print(f"Skin index not set correctly: {new_props}")
            return False

        self.infrastructure.output.success("skin_index property works")
        return True

    async def test_list_elements_extended(self) -> bool:
        """Test that list_elements returns extended properties."""
        print("\n=== List Elements Extended Test ===")
        self.set_test_context("test_list_elements_extended")

        await self.send_command("new_circuit", {})

        # Create a Clock with known frequency
        await self.send_command("create_element", {"type": "Clock", "x": 100.0, "y": 100.0, "label": "TestClock"})
        await self.send_command("create_element", {"type": "InputSwitch", "x": 200.0, "y": 100.0, "label": "TestSwitch"})
        await self.send_command("create_element", {"type": "Led", "x": 300.0, "y": 100.0, "label": "TestLed"})

        resp = await self.send_command("list_elements", {})
        if not resp.success or not resp.result:
            print(f"Failed to list elements: {resp.error}")
            return False

        elements = resp.result.get("elements", [])
        all_passed = True

        for elem in elements:
            etype = elem.get("type", "")

            # All elements should have rotation, input_size, output_size
            if "rotation" not in elem:
                print(f"Missing 'rotation' in {etype}")
                all_passed = False
            if "input_size" not in elem:
                print(f"Missing 'input_size' in {etype}")
                all_passed = False
            if "output_size" not in elem:
                print(f"Missing 'output_size' in {etype}")
                all_passed = False

            # Clock should have frequency
            if etype == "Clock" and "frequency" not in elem:
                print("Missing 'frequency' in Clock")
                all_passed = False

            # InputSwitch should have locked
            if etype == "InputSwitch" and "locked" not in elem:
                print("Missing 'locked' in InputSwitch")
                all_passed = False

            # Led should have color
            if etype == "Led" and "color" not in elem:
                print("Missing 'color' in Led")
                all_passed = False

        if all_passed:
            self.infrastructure.output.success("list_elements extended fields present")

        return all_passed
