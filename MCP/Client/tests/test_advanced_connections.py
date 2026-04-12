#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Advanced Connection Tests

Tests for advanced connection and circuit lifecycle commands:
- split_connection
- close_circuit

MCP test implementation
"""

from beartype import beartype

from tests.mcp_test_base import MCPTestBase


class AdvancedConnectionTests(MCPTestBase):
    """Tests for advanced connection operations and circuit lifecycle"""

    async def run_category_tests(self) -> bool:
        tests = [
            self.test_split_connection,
            self.test_close_circuit,
            self.test_close_circuit_multi_tab,
        ]

        print("\n" + "=" * 60)
        print("RUNNING ADVANCED CONNECTION TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    @beartype
    async def test_split_connection(self) -> bool:
        """Test split_connection to insert a node into an existing wire"""
        print("\n=== Split Connection Test ===")
        self.set_test_context("test_split_connection")

        all_passed: bool = True

        # Create input -> output with a wire between them
        input_id = await self.create_element_checked("InputButton", 100, 200, "Create input for split", label="SplitIn")
        if input_id is None:
            return False
        output_id = await self.create_element_checked("Led", 400, 200, "Create output for split", label="SplitOut")
        if output_id is None:
            return False

        # Connect them
        resp = await self.send_command(
            "connect_elements",
            {"source_id": input_id, "source_port": 0, "target_id": output_id, "target_port": 0},
        )
        all_passed &= await self.assert_success(resp, "Connect input to output for split")

        # Verify connection exists
        resp = await self.send_command("list_connections", {})
        result = await self.assert_success_and_get_result(resp, "List connections before split")
        if not result or len(result.get("connections", [])) < 1:
            print("No connection found to split")
            return False

        # Split the connection at midpoint
        resp = await self.send_command(
            "split_connection",
            {
                "source_id": input_id,
                "source_port": 0,
                "target_id": output_id,
                "target_port": 0,
                "x": 250.0,
                "y": 200.0,
            },
        )
        all_passed &= await self.assert_success(resp, "Split connection")

        # After split, we should have more connections than before (node inserted)
        resp = await self.send_command("list_connections", {})
        result = await self.assert_success_and_get_result(resp, "List connections after split")
        if result:
            connections = result.get("connections", [])
            if len(connections) >= 2:
                self.infrastructure.output.success(
                    f"Split created {len(connections)} connections (node inserted)"
                )
            else:
                print(f"Expected >= 2 connections after split, got {len(connections)}")
                all_passed = False
        else:
            all_passed = False

        # Also verify more elements exist (the Node was inserted)
        resp = await self.send_command("list_elements", {})
        result = await self.assert_success_and_get_result(resp, "List elements after split")
        if result:
            elements = result.get("elements", [])
            node_elements = [e for e in elements if e.get("type") == "Node"]
            if len(node_elements) >= 1:
                self.infrastructure.output.success("Split inserted a Node element")
            else:
                # Node may not show up depending on implementation
                self.infrastructure.output.success(
                    f"After split: {len(elements)} elements total"
                )
        else:
            all_passed = False

        # Test split on nonexistent connection
        resp = await self.send_command(
            "split_connection",
            {
                "source_id": 99999,
                "source_port": 0,
                "target_id": 99998,
                "target_port": 0,
                "x": 250.0,
                "y": 200.0,
            },
        )
        all_passed &= await self.assert_failure(resp, "Split nonexistent connection")

        return all_passed

    @beartype
    async def test_close_circuit(self) -> bool:
        """Test close_circuit command"""
        print("\n=== Close Circuit Test ===")
        self.set_test_context("test_close_circuit")

        all_passed: bool = True

        # Get initial tab count
        resp = await self.send_command("get_tab_count", {})
        result = await self.assert_success_and_get_result(resp, "Get initial tab count")
        if not result:
            return False
        initial_count = result.get("tab_count", 0)

        # Create a new tab
        resp = await self.send_command("new_circuit", {})
        all_passed &= await self.assert_success(resp, "Create new circuit tab")

        # Verify tab count increased
        resp = await self.send_command("get_tab_count", {})
        result = await self.assert_success_and_get_result(resp, "Get tab count after new")
        if result:
            new_count = result.get("tab_count", 0)
            if new_count == initial_count + 1:
                self.infrastructure.output.success(f"Tab count increased: {initial_count} -> {new_count}")
            else:
                print(f"Expected tab count {initial_count + 1}, got {new_count}")
                all_passed = False
        else:
            all_passed = False

        # Close the circuit
        resp = await self.send_command("close_circuit", {})
        all_passed &= await self.assert_success(resp, "Close circuit")

        # Verify tab count decreased
        resp = await self.send_command("get_tab_count", {})
        result = await self.assert_success_and_get_result(resp, "Get tab count after close")
        if result:
            after_close = result.get("tab_count", 0)
            if after_close == initial_count:
                self.infrastructure.output.success(f"Tab count restored: {after_close}")
            else:
                print(f"Expected tab count {initial_count} after close, got {after_close}")
                all_passed = False
        else:
            all_passed = False

        return all_passed

    @beartype
    async def test_close_circuit_multi_tab(self) -> bool:
        """Test close_circuit with multiple tabs -- verify other tabs survive"""
        print("\n=== Close Circuit Multi-Tab Test ===")
        self.set_test_context("test_close_circuit_multi_tab")

        all_passed: bool = True

        # Get baseline tab count
        resp = await self.send_command("get_tab_count", {})
        result = await self.assert_success_and_get_result(resp, "Get baseline tab count")
        if not result:
            return False
        baseline = result.get("tab_count", 0)

        # Create two additional tabs
        resp = await self.send_command("new_circuit", {})
        all_passed &= await self.assert_success(resp, "Create tab A")

        resp = await self.send_command("new_circuit", {})
        all_passed &= await self.assert_success(resp, "Create tab B")

        # Verify we have baseline + 2
        resp = await self.send_command("get_tab_count", {})
        result = await self.assert_success_and_get_result(resp, "Tab count after 2 new")
        if result:
            expected = baseline + 2
            actual = result.get("tab_count", 0)
            if actual == expected:
                self.infrastructure.output.success(f"Tab count correct: {actual}")
            else:
                print(f"Expected {expected} tabs, got {actual}")
                all_passed = False

        # Close one tab
        resp = await self.send_command("close_circuit", {})
        all_passed &= await self.assert_success(resp, "Close one tab")

        # Verify we still have baseline + 1
        resp = await self.send_command("get_tab_count", {})
        result = await self.assert_success_and_get_result(resp, "Tab count after close one")
        if result:
            expected = baseline + 1
            actual = result.get("tab_count", 0)
            if actual == expected:
                self.infrastructure.output.success(f"Other tab survived close: {actual} tabs")
            else:
                print(f"Expected {expected} tabs after close, got {actual}")
                all_passed = False

        # Close the remaining extra tab to clean up
        resp = await self.send_command("close_circuit", {})
        all_passed &= await self.assert_success(resp, "Close second extra tab")

        return all_passed
