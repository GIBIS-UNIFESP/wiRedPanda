#!/usr/bin/env python3
"""
MCPInfrastructure: Process management and communication for MCP testing

This module provides infrastructure methods for MCP (Model Context Protocol)
testing including setup, teardown, communication, and process management.
"""

import asyncio
import json
import os
import sys
from typing import TYPE_CHECKING, Any, Dict, Optional

from beartype import beartype
from mcp_models import MCPResponse, create_error_response
from mcp_output import MCPOutput
from mcp_schema_validator import validate_command, validate_response
from wiredpanda_bridge import ProcessManager

if TYPE_CHECKING:
    from mcp_organizer import MCPTestOrganizer

# Fix encoding for Windows console output
if sys.platform == "win32" and hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")  # type: ignore[attr-defined]
if sys.platform == "win32" and hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8")  # type: ignore[attr-defined]

# Global test process manager instance using shared ProcessManager
_test_process_manager = ProcessManager()


class MCPInfrastructure:
    """Infrastructure methods: Setup, teardown, communication, process management"""

    @beartype
    def __init__(self, enable_validation: bool = True, verbose: bool = False, mcp_gui: bool = False) -> None:
        self.process: Optional[asyncio.subprocess.Process] = None
        self.enable_validation = enable_validation
        self.verbose = verbose
        self.mcp_gui = mcp_gui
        self.output = MCPOutput(verbose=verbose)
        self._request_id_counter = 1
        self._stderr_drain_task: Optional[asyncio.Task] = None

    @beartype
    async def start_mcp(self) -> bool:
        """Start MCP process"""
        # Try to get executable from config first, then fallback to manual search
        from tests.test_config import config

        executable_path = None

        # Try the config's smart detection
        if config.wiredpanda_exe and config.wiredpanda_exe.exists():
            executable_path = str(config.wiredpanda_exe)
        else:
            # Fallback to manual search relative to script location
            script_dir = os.path.dirname(os.path.abspath(__file__))
            project_root = os.path.dirname(os.path.dirname(script_dir))  # Go up from client/ to project root

            if sys.platform == "win32":
                # Windows paths relative to project root
                possible_paths = [
                    os.path.join(project_root, "build", "Debug", "wiredpanda.exe"),
                    os.path.join(project_root, "build", "Release", "wiredpanda.exe"),
                    os.path.join(project_root, "build", "wiredpanda.exe"),
                ]
            else:
                # Unix/Linux/macOS paths relative to project root
                possible_paths = [
                    os.path.join(project_root, "build", "wiredpanda"),
                ]

            for path in possible_paths:
                if os.path.exists(path):
                    executable_path = path
                    break

        if not executable_path:
            platform_name = "Windows" if sys.platform == "win32" else "Unix"
            print(f"❌ Could not find wiredpanda executable ({platform_name})")
            return False

        # Check executable permissions on Unix systems
        if sys.platform != "win32":
            if not os.access(executable_path, os.X_OK):
                print(f"❌ Executable {executable_path} exists but is not executable")
                print(f"   Run: chmod +x {executable_path}")
                return False

        print(f"🔍 Using executable: {executable_path}")

        try:
            # Add debug info about the process start
            if sys.platform != "win32":
                print("🔍 Starting MCP process on Unix system...")
                print("   Note: Ensure Qt libraries are available and DISPLAY is set if needed")

            # Build command arguments
            cmd_args = [executable_path, "--mcp"]
            if self.mcp_gui:
                cmd_args.append("--mcp-gui")

            # Create async subprocess with SEPARATE stderr
            # This prevents deadlock by allowing concurrent draining of both pipes
            self.process = await asyncio.create_subprocess_exec(
                *cmd_args,
                stdin=asyncio.subprocess.PIPE,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE,  # SEPARATE from stdout (not merged)
            )

            # Test if process started successfully by checking if it's still running
            await asyncio.sleep(0.1)  # Give process a moment to start

            if self.process.returncode is not None:
                # Process exited immediately
                return_code = self.process.returncode
                print(f"❌ MCP process exited immediately with code {return_code}")
                print("   This usually indicates:")
                print("   - Missing Qt libraries (try: export QT_QPA_PLATFORM=offscreen)")
                print("   - Missing DISPLAY on Linux (try: export DISPLAY=:0)")
                print("   - Wrong executable or missing dependencies")
                return False

            # Register process for orphan prevention
            _test_process_manager.register_process(self.process)

            # Start background task to continuously drain stderr
            # This prevents the stderr buffer from filling up and causing deadlock
            self._stderr_drain_task = asyncio.create_task(self._drain_stderr())

            return True
        except (OSError, FileNotFoundError) as e:
            print(f"❌ Failed to start MCP process: {e}")
            if sys.platform != "win32":
                print("   Common fixes for Linux:")
                print("   - Install Qt6: sudo apt-get install qt6-base-dev")
                print("   - Set headless mode: export QT_QPA_PLATFORM=offscreen")
                print("   - Make executable: chmod +x {executable_path}")
            return False

    @beartype
    async def _drain_stderr(self) -> None:
        """Background task: continuously drain stderr to prevent buffer deadlock.

        The wiredpanda process outputs debug messages to stderr. Without actively
        draining this pipe, the buffer can fill up (~64KB), causing the process
        to block when writing to stderr. This prevents the process from sending
        responses on stdout, creating a deadlock situation.

        This task continuously reads and optionally displays stderr output.
        """
        if not self.process or not self.process.stderr:
            return

        try:
            while self.process and self.process.returncode is None:
                try:
                    # Read stderr line with timeout to allow cancellation
                    line_bytes = await asyncio.wait_for(self.process.stderr.readline(), timeout=1.0)
                    if not line_bytes:
                        # EOF reached
                        break

                    # Optionally display debug output
                    line = line_bytes.decode("utf-8", errors="replace").strip()
                    if line and self.verbose:
                        print(f"[DEBUG] {line}")

                except asyncio.TimeoutError:
                    # Timeout is normal - just continue waiting
                    continue
                except asyncio.CancelledError:
                    # Task was cancelled during shutdown
                    break

        except Exception:
            # Silently exit on any other error (process might have exited)
            pass

    @beartype
    async def stop_mcp(self) -> None:
        """Stop MCP process and clean up resources"""
        # Cancel stderr drain task first
        if self._stderr_drain_task:
            self._stderr_drain_task.cancel()
            try:
                await self._stderr_drain_task
            except asyncio.CancelledError:
                pass
            self._stderr_drain_task = None

        if self.process:
            # Unregister from process manager
            _test_process_manager.unregister_process(self.process)

            try:
                # Close stdin first to signal graceful shutdown
                if self.process.stdin:
                    self.process.stdin.close()
                    await self.process.stdin.wait_closed()

                # Attempt graceful termination with timeout
                self.process.terminate()
                try:
                    await asyncio.wait_for(self.process.wait(), timeout=3.0)
                except asyncio.TimeoutError:
                    # Force kill if graceful termination fails
                    self.process.kill()
                    await self.process.wait()  # This will not hang after kill()
            except (OSError, ProcessLookupError):
                # Process may have already terminated
                pass

            self.process = None

    async def cleanup_circuit(
        self,
        test_name: str,
        organizer: Optional["MCPTestOrganizer"] = None,
        keep_temp_files: bool = False,
        _verbose: bool = False,
    ) -> bool:
        """Properly cleanup circuit after test to prevent resource accumulation"""

        # Export final circuit image before clearing if organizer is provided
        if organizer:
            await organizer.export_circuit_image(f"{test_name}_final", self)

        # Handle circuit saving/cleanup based on keep_temp_files setting
        if keep_temp_files:
            # Save current circuit to test_circuits folder before closing
            from tests.test_config import get_temp_files_dir

            temp_dir = get_temp_files_dir()
            circuit_filename = os.path.join(temp_dir, f"{test_name}_circuit.panda")

            save_resp = await self.send_command("save_circuit", {"filename": circuit_filename})
            if save_resp and save_resp.success:
                print(f"   💾 Circuit saved to test_circuits folder: {circuit_filename}")
            else:
                print(f"   ⚠️  Failed to save circuit: {save_resp.error if save_resp else 'No response'}")

        # Stop any running simulation
        resp = await self.send_command("simulation_control", {"action": "stop"})
        if resp and resp.success:
            self.output.success("   ✅ Simulation stopped")
        else:
            self.output.warning(f"   ⚠️  Simulation stop failed: {resp.error if resp else 'No response'}")

        # Get current tab count and close all tabs (some tests create extra circuits)
        tab_count_resp = await self.send_command("get_tab_count", {}, timeout=5.0)
        if tab_count_resp and tab_count_resp.success:
            result = tab_count_resp.result if hasattr(tab_count_resp, "result") else {}
            tab_count = result.get("tab_count", 1) if result else 1

            # Close all remaining tabs
            for i in range(tab_count):
                resp = await self.send_command("close_circuit", {})
                if resp and resp.success:
                    self.output.success(f"   ✅ Closed circuit tab {i+1}/{tab_count}")
                else:
                    self.output.warning(
                        f"   ⚠️  Failed to close tab {i+1}/{tab_count}: {resp.error if resp else 'No response'}"
                    )
                    break
            return True

        # Fallback: try to close at least one tab
        resp = await self.send_command("close_circuit", {})
        if resp and resp.success:
            self.output.success("   ✅ Circuit closed (autosave cleanup)")
            return True
        self.output.warning(f"   ⚠️  Circuit close failed: {resp.error if resp else 'No response'}")
        return False

    @beartype
    async def restart_mcp_if_needed(self) -> bool:
        """Restart MCP process if it has terminated"""
        try:
            # Test if process is responsive
            if self.process and self.process.returncode is not None:
                print("⚠️  MCP process terminated, restarting...")
                await self.stop_mcp()
                return await self.start_mcp()

            # Test with a simple command to see if process is still responsive
            test_resp = await self.send_command("new_circuit", {})
            if not test_resp.success:
                print("⚠️  MCP process unresponsive, restarting...")
                await self.stop_mcp()
                return await self.start_mcp()

            return True
        except (BrokenPipeError, RuntimeError, OSError) as e:
            print(f"⚠️  MCP process error ({e}), restarting...")
            await self.stop_mcp()
            return await self.start_mcp()

    # Communication Methods
    @beartype
    async def send_command(self, command: str, parameters: Dict[str, Any], timeout: float = 10.0) -> MCPResponse:
        """Send command with validation and get response"""
        if not self.process or self.process.stdin is None or self.process.stdout is None:
            raise RuntimeError("MCP process not started")

        # Generate unique request ID
        request_id = self._request_id_counter
        self._request_id_counter += 1

        # Create JSON-RPC 2.0 request
        request = {"jsonrpc": "2.0", "method": command, "params": parameters, "id": request_id}

        # Pre-validate command if validation is enabled
        if hasattr(self, "enable_validation") and self.enable_validation:
            try:
                validation_result = validate_command(request)
                if not validation_result["overall_valid"]:
                    error_msg = f"Command validation failed: {validation_result['validation_summary']}"
                    if validation_result["json_schema"]["errors"]:
                        error_msg += f" Schema errors: {validation_result['json_schema']['errors'][:2]}"
                    if validation_result["pydantic"]["errors"]:
                        error_msg += f" Pydantic errors: {validation_result['pydantic']['errors'][:2]}"
                    return create_error_response(error_msg, request_id)
            except Exception:
                # Validation system error - continue with basic validation
                pass

        request_json = json.dumps(request) + "\n"

        try:
            self.process.stdin.write(request_json.encode("utf-8"))
            await self.process.stdin.drain()
        except (BrokenPipeError, OSError) as e:
            # Process has crashed or pipe is broken
            return create_error_response(f"Process communication failed: {e}", request_id)

        # Read response with timeout and debug message filtering
        response_line = await self._read_json_response_with_timeout(timeout)
        if not response_line:
            # Check if process has exited
            if self.process.returncode is not None:
                return_code = self.process.returncode
                return create_error_response(
                    f"MCP process exited with code {return_code} before responding", request_id
                )
            return create_error_response(f"No response for command '{command}'", request_id)

        try:
            raw_response = json.loads(response_line)

            # Post-validate response if validation is enabled
            if hasattr(self, "enable_validation") and self.enable_validation:
                try:
                    response_validation = validate_response(raw_response, expected_command=command)
                    if not response_validation["overall_valid"]:
                        # Schema validation failure should be treated as an error
                        error_details = response_validation["validation_summary"]
                        if response_validation.get("json_schema", {}).get("errors"):
                            error_details += f" Schema errors: {response_validation['json_schema']['errors'][:2]}"
                        if response_validation.get("pydantic", {}).get("errors"):
                            error_details += f" Pydantic errors: {response_validation['pydantic']['errors'][:2]}"

                        return create_error_response(f"Response validation failed: {error_details}", request_id)
                except Exception as e:
                    # Validation system error - return error instead of continuing
                    return create_error_response(f"Response validation system error: {e}", request_id)

            return MCPResponse(**raw_response)
        except (json.JSONDecodeError, ValueError, KeyError) as e:
            # Enhanced error reporting for debugging
            error_msg = f"Invalid response format: {e}"
            if len(response_line) == 0:
                return create_error_response(error_msg, request_id)

            # Show first 100 characters of the response for debugging
            preview = response_line[:100]
            if len(response_line) > 100:
                preview += "..."
            error_msg += f"\nReceived: '{preview}'"

            # Check if it looks like an error message from the executable
            if "error" in response_line.lower() or "failed" in response_line.lower():
                error_msg += "\nThis looks like an error message from the executable"

            return create_error_response(error_msg, request_id)

    @beartype
    async def _read_json_response_with_timeout(self, timeout: float) -> Optional[str]:
        """
        Read a JSON response from stdout with timeout.

        NOTE: With separate stderr pipe and background draining task, stdout should
        only contain JSON responses. No filtering of debug messages is needed.

        Args:
            timeout: Timeout in seconds

        Returns:
            JSON response string or None if timeout/no valid response
        """
        if not self.process or not self.process.stdout:
            return None

        start_time = asyncio.get_event_loop().time()

        while True:
            elapsed = asyncio.get_event_loop().time() - start_time
            if elapsed > timeout:
                break

            remaining_timeout = timeout - elapsed
            if remaining_timeout <= 0:
                break

            try:
                # Read line with timeout
                response_line_bytes = await asyncio.wait_for(self.process.stdout.readline(), timeout=remaining_timeout)
                if not response_line_bytes:
                    # EOF reached
                    return None

                response_line = response_line_bytes.decode("utf-8").strip()
                if not response_line:
                    # Empty line, keep reading
                    continue

                # Try to parse as JSON
                try:
                    json.loads(response_line)
                    # Valid JSON response found
                    return response_line
                except json.JSONDecodeError:
                    # This shouldn't happen with separate stderr pipe
                    # But if it does, log and skip
                    if self.verbose:
                        print(f"[WARN] Non-JSON on stdout: {response_line[:100]}")
                    continue

            except asyncio.TimeoutError:
                # Timeout reached
                break
            except Exception as e:
                print(f"[ERROR] Failed to read stdout: {e}")
                return None

        return None
