#!/usr/bin/env python3
"""
Pydantic models for MCP (Model Context Protocol) commands and responses.
These models provide type validation and documentation for all MCP interactions.

This module now serves as a compatibility layer, re-exporting protocol classes
and containing domain-specific enums and data models.
"""

from enum import Enum
from typing import Annotated, Any, Dict, List, Optional, Union, cast, TYPE_CHECKING

from beartype import beartype
from pydantic import BaseModel, Field, field_validator, model_validator

# Import for runtime use
from type_models import EmptyParameters, CircuitElement, CircuitConnection

# Import protocol classes (extracted to protocol package)
from protocol import (
    MCPCommand, MCPResponse,
    # Command classes
    NewCircuitCommand, CloseCircuitCommand, GetTabCountCommand,
    CreateElementCommand, DeleteElementCommand, ConnectElementsCommand,
    DisconnectElementsCommand, SimulationControlCommand, SetInputValueCommand,
    GetOutputValueCommand, ListConnectionsCommand, ListElementsCommand,
    SaveCircuitCommand, LoadCircuitCommand, ExportImageCommand,
    CreateWaveformCommand, ExportWaveformCommand, CreateIcCommand,
    InstantiateIcCommand, ListIcsCommand, MoveElementCommand,
    SetElementPropertiesCommand,
    # Result classes
    ElementCreationResult, ElementListResult, ConnectionListResult,
    OutputValueResult, TestResults,
)

if TYPE_CHECKING:
    pass  # All imports now done at runtime


# Enums are now imported from protocol package above
# (No longer defined locally to avoid duplication)


# Import from domain package
from domain import (
    ElementType, SimulationAction, ImageExportFormat, WaveformExportFormat,
    ElementInfo, ICInfo, ElementSpec, ConnectionSpec, CircuitSpecification,
    TruthTableEntry, StateTableEntry,
)

# Import helper functions and types from protocol package
from protocol import (
    MCPCommandTypes, parse_mcp_command,
    create_success_response, create_error_response
)
