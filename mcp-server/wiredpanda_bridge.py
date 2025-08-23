#!/usr/bin/env python3
"""
wiRedPanda MCP Bridge

This module provides a Python interface to control the wiRedPanda application
through its MCP (Model Context Protocol) stdin/stdout interface.
"""

import json
import os
import subprocess  # nosec B404 - subprocess needed for process control
import time
from typing import Any, Dict, List, Optional, Tuple, Literal

from beartype import beartype
from pydantic import BaseModel, Field, ConfigDict, validator


# Pydantic Models for Data Validation
class MCPCommand(BaseModel):
    """Model for MCP command structure."""
    model_config = ConfigDict(extra='forbid', frozen=True)

    command: str = Field(..., min_length=1, description="Command name")
    parameters: Dict[str, Any] = Field(default_factory=dict, description="Command parameters")


class MCPResponse(BaseModel):
    """Model for MCP response structure."""
    model_config = ConfigDict(extra='allow')

    status: Literal["success", "error"] = Field(..., description="Response status")
    result: Optional[Dict[str, Any]] = Field(None, description="Response data")
    error: Optional[str] = Field(None, description="Error message if status is error")


class ElementInfo(BaseModel):
    """Model for circuit element information."""
    model_config = ConfigDict(extra='allow')

    element_id: int = Field(..., ge=0, description="Unique element identifier")
    type: str = Field(..., min_length=1, description="Element type")
    x: float = Field(..., description="X position")
    y: float = Field(..., description="Y position")
    label: Optional[str] = Field(None, description="Element label")


class TruthTableTest(BaseModel):
    """Model for truth table test case."""
    model_config = ConfigDict(frozen=True)

    inputs: List[bool] = Field(..., description="Input values")
    expected: bool = Field(..., description="Expected output")
    actual: Optional[bool] = Field(None, description="Actual output")
    passed: Optional[bool] = Field(None, description="Whether test passed")


class TestResults(BaseModel):
    """Model for truth table test results."""
    model_config = ConfigDict(extra='forbid')

    total_tests: int = Field(..., ge=0, description="Total number of tests")
    passed: int = Field(..., ge=0, description="Number of passed tests")
    failed: int = Field(..., ge=0, description="Number of failed tests")
    details: List[TruthTableTest] = Field(default_factory=list, description="Test details")
    success_rate: float = Field(..., ge=0.0, le=1.0, description="Success rate (0-1)")

    @validator('failed')
    @classmethod
    def validate_totals(cls, v: int, values: Dict[str, Any]) -> int:
        """Ensure passed + failed equals total_tests."""
        if 'passed' in values and 'total_tests' in values:
            if v + values['passed'] != values['total_tests']:
                raise ValueError("passed + failed must equal total_tests")
        return v


class WiredPandaError(Exception):
    """Exception raised when wiRedPanda operations fail."""

    @beartype
    def __init__(self, message: str) -> None:
        super().__init__(message)
        self.message = message


class WiredPandaBridge:
    """Bridge to control real wiRedPanda through MCP stdin/stdout interface."""

    @beartype
    def __init__(
        self,
        executable_path: str = "/workspace/build/wiredpanda",
        headless: bool = True,
        timeout: float = 30.0,
    ) -> None:
        """
        Initialize the wiRedPanda bridge.

        Args:
            executable_path: Path to wiRedPanda executable
            headless: Run in headless mode (no GUI)
            timeout: Default timeout for commands
        """
        self.executable_path: str = executable_path
        self.headless: bool = headless
        self.timeout: float = timeout
        self.process: Optional[subprocess.Popen[str]] = None
        self._element_counter: int = 0

    @beartype
    def start(self) -> None:
        """Start wiRedPanda in MCP mode."""
        if self.process:
            raise WiredPandaError("Bridge already started")

        try:
            env: Dict[str, str] = os.environ.copy()
            if self.headless:
                env["QT_QPA_PLATFORM"] = "offscreen"

            # pylint: disable=consider-using-with
            # Process needs to persist beyond this method scope
            self.process = subprocess.Popen(  # nosec B603 - executable path controlled
                [self.executable_path, "--mcp-mode"],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=None,  # Let stderr go to console for debug output
                text=True,
                env=env,
                bufsize=1,
            )

            # Give wiRedPanda time to start
            time.sleep(0.5)

            # Create initial new circuit
            self.new_circuit()

        except FileNotFoundError as exc:
            raise WiredPandaError(
                f"wiRedPanda executable not found: {self.executable_path}"
            ) from exc
        except Exception as e:
            raise WiredPandaError(f"Failed to start wiRedPanda: {e}") from e

    def stop(self) -> None:
        """Stop wiRedPanda process."""
        if self.process:
            try:
                self.process.terminate()
                self.process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
            finally:
                self.process = None

    def __enter__(self) -> 'WiredPandaBridge':
        """Context manager entry."""
        self.start()
        return self

    def __exit__(
        self,
        exc_type: Optional[type],
        exc_val: Optional[BaseException],
        exc_tb: Optional[Any],
    ) -> None:  # pylint: disable=unused-argument
        """Context manager exit."""
        self.stop()

    @beartype
    def _send_command(
        self, command: str, parameters: Optional[Dict[str, Any]] = None
    ) -> MCPResponse:
        """
        Send a command to wiRedPanda and get response.

        Args:
            command: Command name
            parameters: Command parameters

        Returns:
            Validated MCP response

        Raises:
            WiredPandaError: If command fails
        """
        if not self.process:
            raise WiredPandaError("Bridge not started")
        if not self.process.stdin or not self.process.stdout:
            raise WiredPandaError("Process streams not available")

        # Create and validate command using Pydantic
        try:
            cmd_model = MCPCommand(command=command, parameters=parameters or {})
        except Exception as e:
            raise WiredPandaError(f"Invalid command structure: {e}") from e

        try:
            # Send command
            cmd_json = cmd_model.model_dump_json() + "\n"
            self.process.stdin.write(cmd_json)
            self.process.stdin.flush()

            # Read response
            response_line = self.process.stdout.readline()
            if not response_line:
                raise WiredPandaError("No response received from wiRedPanda")

            # Parse and validate response using Pydantic
            try:
                response_data = json.loads(response_line.strip())
                response = MCPResponse.model_validate(response_data)
            except Exception as e:
                raise WiredPandaError(f"Invalid response format: {e}") from e

            if response.status == "error":
                raise WiredPandaError(
                    f"Command '{command}' failed: {response.error or 'Unknown error'}"
                )

            return response

        except json.JSONDecodeError as e:
            raise WiredPandaError(f"Invalid JSON response: {e}") from e
        except BrokenPipeError as exc:
            raise WiredPandaError("wiRedPanda process terminated unexpectedly") from exc
        except Exception as e:
            raise WiredPandaError(f"Command '{command}' failed: {e}") from e

    # Circuit Management
    @beartype
    def new_circuit(self) -> Dict[str, Any]:
        """Create a new empty circuit."""
        response = self._send_command("new_circuit")
        return response.result or {}

    @beartype
    def save_circuit(self, file_path: str) -> Dict[str, Any]:
        """Save circuit to file."""
        response = self._send_command("save_circuit", {"file_path": file_path})
        return response.result or {}

    @beartype
    def load_circuit(self, file_path: str) -> Dict[str, Any]:
        """Load circuit from file."""
        response = self._send_command("load_circuit", {"file_path": file_path})
        return response.result or {}

    # Element Creation and Management
    @beartype
    def create_element(
        self, element_type: str, x: float, y: float, label: str = ""
    ) -> int:
        """
        Create a circuit element.

        Args:
            element_type: Type of element (And, Or, Not, InputButton, Led, etc.)
            x: X position
            y: Y position
            label: Optional label

        Returns:
            Element ID
        """
        params: Dict[str, Any] = {"type": element_type, "x": x, "y": y}
        if label:
            params["label"] = label

        response = self._send_command("create_element", params)
        if not response.result or "element_id" not in response.result:
            raise WiredPandaError("Invalid response: missing element_id")
        return int(response.result["element_id"])

    @beartype
    def delete_element(self, element_id: int) -> Dict[str, Any]:
        """Delete an element."""
        response = self._send_command("delete_element", {"element_id": element_id})
        return response.result or {}

    @beartype
    def list_elements(self) -> List[Dict[str, Any]]:
        """Get list of all elements in circuit."""
        response = self._send_command("list_elements")
        if not response.result or "elements" not in response.result:
            return []
        return list(response.result["elements"])

    @beartype
    def get_element_info(self, element_id: int) -> Dict[str, Any]:
        """Get detailed information about an element."""
        response = self._send_command(
            "get_element_info", {"element_id": element_id}
        )
        return response.result or {}

    # Connections
    @beartype
    def connect_elements(
        self, source_id: int, source_port: int, target_id: int, target_port: int
    ) -> Dict[str, Any]:
        """Connect two elements."""
        response = self._send_command(
            "connect_elements",
            {
                "source_id": source_id,
                "source_port": source_port,
                "target_id": target_id,
                "target_port": target_port,
            },
        )
        return response.result or {}

    def disconnect_elements(
        self, source_id: int, source_port: int, target_id: int, target_port: int
    ) -> Dict[str, Any]:
        """Disconnect two elements."""
        response = self._send_command(
            "disconnect_elements",
            {
                "source_id": source_id,
                "source_port": source_port,
                "target_id": target_id,
                "target_port": target_port,
            },
        )
        return response.result or {}

    # Simulation Control
    def start_simulation(self) -> Dict[str, Any]:
        """Start circuit simulation."""
        response = self._send_command("simulation_control", {"action": "start"})
        return response.result or {}

    def stop_simulation(self) -> Dict[str, Any]:
        """Stop circuit simulation."""
        response = self._send_command("simulation_control", {"action": "stop"})
        return response.result or {}

    def restart_simulation(self) -> Dict[str, Any]:
        """Restart circuit simulation."""
        response = self._send_command("simulation_control", {"action": "reset"})
        return response.result or {}

    def update_simulation(self) -> Dict[str, Any]:
        """Force a single simulation update with convergence."""
        response = self._send_command("simulation_control", {"action": "update"})
        return response.result or {}

    @beartype
    def set_input_value(self, element_id: int, value: bool) -> Dict[str, Any]:
        """Set value of an input element."""
        response = self._send_command(
            "set_input_value", {"element_id": element_id, "value": value}
        )
        return response.result or {}

    @beartype
    def get_output_value(self, element_id: int) -> bool:
        """Get value of an output element."""
        response = self._send_command(
            "get_output_value", {"element_id": element_id}
        )
        if not response.result or "value" not in response.result:
            raise WiredPandaError("Invalid response: missing value")
        return bool(response.result["value"])

    def export_image(
        self, file_path: str, image_format: str = "png"
    ) -> Dict[str, Any]:
        """
        Export current circuit to image file

        Args:
            file_path: Path where to save the image
            image_format: Format of the image (png, pdf, svg)

        Returns:
            Response dictionary with success/failure status
        """
        response = self._send_command(
            "export_image", {"file_path": file_path, "format": image_format.lower()}
        )
        return response.result or {}

    def export_png(self, file_path: str) -> Dict[str, Any]:
        """Export current circuit to PNG image."""
        return self.export_image(file_path, "png")

    def export_pdf(self, file_path: str) -> Dict[str, Any]:
        """Export current circuit to PDF."""
        return self.export_image(file_path, "pdf")

    def export_svg(self, file_path: str) -> Dict[str, Any]:
        """Export current circuit to SVG."""
        return self.export_image(file_path, "svg")

    # Convenience methods for common operations
    def create_and_gate(self, x: float, y: float, label: str = "") -> int:
        """Create an AND gate."""
        return self.create_element("And", x, y, label)

    def create_or_gate(self, x: float, y: float, label: str = "") -> int:
        """Create an OR gate."""
        return self.create_element("Or", x, y, label)

    def create_not_gate(self, x: float, y: float, label: str = "") -> int:
        """Create a NOT gate."""
        return self.create_element("Not", x, y, label)

    def create_nand_gate(self, x: float, y: float, label: str = "") -> int:
        """Create a NAND gate."""
        return self.create_element("Nand", x, y, label)

    def create_nor_gate(self, x: float, y: float, label: str = "") -> int:
        """Create a NOR gate."""
        return self.create_element("Nor", x, y, label)

    def create_xor_gate(self, x: float, y: float, label: str = "") -> int:
        """Create a XOR gate."""
        return self.create_element("Xor", x, y, label)

    def create_xnor_gate(self, x: float, y: float, label: str = "") -> int:
        """Create a XNOR gate."""
        return self.create_element("Xnor", x, y, label)

    def create_input_button(self, x: float, y: float, label: str = "") -> int:
        """Create an input button."""
        return self.create_element("InputButton", x, y, label)

    def create_led(self, x: float, y: float, label: str = "") -> int:
        """Create an LED."""
        return self.create_element("Led", x, y, label)

    def create_clock(self, x: float, y: float, label: str = "") -> int:
        """Create a clock."""
        return self.create_element("Clock", x, y, label)

    # High-level circuit building methods
    def build_basic_gate_circuit(
        self, gate_type: str, input_count: int = 2
    ) -> Tuple[List[int], int, int]:
        """
        Build a basic gate circuit with inputs and output

        Args:
            gate_type: Type of gate (And, Or, Not, etc.)
            input_count: Number of inputs

        Returns:
            (input_ids, gate_id, led_id)
        """
        input_ids: List[int] = []
        y_start: int = 100

        # Create input buttons
        for i in range(input_count):
            input_id = self.create_input_button(50, y_start + i * 50, f"Input_{i + 1}")
            input_ids.append(input_id)

        # Create gate
        gate_y: int = y_start + (input_count - 1) * 25  # Center the gate
        gate_id: int = self.create_element(gate_type, 150, gate_y, gate_type)

        # Create LED
        led_id: int = self.create_led(250, gate_y, "Output")

        # Connect inputs to gate
        for i, input_id in enumerate(input_ids):
            self.connect_elements(input_id, 0, gate_id, i)

        # Connect gate to LED
        self.connect_elements(gate_id, 0, led_id, 0)

        return input_ids, gate_id, led_id

    @beartype
    def test_truth_table(
        self,
        input_ids: List[int],
        output_id: int,
        truth_table: List[Tuple[List[bool], bool]],
    ) -> TestResults:
        """
        Test a circuit against a truth table.

        Args:
            input_ids: List of input element IDs
            output_id: Output element ID
            truth_table: List of (inputs, expected_output) tuples

        Returns:
            Validated test results
        """
        self.start_simulation()
        time.sleep(0.1)  # Allow simulation to start

        passed_count = 0
        failed_count = 0
        test_details: List[TruthTableTest] = []

        for inputs, expected in truth_table:
            # Set input values
            for i, input_val in enumerate(inputs):
                self.set_input_value(input_ids[i], input_val)

            # Restart simulation to ensure proper propagation
            self.restart_simulation()
            time.sleep(0.1)  # Allow propagation after restart

            # Get output value
            actual: bool = self.get_output_value(output_id)

            # Record result
            test_passed: bool = actual == expected
            if test_passed:
                passed_count += 1
            else:
                failed_count += 1

            test_case = TruthTableTest(
                inputs=inputs,
                expected=expected,
                actual=actual,
                passed=test_passed
            )
            test_details.append(test_case)

        success_rate = (
            float(passed_count) / float(len(truth_table))
            if len(truth_table) > 0
            else 0.0
        )

        return TestResults(
            total_tests=len(truth_table),
            passed=passed_count,
            failed=failed_count,
            details=test_details,
            success_rate=success_rate
        )
