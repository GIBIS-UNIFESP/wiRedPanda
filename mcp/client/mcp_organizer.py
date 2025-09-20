#!/usr/bin/env python3
"""
MCPTestOrganizer: Test management and reporting for MCP testing

This module provides test organization methods for MCP (Model Context Protocol)
testing including test management, reporting, and image export.
"""

import asyncio
import os
import glob
from typing import TYPE_CHECKING

from beartype import beartype
from mcp_models import TestResults

if TYPE_CHECKING:
    from mcp_infrastructure import MCPInfrastructure


class MCPTestOrganizer:
    """Test Organization methods: Test management, reporting, image export"""

    @beartype
    def __init__(self, test_results: TestResults, export_images: bool = False, keep_temp_files: bool = False, verbose: bool = False) -> None:
        self.test_results = test_results
        self.export_images = export_images
        self.keep_temp_files = keep_temp_files
        self.verbose = verbose
        self.image_counter = 0
        self.current_test_context = ""

    # Test Management
    @beartype
    def set_test_context(self, context: str) -> None:
        """Set the current test context for image naming"""
        self.current_test_context = context

    @beartype
    def note_known_issue(self, issue: str) -> None:
        """Record a known issue"""
        self.test_results.known_issues.append(issue)
        print(f"⚠️  KNOWN ISSUE: {issue}")

    @beartype
    def note_process_issue(self, issue: str) -> None:
        """Record a process/stability issue"""
        self.test_results.process_issues.append(issue)
        print(f"⚠️  PROCESS ISSUE: {issue}")

    @staticmethod
    @beartype
    def should_capture_image(test_name: str) -> bool:
        """Determine if this operation should trigger an image capture"""
        # Capture images for operations that modify the circuit visually
        image_triggers = ["Create", "Delete", "Connect", "Disconnect", "Move", "Start simulation", "Stop simulation", "Set input", "List elements", "Load circuit", "Save circuit", "New circuit"]
        return any(trigger in test_name for trigger in image_triggers)

    async def export_circuit_image(self, test_name: str, infrastructure: "MCPInfrastructure") -> None:
        """Export circuit image with sequential numbering"""
        if not self.export_images:
            return

        try:
            # Use centralized configuration for image directory
            from tests.test_config import get_test_images_dir

            images_dir = get_test_images_dir()

            # Increment counter for sequential ordering
            self.image_counter += 1

            # Sanitize test name for filename
            safe_test_name = "".join(c for c in test_name if c.isalnum() or c in ("_", "-")).strip()
            filename = os.path.join(images_dir, f"{self.image_counter:03d}_{safe_test_name}.png")

            # Export the image with longer timeout for complex circuits
            resp = await infrastructure.send_command("export_image", {"filename": filename, "format": "png", "padding": 20}, timeout=45.0)

            if resp and resp.success:
                print(f"   📸 Circuit image exported: {filename}")
            else:
                print(f"   ⚠️  Image export failed: {resp.error if resp else 'No response'}")
                # Still create a placeholder for debugging
                print(f"   🔍 Debug: Attempted to export {safe_test_name}")

        except Exception as e:
            print(f"   ⚠️  Image export error: {e}")

    def cleanup_temp_files(self, test_name: str) -> bool:
        """Clean up temporary files created during testing

        Args:
            test_name: Name of the test for logging purposes

        Returns:
            bool: True if cleanup succeeded, False otherwise
        """
        if self.keep_temp_files:
            print(f"   📁 Keeping temp files for {test_name} (--keep-files enabled)")
            return True

        try:
            # Get temp files directory
            from tests.test_config import get_temp_files_dir
            temp_dir = get_temp_files_dir()

            # Clean up .panda files in temp directory
            panda_files = glob.glob(os.path.join(temp_dir, "*.panda"))
            cleaned_count = 0

            for file_path in panda_files:
                try:
                    if os.path.exists(file_path):
                        os.remove(file_path)
                        cleaned_count += 1
                except OSError as e:
                    print(f"   ⚠️  Failed to remove {file_path}: {e}")

            if cleaned_count > 0:
                print(f"   🧹 Cleaned up {cleaned_count} temp file(s) after {test_name}")
            return True

        except Exception as e:
            print(f"   ⚠️  Error cleaning temp files after {test_name}: {e}")
            return False
