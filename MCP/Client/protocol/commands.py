#!/usr/bin/env python3
"""
MCP Protocol Commands

This module contains all command classes for MCP (Model Context Protocol).
These classes define the structure and validation rules for all available commands.
"""

from typing import Annotated, Dict, List, Optional

from beartype import beartype

# Import enums from domain package
from domain.enums import ElementType, ImageExportFormat, SimulationAction, WaveformExportFormat
from pydantic import BaseModel, ConfigDict, Field, field_validator, model_validator
from type_models import EmptyParameters

from .core import MCPCommand


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

        @field_validator("label")
        @classmethod
        def validate_label(cls, v: Optional[str]) -> Optional[str]:
            """Validate label contains only printable characters"""
            if v is not None:
                if not all(c.isprintable() or c.isspace() for c in v):
                    raise ValueError("Label must contain only printable characters")
                if v.strip() == "":
                    return None  # Empty label becomes None
            return v

        @field_validator("x", "y")
        @classmethod
        def validate_coordinates(cls, v: float) -> float:
            """Validate coordinates are not NaN or infinite"""
            import math

            if not math.isfinite(v):
                raise ValueError("Coordinates must be finite numbers")
            return v

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class DeleteElementCommand(MCPCommand):
    """Model for delete_element command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class ConnectElementsCommand(MCPCommand):
    """Model for connect_elements command

    Ports can be specified by index (source_port/target_port) or by label
    (source_port_label/target_port_label). At least one of each pair must be provided.
    """

    class Parameters(BaseModel):
        source_id: Annotated[int, Field(gt=0, description="Source element ID")]
        source_port: Annotated[Optional[int], Field(ge=0, le=63, description="Source output port index")] = None
        source_port_label: Annotated[Optional[str], Field(min_length=1, description="Source output port label")] = None
        target_id: Annotated[int, Field(gt=0, description="Target element ID")]
        target_port: Annotated[Optional[int], Field(ge=0, le=63, description="Target input port index")] = None
        target_port_label: Annotated[Optional[str], Field(min_length=1, description="Target input port label")] = None

        @model_validator(mode="after")
        def validate_ports_and_self_connection(self) -> "ConnectElementsCommand.Parameters":
            """Ensure valid port specification and no self-connection"""
            if self.source_id == self.target_id:
                raise ValueError("Element cannot connect to itself")
            if self.source_port is None and self.source_port_label is None:
                raise ValueError("Either source_port or source_port_label must be provided")
            if self.target_port is None and self.target_port_label is None:
                raise ValueError("Either target_port or target_port_label must be provided")
            return self

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class DisconnectElementsCommand(MCPCommand):
    """Model for disconnect_elements command"""

    class Parameters(BaseModel):
        source_id: Annotated[int, Field(gt=0)]
        target_id: Annotated[int, Field(gt=0)]

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class SimulationControlCommand(MCPCommand):
    """Model for simulation_control command"""

    class Parameters(BaseModel):
        action: SimulationAction = Field(description="Simulation action to perform")

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class SetInputValueCommand(MCPCommand):
    """Model for set_input_value command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        value: bool = Field(description="Input value to set")

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class GetOutputValueCommand(MCPCommand):
    """Model for get_output_value command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]

        model_config = ConfigDict(extra="forbid")

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

        model_config = ConfigDict(extra="forbid")

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

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class ExportImageCommand(MCPCommand):
    """Model for export_image command"""

    class Parameters(BaseModel):
        filename: Annotated[str, Field(min_length=1)]
        format: ImageExportFormat = Field(description="Export format")
        padding: Optional[Annotated[int, Field(ge=0, description="Padding in pixels")]] = 20

        model_config = ConfigDict(extra="forbid")

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

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class ExportWaveformCommand(MCPCommand):
    """Model for export_waveform command"""

    class Parameters(BaseModel):
        filename: Annotated[str, Field(min_length=1)]
        format: WaveformExportFormat = Field(description="Export format")

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class CreateIcCommand(MCPCommand):
    """Model for create_ic command"""

    class Parameters(BaseModel):
        name: Annotated[str, Field(min_length=1)]
        description: Optional[str] = Field(default=None, description="Optional IC description")

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class InstantiateIcCommand(MCPCommand):
    """Model for instantiate_ic command"""

    class Parameters(BaseModel):
        ic_name: Annotated[str, Field(min_length=1)]
        x: Annotated[float, Field(ge=-2147483648, le=2147483647, description="X coordinate")]
        y: Annotated[float, Field(ge=-2147483648, le=2147483647, description="Y coordinate")]
        label: Optional[str] = Field(default=None, description="Optional instance label")

        model_config = ConfigDict(extra="forbid")

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

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class SetElementPropertiesCommand(MCPCommand):
    """Model for set_element_properties command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        label: Optional[str] = Field(default=None)
        color: Optional[str] = Field(default=None)
        frequency: Optional[Annotated[float, Field(ge=0)]] = None
        rotation: Optional[float] = Field(default=None)
        wireless_mode: Optional[Annotated[int, Field(ge=0, le=2)]] = Field(
            default=None, description="Wireless mode: 0=None, 1=Tx, 2=Rx (Node elements only)"
        )

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class RotateElementCommand(MCPCommand):
    """Model for rotate_element command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        angle: int = Field(description="Rotation angle in degrees")

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class FlipElementCommand(MCPCommand):
    """Model for flip_element command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        axis: Annotated[int, Field(ge=0, le=1, description="0=horizontal, 1=vertical")]

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class UpdateElementCommand(MCPCommand):
    """Model for update_element command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        label: Optional[str] = Field(default=None)
        color: Optional[str] = Field(default=None)
        frequency: Optional[Annotated[float, Field(ge=0)]] = None
        rotation: Optional[float] = Field(default=None)

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class ChangeInputSizeCommand(MCPCommand):
    """Model for change_input_size command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        size: Annotated[int, Field(gt=0, description="New input port count")]

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class ChangeOutputSizeCommand(MCPCommand):
    """Model for change_output_size command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        size: Annotated[int, Field(gt=0, description="New output port count")]

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class ToggleTruthTableOutputCommand(MCPCommand):
    """Model for toggle_truth_table_output command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        position: Annotated[int, Field(ge=0, description="Truth table output position to toggle")]

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class MorphElementCommand(MCPCommand):
    """Model for morph_element command"""

    class Parameters(BaseModel):
        element_ids: List[Annotated[int, Field(gt=0)]] = Field(
            min_length=1, description="Element IDs to morph"
        )
        target_type: str = Field(min_length=1, description="Target element type")

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class SplitConnectionCommand(MCPCommand):
    """Model for split_connection command"""

    class Parameters(BaseModel):
        source_id: Annotated[int, Field(gt=0)]
        source_port: Annotated[int, Field(ge=0)]
        target_id: Annotated[int, Field(gt=0)]
        target_port: Annotated[int, Field(ge=0)]
        x: float = Field(description="X coordinate for split point")
        y: float = Field(description="Y coordinate for split point")

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class UndoCommand(MCPCommand):
    """Model for undo command"""

    params: EmptyParameters = Field(default_factory=EmptyParameters)


class RedoCommand(MCPCommand):
    """Model for redo command"""

    params: EmptyParameters = Field(default_factory=EmptyParameters)


class GetUndoStackCommand(MCPCommand):
    """Model for get_undo_stack command"""

    params: EmptyParameters = Field(default_factory=EmptyParameters)


class GetServerInfoCommand(MCPCommand):
    """Model for get_server_info command"""

    params: EmptyParameters = Field(default_factory=EmptyParameters)
