#!/usr/bin/env python3
"""
MCP Protocol Layer

This package contains the protocol-level definitions for MCP (Model Context Protocol).
It defines the communication contracts, command structures, and response formats.
"""

# Re-export enums from domain package
from domain.enums import ElementType, ImageExportFormat, SimulationAction, WaveformExportFormat

# Re-export all command classes
from .commands import (
    CloseCircuitCommand,
    ConnectElementsCommand,
    CreateElementCommand,
    CreateIcCommand,
    CreateWaveformCommand,
    DeleteElementCommand,
    DisconnectElementsCommand,
    ExportImageCommand,
    ExportWaveformCommand,
    GetOutputValueCommand,
    GetTabCountCommand,
    InstantiateIcCommand,
    ListConnectionsCommand,
    ListElementsCommand,
    ListIcsCommand,
    LoadCircuitCommand,
    MoveElementCommand,
    NewCircuitCommand,
    SaveCircuitCommand,
    SetElementPropertiesCommand,
    SetInputValueCommand,
    SimulationControlCommand,
)

# Re-export core protocol classes
from .core import MCPCommand, MCPResponse

# Re-export helper functions and types
from .helpers import MCPCommandTypes, create_error_response, parse_mcp_command

# Re-export all result classes
from .results import ConnectionListResult, ElementCreationResult, ElementListResult, OutputValueResult, TestResults

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
    # Result classes
    "ElementCreationResult",
    "ElementListResult",
    "ConnectionListResult",
    "OutputValueResult",
    "TestResults",
    # Helper functions and types
    "MCPCommandTypes",
    "parse_mcp_command",
    "create_error_response",
]
