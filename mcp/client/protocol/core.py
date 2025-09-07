#!/usr/bin/env python3
"""
MCP Protocol Core Classes

This module contains the base protocol classes for MCP (Model Context Protocol).
These classes define the fundamental structure of commands and responses.
"""

from typing import Annotated, Any, Dict, Optional

from beartype import beartype
from pydantic import BaseModel, Field, field_validator


class MCPCommand(BaseModel):
    """Base model for all MCP commands (JSON-RPC 2.0 format)"""

    jsonrpc: str = Field(default="2.0", description="JSON-RPC version")
    method: Annotated[str, Field(min_length=1)]
    params: Any
    id: Annotated[int, Field(ge=1)] = Field(description="Request ID")

    class Config:
        extra = "forbid"


class MCPResponse(BaseModel):
    """Base model for all MCP responses (JSON-RPC 2.0 format)"""

    jsonrpc: str = Field(default="2.0", description="JSON-RPC version")
    result: Optional[Dict[str, Any]] = Field(default=None, description="Command result data")
    error: Optional[Dict[str, Any]] = Field(default=None, description="Error object with code and message")
    id: Optional[int] = Field(default=None, description="Request ID")

    class Config:
        extra = "forbid"

    @field_validator("result", "error")
    @classmethod
    def validate_result_error_mutual_exclusion(cls, v: Any, info: Any) -> Any:
        """Ensure result and error fields are mutually exclusive"""
        field_name = info.field_name
        other_field = "error" if field_name == "result" else "result"
        other_value = info.data.get(other_field) if info.data else None

        if v is not None and other_value is not None:
            raise ValueError(f"Cannot have both {field_name} and {other_field} in JSON-RPC 2.0 response")
        return v

    @property
    def success(self) -> bool:
        """Computed success property - True if no error present"""
        return self.error is None

    @property
    def message(self) -> Optional[str]:
        """Computed message property - extracts message from error object"""
        if self.error:
            return self.error.get("message", "")
        return None
