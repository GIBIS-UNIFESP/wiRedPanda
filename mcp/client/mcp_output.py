#!/usr/bin/env python3
"""
MCPOutput: Centralized output handler for MCP testing

This module provides a centralized way to handle all test output,
allowing clean separation of verbose vs quiet modes without
scattering "if verbose:" checks throughout the codebase.
"""

from beartype import beartype


class MCPOutput:
    """Centralized output handler that manages verbose vs quiet modes"""

    @beartype
    def __init__(self, verbose: bool = False) -> None:
        self.verbose = verbose

    @beartype
    def success(self, message: str) -> None:
        """Print success messages (✅) only in verbose mode"""
        if self.verbose:
            print(message)

    @beartype
    def info(self, message: str) -> None:
        """Print info messages always"""
        print(message)

    @beartype
    def warning(self, message: str) -> None:
        """Print warning messages always"""
        print(message)

    @beartype
    def error(self, message: str) -> None:
        """Print error messages always"""
        print(message)

    @beartype
    def cleanup(self, message: str) -> None:
        """Print cleanup messages always (🧹)"""
        print(message)

    @beartype
    def test_result(self, message: str) -> None:
        """Print test result messages always"""
        print(message)
