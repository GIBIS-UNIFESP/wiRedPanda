#!/usr/bin/env python3
"""
MCP Protocol Commands

This module contains all command classes for MCP (Model Context Protocol).
These classes define the structure and validation rules for all available commands.
"""

import math
from typing import Annotated

from pydantic import BaseModel, ConfigDict, Field, field_validator, model_validator

from domain._validators import require_port_or_label

# Import enums from domain package
from domain.enums import (
    ElementType,
    ImageExportFormat,
    SimulationAction,
    WaveformExportFormat,
)

from .core import MCPCommand
from .type_models import EmptyParameters


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
        label: str | None = Field(default=None, description="Element label")

        @field_validator("label")
        @classmethod
        def validate_label(cls, v: str | None) -> str | None:
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
        source_port: Annotated[int | None, Field(ge=0, le=63, description="Source output port index")] = None
        source_port_label: Annotated[str | None, Field(min_length=1, description="Source output port label")] = None
        target_id: Annotated[int, Field(gt=0, description="Target element ID")]
        target_port: Annotated[int | None, Field(ge=0, le=63, description="Target input port index")] = None
        target_port_label: Annotated[str | None, Field(min_length=1, description="Target input port label")] = None

        @model_validator(mode="after")
        def validate_ports_and_self_connection(self) -> "ConnectElementsCommand.Parameters":
            """Ensure valid port specification and no self-connection"""
            if self.source_id == self.target_id:
                raise ValueError("Element cannot connect to itself")
            require_port_or_label(self.source_port, self.source_port_label, self.target_port, self.target_port_label)
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
        time_per_tick: Annotated[int, Field(ge=0, le=1_000_000)] | None = Field(
            default=None,
            description="ns of sim-time per update (0=functional, >0=temporal); required with "
            "action=set_time_per_tick. Capped at 1,000,000 ns (1 ms), matching the "
            "propagation-delay bound.",
        )

        @model_validator(mode="after")
        def require_time_per_tick_for_set_action(self) -> "SimulationControlCommand.Parameters":
            """time_per_tick is required (not just optional) when action=set_time_per_tick — an
            omitted value would otherwise silently default to 0 (functional mode) server-side."""
            if self.action == SimulationAction.SET_TIME_PER_TICK and self.time_per_tick is None:
                raise ValueError("time_per_tick is required when action=set_time_per_tick")
            return self

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
        padding: Annotated[int, Field(ge=0, description="Padding in pixels")] | None = 20

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class CreateWaveformCommand(MCPCommand):
    """Model for create_waveform command"""

    class Parameters(BaseModel):
        elements: list[Annotated[int, Field(gt=0)]] | None = Field(
            default=None, description="Optional list of element IDs to analyze"
        )
        duration: Annotated[int, Field(ge=1, le=1024)] | None = Field(
            default=64, description="Number of simulation steps"
        )
        input_patterns: dict[str, list[Annotated[int, Field(ge=0, le=1)]]] | None = Field(
            default=None, description="Input patterns for specific input elements"
        )
        temporal: bool | None = Field(
            default=None, description="Sweep with per-element propagation delays (column-lag)"
        )
        ns_per_column: Annotated[int, Field(ge=1, le=1_000_000)] | None = Field(
            default=None, description="Sim-time (ns) advanced per waveform column in temporal mode"
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


class WatchSignalCommand(MCPCommand):
    """Model for watch_signal command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        port: Annotated[int, Field(ge=0)] | None = Field(default=None, description="Output port index (default 0)")
        name: str | None = Field(default=None, description="Display name (defaults to the element label/type)")

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class ClearWatchedSignalsCommand(MCPCommand):
    """Model for clear_watched_signals command"""

    params: EmptyParameters = Field(default_factory=EmptyParameters)


class GetWaveformTraceCommand(MCPCommand):
    """Model for get_waveform_trace command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)] | None = Field(
            default=None, description="Filter to this element (optional)"
        )
        port: Annotated[int, Field(ge=0)] | None = Field(
            default=None, description="Filter to this output port (optional)"
        )

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class CreateIcCommand(MCPCommand):
    """Model for create_ic command"""

    class Parameters(BaseModel):
        name: Annotated[str, Field(min_length=1)]
        description: str | None = Field(default=None, description="Optional IC description")

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class InstantiateIcCommand(MCPCommand):
    """Model for instantiate_ic command"""

    class Parameters(BaseModel):
        ic_name: Annotated[str, Field(min_length=1)]
        x: Annotated[float, Field(ge=-2147483648, le=2147483647, description="X coordinate")]
        y: Annotated[float, Field(ge=-2147483648, le=2147483647, description="Y coordinate")]
        label: str | None = Field(default=None, description="Optional instance label")
        inline: bool | None = Field(default=False, description="Create as embedded IC")
        blob_name: str | None = Field(default=None, description="Custom blob name for inline mode")

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class EmbedIcCommand(MCPCommand):
    """Model for embed_ic command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        blob_name: str | None = Field(default=None, description="Optional blob name override")

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class ExtractIcCommand(MCPCommand):
    """Model for extract_ic command"""

    class Parameters(BaseModel):
        blob_name: Annotated[str, Field(min_length=1)]
        file_name: str | None = Field(default=None, description="Optional output file path")
        overwrite: bool | None = Field(default=False, description="Overwrite existing file")

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
        label: str | None = Field(default=None)
        color: str | None = Field(default=None)
        frequency: Annotated[float, Field(ge=0)] | None = None
        rotation: float | None = Field(default=None)
        delay: Annotated[float, Field(ge=0)] | None = None
        propagation_delay: Annotated[int, Field(ge=0, le=1_000_000)] | None = Field(
            default=None,
            description="Inertial propagation delay in ns for temporal simulation. Capped at "
            "1,000,000 ns (1 ms), matching the native setPropagationDelay() bound.",
        )
        trigger: str | None = Field(default=None, description="Keyboard shortcut for input elements")
        audio: str | None = Field(default=None)
        locked: bool | None = Field(default=None, description="Lock state for input elements")
        volume: float | None = Field(default=None, description="Volume for audio elements")
        appearance: str | None = Field(default=None, description="Appearance path or 'default'")
        appearance_index: Annotated[int, Field(ge=0)] | None = Field(
            default=None, description="State index for multi-state element appearances"
        )
        wireless_mode: Annotated[int, Field(ge=0, le=2)] | None = Field(
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
        label: str | None = Field(default=None)
        color: str | None = Field(default=None)
        frequency: Annotated[float, Field(ge=0)] | None = None
        rotation: float | None = Field(default=None)
        propagation_delay: Annotated[int, Field(ge=0, le=1_000_000)] | None = Field(
            default=None,
            description="Inertial propagation delay in ns for temporal simulation. Capped at "
            "1,000,000 ns (1 ms), matching the native setPropagationDelay() bound.",
        )

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class ChangeInputSizeCommand(MCPCommand):
    """Model for change_input_size command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        # Global ceiling matching schema-mcp.json; per-element bounds are
        # enforced by the server.
        size: Annotated[int, Field(gt=0, le=256, description="New input port count")]

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class ChangeOutputSizeCommand(MCPCommand):
    """Model for change_output_size command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        size: Annotated[int, Field(gt=0, le=256, description="New output port count")]

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class ToggleTruthTableOutputCommand(MCPCommand):
    """Model for toggle_truth_table_output command"""

    class Parameters(BaseModel):
        element_id: Annotated[int, Field(gt=0)]
        # The truth-table key holds exactly 2048 bits (256 rows x 8 outputs).
        position: Annotated[int, Field(ge=0, le=2047, description="Truth table output position to toggle")]

        model_config = ConfigDict(extra="forbid")

    params: Parameters


class MorphElementCommand(MCPCommand):
    """Model for morph_element command"""

    class Parameters(BaseModel):
        element_ids: list[Annotated[int, Field(gt=0)]] = Field(min_length=1, description="Element IDs to morph")
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
