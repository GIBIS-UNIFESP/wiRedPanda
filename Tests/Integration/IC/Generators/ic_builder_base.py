#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
IC Builder Base Module

Provides common functionality for all IC generator scripts.
Contains base classes and utility functions used across IC builders.

Dependencies:
  - beartype>=0.15.0: Runtime type validation (required)
  - pydantic>=2.4.0: Data validation via mcp_infrastructure (required)
  - jsonschema>=4.17.0: JSON Schema validation via mcp_infrastructure (required)

  Install: pip install -r requirements.txt
  Or set: PYTHONPATH=/path/to/wiredpanda/MCP/Client:$PYTHONPATH
"""

import sys
from pathlib import Path
from typing import Any

from beartype import beartype

try:
    from mcp_infrastructure import MCPInfrastructure
except ImportError:
    # MCP/Client is not on sys.path yet. Prefer setting PYTHONPATH before
    # invoking generator scripts (run_all_generators.py does this automatically).
    # This fallback supports ad-hoc standalone invocation without any env setup.
    _mcp_root = Path(__file__).resolve().parents[4]  # Tests/Integration/IC/Generators → root
    _mcp_client = _mcp_root / "MCP" / "Client"
    if not _mcp_client.is_dir():
        raise ImportError(
            f"Cannot locate MCP/Client at {_mcp_client}\n"
            f"Expected structure: .../MCP/Client/mcp_infrastructure.py\n"
            f"Set PYTHONPATH manually: export PYTHONPATH={_mcp_client}:$PYTHONPATH"
        )
    sys.path.insert(0, str(_mcp_client))
    del _mcp_client, _mcp_root
    from mcp_infrastructure import MCPInfrastructure

# Output directory for all IC components
IC_COMPONENTS_DIR = Path(__file__).parent.parent / "Components"
IC_COMPONENTS_DIR.mkdir(parents=True, exist_ok=True)


class ErrorContext:
    """Context manager for improved error messages with step tracking"""

    def __init__(self) -> None:
        self.ic_name = ""
        self.current_step = 0
        self.total_steps = 0

    def set_context(self, ic_name: str, current_step: int = 0, total_steps: int = 0) -> None:
        """Set the context for error reporting"""
        self.ic_name = ic_name
        self.current_step = current_step
        self.total_steps = total_steps

    def format_error(self, operation: str, error_detail: Any = None) -> str:
        """Format an error message with context information"""
        location = ""
        if self.ic_name:
            location = f" [{self.ic_name}"
            if self.total_steps > 0:
                location += f" Step {self.current_step}/{self.total_steps}"
            location += "]"

        detail_str = ""
        if error_detail:
            if isinstance(error_detail, dict):
                detail_str = f": {error_detail.get('message', str(error_detail))}"
            else:
                detail_str = f": {error_detail}"

        return f"❌ {operation}{location}{detail_str}"


class ICBuilderBase:
    """Base class for IC component builders using MCP infrastructure"""

    @beartype
    def __init__(self, mcp: MCPInfrastructure, verbose: bool = True) -> None:
        self.mcp = mcp
        self.verbose = verbose
        self.element_count = 0
        self.connection_count = 0
        self.error_context = ErrorContext()

    def check_dependency(self, ic_name: str) -> bool:
        """Verify that a required IC .panda file exists before instantiation."""
        panda_file = Path(ic_name + ".panda")
        if not panda_file.exists():
            self.log_error(f"Missing dependency: {panda_file.name} (run its generator first)")
            return False
        return True

    @beartype
    async def log(self, message: str) -> None:
        """Log a progress message. Suppressed when verbose=False."""
        if self.verbose:
            print(message)

    def log_error(self, operation: str, detail: Any = None) -> None:
        """Log an error. Always prints regardless of verbose setting."""
        print(self.error_context.format_error(operation, detail))

    async def begin_build(self, component_name: str) -> None:
        """Initialise builder state and log the start of a build."""
        self.error_context.set_context(component_name)
        self.element_count = 0
        self.connection_count = 0
        await self.log(f"🔧 Creating {component_name} IC...")

    async def create_element(self, elem_type: str, x: float, y: float,
                             label: str = "") -> "int | None":
        """Create a graphic element. Returns element_id, or None on failure."""
        response = await self.mcp.send_command("create_element", {
            "type": elem_type, "x": x, "y": y, "label": label
        })
        if not response.success:
            self.log_error(f"create {elem_type} '{label}'")
            return None
        elem_id = response.result.get('element_id') if response.result else None
        if elem_id is not None:
            self.element_count += 1
        return elem_id

    async def instantiate_ic(self, ic_name: str, x: float, y: float,
                             label: str = "") -> "int | None":
        """Instantiate an IC component. Returns element_id, or None on failure."""
        response = await self.mcp.send_command("instantiate_ic", {
            "ic_name": ic_name, "x": x, "y": y, "label": label
        })
        if not response.success:
            self.log_error(f"instantiate IC '{label}' ({ic_name})")
            return None
        elem_id = response.result.get('element_id') if response.result else None
        if elem_id is not None:
            self.element_count += 1
        return elem_id

    async def connect(self,
                      source_id: int,
                      target_id: int,
                      source_port: "int | None" = 0,
                      target_port: "int | None" = 0,
                      source_port_label: "str | None" = None,
                      target_port_label: "str | None" = None,
                      description: str = "") -> bool:
        """Connect two elements. Returns True on success.

        Port resolution (per side): label takes priority over numeric index.
        Both source_port and target_port default to 0 for the common case.
        Pass source_port=None / target_port=None to omit when using labels.
        """
        params: dict = {"source_id": source_id, "target_id": target_id}
        if source_port_label is not None:
            params["source_port_label"] = source_port_label
        elif source_port is not None:
            params["source_port"] = source_port
        if target_port_label is not None:
            params["target_port_label"] = target_port_label
        elif target_port is not None:
            params["target_port"] = target_port

        conn = await self.mcp.send_command("connect_elements", params)
        if not conn.success:
            self.log_error(description or f"connect {source_id} to {target_id}")
            return False
        self.connection_count += 1
        return True

    async def create_new_circuit(self) -> bool:
        """Create a new circuit. Returns True on success."""
        response = await self.mcp.send_command("new_circuit", {})
        if not response.success:
            self.log_error("create new circuit")
            return False
        return True

    async def save_circuit(self, output_file: str) -> bool:
        """Save the circuit to file. Returns True on success."""
        response = await self.mcp.send_command("save_circuit", {
            "filename": output_file
        })
        if not response.success:
            self.log_error("save IC", response.error)
            return False
        return True


@beartype
async def run_ic_builder(builder_func, ic_name: str) -> int:
    """
    Common main entry point for IC builder scripts.

    Args:
        builder_func: Async function that takes MCPInfrastructure and creates the IC
        ic_name: Name of the IC being created (for display)

    Returns:
        Exit code (0 for success, 1 for failure)
    """
    mcp = MCPInfrastructure(enable_validation=True, verbose=False)

    try:
        print(f"🚀 Starting MCP process for {ic_name}...")
        if not await mcp.start_mcp():
            print("❌ Failed to start MCP process")
            return 1

        print()

        if not await builder_func(mcp):
            print(f"❌ Failed to create {ic_name}")
            return 1

        print()
        print(f"✨ {ic_name} generation complete!")
        return 0

    finally:
        print("🛑 Stopping MCP process...")
        await mcp.stop_mcp()
