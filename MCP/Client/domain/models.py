#!/usr/bin/env python3
"""
Domain Models

This module contains domain-specific models for circuit elements,
connections, specifications, and test data structures.
"""

from typing import Annotated

from pydantic import BaseModel, ConfigDict, Field, field_validator, model_validator

from ._validators import require_port_or_label
from .enums import ElementType


class ElementInfo(BaseModel):
    """Model for element information returned by list_elements"""

    element_id: Annotated[int, Field(gt=0)]
    type: ElementType
    x: Annotated[float, Field(ge=-2147483648, le=2147483647)]
    y: Annotated[float, Field(ge=-2147483648, le=2147483647)]
    label: str | None = None
    width: float | None = None
    height: float | None = None
    input_size: Annotated[int, Field(ge=0, le=64)] | None = None
    output_size: Annotated[int, Field(ge=0, le=64)] | None = None
    rotation: Annotated[float, Field(ge=-360, le=360)] | None = None
    color: str | None = None
    frequency: Annotated[float, Field(ge=0)] | None = None
    delay: Annotated[float, Field(ge=0)] | None = None
    trigger: str | None = None
    audio: str | None = None
    volume: float | None = None
    locked: bool | None = None

    model_config = ConfigDict(extra="forbid")


class ICInfo(BaseModel):
    """Model for integrated circuit information"""

    name: Annotated[str, Field(min_length=1)]
    description: str | None = None
    input_count: Annotated[int, Field(ge=0)]
    output_count: Annotated[int, Field(ge=1)]

    @field_validator("name")
    @classmethod
    def validate_identifier(cls, v: str) -> str:
        """Reject names containing characters other than letters, digits, underscores, or hyphens."""
        if not v.replace("_", "").replace("-", "").isalnum():
            raise ValueError("Name must contain only letters, numbers, underscores, and hyphens")
        return v

    model_config = ConfigDict(extra="forbid")


class ElementSpec(BaseModel):
    """Model for element specification in circuit creation"""

    id: str
    type: ElementType
    x: Annotated[float, Field(ge=-2147483648, le=2147483647)]
    y: Annotated[float, Field(ge=-2147483648, le=2147483647)]
    label: str | None = None

    @field_validator("id")
    @classmethod
    def validate_id(cls, v: str) -> str:
        """Reject element IDs containing characters other than letters, digits, or underscores."""
        if not v.replace("_", "").isalnum():
            raise ValueError("Element ID must contain only letters, numbers, and underscores")
        return v


class ConnectionSpec(BaseModel):
    """Model for connection specification in circuit creation

    Ports can be specified by index (source_port/target_port) or by label
    (source_port_label/target_port_label). At least one of each pair must be provided.
    """

    source: str
    target: str
    source_port: Annotated[int | None, Field(ge=0, le=63)] = None
    source_port_label: Annotated[str | None, Field(min_length=1)] = None
    target_port: Annotated[int | None, Field(ge=0, le=63)] = None
    target_port_label: Annotated[str | None, Field(min_length=1)] = None

    @model_validator(mode="after")
    def validate_ports(self) -> "ConnectionSpec":
        """Require at least one of (port index, port label) on each side of the connection."""
        require_port_or_label(self.source_port, self.source_port_label, self.target_port, self.target_port_label)
        return self


class CircuitSpecification(BaseModel):
    """Model for complete circuit specification"""

    elements: list[ElementSpec]
    connections: list[ConnectionSpec]

    @model_validator(mode="after")
    def validate_unique_ids(self) -> "CircuitSpecification":
        """Require element IDs to be unique and every connection endpoint to reference an existing element."""
        ids = [elem.id for elem in self.elements]
        if len(ids) != len(set(ids)):
            raise ValueError("Element IDs must be unique")

        # Validate connection references
        element_ids = set(ids)
        for conn in self.connections:
            if conn.source not in element_ids:
                raise ValueError(f"Connection source '{conn.source}' not found in elements")
            if conn.target not in element_ids:
                raise ValueError(f"Connection target '{conn.target}' not found in elements")

        return self


class TruthTableEntry(BaseModel):
    """Model for truth table entries in testing"""

    inputs: list[Annotated[int, Field(ge=0, le=1)]]
    outputs: list[Annotated[int, Field(ge=0, le=1)]]
    description: str | None = None


class StateTableEntry(BaseModel):
    """Model for state table entries in sequential circuit testing"""

    inputs: list[bool]
    clock_edge: Annotated[str, Field(pattern=r"^(rising|falling|level)$")] = "rising"
    expected_outputs: list[bool]
    next_state: str | None = None
    description: str | None = None
