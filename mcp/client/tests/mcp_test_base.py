#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Base Infrastructure for MCP Test Suite

Abstract base class and orchestrator for all MCP test categories.
Provides common functionality for all test categories.
"""

import asyncio
import sys
from abc import ABC, abstractmethod
from typing import Any, Dict, List, Optional, TYPE_CHECKING, cast

from beartype import beartype

# Import for runtime use
from mcp_runner import MCPTestRunner
from mcp_models import MCPResponse

if TYPE_CHECKING:
    pass  # All imports now done at runtime

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

    async def validate_truth_table(self, inputs: List[int], outputs: List[int], truth_table: List[Dict[str, Any]], test_name: str) -> bool:
        """Validate truth table for logic gates"""
        return await self._runner.validate_truth_table(inputs, outputs, truth_table, test_name)

    async def validate_sequential_state_table(self, inputs: List[int], outputs: List[int], clock_id: int, state_table: List[Dict[str, Any]], test_name: str) -> bool:
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
            result = tab_count_resp.result if hasattr(tab_count_resp, 'result') else {}
            before_test_count = result.get('tab_count', 0) if result else 0
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
                result = tab_count_resp.result if hasattr(tab_count_resp, 'result') else {}
                after_test_count = result.get('tab_count', 0) if result else 0

                # Report tab leak if count increased
                if isinstance(before_test_count, int) and isinstance(after_test_count, int):
                    if after_test_count > before_test_count:
                        print(f"⚠️  TAB LEAK: {test_name} left {after_test_count - before_test_count} extra tabs ({before_test_count} → {after_test_count})")
                    elif after_test_count < before_test_count:
                        self.infrastructure.output.success(f"✅ TAB CLEANUP: {test_name} reduced tabs by {before_test_count - after_test_count} ({before_test_count} → {after_test_count})")

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
        pass


class MCPTestOrchestrator:
    """Main orchestrator for all test categories - replaces original MCPTestSuite"""

    @beartype
    def __init__(self, enable_image_export: bool = False, enable_validation: bool = True, keep_temp_files: bool = False, verbose: bool = False, mcp_gui: bool = False) -> None:
        """Initialize orchestrator with shared runner

        Args:
            enable_image_export: Enable circuit image export during tests
            enable_validation: Enable JSON schema validation
            keep_temp_files: Keep temporary test files after completion
            verbose: Show verbose output including all success messages
            mcp_gui: Run MCP server with GUI instead of headless mode
        """
        # Import locally to avoid circular import
        from mcp_runner import MCPTestRunner

        self._runner = MCPTestRunner(enable_image_export, enable_validation, keep_temp_files, verbose, mcp_gui)

        # Expose runner properties
        self.test_results = self._runner.test_results
        self.infrastructure = self._runner.infrastructure
        self.validation = self._runner.validation
        self.circuit_builder = self._runner.circuit_builder
        self.organizer = self._runner.organizer

        # Test categories will be loaded dynamically
        self.categories: List[MCPTestBase] = []
        self._load_test_categories()

    @property
    def process(self) -> Optional[asyncio.subprocess.Process]:
        """Access to the underlying MCP process"""
        return self._runner.process

    def _load_test_categories(self) -> None:
        """Dynamically load all test category classes"""
        # Import test categories
        try:
            from tests.test_basic_protocol import BasicProtocolTests

            self.categories.append(BasicProtocolTests(self._runner))
            print("📦 Loaded BasicProtocolTests")
        except ImportError as e:
            print(f"⚠️  Could not load BasicProtocolTests: {e}")

        try:
            from tests.test_element_operations import ElementOperationTests

            self.categories.append(ElementOperationTests(self._runner))
            print("📦 Loaded ElementOperationTests")
        except ImportError as e:
            print(f"⚠️  Could not load ElementOperationTests: {e}")

        try:
            from tests.test_circuit_connections import CircuitConnectionTests

            self.categories.append(CircuitConnectionTests(self._runner))
            print("📦 Loaded CircuitConnectionTests")
        except ImportError as e:
            print(f"⚠️  Could not load CircuitConnectionTests: {e}")

        try:
            from tests.test_file_operations import FileOperationTests

            self.categories.append(FileOperationTests(self._runner))
            print("📦 Loaded FileOperationTests")
        except ImportError as e:
            print(f"⚠️  Could not load FileOperationTests: {e}")

        try:
            from tests.test_simulation import SimulationTests

            self.categories.append(SimulationTests(self._runner))
            print("📦 Loaded SimulationTests")
        except ImportError as e:
            print(f"⚠️  Could not load SimulationTests: {e}")

        try:
            from tests.test_logic_gates import LogicGateTests

            self.categories.append(LogicGateTests(self._runner))
            print("📦 Loaded LogicGateTests")
        except ImportError as e:
            print(f"⚠️  Could not load LogicGateTests: {e}")

        try:
            from tests.test_parameter_validation import ParameterValidationTests

            self.categories.append(ParameterValidationTests(self._runner))
            print("📦 Loaded ParameterValidationTests")
        except ImportError as e:
            print(f"⚠️  Could not load ParameterValidationTests: {e}")

        try:
            from tests.test_circuit_topology import CircuitTopologyTests

            self.categories.append(CircuitTopologyTests(self._runner))
            print("📦 Loaded CircuitTopologyTests")
        except ImportError as e:
            print(f"⚠️  Could not load CircuitTopologyTests: {e}")

        try:
            from tests.test_performance_timing import PerformanceTimingTests

            self.categories.append(PerformanceTimingTests(self._runner))
            print("📦 Loaded PerformanceTimingTests")
        except ImportError as e:
            print(f"⚠️  Could not load PerformanceTimingTests: {e}")

        try:
            from tests.test_edge_cases_validation import EdgeCasesValidationTests

            self.categories.append(EdgeCasesValidationTests(self._runner))
            print("📦 Loaded EdgeCasesValidationTests")
        except ImportError as e:
            print(f"⚠️  Could not load EdgeCasesValidationTests: {e}")

        try:
            from tests.test_element_type_coverage import ElementTypeCoverageTests

            self.categories.append(ElementTypeCoverageTests(self._runner))
            print("📦 Loaded ElementTypeCoverageTests")
        except ImportError as e:
            print(f"⚠️  Could not load ElementTypeCoverageTests: {e}")

        try:
            from tests.test_advanced_sequential_logic import AdvancedSequentialLogicTests

            self.categories.append(AdvancedSequentialLogicTests(self._runner))
            print("📦 Loaded AdvancedSequentialLogicTests")
        except ImportError as e:
            print(f"⚠️  Could not load AdvancedSequentialLogicTests: {e}")

        try:
            from tests.test_waveform_analysis import WaveformAnalysisTests

            self.categories.append(WaveformAnalysisTests(self._runner))
            print("📦 Loaded WaveformAnalysisTests")
        except ImportError as e:
            print(f"⚠️  Could not load WaveformAnalysisTests: {e}")

        try:
            from tests.test_ic_operations import ICOperationTests

            self.categories.append(ICOperationTests(self._runner))
            print("📦 Loaded ICOperationTests")
        except ImportError as e:
            print(f"⚠️  Could not load ICOperationTests: {e}")

        # Integration test categories
        try:
            from tests.test_performance import PerformanceTests

            self.categories.append(PerformanceTests(self._runner))
            print("📦 Loaded PerformanceTests (Integration)")
        except ImportError as e:
            print(f"⚠️  Could not load PerformanceTests (Integration): {e}")

        try:
            from tests.test_sequential import SequentialTests

            self.categories.append(SequentialTests(self._runner))
            print("📦 Loaded SequentialTests (Integration)")
        except ImportError as e:
            print(f"⚠️  Could not load SequentialTests (Integration): {e}")

        try:
            from tests.test_integration_workflows import IntegrationWorkflowTests

            self.categories.append(IntegrationWorkflowTests(self._runner))
            print("📦 Loaded IntegrationWorkflowTests")
        except ImportError as e:
            print(f"⚠️  Could not load IntegrationWorkflowTests: {e}")

    async def run_all_tests(self) -> bool:
        """Run all test categories

        Returns:
            bool: True if all tests passed, False otherwise
        """
        # Run test categories
        if self.categories:
            print("🚀 Starting MCP Test Suite")
            print("=" * 60)

            # Start MCP process before running tests
            if not await self._runner.infrastructure.start_mcp():
                print("❌ Failed to start MCP process")
                return False

            try:
                print("🚀 Running test categories...")
                category_success = True

                for category in self.categories:
                    try:
                        success = await category.run_category_tests()
                        if not success:
                            category_success = False
                    except Exception as e:
                        print(f"❌ Category {category.__class__.__name__} failed: {e}")
                        category_success = False

                # Print final results
                print("\n" + "=" * 60)
                print("📊 TEST RESULTS")
                print("=" * 60)
                print(f"Total Tests:    {self.test_results.total}")
                print(f"Passed:         {self.test_results.passed}")
                print(f"Failed:         {self.test_results.failed}")

                if self.test_results.total > 0:
                    success_rate = self.test_results.passed / self.test_results.total * 100
                    print(f"Success Rate:   {success_rate:.1f}%")
                else:
                    print("Success Rate:   N/A")

                # Print failed test details if any failures occurred
                if self.test_results.failed > 0 and self.test_results.errors:
                    print("\n" + "=" * 60)
                    print("❌ FAILED TESTS")
                    print("=" * 60)
                    for error in self.test_results.errors:
                        print(f"• {error}")

                return category_success

            finally:
                # Always stop MCP process
                await self._runner.infrastructure.stop_mcp()
        else:
            # Fallback: no test categories loaded, just return success
            print("⚠️  No test categories loaded")
            return False

    async def run_category(self, category_name: str) -> bool:
        """Run a specific test category

        Args:
            category_name: Name of the category to run

        Returns:
            bool: True if category tests passed, False otherwise
        """
        # Find the matching category
        matching_category = None
        for category in self.categories:
            if category.__class__.__name__.lower().replace("tests", "") == category_name.lower():
                matching_category = category
                break

        if not matching_category:
            print(f"❌ Category '{category_name}' not found")
            return False

        # Start MCP process before running tests
        if not await self._runner.infrastructure.start_mcp():
            print("❌ Failed to start MCP process")
            return False

        try:
            return await matching_category.run_category_tests()
        finally:
            # Always stop MCP process
            await self._runner.infrastructure.stop_mcp()
