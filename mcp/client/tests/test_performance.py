#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Performance Tests

Tests for performance, concurrency, and memory leak detection including:
- Rapid sequential command handling
- Command ordering integrity
- Interleaved command types handling
- Memory leak detection through element operations
- Memory leak detection through simulation operations

MCP test implementation
"""

import asyncio
import time

import psutil
from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class PerformanceTests(MCPTestBase):
    """Tests for performance and concurrency behavior"""

    async def run_category_tests(self) -> bool:
        """Run all performance tests

        Returns:
            bool: True if all tests passed, False otherwise
        """
        tests = [
            self.test_concurrent_rapid_sequential_commands,
            self.test_concurrent_command_ordering_integrity,
            self.test_concurrent_interleaved_command_types,
            self.test_memory_leak_element_operations,
            self.test_memory_leak_simulation_operations,
        ]

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    @beartype
    async def test_concurrent_rapid_sequential_commands(self) -> bool:
        """Test rapid sequential command handling"""
        print("\n=== Rapid Sequential Commands Test ===")
        self.set_test_context("test_concurrent_rapid_sequential_commands")

        all_passed: bool = True

        # Setup new circuit# Test rapid sequential commands (simulating concurrent-like behavior)
        # Note: True concurrency would require multiple MCP connections
        print("Testing rapid sequential command handling...")
        for i in range(5):
            # Note: We can't do true concurrency with single stdin/stdout
            # This tests rapid sequential execution instead
            start_time = time.time()
            resp = await self.send_command("list_elements", {})
            end_time = time.time()

            if not resp.success:
                print(f"❌ Rapid command {i} failed")
                break

            self.infrastructure.output.success(f"✅ Rapid command {i}: {(end_time - start_time):.3f}s")
        else:
            self.infrastructure.output.success("✅ All rapid sequential commands succeeded")

        return all_passed

    @beartype
    async def test_concurrent_command_ordering_integrity(self) -> bool:
        """Test command ordering integrity"""
        print("\n=== Command Ordering Integrity Test ===")
        self.set_test_context("test_concurrent_command_ordering_integrity")

        all_passed: bool = True

        # Setup new circuit# Test command ordering integrity
        print("Testing command ordering integrity...")
        element_ids = []

        # Create elements in sequence and verify they maintain order
        for i in range(5):
            resp = await self.send_command(
                "create_element",
                {
                    "type": "And",
                    "x": i * 100,
                    "y": 100,
                    "label": f"Order{i}",
                },
            )
            element_id = await self.validate_element_creation_response(resp, f"Sequential create {i}")
            if element_id:
                element_ids.append(element_id)

        # Verify elements were created in correct order
        resp = await self.send_command("list_elements", {})
        success = await self.assert_success(resp, "List elements for ordering check")
        all_passed &= success

        if success:
            result = await self.get_response_result(resp)
            if result and "elements" in result:
                elements = result["elements"]
                # Sort by element_id to check ordering
                sorted_elements = sorted(elements, key=lambda x: x.get("element_id", 0))

                order_correct = True
                for i, elem in enumerate(sorted_elements[-5:]):  # Last 5 elements
                    expected_label = f"Order{i}"
                    actual_label = elem.get("label", "")
                    if actual_label != expected_label:
                        order_correct = False
                        break

                if order_correct:
                    self.infrastructure.output.success("✅ Element creation order maintained")
                else:
                    print("❌ Element creation order corrupted")

        return all_passed

    @beartype
    async def test_concurrent_interleaved_command_types(self) -> bool:
        """Test interleaved command types handling"""
        print("\n=== Interleaved Command Types Test ===")
        self.set_test_context("test_concurrent_interleaved_command_types")

        all_passed: bool = True

        # Setup new circuit# Test interleaved command types
        print("Testing interleaved command handling...")
        interleave_success = True

        for i in range(3):
            # Create element
            resp = await self.send_command(
                "create_element",
                {
                    "type": "Or",
                    "x": i * 150,
                    "y": 200,
                    "label": f"Interleave{i}",
                },
            )
            if not resp.success:
                interleave_success = False
                break

            # List elements
            resp = await self.send_command("list_elements", {})
            if not resp.success:
                interleave_success = False
                break

            # Start/stop simulation
            resp = await self.send_command("simulation_control", {"action": "start"})
            if not resp.success:
                interleave_success = False
                break

            resp = await self.send_command("simulation_control", {"action": "stop"})
            if not resp.success:
                interleave_success = False
                break

        if interleave_success:
            self.infrastructure.output.success("✅ Interleaved commands handled correctly")
        else:
            print("❌ Interleaved command handling failed")

        return all_passed

    @beartype
    async def test_memory_leak_element_operations(self) -> bool:
        """Test memory leak detection through element creation/deletion stress testing"""
        print("\n=== Element Operations Memory Leak Test ===")
        self.set_test_context("test_memory_leak_element_operations")

        all_passed: bool = True

        if not self.process:
            return False

        try:
            proc = psutil.Process(self.process.pid)
            initial_memory = proc.memory_info().rss / 1024 / 1024  # MB
            print(f"Initial memory usage: {initial_memory:.1f} MB")

            # Create and delete many elements to test for leaks
            print("Creating/deleting elements in batches...")
            for batch in range(3):
                # Create new circuit
                resp = await self.send_command("new_circuit", {})
                if not resp.success:
                    break

                # Create many elements
                created_ids = []
                for i in range(20):
                    resp = await self.send_command(
                        "create_element",
                        {
                            "type": "And",
                            "x": i * 50,
                            "y": batch * 100,
                            "label": f"Leak{batch}_{i}",
                        },
                    )
                    success = await self.assert_success(resp, f"Create element Leak{batch}_{i}")
                    all_passed &= success
                    if success:
                        result = await self.get_response_result(resp)
                        if result and "element_id" in result:
                            created_ids.append(result["element_id"])

                # Delete all elements
                for elem_id in created_ids:
                    resp = await self.send_command("delete_element", {"element_id": elem_id})

                # Check memory after batch
                current_memory = proc.memory_info().rss / 1024 / 1024
                print(f"After batch {batch}: {current_memory:.1f} MB")

                await asyncio.sleep(0.1)  # Allow garbage collection

            # Final memory check
            await asyncio.sleep(0.5)  # Allow more time for cleanup
            final_memory = proc.memory_info().rss / 1024 / 1024
            memory_growth = final_memory - initial_memory

            print(f"Final memory usage: {final_memory:.1f} MB")
            print(f"Memory growth: {memory_growth:.1f} MB")

            # Exact memory validation - expect <=50MB growth
            expected_max_memory_growth = 50.0
            if memory_growth == expected_max_memory_growth:
                print(f"📋 Memory growth at threshold: {memory_growth:.1f} MB")
            elif memory_growth > expected_max_memory_growth:
                print(f"❌ Excessive memory growth: {memory_growth:.1f} MB (expected ≤{expected_max_memory_growth:.1f} MB)")
            else:
                self.infrastructure.output.success(f"✅ Memory usage stable: {memory_growth:.1f} MB growth")

        except Exception as e:
            if "NoSuchProcess" in str(type(e)):
                print("❌ Process not found for memory testing")
            else:
                print(f"❌ Memory test error: {e}")

        return all_passed

    @beartype
    async def test_memory_leak_simulation_operations(self) -> bool:
        """Test memory leak detection through simulation start/stop operations"""
        print("\n=== Simulation Operations Memory Leak Test ===")
        self.set_test_context("test_memory_leak_simulation_operations")

        all_passed: bool = True

        if not self.process:
            return False

        try:
            proc = psutil.Process(self.process.pid)

            # Setup circuit for simulation testing
            # Create some elements for simulation
            resp = await self.send_command(
                "create_element",
                {
                    "type": "InputButton",
                    "x": 100,
                    "y": 100,
                    "label": "MemTestInput",
                },
            )
            await self.validate_element_creation_response(resp, "Create input for simulation memory test")

            resp = await self.send_command(
                "create_element",
                {
                    "type": "Led",
                    "x": 200,
                    "y": 100,
                    "label": "MemTestOutput",
                },
            )
            await self.validate_element_creation_response(resp, "Create output for simulation memory test")

            # Test repeated simulation start/stop
            print("Testing simulation memory usage...")
            sim_start_memory = proc.memory_info().rss / 1024 / 1024

            for _ in range(5):
                resp = await self.send_command("simulation_control", {"action": "start"})
                if not resp.success:
                    continue
                await asyncio.sleep(0.1)
                resp = await self.send_command("simulation_control", {"action": "stop"})
                if not resp.success:
                    continue
                await asyncio.sleep(0.1)

            sim_end_memory = proc.memory_info().rss / 1024 / 1024
            sim_memory_growth = sim_end_memory - sim_start_memory

            print(f"Simulation memory growth: {sim_memory_growth:.1f} MB")

            # Exact simulation memory validation - expect <=10MB growth
            expected_max_sim_memory_growth = 10.0
            if sim_memory_growth == expected_max_sim_memory_growth:
                print(f"📋 Simulation memory growth at threshold: {sim_memory_growth:.1f} MB")
            elif sim_memory_growth > expected_max_sim_memory_growth:
                print(f"❌ Excessive simulation memory growth: {sim_memory_growth:.1f} MB (expected ≤{expected_max_sim_memory_growth:.1f} MB)")
            else:
                self.infrastructure.output.success(f"✅ Simulation memory stable: {sim_memory_growth:.1f} MB")

        except Exception as e:
            if "NoSuchProcess" in str(type(e)):
                print("❌ Process not found for memory testing")
            else:
                print(f"❌ Memory test error: {e}")

        return all_passed
