#!/usr/bin/env python3
"""
MCP Protocol Layer

This package contains the protocol-level definitions for MCP (Model Context Protocol).
It defines the communication contracts, command structures, and response formats.
"""

# Re-export core protocol classes
from .core import MCPCommand, MCPResponse

# Re-export enums from domain package
from domain.enums import ElementType, SimulationAction, ImageExportFormat, WaveformExportFormat

# Re-export all command classes
from .commands import (
    NewCircuitCommand,
    CloseCircuitCommand,
    GetTabCountCommand,
    CreateElementCommand,
    DeleteElementCommand,
    ConnectElementsCommand,
    DisconnectElementsCommand,
    SimulationControlCommand,
    SetInputValueCommand,
    GetOutputValueCommand,
    ListConnectionsCommand,
    ListElementsCommand,
    SaveCircuitCommand,
    LoadCircuitCommand,
    ExportImageCommand,
    CreateWaveformCommand,
    ExportWaveformCommand,
    CreateIcCommand,
    InstantiateIcCommand,
    ListIcsCommand,
    MoveElementCommand,
    SetElementPropertiesCommand,
    RotateElementCommand,
    FlipElementCommand,
    UpdateElementCommand,
    ChangeInputSizeCommand,
    ChangeOutputSizeCommand,
    ToggleTruthTableOutputCommand,
    UndoCommand,
    RedoCommand,
    GetUndoStackCommand,
    SplitConnectionCommand,
    MorphElementCommand,
)

# Re-export all result classes
from .results import (
    ElementCreationResult,
    ElementListResult,
    ConnectionListResult,
    OutputValueResult,
    TestResults,
)

# Re-export helper functions and types
from .helpers import (
    MCPCommandTypes,
    parse_mcp_command,
    create_success_response,
    create_error_response,
)

__all__ = [
    # Core classes
    "MCPCommand",
    "MCPResponse",
    # Enums (re-exported from domain)
    "ElementType",
    "SimulationAction",
    "ImageExportFormat",
    "WaveformExportFormat",
    # Command classes
    "NewCircuitCommand",
    "CloseCircuitCommand",
    "GetTabCountCommand",
    "CreateElementCommand",
    "DeleteElementCommand",
    "ConnectElementsCommand",
    "DisconnectElementsCommand",
    "SimulationControlCommand",
    "SetInputValueCommand",
    "GetOutputValueCommand",
    "ListConnectionsCommand",
    "ListElementsCommand",
    "SaveCircuitCommand",
    "LoadCircuitCommand",
    "ExportImageCommand",
    "CreateWaveformCommand",
    "ExportWaveformCommand",
    "CreateIcCommand",
    "InstantiateIcCommand",
    "ListIcsCommand",
    "MoveElementCommand",
    "SetElementPropertiesCommand",
    "RotateElementCommand",
    "FlipElementCommand",
    "UpdateElementCommand",
    "ChangeInputSizeCommand",
    "ChangeOutputSizeCommand",
    "ToggleTruthTableOutputCommand",
    "UndoCommand",
    "RedoCommand",
    "GetUndoStackCommand",
    "SplitConnectionCommand",
    "MorphElementCommand",
    # Result classes
    "ElementCreationResult",
    "ElementListResult",
    "ConnectionListResult",
    "OutputValueResult",
    "TestResults",
    # Helper functions and types
    "MCPCommandTypes",
    "parse_mcp_command",
    "create_success_response",
    "create_error_response",
]
