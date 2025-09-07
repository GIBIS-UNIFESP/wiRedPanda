#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Test Configuration for MCP Test Suite

Centralized configuration for all test-related paths and settings.
This avoids duplication and makes maintenance easier.
"""

from pathlib import Path
from typing import Optional


class TestConfig:
    """Centralized test configuration"""

    def __init__(self) -> None:
        # Base directory is the mcp directory (parent of tests/)
        self.base_dir = Path(__file__).parent.parent

        # Test output directories - all use the temp/ folder
        self.temp_dir = self.base_dir / "temp"
        self.test_images_dir = self.temp_dir / "test_images"
        self.test_exports_dir = self.temp_dir / "test_exports"
        self.temp_files_dir = self.temp_dir / "test_circuits"

        # Executable paths
        self.wiredpanda_exe = self._find_wiredpanda_executable()

        # Test settings
        self.default_timeout = 300  # 5 minutes
        self.long_timeout = 900     # 15 minutes for performance tests

    def _find_wiredpanda_executable(self) -> Optional[Path]:
        """Find the WiredPanda executable relative to the mcp directory"""
        # Look for the executable in ../build/Release/
        build_dir = self.base_dir.parent / "build" / "Release"

        # Check for different possible executable names
        possible_names = [
            "wiredpanda.exe",  # Windows
            "wiredpanda",      # Linux/macOS
            "wiRedPanda.exe",  # Alternative Windows name
            "wiRedPanda"       # Alternative Linux/macOS name
        ]

        for name in possible_names:
            exe_path = build_dir / name
            if exe_path.exists():
                return exe_path

        return None

    def ensure_directories(self) -> None:
        """Create test directories if they don't exist"""
        # Create the main temp directory first
        self.temp_dir.mkdir(parents=True, exist_ok=True)
        # Then create subdirectories
        for directory in [self.test_images_dir, self.test_exports_dir, self.temp_files_dir]:
            directory.mkdir(parents=True, exist_ok=True)

    def get_test_images_dir(self) -> str:
        """Get the test images directory path as string"""
        self.test_images_dir.mkdir(parents=True, exist_ok=True)
        return str(self.test_images_dir)

    def get_test_exports_dir(self) -> str:
        """Get the test exports directory path as string"""
        self.test_exports_dir.mkdir(parents=True, exist_ok=True)
        return str(self.test_exports_dir)

    def get_temp_files_dir(self) -> str:
        """Get the test circuits directory path as string"""
        self.temp_files_dir.mkdir(parents=True, exist_ok=True)
        return str(self.temp_files_dir)

    def get_wiredpanda_executable(self) -> str:
        """Get the WiredPanda executable path as string"""
        if self.wiredpanda_exe:
            return str(self.wiredpanda_exe)

        # Fallback to the current hardcoded path
        return "../build/Release/wiredpanda.exe"


# Global configuration instance
config = TestConfig()

# Convenience functions


def get_test_images_dir() -> str:
    """Get the test images directory path"""
    return config.get_test_images_dir()


def get_test_exports_dir() -> str:
    """Get the test exports directory path"""
    return config.get_test_exports_dir()


def get_temp_files_dir() -> str:
    """Get the test circuits directory path"""
    return config.get_temp_files_dir()


def get_wiredpanda_executable() -> str:
    """Get the WiredPanda executable path"""
    return config.get_wiredpanda_executable()
