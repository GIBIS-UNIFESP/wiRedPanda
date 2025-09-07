#!/usr/bin/env python3
# Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Simple Circuit Example
Demonstrates basic wiRedPanda MCP usage for creating a simple logic circuit using asyncio.
"""

import asyncio
import sys
from pathlib import Path
from typing import Annotated

from beartype import beartype
from pydantic import BaseModel, Field

# Add parent directory to path to import the infrastructure
sys.path.insert(0, str(Path(__file__).parent.parent))

from mcp_infrastructure import MCPInfrastructure


class ElementPosition(BaseModel):
    """Model for element positioning in circuits."""

    x: Annotated[float, Field(ge=0.0, le=5000.0, description="X coordinate position")]
    y: Annotated[float, Field(ge=0.0, le=5000.0, description="Y coordinate position")]

    class Config:
        frozen = True


class CircuitElementSpec(BaseModel):
    """Specification for circuit element creation."""

    element_type: str = Field(min_length=1, description="Type of circuit element")
    position: ElementPosition = Field(description="Element position coordinates")
    label: str = Field(min_length=1, max_length=50, description="Element label")

    class Config:
        frozen = True


@beartype
async def create_element_from_spec(mcp: MCPInfrastructure, spec: CircuitElementSpec) -> int | None:
    """Create a circuit element using validated specification."""
    response = await mcp.send_command("create_element", {
        "type": spec.element_type,
        "x": spec.position.x,
        "y": spec.position.y,
        "label": spec.label
    })

    if not response or not response.success:
        print(f"✗ Failed to create {spec.element_type}: {response.error if response else 'No response'}")
        return None

    return response.result.get('element_id') if response.result else None


@beartype
async def create_simple_and_circuit(mcp: MCPInfrastructure) -> bool:
    """Create and test a simple AND gate circuit."""

    print("Creating simple AND gate circuit...")

    # Create new circuit
    response = await mcp.send_command("new_circuit", {})
    if not response or not response.success:
        print(f"✗ Failed to create new circuit: {response.error if response else 'No response'}")
        return False

    # Create elements
    input_a_response = await mcp.send_command("create_element", {
        "type": "InputButton",
        "x": 100.0,
        "y": 100.0,
        "label": "InputA"
    })
    if not input_a_response or not input_a_response.success:
        print(f"✗ Failed to create InputA: {input_a_response.error if input_a_response else 'No response'}")
        return False
    input_a = input_a_response.result.get('element_id') if input_a_response.result else None

    input_b_response = await mcp.send_command("create_element", {
        "type": "InputButton",
        "x": 100.0,
        "y": 200.0,
        "label": "InputB"
    })
    if not input_b_response or not input_b_response.success:
        print(f"✗ Failed to create InputB: {input_b_response.error if input_b_response else 'No response'}")
        return False
    input_b = input_b_response.result.get('element_id') if input_b_response.result else None

    and_gate_response = await mcp.send_command("create_element", {
        "type": "And",
        "x": 300.0,
        "y": 150.0,
        "label": "AndGate"
    })
    if not and_gate_response or not and_gate_response.success:
        print(f"✗ Failed to create AND gate: {and_gate_response.error if and_gate_response else 'No response'}")
        return False
    and_gate = and_gate_response.result.get('element_id') if and_gate_response.result else None

    led_response = await mcp.send_command("create_element", {
        "type": "Led",
        "x": 500.0,
        "y": 150.0,
        "label": "Output"
    })
    if not led_response or not led_response.success:
        print(f"✗ Failed to create LED: {led_response.error if led_response else 'No response'}")
        return False
    led_output = led_response.result.get('element_id') if led_response.result else None

    print(f"Created elements: A={input_a}, B={input_b}, Gate={and_gate}, LED={led_output}")

    # Connect elements
    conn1_response = await mcp.send_command("connect_elements", {
        "source_id": input_a,
        "source_port": 0,
        "target_id": and_gate,
        "target_port": 0
    })
    if not conn1_response or not conn1_response.success:
        print(f"✗ Failed to connect InputA to AND gate: {conn1_response.error if conn1_response else 'No response'}")
        return False

    conn2_response = await mcp.send_command("connect_elements", {
        "source_id": input_b,
        "source_port": 0,
        "target_id": and_gate,
        "target_port": 1
    })
    if not conn2_response or not conn2_response.success:
        print(f"✗ Failed to connect InputB to AND gate: {conn2_response.error if conn2_response else 'No response'}")
        return False

    conn3_response = await mcp.send_command("connect_elements", {
        "source_id": and_gate,
        "source_port": 0,
        "target_id": led_output,
        "target_port": 0
    })
    if not conn3_response or not conn3_response.success:
        print(f"✗ Failed to connect AND gate to LED: {conn3_response.error if conn3_response else 'No response'}")
        return False

    print("Connected all elements")

    # Start simulation
    sim_response = await mcp.send_command("simulation_control", {"action": "start"})
    if not sim_response or not sim_response.success:
        print(f"✗ Failed to start simulation: {sim_response.error if sim_response else 'No response'}")
        return False
    print("Started simulation")

    # Test all input combinations
    print("\nTesting AND gate truth table:")
    print("A | B | Output")
    print("--+---+-------")

    test_cases = [
        (False, False),
        (False, True),
        (True, False),
        (True, True),
    ]

    for a_val, b_val in test_cases:
        # Set input values
        await mcp.send_command("set_input_value", {
            "element_id": input_a,
            "value": a_val
        })
        await mcp.send_command("set_input_value", {
            "element_id": input_b,
            "value": b_val
        })

        # Read output value
        output_response = await mcp.send_command("get_output_value", {
            "element_id": led_output
        })
        if not output_response or not output_response.success:
            print(f"✗ Failed to get output value: {output_response.error if output_response else 'No response'}")
            return False
        output_val = output_response.result.get('value', False) if output_response.result else False

        # Display result
        print(f"{int(a_val)} | {int(b_val)} | {int(output_val)}")

        # Verify AND gate logic
        expected = a_val and b_val
        if output_val != expected:
            print(f"✗ ERROR: Expected {expected}, got {output_val}")
            return False

    # Save the circuit
    circuit_file = "simple_and_circuit.panda"
    save_response = await mcp.send_command("save_circuit", {"filename": circuit_file})
    if not save_response or not save_response.success:
        print(f"✗ Failed to save circuit: {save_response.error if save_response else 'No response'}")
        return False
    print(f"\n✓ Circuit saved as {circuit_file}")

    return True


@beartype
async def create_simple_or_circuit(mcp: MCPInfrastructure) -> bool:
    """Create and test a simple OR gate circuit."""

    print("\nCreating simple OR gate circuit...")

    # Create new circuit
    response = await mcp.send_command("new_circuit", {})
    if not response or not response.success:
        print(f"✗ Failed to create new circuit: {response.error if response else 'No response'}")
        return False

    # Create elements
    input_a_response = await mcp.send_command("create_element", {
        "type": "InputButton",
        "x": 100.0,
        "y": 100.0,
        "label": "InputA"
    })
    if not input_a_response or not input_a_response.success:
        print(f"✗ Failed to create InputA: {input_a_response.error if input_a_response else 'No response'}")
        return False
    input_a = input_a_response.result.get('element_id') if input_a_response.result else None

    input_b_response = await mcp.send_command("create_element", {
        "type": "InputButton",
        "x": 100.0,
        "y": 200.0,
        "label": "InputB"
    })
    if not input_b_response or not input_b_response.success:
        print(f"✗ Failed to create InputB: {input_b_response.error if input_b_response else 'No response'}")
        return False
    input_b = input_b_response.result.get('element_id') if input_b_response.result else None

    or_gate_response = await mcp.send_command("create_element", {
        "type": "Or",
        "x": 300.0,
        "y": 150.0,
        "label": "OrGate"
    })
    if not or_gate_response or not or_gate_response.success:
        print(f"✗ Failed to create OR gate: {or_gate_response.error if or_gate_response else 'No response'}")
        return False
    or_gate = or_gate_response.result.get('element_id') if or_gate_response.result else None

    led_response = await mcp.send_command("create_element", {
        "type": "Led",
        "x": 500.0,
        "y": 150.0,
        "label": "Output"
    })
    if not led_response or not led_response.success:
        print(f"✗ Failed to create LED: {led_response.error if led_response else 'No response'}")
        return False
    led_output = led_response.result.get('element_id') if led_response.result else None

    # Connect elements
    conn1_response = await mcp.send_command("connect_elements", {
        "source_id": input_a,
        "source_port": 0,
        "target_id": or_gate,
        "target_port": 0
    })
    if not conn1_response or not conn1_response.success:
        print(f"✗ Failed to connect InputA to OR gate: {conn1_response.error if conn1_response else 'No response'}")
        return False

    conn2_response = await mcp.send_command("connect_elements", {
        "source_id": input_b,
        "source_port": 0,
        "target_id": or_gate,
        "target_port": 1
    })
    if not conn2_response or not conn2_response.success:
        print(f"✗ Failed to connect InputB to OR gate: {conn2_response.error if conn2_response else 'No response'}")
        return False

    conn3_response = await mcp.send_command("connect_elements", {
        "source_id": or_gate,
        "source_port": 0,
        "target_id": led_output,
        "target_port": 0
    })
    if not conn3_response or not conn3_response.success:
        print(f"✗ Failed to connect OR gate to LED: {conn3_response.error if conn3_response else 'No response'}")
        return False

    # Start simulation
    sim_response = await mcp.send_command("simulation_control", {"action": "start"})
    if not sim_response or not sim_response.success:
        print(f"✗ Failed to start simulation: {sim_response.error if sim_response else 'No response'}")
        return False

    # Test OR gate
    print("Testing OR gate truth table:")
    print("A | B | Output")
    print("--+---+-------")

    test_cases = [
        (False, False),
        (False, True),
        (True, False),
        (True, True),
    ]

    for a_val, b_val in test_cases:
        # Set input values
        await mcp.send_command("set_input_value", {
            "element_id": input_a,
            "value": a_val
        })
        await mcp.send_command("set_input_value", {
            "element_id": input_b,
            "value": b_val
        })

        # Read output value
        output_response = await mcp.send_command("get_output_value", {
            "element_id": led_output
        })
        if not output_response or not output_response.success:
            print(f"✗ Failed to get output value: {output_response.error if output_response else 'No response'}")
            return False
        output_val = output_response.result.get('value', False) if output_response.result else False

        print(f"{int(a_val)} | {int(b_val)} | {int(output_val)}")

        # Verify OR gate logic
        expected = a_val or b_val
        if output_val != expected:
            print(f"✗ ERROR: Expected {expected}, got {output_val}")
            return False

    # Save the circuit
    save_response = await mcp.send_command("save_circuit", {"filename": "simple_or_circuit.panda"})
    if not save_response or not save_response.success:
        print(f"✗ Failed to save circuit: {save_response.error if save_response else 'No response'}")
        return False
    print("✓ OR circuit saved as simple_or_circuit.panda")

    return True


@beartype
async def main() -> int:
    """Main example function."""
    print("wiRedPanda MCP Simple Circuit Examples")
    print("=" * 45)

    # Create MCP infrastructure
    mcp = MCPInfrastructure(enable_validation=False, verbose=False)

    try:
        # Start MCP process
        print("\n⚡ Starting MCP process...")
        if not await mcp.start_mcp():
            print("✗ Failed to start MCP process")
            return 1
        print("✓ MCP process started")

        # Test AND gate circuit
        success_and = await create_simple_and_circuit(mcp)

        # Test OR gate circuit
        success_or = await create_simple_or_circuit(mcp)

        if success_and and success_or:
            print("\n✓ All tests passed! MCP bridge is working correctly.")
            return 0

        print("\n✗ Some tests failed.")
        return 1

    except FileNotFoundError as e:
        print(f"✗ Error: {e}")
        print("Make sure wiRedPanda is built and available in PATH or build/")
        return 1
    except (OSError, RuntimeError, ValueError) as e:
        print(f"✗ Unexpected error: {e}")
        return 1
    finally:
        # Clean up MCP process
        print("\n⚡ Cleaning up MCP process...")
        await mcp.stop_mcp()
        print("✓ MCP process stopped")


if __name__ == "__main__":
    sys.exit(asyncio.run(main()))
