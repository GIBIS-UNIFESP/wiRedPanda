#!/usr/bin/env python3
"""
MCP Test Runner with Modular Helper Classes

This module provides a test runner for the MCP (Model Context Protocol)
implementation, organized into specialized helper classes:

- MCPInfrastructure: Process management and communication
- MCPValidation: Response validation and assertion helpers
- MCPCircuitBuilder: Circuit construction and validation
- MCPTestOrganizer: Test management and reporting
- MCPTestRunner: Main orchestrator that composes all helpers
"""

import argparse
import asyncio
import sys

from beartype import beartype
from pydantic import BaseModel, ConfigDict, Field

# Fix encoding for Windows console output
if sys.platform == "win32" and hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")  # type: ignore[attr-defined]
if sys.platform == "win32" and hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")  # type: ignore[attr-defined]


class TestConfiguration(BaseModel):
    """Configuration model for MCP test suite execution."""

    images: bool = Field(
        default=False, description="Enable circuit image export (slower execution, creates visual documentation)"
    )
    keep_files: bool = Field(default=False, description="Keep temporary test files after completion")
    verbose: bool = Field(default=False, description="Show verbose output including all success messages")
    mcp_gui: bool = Field(default=False, description="Run MCP server with GUI instead of headless mode")

    model_config = ConfigDict(frozen=True, extra="forbid")


@beartype
async def main() -> None:
    parser = argparse.ArgumentParser(description="MCP Test Suite")
    parser.add_argument(
        "--images",
        action="store_true",
        help="Enable circuit image export (slower execution, creates visual documentation)",
    )
    parser.add_argument(
        "--keep-files", action="store_true", help="Keep temporary test files after completion (default: cleanup files)"
    )
    parser.add_argument(
        "--verbose", action="store_true", help="Show verbose output including all success messages (default: quiet)"
    )
    parser.add_argument(
        "--mcp-gui", action="store_true", help="Run MCP server with GUI instead of headless mode (default: headless)"
    )
    args = parser.parse_args()

    # Create validated configuration using pydantic
    config = TestConfiguration(
        images=args.images, keep_files=args.keep_files, verbose=args.verbose, mcp_gui=args.mcp_gui
    )

    # Create test suite with validated options
    from tests.mcp_test_base import MCPTestOrchestrator

    tester = MCPTestOrchestrator(
        enable_image_export=config.images,
        keep_temp_files=config.keep_files,
        verbose=config.verbose,
        mcp_gui=config.mcp_gui,
    )

    if config.images:
        print("🖼️  Image export enabled (--images)")
    else:
        print("🖼️  Image export disabled (use --images to enable)")

    if config.keep_files:
        print("📁 Keeping temporary files (--keep-files)")
    else:
        print("🧹 Cleaning temporary files after tests (use --keep-files to keep)")

    if config.verbose:
        print("🔊 Verbose output enabled (--verbose)")
    else:
        print("🔇 Quiet output (use --verbose for detailed messages)")

    if config.mcp_gui:
        print("🖥️  GUI mode enabled (--mcp-gui)")
    else:
        print("👤 Headless mode (use --mcp-gui for GUI)")

    success: bool = await tester.run_all_tests()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    asyncio.run(main())
