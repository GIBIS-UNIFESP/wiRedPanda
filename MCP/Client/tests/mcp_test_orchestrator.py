#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
MCP Test Suite Orchestrator

Owns the dynamic loading of test categories. Kept separate from
``mcp_test_base`` so each test module can ``from tests.mcp_test_base
import MCPTestBase`` without forming an import cycle with the
orchestrator that loads those same modules.
"""

import asyncio
from typing import List, Optional

from beartype import beartype

from .mcp_test_base import MCPTestBase


class MCPTestOrchestrator:
    """Main orchestrator for all test categories - replaces original MCPTestSuite"""

    @beartype
    def __init__(
        self,
        enable_image_export: bool = False,
        enable_validation: bool = True,
        keep_temp_files: bool = False,
        verbose: bool = False,
        mcp_gui: bool = False,
    ) -> None:
        """Initialize orchestrator with shared runner

        Args:
            enable_image_export: Enable circuit image export during tests
            enable_validation: Enable JSON schema validation
            keep_temp_files: Keep temporary test files after completion
            verbose: Show verbose output including all success messages
            mcp_gui: Run MCP server with GUI instead of headless mode
        """
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

        try:
            from tests.test_element_transformations import ElementTransformationTests

            self.categories.append(ElementTransformationTests(self._runner))
            print("📦 Loaded ElementTransformationTests")
        except ImportError as e:
            print(f"⚠️  Could not load ElementTransformationTests: {e}")

        try:
            from tests.test_advanced_connections import AdvancedConnectionTests

            self.categories.append(AdvancedConnectionTests(self._runner))
            print("📦 Loaded AdvancedConnectionTests")
        except ImportError as e:
            print(f"⚠️  Could not load AdvancedConnectionTests: {e}")

        try:
            from tests.test_negative_cases import NegativeCaseTests

            self.categories.append(NegativeCaseTests(self._runner))
            print("📦 Loaded NegativeCaseTests")
        except ImportError as e:
            print(f"⚠️  Could not load NegativeCaseTests: {e}")

        try:
            from tests.test_element_properties import ElementPropertiesTests

            self.categories.append(ElementPropertiesTests(self._runner))
            print("📦 Loaded ElementPropertiesTests")
        except ImportError as e:
            print(f"⚠️  Could not load ElementPropertiesTests: {e}")

        try:
            from tests.test_wireless_node import WirelessNodeTests

            self.categories.append(WirelessNodeTests(self._runner))
            print("📦 Loaded WirelessNodeTests")
        except ImportError as e:
            print(f"⚠️  Could not load WirelessNodeTests: {e}")

    async def run_all_tests(self) -> bool:
        """Run all test categories

        Returns:
            bool: True if all tests passed, False otherwise
        """
        if self.categories:
            print("🚀 Starting MCP Test Suite")
            print("=" * 60)

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

                if self.test_results.failed > 0 and self.test_results.errors:
                    print("\n" + "=" * 60)
                    print("❌ FAILED TESTS")
                    print("=" * 60)
                    for error in self.test_results.errors:
                        print(f"• {error}")

                return category_success

            finally:
                await self._runner.infrastructure.stop_mcp()
        else:
            print("⚠️  No test categories loaded")
            return False

    async def run_category(self, category_name: str) -> bool:
        """Run a specific test category

        Args:
            category_name: Name of the category to run

        Returns:
            bool: True if category tests passed, False otherwise
        """
        matching_category = None
        for category in self.categories:
            if category.__class__.__name__.lower().replace("tests", "") == category_name.lower():
                matching_category = category
                break

        if not matching_category:
            print(f"❌ Category '{category_name}' not found")
            return False

        if not await self._runner.infrastructure.start_mcp():
            print("❌ Failed to start MCP process")
            return False

        try:
            return await matching_category.run_category_tests()
        finally:
            await self._runner.infrastructure.stop_mcp()
