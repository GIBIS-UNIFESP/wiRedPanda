#!/usr/bin/env python3
"""
MCP Protocol Results

This module contains result classes for MCP (Model Context Protocol) responses.
These classes define the structure of data returned by various commands.
"""

from typing import Annotated, Any, List

from beartype import beartype
from pydantic import BaseModel, Field, field_validator

# Import from type_models for circuit data structures
from type_models import CircuitElement, CircuitConnection


class ElementCreationResult(BaseModel):
    """Result data for element creation"""

    element_id: Annotated[int, Field(gt=0)]

    class Config:
        extra = "forbid"


class ElementListResult(BaseModel):
    """Result data for element listing"""

    elements: List[CircuitElement] = Field(description="List of elements")

    class Config:
        extra = "forbid"


class ConnectionListResult(BaseModel):
    """Result data for connection listing"""

    connections: List[CircuitConnection] = Field(description="List of connections")

    class Config:
        extra = "forbid"


class OutputValueResult(BaseModel):
    """Result data for output value queries"""

    value: bool = Field(description="Output value")

    class Config:
        extra = "forbid"


# Test Result Models
class TestResults(BaseModel):
    """Model for test execution results"""

    total: Annotated[int, Field(ge=0)]
    passed: Annotated[int, Field(ge=0)]
    failed: Annotated[int, Field(ge=0)]
    errors: List[str] = Field(default_factory=list, description="List of actual failure error messages")
    known_issues: List[str] = Field(default_factory=list, description="List of documented limitations")
    process_issues: List[str] = Field(default_factory=list, description="List of process/stability issues")

    @field_validator("passed", "failed")
    @classmethod
    def validate_totals(cls, v: int, info: Any) -> int:
        """Ensure passed + failed <= total"""
        if info.data and "total" in info.data:
            total = info.data["total"]
            passed = info.data.get("passed", 0) if info.field_name == "failed" else v
            failed = v if info.field_name == "failed" else info.data.get("failed", 0)
            if passed + failed > total:
                raise ValueError("passed + failed cannot exceed total")
        return v

    class Config:
        extra = "forbid"
