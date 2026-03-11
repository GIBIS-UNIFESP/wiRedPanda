#!/usr/bin/env python3
"""
Pydantic models for MCP (Model Context Protocol) commands and responses.
These models provide type validation and documentation for all MCP interactions.

This module now serves as a compatibility layer, re-exporting protocol classes
and containing domain-specific enums and data models.
"""

from enum import Enum
from typing import TYPE_CHECKING, Annotated, Any, Dict, List, Optional, Union, cast

from beartype import beartype

# Import protocol classes (extracted to protocol package)
from protocol import CloseCircuitCommand  # Command classes; Result classes
from protocol import (
    ConnectElementsCommand,
    ConnectionListResult,
    CreateElementCommand,
    CreateIcCommand,
    CreateWaveformCommand,
    DeleteElementCommand,
    DisconnectElementsCommand,
    ElementCreationResult,
    ElementListResult,
    ExportImageCommand,
    ExportWaveformCommand,
    GetOutputValueCommand,
    GetTabCountCommand,
    InstantiateIcCommand,
    ListConnectionsCommand,
    ListElementsCommand,
    ListIcsCommand,
    LoadCircuitCommand,
    MCPCommand,
    MCPResponse,
    MoveElementCommand,
    NewCircuitCommand,
    OutputValueResult,
    SaveCircuitCommand,
    SetElementPropertiesCommand,
    SetInputValueCommand,
    SimulationControlCommand,
    TestResults,
)
from pydantic import BaseModel, Field, field_validator, model_validator

# Import for runtime use
from type_models import CircuitConnection, CircuitElement, EmptyParameters

if TYPE_CHECKING:
    pass  # All imports now done at runtime


# Enums are now imported from protocol package above
# (No longer defined locally to avoid duplication)


# Import from domain package
from domain import (
    CircuitSpecification,
    ConnectionSpec,
    ElementInfo,
    ElementSpec,
    ElementType,
    ICInfo,
    ImageExportFormat,
    SimulationAction,
    StateTableEntry,
    TruthTableEntry,
    WaveformExportFormat,
)

# Import helper functions and types from protocol package
from protocol import MCPCommandTypes, create_error_response, parse_mcp_command
