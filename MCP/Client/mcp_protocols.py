#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-Unifesp and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later
"""
Shared structural protocols for MCP infrastructure and organizer.

These Protocol classes break the circular import between mcp_infrastructure
and mcp_organizer: each module imports only from here, not from each other.
"""

from typing import Any, Dict, Protocol

from mcp_models import MCPResponse


class InfrastructureProtocol(Protocol):
    """Subset of MCPInfrastructure needed by MCPTestOrganizer."""

    async def send_command(self, command: str, parameters: Dict[str, Any], timeout: float = 10.0) -> MCPResponse: ...


class OrganizerProtocol(Protocol):
    """Subset of MCPTestOrganizer needed by MCPInfrastructure."""

    async def export_circuit_image(self, test_name: str, infrastructure: InfrastructureProtocol) -> None: ...
