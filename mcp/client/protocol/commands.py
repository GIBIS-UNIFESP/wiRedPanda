#!/usr/bin/env python3
"""
MCP Protocol Commands

This module contains all command classes for MCP (Model Context Protocol).
These classes define the structure and validation rules for all available commands.
"""

from typing import Annotated, Dict, List, Optional

from beartype import beartype
from pydantic import BaseModel, Field, field_validator, model_validator

from .core import MCPCommand
from type_models import EmptyParameters

# Import enums from domain package
from domain.enums import ElementType, SimulationAction, ImageExportFormat, WaveformExportFormat


# Specific Command Models
class NewCircuitCommand(MCPCommand):
    """Model for new_circuit command"""

    params: EmptyParameters = Field(default_factory=EmptyParameters)


class CloseCircuitCommand(MCPCommand):
    """Model for close_circuit command"""

    params: EmptyParameters = Field(default_factory=EmptyParameters)


class GetTabCountCommand(MCPCommand):
    """Model for get_tab_count command"""

    params: EmptyParameters = Field(default_factory=EmptyParameters)


class CreateElementCommand(MCPCommand):
    """Model for create_element command"""

    class Parameters(BaseModel):
        type: ElementType = Field(description="Type of element to create")
        x: Annotated[float, Field(ge=-2147483648, le=2147483647, description="X coordinate")]
        y: Annotated[float, Field(ge=-2147483648, le=2147483647, description="Y coordinate")]
        label: Optional[str] = Field(default=None, description="Element label")

        @field_validator('label')
        @classmethod
        def validate_label(cls, v: Optional[str]) -> Optional[str]:
            """Validate label contains only printable characters"""
            if v is not None:
                if not all(c.isprintable() or c.isspace() for c in v):
                    raise ValueError("Label must contain only printable characters")
                if v.strip() == "":
                    return None  # Empty label becomes None
            return v

        @field_validator('x', 'y')
        @classmethod
        def validate_coordinates(cls, v: float) -> float:
            """Validate coordinates are not NaN or infinite"""
            import math
            if not math.isfinite(v):
                raise ValueError("Coordinates must be finite numbers")
            return v

        class Config:
            extra = "forbid"

    params: Parameters


class DeleteElementCommand(MCPCommand):
    """Model for delete_element command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]

        class Config:
            extra = "forbid"

    params: Parameters


class ConnectElementsCommand(MCPCommand):
    """Model for connect_elements command"""

    class Parameters(BaseModel):
        source_id: Annotated[int, Field(gt=0, description="Source element ID")]
        source_port: Annotated[int, Field(ge=0, le=63, description="Source output port")]
        target_id: Annotated[int, Field(gt=0, description="Target element ID")]
        target_port: Annotated[int, Field(ge=0, le=63, description="Target input port")]

        @model_validator(mode='after')
        def validate_no_self_connection(self) -> 'ConnectElementsCommand.Parameters':
            """Ensure element cannot connect to itself"""
            if self.source_id == self.target_id:
                raise ValueError("Element cannot connect to itself")
            return self

        class Config:
            extra = "forbid"

    params: Parameters


class DisconnectElementsCommand(MCPCommand):
    """Model for disconnect_elements command"""

    class Parameters(BaseModel):
        source_id: Annotated[int, Field(gt=0)]
        target_id: Annotated[int, Field(gt=0)]

        class Config:
            extra = "forbid"

    params: Parameters


class SimulationControlCommand(MCPCommand):
    """Model for simulation_control command"""

    class Parameters(BaseModel):
        action: SimulationAction = Field(description="Simulation action to perform")

        class Config:
            extra = "forbid"

    params: Parameters


class SetInputValueCommand(MCPCommand):
    """Model for set_input_value command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        value: bool = Field(description="Input value to set")

        class Config:
            extra = "forbid"

    params: Parameters


class GetOutputValueCommand(MCPCommand):
    """Model for get_output_value command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]

        class Config:
            extra = "forbid"

    params: Parameters


class ListConnectionsCommand(MCPCommand):
    """Model for list_connections command"""

    params: EmptyParameters = Field(default_factory=EmptyParameters)


class ListElementsCommand(MCPCommand):
    """Model for list_elements command"""

    params: EmptyParameters = Field(default_factory=EmptyParameters)


class SaveCircuitCommand(MCPCommand):
    """Model for save_circuit command"""

    class Parameters(BaseModel):
        filename: Annotated[str, Field(min_length=1)]

        @field_validator("filename")
        @classmethod
        def validate_extension(cls, v: str) -> str:
            """Ensure filename has .panda extension"""
            if not v.endswith(".panda"):
                raise ValueError("Filename must have .panda extension")
            return v

        class Config:
            extra = "forbid"

    params: Parameters


class LoadCircuitCommand(MCPCommand):
    """Model for load_circuit command"""

    class Parameters(BaseModel):
        filename: Annotated[str, Field(min_length=1)]

        @field_validator("filename")
        @classmethod
        def validate_extension(cls, v: str) -> str:
            """Ensure filename has .panda extension"""
            if not v.endswith(".panda"):
                raise ValueError("Filename must have .panda extension")
            return v

        class Config:
            extra = "forbid"

    params: Parameters


class ExportImageCommand(MCPCommand):
    """Model for export_image command"""

    class Parameters(BaseModel):
        filename: Annotated[str, Field(min_length=1)]
        format: ImageExportFormat = Field(description="Export format")
        padding: Optional[Annotated[int, Field(ge=0, description="Padding in pixels")]] = 20

        class Config:
            extra = "forbid"

    params: Parameters


class CreateWaveformCommand(MCPCommand):
    """Model for create_waveform command"""

    class Parameters(BaseModel):
        elements: Optional[List[Annotated[int, Field(gt=0)]]] = Field(
            default=None, description="Optional list of element IDs to analyze"
        )
        duration: Optional[Annotated[int, Field(ge=1, le=1024)]] = Field(
            default=64, description="Number of simulation steps"
        )
        input_patterns: Optional[Dict[str, List[Annotated[int, Field(ge=0, le=1)]]]] = Field(
            default=None, description="Input patterns for specific input elements"
        )

        class Config:
            extra = "forbid"

    params: Parameters


class ExportWaveformCommand(MCPCommand):
    """Model for export_waveform command"""

    class Parameters(BaseModel):
        filename: Annotated[str, Field(min_length=1)]
        format: WaveformExportFormat = Field(description="Export format")

        class Config:
            extra = "forbid"

    params: Parameters


class CreateIcCommand(MCPCommand):
    """Model for create_ic command"""

    class Parameters(BaseModel):
        name: Annotated[str, Field(min_length=1)]
        description: Optional[str] = Field(default=None, description="Optional IC description")

        class Config:
            extra = "forbid"

    params: Parameters


class InstantiateIcCommand(MCPCommand):
    """Model for instantiate_ic command"""

    class Parameters(BaseModel):
        ic_name: Annotated[str, Field(min_length=1)]
        x: Annotated[float, Field(ge=-2147483648, le=2147483647, description="X coordinate")]
        y: Annotated[float, Field(ge=-2147483648, le=2147483647, description="Y coordinate")]
        label: Optional[str] = Field(default=None, description="Optional instance label")

        class Config:
            extra = "forbid"

    params: Parameters


class ListIcsCommand(MCPCommand):
    """Model for list_ics command"""

    params: EmptyParameters = Field(default_factory=EmptyParameters)


class MoveElementCommand(MCPCommand):
    """Model for move_element command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        x: Annotated[float, Field(ge=-2147483648, le=2147483647, description="New X coordinate")]
        y: Annotated[float, Field(ge=-2147483648, le=2147483647, description="New Y coordinate")]

        class Config:
            extra = "forbid"

    params: Parameters


class SetElementPropertiesCommand(MCPCommand):
    """Model for set_element_properties command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        label: Optional[str] = Field(default=None)
        input_size: Optional[Annotated[int, Field(gt=0)]] = Field(default=None)
        output_size: Optional[Annotated[int, Field(gt=0)]] = Field(default=None)
        color: Optional[str] = Field(default=None)
        frequency: Optional[Annotated[float, Field(ge=0)]] = None
        rotation: Optional[float] = Field(default=None)

        class Config:
            extra = "forbid"

    params: Parameters
