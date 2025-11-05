#!/usr/bin/env python3
"""
MCP Protocol Helper Functions

This module contains helper functions for MCP (Model Context Protocol).
These functions handle command parsing, response creation, and protocol utilities.
"""

from typing import Any, Dict, Optional, Union, cast

from beartype import beartype

from .core import MCPResponse
from .commands import (
    NewCircuitCommand, CloseCircuitCommand, GetTabCountCommand,
    CreateElementCommand, DeleteElementCommand, ConnectElementsCommand,
    DisconnectElementsCommand, SimulationControlCommand, SetInputValueCommand,
    GetOutputValueCommand, ListConnectionsCommand, ListElementsCommand,
    SaveCircuitCommand, LoadCircuitCommand, ExportImageCommand,
    CreateWaveformCommand, ExportWaveformCommand, CreateIcCommand,
    InstantiateIcCommand, ListIcsCommand, MoveElementCommand,
    SetElementPropertiesCommand, UndoCommand, RedoCommand, GetUndoStackCommand,
    SplitConnectionCommand, RotateElementCommand, FlipElementCommand,
    UpdateElementCommand, ChangeInputSizeCommand, ChangeOutputSizeCommand,
    ToggleTruthTableOutputCommand, MorphElementCommand,
)

# Union type for command parsing
MCPCommandTypes = Union[
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
]


@beartype
def parse_mcp_command(data: Dict[str, Any]) -> MCPCommandTypes:
    """Parse raw command data into the appropriate pydantic model"""
    command_type = data.get("method", "").lower()

    command_map = {
        "new_circuit": NewCircuitCommand,
        "close_circuit": CloseCircuitCommand,
        "get_tab_count": GetTabCountCommand,
        "create_element": CreateElementCommand,
        "delete_element": DeleteElementCommand,
        "connect_elements": ConnectElementsCommand,
        "disconnect_elements": DisconnectElementsCommand,
        "simulation_control": SimulationControlCommand,
        "set_input_value": SetInputValueCommand,
        "get_output_value": GetOutputValueCommand,
        "list_connections": ListConnectionsCommand,
        "list_elements": ListElementsCommand,
        "save_circuit": SaveCircuitCommand,
        "load_circuit": LoadCircuitCommand,
        "export_image": ExportImageCommand,
        "create_waveform": CreateWaveformCommand,
        "export_waveform": ExportWaveformCommand,
        "create_ic": CreateIcCommand,
        "instantiate_ic": InstantiateIcCommand,
        "list_ics": ListIcsCommand,
        "move_element": MoveElementCommand,
        "set_element_properties": SetElementPropertiesCommand,
        "rotate_element": RotateElementCommand,
        "flip_element": FlipElementCommand,
        "update_element": UpdateElementCommand,
        "change_input_size": ChangeInputSizeCommand,
        "change_output_size": ChangeOutputSizeCommand,
        "toggle_truth_table_output": ToggleTruthTableOutputCommand,
        "undo": UndoCommand,
        "redo": RedoCommand,
        "get_undo_stack": GetUndoStackCommand,
        "split_connection": SplitConnectionCommand,
        "morph_element": MorphElementCommand,
    }

    model_class = command_map.get(command_type)
    if not model_class:
        raise ValueError(f"Unknown command type: {command_type}")

    return cast(MCPCommandTypes, model_class(**data))


@beartype
def create_success_response(result: Optional[Dict[str, Any]] = None, request_id: Optional[int] = None) -> MCPResponse:
    """Create a successful MCP response (JSON-RPC 2.0 format)"""
    return MCPResponse(result=result or {}, id=request_id)


@beartype
def create_error_response(error: str, request_id: Optional[int] = None) -> MCPResponse:
    """Create an error MCP response (JSON-RPC 2.0 format)"""
    error_obj = {
        "code": -32603,  # Internal error code
        "message": error
    }
    return MCPResponse(error=error_obj, id=request_id)
