"""
MCP Test Suite Package

Organized test categories and modular components for the MCP (Model Context Protocol) functionality.

Modular Architecture:
- mcp_infrastructure: Process management and communication
- mcp_validation: Response validation and assertion helpers
- mcp_circuit_builder: Circuit construction and validation helpers
- mcp_organizer: Test management, reporting, and image export
- mcp_runner: Main test runner that orchestrates all components
- mcp_test_base: Base classes and orchestrator for test execution

Test Categories:
- test_basic_protocol: Basic MCP protocol tests
- test_element_operations: Element creation, modification, and deletion
- test_circuit_connections: Connection topology and validation
- test_file_operations: Save/load circuit functionality
- test_simulation: Simulation control and behavior
- test_logic_gates: Logic gate functionality and truth tables
- test_parameter_validation: Input validation and error handling
- test_circuit_topology: Complex circuit structure tests
- test_performance_timing: Performance and timing validation
- test_edge_cases_validation: Edge cases and error conditions
- test_element_type_coverage: Element type coverage tests
- test_advanced_sequential_logic: Sequential circuit testing
- test_waveform_analysis: Waveform analysis and validation
- test_ic_operations: Integrated circuit operations
- test_performance: Integration performance tests
- test_sequential: Sequential logic integration tests
- test_integration_workflows: End-to-end workflow tests

Utility Modules:
- test_config: Test configuration and paths
- test_results: Test result data structures
- mcp_test_fixtures: Test fixtures and data
- mcp_test_helpers: Helper functions for testing
"""

from mcp_circuit_builder import MCPCircuitBuilder
# Core modular components
from mcp_infrastructure import MCPInfrastructure
from mcp_organizer import MCPTestOrganizer
from mcp_runner import MCPTestRunner
from .mcp_test_base import MCPTestBase, MCPTestOrchestrator
from mcp_validation import MCPValidation
# Configuration and utilities
from .test_config import config, get_test_images_dir

# Make commonly used classes available at package level
__all__ = [
    # Core modular components
    "MCPInfrastructure",
    "MCPValidation",
    "MCPCircuitBuilder",
    "MCPTestOrganizer",
    "MCPTestRunner",
    # Base classes and orchestrator
    "MCPTestBase",
    "MCPTestOrchestrator",
    # Configuration
    "config",
    "get_test_images_dir",
]
