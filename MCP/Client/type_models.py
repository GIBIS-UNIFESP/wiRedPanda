#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Additional Type Models for MCP Server

Specific type definitions to replace Any types with proper typing.
These models represent common data structures used throughout the MCP system.
"""

from typing import Annotated, Any, Dict, List, Optional, Protocol, runtime_checkable

from pydantic import BaseModel, ConfigDict, Field

# ==================== CIRCUIT SPECIFICATION TYPES ====================


class CircuitElement(BaseModel):
    """Represents a circuit element in specifications"""

    id: int = Field(description="Element identifier")
    type: str = Field(description="Element type name")
    x: float = Field(description="X coordinate")
    y: float = Field(description="Y coordinate")
    label: Optional[str] = Field(default=None, description="Optional element label")

    model_config = ConfigDict(extra="forbid", frozen=True)


class CircuitConnection(BaseModel):
    """Represents a connection between circuit elements

    Ports can be specified by index (source_port/target_port) or by label
    (source_port_label/target_port_label). At least one of each pair must be provided.
    """

    source: int = Field(description="Source element ID")
    target: int = Field(description="Target element ID")
    source_port: Annotated[Optional[int], Field(ge=0, description="Source port index")] = None
    source_port_label: Annotated[Optional[str], Field(min_length=1, description="Source port label")] = None
    target_port: Annotated[Optional[int], Field(ge=0, description="Target port index")] = None
    target_port_label: Annotated[Optional[str], Field(min_length=1, description="Target port label")] = None

    model_config = ConfigDict(extra="forbid", frozen=True)


class TimingTestCase(BaseModel):
    """Test case for timing validation"""

    operation: str = Field(description="Operation being tested")
    inputs: List[bool] = Field(description="Input values")
    delay_ms: Annotated[float, Field(ge=0.0, description="Expected delay in milliseconds")]
    tolerance_ms: Annotated[float, Field(ge=0.0, description="Allowed timing tolerance")]

    model_config = ConfigDict(extra="forbid", frozen=True)


# ==================== VALIDATION RESULT TYPES ====================


class ValidationResult(BaseModel):
    """Result of a validation operation"""

    valid: bool = Field(description="Whether validation passed")
    errors: List[str] = Field(default_factory=list, description="Validation error messages")
    warnings: List[str] = Field(default_factory=list, description="Validation warnings")
    details: Optional[str] = Field(default=None, description="Additional result details")

    model_config = ConfigDict(extra="forbid")


class SchemaValidationResult(BaseModel):
    """Result of JSON schema validation"""

    json_schema_valid: bool = Field(description="JSON schema validation result")
    pydantic_valid: bool = Field(description="Pydantic validation result")
    errors: List[str] = Field(default_factory=list, description="Validation errors")
    warnings: List[str] = Field(default_factory=list, description="Validation warnings")
    summary: str = Field(description="Validation summary message")

    model_config = ConfigDict(extra="forbid")


# ==================== GENERIC TYPE PROTOCOLS ====================


@runtime_checkable
class TestRunnable(Protocol):
    """Protocol for objects that can run tests"""

    async def run_test_method(self, test_method: Any) -> bool:
        """Run a single test method"""
        ...


@runtime_checkable
class CommandSender(Protocol):
    """Protocol for objects that can send MCP commands"""

    async def send_command(self, command: str, parameters: Dict[str, Any]) -> Any:
        """Send an MCP command (internally converts to JSON-RPC 2.0 format)"""
        ...


# ==================== PERFORMANCE BENCHMARK TYPES ====================


class BenchmarkResult(BaseModel):
    """Result of a performance benchmark"""

    avg_time: Annotated[float, Field(ge=0.0, description="Average execution time in seconds")]
    max_time: Annotated[float, Field(ge=0.0, description="Maximum execution time in seconds")]
    min_time: Annotated[float, Field(ge=0.0, description="Minimum execution time in seconds")]
    iterations: Annotated[int, Field(ge=1, description="Number of iterations executed")]
    success: bool = Field(description="Whether benchmark passed threshold")
    threshold: Annotated[float, Field(ge=0.0, description="Performance threshold used")]

    model_config = ConfigDict(extra="forbid")


class TimingMeasurement(BaseModel):
    """Single timing measurement"""

    operation_name: str = Field(description="Name of the timed operation")
    elapsed_seconds: Annotated[float, Field(ge=0.0, description="Elapsed time in seconds")]
    threshold_seconds: Annotated[float, Field(ge=0.0, description="Performance threshold")]
    passed: bool = Field(description="Whether measurement passed threshold")

    model_config = ConfigDict(extra="forbid")


# ==================== COMMAND PARAMETER TYPES ====================


class EmptyParameters(BaseModel):
    """Empty parameters for commands that don't require parameters"""

    model_config = ConfigDict(extra="forbid", frozen=True)

    @classmethod
    def create(cls) -> "EmptyParameters":
        """Factory method to create empty parameters"""
        return cls()


# ==================== RESPONSE RESULT TYPES ====================


class ElementCreationResult(BaseModel):
    """Result for element creation commands"""

    element_id: Annotated[int, Field(gt=0, description="Created element ID")]

    model_config = ConfigDict(extra="forbid", frozen=True)


class ConnectionCreationResult(BaseModel):
    """Result for connection creation commands"""

    connection_id: Annotated[int, Field(gt=0, description="Created connection ID")]

    model_config = ConfigDict(extra="forbid", frozen=True)


class OutputValueResult(BaseModel):
    """Result for output value queries"""

    value: bool = Field(description="Output value")

    model_config = ConfigDict(extra="forbid", frozen=True)


class TabCountResult(BaseModel):
    """Result for tab count queries"""

    count: Annotated[int, Field(ge=0, description="Number of open tabs")]

    model_config = ConfigDict(extra="forbid", frozen=True)


