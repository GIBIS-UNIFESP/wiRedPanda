#!/usr/bin/env python3
"""
Domain Models

This module contains domain-specific models for circuit elements,
connections, specifications, and test data structures.
"""

from typing import Annotated, List, Optional

from pydantic import BaseModel, ConfigDict, Field, field_validator, model_validator

from .enums import ElementType


class ElementInfo(BaseModel):
    """Model for element information returned by list_elements"""

    element_id: Annotated[int, Field(gt=0)]
    type: ElementType
    x: Annotated[float, Field(ge=-2147483648, le=2147483647)]
    y: Annotated[float, Field(ge=-2147483648, le=2147483647)]
    label: Optional[str] = None
    width: Optional[float] = None
    height: Optional[float] = None
    input_size: Optional[Annotated[int, Field(ge=0, le=64)]] = None
    output_size: Optional[Annotated[int, Field(ge=0, le=64)]] = None
    rotation: Optional[Annotated[float, Field(ge=-360, le=360)]] = None
    color: Optional[str] = None
    frequency: Optional[Annotated[float, Field(ge=0)]] = None
    delay: Optional[Annotated[float, Field(ge=0)]] = None
    trigger: Optional[str] = None
    audio: Optional[str] = None
    volume: Optional[float] = None
    locked: Optional[bool] = None

    model_config = ConfigDict(extra="forbid")


class ICInfo(BaseModel):
    """Model for integrated circuit information"""

    name: Annotated[str, Field(min_length=1)]
    description: Optional[str] = None
    input_count: Annotated[int, Field(ge=0)]
    output_count: Annotated[int, Field(ge=1)]

    @field_validator("name")
    @classmethod
    def validate_identifier(cls, v: str) -> str:
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
    label: Optional[str] = None

    @field_validator("id")
    @classmethod
    def validate_id(cls, v: str) -> str:
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
    source_port: Annotated[Optional[int], Field(ge=0, le=63)] = None
    source_port_label: Annotated[Optional[str], Field(min_length=1)] = None
    target_port: Annotated[Optional[int], Field(ge=0, le=63)] = None
    target_port_label: Annotated[Optional[str], Field(min_length=1)] = None

    @model_validator(mode="after")
    def validate_ports(self) -> "ConnectionSpec":
        if self.source_port is None and self.source_port_label is None:
            raise ValueError("Either source_port or source_port_label must be provided")
        if self.target_port is None and self.target_port_label is None:
            raise ValueError("Either target_port or target_port_label must be provided")
        return self


class CircuitSpecification(BaseModel):
    """Model for complete circuit specification"""

    elements: List[ElementSpec]
    connections: List[ConnectionSpec]

    @model_validator(mode="after")
    def validate_unique_ids(self) -> "CircuitSpecification":
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

    inputs: List[Annotated[int, Field(ge=0, le=1)]]
    outputs: List[Annotated[int, Field(ge=0, le=1)]]
    description: Optional[str] = None


class StateTableEntry(BaseModel):
    """Model for state table entries in sequential circuit testing"""

    inputs: List[bool]
    clock_edge: Annotated[str, Field(pattern=r"^(rising|falling|level)$")] = "rising"
    expected_outputs: List[bool]
    next_state: Optional[str] = None
    description: Optional[str] = None
