#!/usr/bin/env python3
# Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
wiRedPanda MCP Bridge
Provides Python automation interface for wiRedPanda circuit design and simulation.
Enhanced with beartype runtime type validation and pydantic models.
"""

import atexit
import signal
import sys
import threading
from typing import Any, Set, Annotated
import asyncio

from beartype import beartype
from pydantic import BaseModel, Field

# Export ProcessManager and ProcessConfig for use by other modules
__all__ = ["ProcessManager", "ProcessConfig"]


class ProcessConfig(BaseModel):
    """Configuration model for process management."""

    auto_cleanup: bool = Field(
        default=True,
        description="Automatically register cleanup handlers for signal handling"
    )
    max_processes: Annotated[int, Field(
        ge=1,
        le=1000,
        description="Maximum number of processes to track simultaneously"
    )] = 100

    class Config:
        """Pydantic configuration."""
        frozen = True
        extra = "forbid"


class ProcessManager:
    """Manages subprocess lifecycle to prevent orphaned processes."""

    @beartype
    def __init__(self, config: ProcessConfig | None = None) -> None:
        self._config = config or ProcessConfig()
        self._processes: Set[asyncio.subprocess.Process] = set()
        self._lock = threading.Lock()
        self._cleanup_registered = False

    @beartype
    def register_process(self, process: asyncio.subprocess.Process) -> None:
        """Register a process for tracking and cleanup."""
        with self._lock:
            if len(self._processes) >= self._config.max_processes:
                raise ValueError(f"Cannot register process: maximum limit of {self._config.max_processes} reached")

            self._processes.add(process)
            if not self._cleanup_registered and self._config.auto_cleanup:
                self._register_cleanup_handlers()
                self._cleanup_registered = True

    @beartype
    def unregister_process(self, process: asyncio.subprocess.Process) -> None:
        """Unregister a completed process."""
        with self._lock:
            self._processes.discard(process)

    @beartype
    def cleanup_all(self) -> None:
        """Clean up all tracked processes."""
        with self._lock:
            processes_to_cleanup = self._processes.copy()

        for process in processes_to_cleanup:
            try:
                # Handle async processes only
                if process.returncode is None:  # Still running
                    if hasattr(process, "stdin") and process.stdin:
                        process.stdin.close()
                    process.terminate()
                    # For async processes, we can't wait synchronously here
                    # The caller should handle proper async cleanup
            except (OSError, AttributeError):
                pass  # Process may have already terminated

        with self._lock:
            self._processes.clear()

    def _register_cleanup_handlers(self) -> None:
        """Register cleanup handlers for various termination scenarios."""
        # Register atexit handler for normal termination
        atexit.register(self.cleanup_all)

        # Register signal handlers for common termination signals
        def signal_handler(_signum: int, _frame: Any) -> None:
            self.cleanup_all()
            sys.exit(1)

        # Handle common termination signals
        for sig in [signal.SIGTERM, signal.SIGINT]:
            try:
                signal.signal(sig, signal_handler)
            except (OSError, ValueError):
                # Some signals may not be available on all platforms
                pass


# Global process manager instance
_mcp_process_manager = ProcessManager()
