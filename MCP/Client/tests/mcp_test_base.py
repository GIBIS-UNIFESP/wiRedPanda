#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Base Infrastructure for MCP Test Suite

Abstract base class for all MCP test categories. Each concrete category
subclasses :class:`MCPTestBase`; orchestration lives in
:mod:`mcp_test_orchestrator` so this module has no dependency on the
sibling ``tests.*`` packages and the import graph is acyclic.
"""

import asyncio
import sys
from abc import ABC, abstractmethod
from typing import Any, Dict, List, Optional, Tuple

from beartype import beartype

from mcp_models import MCPResponse
from mcp_runner import MCPTestRunner

# Add parent directory to path for imports
sys.path.insert(0, "..")


class MCPTestBase(ABC):
    """Abstract base class for all MCP test categories"""

    @beartype
    def __init__(self, runner: MCPTestRunner) -> None:
        """Initialize with shared runner instance

        Args:
            runner: MCPTestRunner instance with all the testing infrastructure
        """
        # Store runner reference - this provides access to all MCP functionality
        self._runner = runner

        # Expose commonly used properties for convenience
        self.test_results = runner.test_results
        self.infrastructure = runner.infrastructure
        self.validation = runner.validation
        self.circuit_builder = runner.circuit_builder
        self.organizer = runner.organizer

    @property
    def process(self) -> Optional[asyncio.subprocess.Process]:
        """Access to the underlying MCP process"""
        return self._runner.process

    # COMMON HELPER METHODS
    # These delegate to the runner but provide a cleaner interface for test methods

    @beartype
    async def send_command(self, command: str, parameters: Dict[str, Any]) -> MCPResponse:
        """Send command to MCP process"""
        return await self._runner.send_command(command, parameters)

    @beartype
    async def assert_success(self, response: MCPResponse, test_name: str) -> bool:
        """Assert command was successful"""
        return await self._runner.assert_success(response, test_name)

    @beartype
    async def assert_success_with_image(self, response: MCPResponse, test_name: str, test_context: str = "") -> bool:
        """Assert command was successful and export circuit image"""
        return await self._runner.assert_success_with_image(response, test_name, test_context)

    async def assert_failure(self, response: MCPResponse, test_name: str) -> bool:
        """Assert command was expected to fail"""
        return await self._runner.assert_failure(response, test_name)

    async def validate_element_creation_response(self, response: MCPResponse, test_name: str) -> Optional[int]:
        """Validate element creation response and return element_id"""
        return await self._runner.validate_element_creation_response(response, test_name)

    async def validate_element_id(self, element_id: int, test_name: str) -> bool:
        """Validate element ID is valid"""
        return await self._runner.validation.validate_element_id(element_id, test_name)

    async def create_circuit_from_spec(self, circuit_spec: Dict[str, Any]) -> Dict[int, int]:
        """Create a circuit from specification and return element ID mapping"""
        return await self._runner.create_circuit_from_spec(circuit_spec)

    async def verify_output_value(self, element_id: int, expected_value: bool, test_name: str) -> bool:
        """Verify output element has expected value"""
        return await self._runner.verify_output_value(element_id, expected_value, test_name)

    def set_test_context(self, test_name: str) -> None:
        """Set test context for image export"""
        self._runner.set_test_context(test_name)

    async def cleanup_circuit(self, test_name: str) -> bool:
        """Cleanup circuit after test completion"""
        return await self._runner.cleanup_circuit(test_name)

    async def verify_connection_exists(self, source_id: int, target_id: int, test_name: str) -> bool:
        """Verify connection exists between elements"""
        return await self._runner.verify_connection_exists(source_id, target_id, test_name)

    async def get_response_result(self, response: MCPResponse) -> Optional[Dict[str, Any]]:
        """Get result data from a response (use after assert_success)"""
        return await self._runner.validation.get_response_result(response)

    async def assert_success_and_get_result(self, response: MCPResponse, test_name: str) -> Optional[Dict[str, Any]]:
        """Assert success and return result data if successful, None if failed"""
        if await self.assert_success(response, test_name):
            return await self.get_response_result(response)
        return None

    async def create_element_checked(
        self,
        elem_type: str,
        x: float,
        y: float,
        test_name: str,
        label: str = "",
    ) -> Optional[int]:
        """Create an element and return its ID, or None on failure."""
        params: Dict[str, Any] = {"type": elem_type, "x": x, "y": y}
        if label:
            params["label"] = label
        resp = await self.send_command("create_element", params)
        return await self.validate_element_creation_response(resp, test_name)

    async def create_simple_circuit(self, test_name: str) -> Tuple[Optional[int], Optional[int]]:
        """Create an InputButton at (100,100) and Led at (300,100), return (input_id, output_id)."""
        input_id = await self.create_element_checked("InputButton", 100, 100, f"{test_name}: Create input")
        output_id = await self.create_element_checked("Led", 300, 100, f"{test_name}: Create output")
        return input_id, output_id

    async def validate_truth_table(
        self, inputs: List[int], outputs: List[int], truth_table: List[Dict[str, Any]], test_name: str
    ) -> bool:
        """Validate truth table for logic gates"""
        return await self._runner.validate_truth_table(inputs, outputs, truth_table, test_name)

    async def validate_sequential_state_table(
        self, inputs: List[int], outputs: List[int], clock_id: int, state_table: List[Dict[str, Any]], test_name: str
    ) -> bool:
        """Validate sequential circuit state table behavior with clock transitions"""
        return await self._runner.validate_sequential_state_table(inputs, outputs, clock_id, state_table, test_name)

    async def setup_test(self, test_name: str) -> bool:
        """Setup test with fresh circuit - called before each test method

        Args:
            test_name: Name of the test method

        Returns:
            bool: True if setup succeeded, False otherwise
        """
        resp = await self.send_command("new_circuit", {})
        if resp and resp.success:
            return True

        print(f"❌ Test setup failed for {test_name}: {resp.error if resp else 'No response'}")
        return False

    async def teardown_test(self, test_name: str) -> bool:
        """Cleanup test - called after each test method (even on failure)

        Args:
            test_name: Name of the test method

        Returns:
            bool: True if teardown succeeded, False otherwise
        """
        return await self.cleanup_circuit(test_name)

    async def run_test_method(self, test_method: Any) -> bool:
        """Execute a single test method with automatic setup/teardown

        Args:
            test_method: The test method to execute

        Returns:
            bool: True if test passed, False if failed
        """
        test_name = test_method.__name__

        # Debug: Check tab count before test
        tab_count_resp = await self._runner.send_command("get_tab_count", {}, timeout=5.0)
        if tab_count_resp and tab_count_resp.success:
            result = tab_count_resp.result if hasattr(tab_count_resp, "result") else {}
            before_test_count = result.get("tab_count", 0) if result else 0
        else:
            before_test_count = "unknown"

        # Setup phase
        if not await self.setup_test(test_name):
            self._runner.test_results.total += 1
            self._runner.test_results.failed += 1
            self._runner.test_results.errors.append(f"{test_name}: Setup failed")
            print(f"🔴 {test_name} FAILED (setup)")
            return False

        test_result = False
        test_exception = None
        teardown_success = False
        final_result = False

        try:
            try:
                # Execute test method
                test_result = await test_method()

            except Exception as e:
                # Handle exceptions as test failures
                test_result = False
                test_exception = e

        finally:
            # Teardown phase - always execute, even on test failure
            teardown_success = await self.teardown_test(test_name)

            # Debug: Check tab count after test
            tab_count_resp = await self._runner.send_command("get_tab_count", {}, timeout=5.0)
            if tab_count_resp and tab_count_resp.success:
                result = tab_count_resp.result if hasattr(tab_count_resp, "result") else {}
                after_test_count = result.get("tab_count", 0) if result else 0

                # Report tab leak if count increased
                if isinstance(before_test_count, int) and isinstance(after_test_count, int):
                    if after_test_count > before_test_count:
                        print(
                            f"⚠️  TAB LEAK: {test_name} left {after_test_count - before_test_count} extra tabs "
                            f"({before_test_count} → {after_test_count})"
                        )
                    elif after_test_count < before_test_count:
                        self.infrastructure.output.success(
                            f"✅ TAB CLEANUP: {test_name} reduced tabs by {before_test_count - after_test_count} "
                            f"({before_test_count} → {after_test_count})"
                        )

            # Determine final result considering both test and teardown
            final_result = test_result and teardown_success

            # Update counters based on final result
            self._runner.test_results.total += 1
            if final_result:
                self._runner.test_results.passed += 1
                print(f"🟢 {test_name} PASSED")
            else:
                self._runner.test_results.failed += 1
                # Determine failure reason for error reporting and logging
                if test_exception:
                    self._runner.test_results.errors.append(f"{test_name}: Exception - {test_exception}")
                    print(f"🔴 {test_name} FAILED with exception: {test_exception}")
                elif not test_result:
                    self._runner.test_results.errors.append(f"{test_name}: Test assertion failed")
                    print(f"🔴 {test_name} FAILED")
                elif not teardown_success:
                    self._runner.test_results.errors.append(f"{test_name}: Teardown failed - resource cleanup error")
                    print(f"🔴 {test_name} FAILED (teardown cleanup failed)")

        return final_result

    @abstractmethod
    async def run_category_tests(self) -> bool:
        """Run all tests in this category

        Returns:
            bool: True if all tests passed, False otherwise
        """
