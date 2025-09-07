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

            actual_id = await self.validation.validate_element_creation_response(resp, f"Create {elem_spec['type']} (spec_id={elem_spec['id']})")
            if actual_id:
                element_mapping[elem_spec["id"]] = actual_id

        # Create connections
        for conn_spec in circuit_spec.get("connections", []):
            source_id = element_mapping.get(conn_spec["source"])
            target_id = element_mapping.get(conn_spec["target"])

            if source_id and target_id:
                # Require explicit port specifications to prevent silent errors
                if "source_port" not in conn_spec:
                    raise ValueError(f"Connection {conn_spec['source']} -> {conn_spec['target']} missing required 'source_port' specification")
                if "target_port" not in conn_spec:
                    raise ValueError(f"Connection {conn_spec['source']} -> {conn_spec['target']} missing required 'target_port' specification")

                resp = await self.infrastructure.send_command(
                    "connect_elements",
                    {
                        "source_id": source_id,
                        "source_port": conn_spec["source_port"],
                        "target_id": target_id,
                        "target_port": conn_spec["target_port"],
                    },
                )
                await self.validation.assert_success(resp, f"Connect {conn_spec['source']} -> {conn_spec['target']}")

        return element_mapping

    @beartype
    async def validate_truth_table(self, inputs: List[int], outputs: List[int], truth_table: List[Dict[str, Any]], test_name: str) -> bool:
        """Validate a circuit's truth table behavior"""
        all_passed: bool = True

        for i, row in enumerate(truth_table):
            # Set input values
            input_values = row["inputs"]
            expected_outputs = row["outputs"]

            # Apply inputs
            for j, input_id in enumerate(inputs):
                if j < len(input_values):
                    resp = await self.infrastructure.send_command(
                        "set_input_value",
                        {
                            "element_id": input_id,
                            "value": input_values[j],
                        },
                    )
                    if not resp.success:
                        print(f"❌ {test_name} row {i}: Failed to set input {j}")
                        all_passed = False
                        continue

            # Enhanced propagation handling for complex circuits
            await asyncio.sleep(0.1)  # Longer delay for complex circuits

            # Force multiple simulation updates for complex propagation
            resp = await self.infrastructure.send_command("simulation_control", {"action": "update"})

            # Check outputs
            for j, output_id in enumerate(outputs):
                if j >= len(expected_outputs):
                    continue

                resp = await self.infrastructure.send_command("get_output_value", {"element_id": output_id})
                if not (resp.success and resp.result):
                    print(f"❌ {test_name} row {i}: Failed to get output {j}")
                    all_passed = False
                    continue

                actual_value = resp.result.get("value")
                expected_value = expected_outputs[j]
                input_str = ", ".join(str(v) for v in input_values)

                if actual_value != expected_value:
                    print(f"❌ {test_name} row {i}: inputs=[{input_str}] output{j} expected {expected_value}, got {actual_value}")
                    all_passed = False
                else:
                    self.infrastructure.output.success(f"✅ {test_name} row {i}: inputs=[{input_str}] output{j}={actual_value}")

        return all_passed

    @beartype
    async def validate_sequential_state_table(self, inputs: List[int], outputs: List[int], clock_id: int, state_table: List[Dict[str, Any]], test_name: str) -> bool:
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

            # Set input values
            for j, input_id in enumerate(inputs):
                if j < len(input_values):
                    resp = await self.infrastructure.send_command(
                        "set_input_value",
                        {
                            "element_id": input_id,
                            "value": input_values[j],
                        },
                    )
                    if not resp.success:
                        print(f"❌ {test_name} step {i}: Failed to set input {j}")
                        all_passed = False
                        continue

            # Handle clock transition
            if clock_edge == "rising":
                # Low to High transition
                resp = await self.infrastructure.send_command("set_input_value", {"element_id": clock_id, "value": False})
                await asyncio.sleep(0.05)
                resp = await self.infrastructure.send_command("set_input_value", {"element_id": clock_id, "value": True})
                await asyncio.sleep(0.1)
            elif clock_edge == "falling":
                # High to Low transition
                resp = await self.infrastructure.send_command("set_input_value", {"element_id": clock_id, "value": True})
                await asyncio.sleep(0.05)
                resp = await self.infrastructure.send_command("set_input_value", {"element_id": clock_id, "value": False})
                await asyncio.sleep(0.1)
            elif clock_edge == "level":
                # Level sensitive (for latches)
                clock_value = transition.get("clock_level", True)
                resp = await self.infrastructure.send_command("set_input_value", {"element_id": clock_id, "value": clock_value})
                await asyncio.sleep(0.1)

            # Enhanced propagation for sequential circuits using quadruple update
            # Use new MCP update action for proper sequential element propagation
            # Exactly matches BewavedDolphin quadruple update hack (hardcoded 4 cycles)
            resp = await self.infrastructure.send_command("simulation_control", {"action": "update"})

            # Check outputs
            transition_passed = True
            for j, output_id in enumerate(outputs):
                if j >= len(expected_outputs):
                    continue

                resp = await self.infrastructure.send_command("get_output_value", {"element_id": output_id})
                if not (resp.success and resp.result):
                    print(f"❌ Failed to get output {j}")
                    transition_passed = False
                    all_passed = False
                    continue

                actual_value = resp.result.get("value")
                expected_value = expected_outputs[j]

                if actual_value != expected_value:
                    print(f"❌ Output {j}: expected {expected_value}, got {actual_value}")
                    transition_passed = False
                    all_passed = False
                else:
                    self.infrastructure.output.success(f"✅ Output {j}: {actual_value}")

            if transition_passed:
                self.infrastructure.output.success(f"✅ Transition {i} passed")
            else:
                print(f"❌ Transition {i} failed")

                # Intentionally unused - for potential debugging
                _current_state = expected_next_state

        return all_passed
