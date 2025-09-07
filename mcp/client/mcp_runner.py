#!/usr/bin/env python3
"""
MCPTestRunner: Main test runner that composes all specialized helper classes

This module provides the main test runner for MCP (Model Context Protocol)
testing that orchestrates all the specialized helper classes.
"""

import asyncio
from typing import Any, Optional, cast

from beartype import beartype
from mcp_models import TestResults

from mcp_circuit_builder import MCPCircuitBuilder
from mcp_infrastructure import MCPInfrastructure
from mcp_organizer import MCPTestOrganizer
from mcp_validation import MCPValidation


class MCPTestRunner:
    """Main test runner that composes all the specialized helper classes"""

    @beartype
    def __init__(self, enable_image_export: bool = False, enable_validation: bool = True, keep_temp_files: bool = False, verbose: bool = False, mcp_gui: bool = False) -> None:
        # Initialize test results
        self.test_results: TestResults = TestResults(
            total=0,
            passed=0,
            failed=0,
            errors=[],
            known_issues=[],
            process_issues=[],
        )

        # Store options
        self.keep_temp_files = keep_temp_files
        self.verbose = verbose
        self.mcp_gui = mcp_gui

        # Create component instances
        self.infrastructure = MCPInfrastructure(enable_validation=enable_validation, verbose=verbose, mcp_gui=mcp_gui)
        self.organizer = MCPTestOrganizer(self.test_results, export_images=enable_image_export, keep_temp_files=keep_temp_files, verbose=verbose)
        self.validation = MCPValidation(self.test_results, self.infrastructure, self.organizer)
        self.circuit_builder = MCPCircuitBuilder(self.infrastructure, self.validation)

        # Expose commonly used async methods
        self.send_command = self.infrastructure.send_command
        self.start_mcp = self.infrastructure.start_mcp
        self.stop_mcp = self.infrastructure.stop_mcp
        self.assert_success = self.validation.assert_success
        self.assert_failure = self.validation.assert_failure
        # cleanup_circuit method defined below with proper async typing
        self.set_test_context = self.organizer.set_test_context
        self.note_known_issue = self.organizer.note_known_issue
        self.note_process_issue = self.organizer.note_process_issue
        self.export_circuit_image = lambda test_name: self.organizer.export_circuit_image(test_name, self.infrastructure)
        self.validate_element_creation_response = self.validation.validate_element_creation_response
        self.assert_success_with_image = self.validation.assert_success_with_image
        self.create_circuit_from_spec = self.circuit_builder.create_circuit_from_spec
        self.validate_truth_table = self.circuit_builder.validate_truth_table
        self.validate_sequential_state_table = self.circuit_builder.validate_sequential_state_table
        self.verify_output_value = self.validation.verify_output_value
        self.verify_connection_exists = self.validation.verify_connection_exists

    @property
    def process(self) -> Optional[asyncio.subprocess.Process]:
        """Expose process attribute"""
        return self.infrastructure.process

    @beartype
    async def cleanup_circuit(self, test_name: str) -> bool:
        """Cleanup circuit after test with proper async typing"""
        return await self.infrastructure.cleanup_circuit(test_name, self.organizer, self.keep_temp_files, self.verbose)

    @beartype
    async def run_all_tests(self, test_suite: Optional[Any] = None) -> bool:
        """Run complete test suite with process management

        Args:
            test_suite: The test suite instance containing the test methods.
                       If provided and has run_all_tests method, delegate to it.
        """
        # If we have an orchestrator with its own run_all_tests method, delegate to it
        if test_suite is not None and hasattr(test_suite, "run_all_tests"):
            return cast(bool, await test_suite.run_all_tests())

        # No individual tests to run - this runner is only a helper for the orchestrator
        print("⚠️  No test suite provided - use MCPTestOrchestrator instead")
        return False
