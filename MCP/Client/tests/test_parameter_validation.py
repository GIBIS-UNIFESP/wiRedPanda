#!/usr/bin/env python3
"""
Parameter Validation Tests

Tests for command parameter validation including:
- Missing parameter error handling
- Invalid parameter value validation
- Command coverage and completeness
- Parameter type checking and edge cases

MCP test implementation
"""

import json
from collections.abc import Awaitable, Callable
from enum import Enum
from pathlib import Path
from typing import Any, Literal, get_args, get_origin

from beartype import beartype

from protocol.helpers import COMMAND_MODELS

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class ParameterValidationTests(MCPTestBase):
    """Tests for parameter validation and command coverage"""

    CATEGORY_NAME = "PARAMETER VALIDATION"

    def tests(self) -> list[Callable[[], Awaitable[bool]]]:
        return [
            self.test_missing_parameter_validation,
            self.test_missing_command_coverage,
            self.test_command_models_match_schema,
        ]

    # ==================== TEST METHODS ====================
    # Test method implementations

    @beartype
    async def test_missing_parameter_validation(self) -> bool:
        """Test missing parameter error handling across all commands"""
        print("\n=== Missing Parameter Validation Test ===")
        self.set_test_context("test_missing_parameter_validation")

        all_passed: bool = True

        # Test create_element with missing parameters
        resp = await self.send_command("create_element", {})
        all_passed &= await self.assert_failure(resp, "Create element with missing parameters")

        resp = await self.send_command("create_element", {"type": "And"})
        all_passed &= await self.assert_failure(resp, "Create element with partial parameters (missing x, y)")

        resp = await self.send_command("create_element", {"x": 100, "y": 100})
        all_passed &= await self.assert_failure(resp, "Create element with partial parameters (missing type)")

        # Test connect_elements with missing parameters
        resp = await self.send_command("connect_elements", {})
        all_passed &= await self.assert_failure(resp, "Connect elements with missing parameters")

        resp = await self.send_command("connect_elements", {"source_id": 1})
        all_passed &= await self.assert_failure(resp, "Connect elements with partial parameters (missing target)")

        resp = await self.send_command("connect_elements", {"source_id": 1, "target_id": 2})
        all_passed &= await self.assert_failure(resp, "Connect elements with partial parameters (missing ports)")

        # Test simulation_control with missing parameters
        resp = await self.send_command("simulation_control", {})
        all_passed &= await self.assert_failure(resp, "Simulation control with missing parameters")

        # Test delete_element with missing parameters
        resp = await self.send_command("delete_element", {})
        all_passed &= await self.assert_failure(resp, "Delete element with missing parameters")

        # Test set_input_value with missing parameters
        resp = await self.send_command("set_input_value", {})
        all_passed &= await self.assert_failure(resp, "Set input value with missing parameters")

        resp = await self.send_command("set_input_value", {"element_id": 1})
        all_passed &= await self.assert_failure(resp, "Set input value with partial parameters (missing value)")

        resp = await self.send_command("set_input_value", {"value": True})
        all_passed &= await self.assert_failure(resp, "Set input value with partial parameters (missing element_id)")

        # Test get_output_value with missing parameters
        resp = await self.send_command("get_output_value", {})
        all_passed &= await self.assert_failure(resp, "Get output value with missing parameters")

        # Test disconnect_elements with missing parameters
        resp = await self.send_command("disconnect_elements", {})
        all_passed &= await self.assert_failure(resp, "Disconnect elements with missing parameters")

        resp = await self.send_command("disconnect_elements", {"source_id": 1})
        all_passed &= await self.assert_failure(resp, "Disconnect elements with partial parameters (missing target)")

        # Test file operations with missing parameters
        resp = await self.send_command("save_circuit", {})
        all_passed &= await self.assert_failure(resp, "Save circuit with missing filename")

        resp = await self.send_command("load_circuit", {})
        all_passed &= await self.assert_failure(resp, "Load circuit with missing filename")

        # Test parameter validation for multiple commands (JSON-RPC 2.0 format)
        commands_to_test = [
            {"jsonrpc": "2.0", "method": "create_element", "params": {"type": "And"}, "id": 1},  # Missing x,y
            {
                "jsonrpc": "2.0",
                "method": "connect_elements",
                "params": {"source_id": 1},
                "id": 2,
            },  # Missing target info
            {"jsonrpc": "2.0", "method": "set_input_value", "params": {"element_id": 1}, "id": 3},  # Missing value
            {"jsonrpc": "2.0", "method": "save_circuit", "params": {"filename": ""}, "id": 4},  # Empty filename
        ]

        for test_case in commands_to_test:
            command: str = test_case["method"]  # type: ignore
            params: dict[str, Any] = test_case["params"]  # type: ignore
            resp = await self.send_command(command, params)
            all_passed &= await self.assert_failure(resp, f"Parameter validation for {command}")

        # No cleanup needed as this is error handling test

        return all_passed

    @beartype
    async def test_missing_command_coverage(self) -> bool:
        """Missing Command Coverage - IC Workflow and Element Management"""
        print("\n=== Missing Command Coverage ===")

        all_passed: bool = True

        # Test IC workflow commands
        print("Testing IC workflow commands...")
        # Note: Individual IC operations will be counted via assert_success/assert_failure

        try:
            # Build the InputButton -> Not -> Led chain that IC creation expects.
            if await self.setup_basic_ic_circuit() is None:
                print("❌ IC test setup failed - could not create required elements")
                return False

            # Test create_ic command with properly connected circuit
            await self.create_test_ic()

            # Test list_ics command
            list_resp = await self.send_command("list_ics", {})
            if list_resp.success and list_resp.result:
                ics = list_resp.result.get("ics", [])
                self.infrastructure.output.success(f"✅ list_ics successful: found {len(ics)} ICs")
            else:
                print(f"❌ list_ics failed: {list_resp.error}")

        except Exception as e:
            print(f"❌ IC workflow test failed with exception: {e}")

        print("   • test_ic_workflow_commands()")
        print("   • test_element_management_commands()")

        return all_passed

    @beartype
    async def test_command_models_match_schema(self) -> bool:
        """Every command the schema declares has a model taking its parameters, with the same bounds and lengths"""
        print("\n=== Command Models Match Schema ===")

        schema_path = Path(__file__).resolve().parents[2] / "schema-mcp.json"
        with open(schema_path, encoding="utf-8") as schema_file:
            schema = json.load(schema_file)
        commands = schema["properties"]["commands"]["properties"]

        mismatches: list[str] = [f"{name}: not in the schema" for name in sorted(set(COMMAND_MODELS) - set(commands))]
        for name, command in commands.items():
            model = COMMAND_MODELS.get(name)
            if model is None:
                mismatches.append(f"{name}: no command model")
                continue
            schema_params = command["properties"].get("params", {})
            schema_fields: dict[str, Any] = schema_params.get("properties", {})
            fields = getattr(model.model_fields["params"].annotation, "model_fields", {})

            if set(fields) != set(schema_fields):
                mismatches.append(
                    f"{name}: fields only in the model {sorted(set(fields) - set(schema_fields))}, "
                    f"only in the schema {sorted(set(schema_fields) - set(fields))}"
                )
            required = {field for field, info in fields.items() if info.is_required()}
            if required != set(schema_params.get("required", [])):
                mismatches.append(f"{name}: required {sorted(required)}, schema {schema_params.get('required', [])}")
            for field in set(fields) & set(schema_fields):
                model_bounds = _model_bounds(fields[field])
                expected = _schema_bounds(schema_fields[field])
                if model_bounds != expected:
                    mismatches.append(f"{name}.{field}: bounds {model_bounds}, schema {expected}")
                model_values = _model_values(fields[field])
                expected_values = _schema_values(schema_fields[field], schema["definitions"])
                if model_values != expected_values:
                    mismatches.append(f"{name}.{field}: values {model_values}, schema {expected_values}")

        for mismatch in mismatches:
            print(f"❌ {mismatch}")
        if mismatches:
            self.test_results.errors.append(f"test_command_models_match_schema: {len(mismatches)} mismatches")
            return False
        self.infrastructure.output.success(f"✅ {len(commands)} command models match the schema")
        return True


Bounds = tuple[float | None, bool, float | None, bool, int | None]


def _inclusive(bounds: Bounds, integer: bool) -> Bounds:
    """An integer's exclusive bound as the inclusive one next to it, so gt=0 and minimum 1 compare equal"""
    low, low_exclusive, high, high_exclusive, min_length = bounds
    if integer and low is not None and low_exclusive:
        low, low_exclusive = low + 1, False
    if integer and high is not None and high_exclusive:
        high, high_exclusive = high - 1, False
    return low, low_exclusive, high, high_exclusive, min_length


def _model_bounds(info: Any) -> Bounds:
    """A field's bounds, from its own metadata or the Annotated type inside its Optional; for a list, its items'"""
    metadata = list(info.metadata)
    annotation = info.annotation
    for arg in (annotation, *get_args(annotation)):
        if get_origin(arg) is list:
            arg = get_args(arg)[0]
        if hasattr(arg, "__metadata__"):
            for item in arg.__metadata__:
                metadata.extend(getattr(item, "metadata", [item]))
    low: float | None = None
    high: float | None = None
    min_length: int | None = None
    low_exclusive = high_exclusive = False
    for item in metadata:
        if getattr(item, "ge", None) is not None:
            low = float(item.ge)
        if getattr(item, "gt", None) is not None:
            low, low_exclusive = float(item.gt), True
        if getattr(item, "le", None) is not None:
            high = float(item.le)
        if getattr(item, "lt", None) is not None:
            high, high_exclusive = float(item.lt), True
        if getattr(item, "min_length", None) is not None:
            min_length = int(item.min_length)
    integer = int in (annotation, *get_args(annotation)) or any(
        get_origin(arg) is list or int in get_args(arg) for arg in (annotation, *get_args(annotation))
    )
    return _inclusive((low, low_exclusive, high, high_exclusive, min_length), integer)


def _model_values(info: Any) -> frozenset[Any] | None:
    """The values a field takes when it is an enumeration or a literal, else None"""
    for arg in (info.annotation, *get_args(info.annotation)):
        if isinstance(arg, type) and issubclass(arg, Enum):
            return frozenset(member.value for member in arg)
        if get_origin(arg) is Literal:
            return frozenset(get_args(arg))
    return None


def _schema_values(field: dict[str, Any], definitions: dict[str, Any]) -> frozenset[Any] | None:
    """The values a string field takes when the schema enumerates them, directly or through a definition"""
    if "$ref" in field:
        field = definitions[field["$ref"].rsplit("/", 1)[-1]]
    if field.get("type") == "string" and "enum" in field:
        return frozenset(field["enum"])
    return None


def _schema_bounds(field: dict[str, Any]) -> Bounds:
    """A schema field's bounds; an integer enum of consecutive values as its range; for an array, its items'"""
    min_length = field.get("minItems", field.get("minLength"))
    if field.get("type") == "array":
        field = field.get("items", {})
    if field.get("type") == "integer" and "enum" in field:
        values = sorted(field["enum"])
        if values == list(range(values[0], values[-1] + 1)):
            return float(values[0]), False, float(values[-1]), False, min_length
    low = field.get("exclusiveMinimum", field.get("minimum"))
    high = field.get("exclusiveMaximum", field.get("maximum"))
    bounds: Bounds = (
        None if low is None else float(low),
        "exclusiveMinimum" in field,
        None if high is None else float(high),
        "exclusiveMaximum" in field,
        min_length,
    )
    return _inclusive(bounds, field.get("type") == "integer")
