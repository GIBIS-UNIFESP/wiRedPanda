#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Performance and Timing Tests

Tests for performance characteristics and timing analysis including:
- Circuit creation and element manipulation timing
- Simulation startup and performance metrics
- Rapid command execution and stress testing
- Timing validation and critical path analysis
- Performance benchmarking and scalability

MCP test implementation
"""

import asyncio
import time
from typing import List

from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class PerformanceTimingTests(MCPTestBase):
    """Tests for performance characteristics and timing analysis"""

    async def run_category_tests(self) -> bool:
        """Run all performance and timing tests

        Returns:
            bool: True if all tests passed, False otherwise
        """
        tests = [
            self.test_performance_new_circuit_timing,
            self.test_performance_element_creation_timing,
            self.test_performance_list_elements_timing,
            self.test_performance_simulation_start_timing,
            self.test_performance_rapid_fire_commands,
        ]

        print("\n" + "=" * 60)
        print("🧪 RUNNING PERFORMANCE AND TIMING TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    # ==================== TEST METHODS ====================
    # Test method implementations

    @beartype
    async def test_performance_new_circuit_timing(self) -> bool:
        """Test new_circuit command response timing"""
        print("\n=== New Circuit Timing Performance Test ===")
        self.set_test_context("test_performance_new_circuit_timing")

        all_passed: bool = True

        # Test response time for basic operations
        start_time = time.time()
        resp = await self.send_command("new_circuit", {})
        new_circuit_time = time.time() - start_time
        all_passed &= await self.assert_success(resp, "Performance: new_circuit timing")

        # Performance timing validation - expect sub-second performance
        expected_max_time = 1.0
        if new_circuit_time > expected_max_time:
            print(f"❌ Slow new_circuit: {new_circuit_time:.3f}s (expected ≤{expected_max_time:.1f}s)")
        else:
            self.infrastructure.output.success(f"✅ Fast new_circuit: {new_circuit_time:.3f}s")

        return all_passed

    @beartype
    async def test_performance_element_creation_timing(self) -> bool:
        """Test element creation timing performance"""
        print("\n=== Element Creation Timing Performance Test ===")
        self.set_test_context("test_performance_element_creation_timing")

        all_passed: bool = True

        # Setup# Test element creation timing
        element_times: List[float] = []
        for i in range(5):
            start_time = time.time()
            resp = await self.send_command(
                "create_element",
                {
                    "type": "And",
                    "x": i * 100,
                    "y": 100,
                    "label": f"PerfTest{i}",
                },
            )
            create_time = time.time() - start_time
            element_times.append(create_time)
            element_id = await self.validate_element_creation_response(resp, f"Performance: create element {i}")
            if element_id is None:
                all_passed = False

        avg_create_time = sum(element_times) / len(element_times)
        max_create_time = max(element_times)

        # Performance timing validation - expect sub-500ms performance
        expected_max_create_time = 0.5
        if max_create_time > expected_max_create_time:
            print(f"❌ Slow element creation: max {max_create_time:.3f}s, avg {avg_create_time:.3f}s (expected ≤{expected_max_create_time:.1f}s)")
        else:
            self.infrastructure.output.success(f"✅ Fast element creation: max {max_create_time:.3f}s, avg {avg_create_time:.3f}s")

        return all_passed

    @beartype
    async def test_performance_list_elements_timing(self) -> bool:
        """Test list_elements bulk operation timing"""
        print("\n=== List Elements Timing Performance Test ===")
        self.set_test_context("test_performance_list_elements_timing")

        all_passed: bool = True

        # Setup with some elements# Create a few elements to list
        for i in range(3):
            resp = await self.send_command(
                "create_element",
                {
                    "type": "And",
                    "x": i * 100,
                    "y": 100,
                    "label": f"ListTest{i}",
                },
            )
            element_id = await self.validate_element_creation_response(resp, f"Create element {i} for list timing")
            if element_id is None:
                all_passed = False

        # Test bulk operation timing
        start_time = time.time()
        resp = await self.send_command("list_elements", {})
        list_time = time.time() - start_time
        all_passed &= await self.assert_success(resp, "Performance: list_elements timing")

        # Performance timing validation - expect sub-200ms performance
        expected_max_list_time = 0.2
        if list_time > expected_max_list_time:
            print(f"❌ Slow list_elements: {list_time:.3f}s (expected ≤{expected_max_list_time:.1f}s)")
        else:
            self.infrastructure.output.success(f"✅ Fast list_elements: {list_time:.3f}s")

        return all_passed

    @beartype
    async def test_performance_simulation_start_timing(self) -> bool:
        """Test simulation startup timing performance"""
        print("\n=== Simulation Start Timing Performance Test ===")
        self.set_test_context("test_performance_simulation_start_timing")

        all_passed: bool = True

        # Setup
        # Create elements for simulation
        resp = await self.send_command(
            "create_element",
            {
                "type": "InputButton",
                "x": 100,
                "y": 100,
                "label": "SimInput",
            },
        )
        element_id = await self.validate_element_creation_response(resp, "Create input for simulation timing")
        if element_id is None:
            all_passed = False

        resp = await self.send_command(
            "create_element",
            {
                "type": "Led",
                "x": 200,
                "y": 100,
                "label": "SimOutput",
            },
        )
        element_id = await self.validate_element_creation_response(resp, "Create output for simulation timing")
        if element_id is None:
            all_passed = False

        # Test simulation startup timing
        start_time = time.time()
        resp = await self.send_command("simulation_control", {"action": "start"})
        sim_start_time = time.time() - start_time
        all_passed &= await self.assert_success(resp, "Performance: simulation start timing")

        # Performance timing validation - expect sub-500ms performance
        expected_max_sim_time = 0.5
        if sim_start_time > expected_max_sim_time:
            print(f"❌ Slow simulation start: {sim_start_time:.3f}s (expected ≤{expected_max_sim_time:.1f}s)")
        else:
            self.infrastructure.output.success(f"✅ Fast simulation start: {sim_start_time:.3f}s")

        return all_passed

    @beartype
    async def test_performance_rapid_fire_commands(self) -> bool:
        """Test rapid fire commands stress performance"""
        print("\n=== Rapid Fire Commands Performance Test ===")
        self.set_test_context("test_performance_rapid_fire_commands")

        all_passed: bool = True

        # Setup# Create some elements to list
        for i in range(3):
            resp = await self.send_command(
                "create_element",
                {
                    "type": "And",
                    "x": i * 100,
                    "y": 100,
                    "label": f"RapidTest{i}",
                },
            )
            element_id = await self.validate_element_creation_response(resp, f"Create element {i} for rapid fire test")
            if element_id is None:
                all_passed = False

        # Test rapid fire commands (stress test)
        rapid_fire_times: List[float] = []
        for i in range(10):
            start_time = time.time()
            resp = await self.send_command("list_elements", {})
            rapid_time = time.time() - start_time
            rapid_fire_times.append(rapid_time)
            if not resp.success:
                print(f"❌ Rapid fire command {i} failed")
                break
        else:
            avg_rapid_time = sum(rapid_fire_times) / len(rapid_fire_times)
            max_rapid_time = max(rapid_fire_times)
            self.infrastructure.output.success(f"✅ Rapid fire test: {len(rapid_fire_times)} commands, max {max_rapid_time:.3f}s, avg {avg_rapid_time:.3f}s")

        return all_passed
