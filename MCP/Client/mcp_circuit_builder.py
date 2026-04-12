#!/usr/bin/env python3
"""
MCPCircuitBuilder: Circuit construction and validation helpers for MCP testing

This module provides circuit building methods for MCP (Model Context Protocol)
testing including circuit construction and validation helpers.
"""

import asyncio
from typing import TYPE_CHECKING, Any, Dict, List, Optional

from beartype import beartype

if TYPE_CHECKING:
    from mcp_infrastructure import MCPInfrastructure
    from mcp_validation import MCPValidation


class MCPCircuitBuilder:
    """Circuit Building methods: Circuit construction and validation helpers"""

    def __init__(self, infrastructure: "MCPInfrastructure", validation: "MCPValidation") -> None:
        self.infrastructure = infrastructure
        self.validation = validation

    @beartype
    async def create_circuit_from_spec(self, circuit_spec: Dict[str, Any]) -> Dict[int, int]:
        """Create a circuit from specification and return element ID mapping"""
        element_mapping = {}  # spec_id -> actual_id

        # Create elements
        for elem_spec in circuit_spec.get("elements", []):
            resp = await self.infrastructure.send_command(
                "create_element",
                {
                    "type": elem_spec["type"],
                    "x": elem_spec["x"],
                    "y": elem_spec["y"],
                    "label": elem_spec.get("label", f"{elem_spec['type']}_{elem_spec['id']}"),
                },
            )

            actual_id = await self.validation.validate_element_creation_response(
                resp, f"Create {elem_spec['type']} (spec_id={elem_spec['id']})"
            )
            if actual_id:
                element_mapping[elem_spec["id"]] = actual_id

        # Create connections
        for conn_spec in circuit_spec.get("connections", []):
            source_id = element_mapping.get(conn_spec["source"])
            target_id = element_mapping.get(conn_spec["target"])

            if source_id and target_id:
                # Require explicit port specifications to prevent silent errors
                # Accept either index (source_port) or label (source_port_label)
                has_source_port = "source_port" in conn_spec or "source_port_label" in conn_spec
                has_target_port = "target_port" in conn_spec or "target_port_label" in conn_spec

                if not has_source_port:
                    raise ValueError(
                        f"Connection {conn_spec['source']} -> {conn_spec['target']} missing required source port specification (provide 'source_port' or 'source_port_label')"
                    )
                if not has_target_port:
                    raise ValueError(
                        f"Connection {conn_spec['source']} -> {conn_spec['target']} missing required target port specification (provide 'target_port' or 'target_port_label')"
                    )

                # Build params with only provided fields
                params = {
                    "source_id": source_id,
                    "target_id": target_id,
                }
                for key in ("source_port", "source_port_label", "target_port", "target_port_label"):
                    if key in conn_spec:
                        params[key] = conn_spec[key]

                resp = await self.infrastructure.send_command("connect_elements", params)
                await self.validation.assert_success(resp, f"Connect {conn_spec['source']} -> {conn_spec['target']}")

        return element_mapping

    @beartype
    async def validate_truth_table(
        self, inputs: List[int], outputs: List[int], truth_table: List[Dict[str, Any]], test_name: str
    ) -> bool:
        """Validate a circuit's truth table behavior"""
        all_passed: bool = True

        for i, row in enumerate(truth_table):
            # Set input values
            input_values = row["inputs"]
            expected_outputs = row["outputs"]

            if not await self._apply_input_values(inputs, input_values, f"{test_name} row {i}"):
                all_passed = False

            # Enhanced propagation handling for complex circuits
            await asyncio.sleep(0.1)  # Longer delay for complex circuits

            # Force multiple simulation updates for complex propagation
            await self.infrastructure.send_command("simulation_control", {"action": "update"})

            input_str = ", ".join(str(v) for v in input_values)
            if not await self._check_output_values(
                outputs, expected_outputs, f"{test_name} row {i} inputs=[{input_str}]"
            ):
                all_passed = False

        return all_passed

    @beartype
    async def validate_sequential_state_table(
        self, inputs: List[int], outputs: List[int], clock_id: int, state_table: List[Dict[str, Any]], test_name: str
    ) -> bool:
        """Validate sequential circuit state table behavior with clock transitions"""
        all_passed: bool = True
        _current_state: Optional[Any] = None

        print(f"\n--- {test_name} State Table Validation ---")

        for i, transition in enumerate(state_table):
            # Get current inputs and expected next state/outputs
            input_values = transition["inputs"]
            expected_next_state = transition.get("next_state")
            expected_outputs = transition["outputs"]
            clock_edge = transition.get("clock_edge", "rising")  # rising/falling/level

            print(f"Transition {i}: {transition.get('description', '')}")

            if not await self._apply_input_values(inputs, input_values, f"{test_name} step {i}"):
                all_passed = False

            # Handle clock transition
            _two_phase = {"rising": (False, True), "falling": (True, False)}
            if clock_edge in _two_phase:
                first_val, second_val = _two_phase[clock_edge]
                await self.infrastructure.send_command(
                    "set_input_value", {"element_id": clock_id, "value": first_val}
                )
                await asyncio.sleep(0.05)
                await self.infrastructure.send_command(
                    "set_input_value", {"element_id": clock_id, "value": second_val}
                )
                await asyncio.sleep(0.1)
            elif clock_edge == "level":
                # Level sensitive (for latches)
                clock_value = transition.get("clock_level", True)
                await self.infrastructure.send_command(
                    "set_input_value", {"element_id": clock_id, "value": clock_value}
                )
                await asyncio.sleep(0.1)

            # Enhanced propagation for sequential circuits using quadruple update
            # Use new MCP update action for proper sequential element propagation
            # Exactly matches BewavedDolphin quadruple update hack (hardcoded 4 cycles)
            resp = await self.infrastructure.send_command("simulation_control", {"action": "update"})

            transition_passed = await self._check_output_values(
                outputs, expected_outputs, f"Transition {i}"
            )
            if not transition_passed:
                all_passed = False

            if transition_passed:
                self.infrastructure.output.success(f"✅ Transition {i} passed")
            else:
                print(f"❌ Transition {i} failed")

        return all_passed

    async def _apply_input_values(
        self, inputs: List[int], input_values: List[Any], label: str
    ) -> bool:
        """Set each input element to its value; returns False if any command fails."""
        success = True
        for j, input_id in enumerate(inputs):
            if j < len(input_values):
                resp = await self.infrastructure.send_command(
                    "set_input_value", {"element_id": input_id, "value": input_values[j]}
                )
                if not resp.success:
                    print(f"❌ {label}: Failed to set input {j}")
                    success = False
        return success

    async def _check_output_values(
        self, outputs: List[int], expected_outputs: List[Any], label: str = ""
    ) -> bool:
        """Check each output against expected; returns False on any mismatch or fetch error."""
        passed = True
        prefix = f"{label}: " if label else ""
        for j, output_id in enumerate(outputs):
            if j >= len(expected_outputs):
                continue
            resp = await self.infrastructure.send_command("get_output_value", {"element_id": output_id})
            if not (resp.success and resp.result):
                print(f"❌ {prefix}Failed to get output {j}")
                passed = False
                continue
            actual_value = resp.result.get("value")
            expected_value = expected_outputs[j]
            if actual_value != expected_value:
                print(f"❌ {prefix}output{j} expected {expected_value}, got {actual_value}")
                passed = False
            else:
                self.infrastructure.output.success(f"✅ {prefix}output{j}={actual_value}")
        return passed
