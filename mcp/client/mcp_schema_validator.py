#!/usr/bin/env python3
"""
MCP JSON Schema Validator
Provides comprehensive validation using both JSON Schema and Pydantic models.
"""

import json
from pathlib import Path
from typing import Any, Dict, List, Optional, TYPE_CHECKING, TypedDict, Union

import jsonschema
from beartype import beartype
from pydantic import ValidationError

from mcp_models import MCPResponse, parse_mcp_command

if TYPE_CHECKING:
    from type_models import ValidationResult, SchemaValidationResult


class PydanticValidationResult(TypedDict):
    """Result structure for individual Pydantic validation"""
    valid: bool
    model_used: Optional[str]
    errors: List[str]
    parsed_command: Any
    command_type: str


class CommandValidationResult(TypedDict):
    """Result structure for comprehensive command validation"""
    overall_valid: bool
    json_schema: Dict[str, Any]
    pydantic: PydanticValidationResult
    command_type: str
    validation_summary: str


class ResponseValidationResult(TypedDict):
    """Result structure for comprehensive response validation"""
    overall_valid: bool
    json_schema: Dict[str, Any]
    pydantic: Dict[str, Any]  # Individual pydantic validation result
    expected_command: Optional[str]
    validation_summary: str


class MCPSchemaValidator:
    """Validates MCP commands and responses against JSON schema and Pydantic models."""

    def __init__(self, schema_path: str = "../schema-mcp.json"):
        """Initialize validator with JSON schema file."""
        self.schema_path = Path(schema_path)
        self._schema: Optional[Dict[str, Any]] = None
        self._command_schemas: Optional[Dict[str, Any]] = None
        self._response_schemas: Optional[Dict[str, Any]] = None
        self._load_schema()

    def _load_schema(self) -> None:
        """Load and parse the JSON schema file."""
        try:
            with open(self.schema_path, 'r', encoding='utf-8') as f:
                self._schema = json.load(f)

            # Log schema info for debugging
            schema_id = self._schema.get("$id", "unknown") if self._schema else "none"
            schema_title = self._schema.get("title", "unknown") if self._schema else "none"
            print(f"✅ Loaded schema: {schema_title} ({schema_id})")

            # Extract command and response schemas for easier access
            if self._schema is not None:
                self._command_schemas = self._schema.get("properties", {}).get("commands", {}).get("properties", {})
                self._response_schemas = self._schema.get("properties", {}).get("responses", {}).get("properties", {})

                # Schema loading completed successfully
                if self._command_schemas:
                    command_count = len(self._command_schemas.keys())
                    print(f"🔍 Loaded {command_count} commands from schema")

        except (FileNotFoundError, json.JSONDecodeError) as e:
            print(f"❌ CRITICAL: Failed to load schema from {self.schema_path}: {e}")
            print("🛑 Stopping execution - schema loading failed")
            exit(1)

    @beartype
    def validate_command_json_schema(self, command_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Validate command against JSON Schema.
        Returns validation result with success status and details.
        """
        result: Dict[str, Any] = {
            "valid": False,
            "schema_used": None,
            "errors": [],
            "command_type": command_data.get("method", "unknown")
        }

        command_type = command_data.get("method", "").lower()

        # Find the appropriate command schema
        schema_key = None
        if self._command_schemas is not None:
            for key in self._command_schemas.keys():
                if key.replace("_", "") == command_type.replace("_", ""):
                    schema_key = key
                    break

        if not schema_key:
            result["errors"].append(f"No JSON schema found for command type: {command_type}")
            return result

        if self._command_schemas is None:
            result["errors"].append("Command schemas not loaded")
            return result
        command_schema = self._command_schemas[schema_key]
        result["schema_used"] = schema_key

        try:
            # Create a resolver for handling $ref references
            if self._schema is None:
                result["errors"].append("Schema not loaded")
                return result
            resolver = jsonschema.RefResolver(base_uri=self._schema.get("$id", ""), referrer=self._schema)
            validator = jsonschema.Draft7Validator(command_schema, resolver=resolver)
            validator.validate(command_data)
            result["valid"] = True
        except jsonschema.ValidationError as e:
            result["errors"].append(f"JSON Schema validation error: {e.message}")
            if e.path:
                result["errors"].append(f"Error path: {' -> '.join(str(p) for p in e.path)}")
        except jsonschema.SchemaError as e:
            result["errors"].append(f"Schema definition error: {e.message}")

        return result

    @beartype
    def validate_response_json_schema(self, response_data: Dict[str, Any], expected_command: Optional[str] = None) -> Dict[str, Any]:
        """
        Validate response against JSON Schema.
        Returns validation result with success status and details.
        """
        result: Dict[str, Any] = {
            "valid": False,
            "schema_used": None,
            "errors": [],
            "command_type": expected_command or "unknown"
        }

        if self._schema is None:
            result["errors"].append("Schema not loaded")
            return result

        # Create a resolver for handling $ref references
        resolver = jsonschema.RefResolver(base_uri=self._schema.get("$id", ""), referrer=self._schema)

        # First validate against base CommandResponse schema
        base_response_schema = self._schema.get("definitions", {}).get("CommandResponse", {})

        try:
            base_validator = jsonschema.Draft7Validator(base_response_schema, resolver=resolver)
            base_validator.validate(response_data)
        except jsonschema.ValidationError as e:
            if isinstance(result["errors"], list):
                result["errors"].append(f"Base response validation error: {e.message}")
            return result

        # If we have a specific command, validate against specific response schema
        if expected_command and self._response_schemas is not None:
            response_schema_key = f"{expected_command}_response"
            if response_schema_key in self._response_schemas:
                response_schema = self._response_schemas[response_schema_key]
                result["schema_used"] = response_schema_key

                try:
                    specific_validator = jsonschema.Draft7Validator(response_schema, resolver=resolver)
                    specific_validator.validate(response_data)
                    result["valid"] = True
                except jsonschema.ValidationError as e:
                    if isinstance(result["errors"], list):
                        result["errors"].append(f"Specific response validation error: {e.message}")
                        if e.path:
                            result["errors"].append(f"Error path: {' -> '.join(str(p) for p in e.path)}")
            else:
                # No specific schema, base validation passed
                result["valid"] = True
                result["schema_used"] = "base_response"
        else:
            # No specific command provided, base validation passed
            result["valid"] = True
            result["schema_used"] = "base_response"

        return result

    @beartype
    def validate_command_pydantic(self, command_data: Dict[str, Any]) -> PydanticValidationResult:
        """
        Validate command using Pydantic models.
        Returns validation result with success status and details.
        """
        result: PydanticValidationResult = {
            "valid": False,
            "model_used": None,
            "errors": [],
            "parsed_command": None,
            "command_type": command_data.get("method", "unknown")
        }

        try:
            parsed_command = parse_mcp_command(command_data)
            result["valid"] = True
            result["parsed_command"] = parsed_command
            result["model_used"] = type(parsed_command).__name__
        except (ValidationError, ValueError) as e:
            if isinstance(e, ValidationError):
                if isinstance(result["errors"], list):
                    for error in e.errors():
                        field_path = " -> ".join(str(loc) for loc in error["loc"])
                        result["errors"].append(f"Pydantic validation error at {field_path}: {error['msg']}")
            else:
                if isinstance(result["errors"], list):
                    result["errors"].append(f"Command parsing error: {str(e)}")

        return result

    @beartype
    def validate_response_pydantic(self, response_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Validate response using Pydantic models.
        Returns validation result with success status and details.
        """
        result: Dict[str, Any] = {
            "valid": False,
            "model_used": "MCPResponse",
            "errors": [],
            "parsed_response": None
        }

        try:
            parsed_response = MCPResponse(**response_data)
            result["valid"] = True
            result["parsed_response"] = parsed_response
        except ValidationError as e:
            if isinstance(result["errors"], list):
                for error in e.errors():
                    field_path = " -> ".join(str(loc) for loc in error["loc"])
                    result["errors"].append(f"Pydantic validation error at {field_path}: {error['msg']}")

        return result

    @beartype
    def comprehensive_command_validation(self, command_data: Dict[str, Any]) -> CommandValidationResult:
        """
        Perform comprehensive validation using both JSON Schema and Pydantic.
        Returns detailed validation results from both methods.
        """
        json_result = self.validate_command_json_schema(command_data)
        pydantic_result = self.validate_command_pydantic(command_data)

        return {
            "overall_valid": json_result["valid"] and pydantic_result["valid"],
            "json_schema": json_result,
            "pydantic": pydantic_result,
            "command_type": command_data.get("method", "unknown"),
            "validation_summary": self._create_validation_summary(json_result, pydantic_result)
        }

    @beartype
    def comprehensive_response_validation(self, response_data: Dict[str, Any], expected_command: Optional[str] = None) -> ResponseValidationResult:
        """
        Perform comprehensive validation using both JSON Schema and Pydantic.
        Returns detailed validation results from both methods.
        """
        json_result = self.validate_response_json_schema(response_data, expected_command)
        pydantic_result = self.validate_response_pydantic(response_data)

        return {
            "overall_valid": json_result["valid"] and pydantic_result["valid"],
            "json_schema": json_result,
            "pydantic": pydantic_result,
            "expected_command": expected_command,
            "validation_summary": self._create_validation_summary(json_result, pydantic_result)
        }

    @beartype
    def _create_validation_summary(self, json_result: Dict[str, Any], pydantic_result: Union[PydanticValidationResult, Dict[str, Any]]) -> str:
        """Create a human-readable validation summary."""
        if json_result["valid"] and pydantic_result["valid"]:
            return "✅ Passed both JSON Schema and Pydantic validation"
        if json_result["valid"] and not pydantic_result["valid"]:
            return "⚠️ Passed JSON Schema but failed Pydantic validation"
        if not json_result["valid"] and pydantic_result["valid"]:
            return "⚠️ Passed Pydantic but failed JSON Schema validation"
        return "❌ Failed both JSON Schema and Pydantic validation"


class _ValidatorSingleton:
    """Singleton validator instance holder."""
    _instance: Optional['MCPSchemaValidator'] = None

    @classmethod
    def get_validator(cls, schema_path: str = "../schema-mcp.json") -> MCPSchemaValidator:
        """Get or create a validator instance."""
        if cls._instance is None:
            # Find schema file relative to this script location
            script_dir = Path(__file__).parent
            schema_file_path = script_dir / schema_path

            if not schema_file_path.exists():
                print(f"❌ CRITICAL: Schema file not found at {schema_file_path.absolute()}")
                print("🛑 Stopping execution - schema is required for validation")
                exit(1)
            cls._instance = MCPSchemaValidator(str(schema_file_path))

        return cls._instance


@beartype
def get_validator(schema_path: str = "../schema-mcp.json") -> MCPSchemaValidator:
    """Get or create a global validator instance."""
    return _ValidatorSingleton.get_validator(schema_path)


@beartype
def validate_command(command_data: Dict[str, Any]) -> CommandValidationResult:
    """
    Validate command using both JSON Schema and Pydantic validation.

    Args:
        command_data: The command data to validate

    Returns:
        Comprehensive validation result with both JSON Schema and Pydantic results
    """
    validator = get_validator()
    return validator.comprehensive_command_validation(command_data)


@beartype
def validate_response(response_data: Dict[str, Any], expected_command: Optional[str] = None) -> ResponseValidationResult:
    """
    Validate response using both JSON Schema and Pydantic validation.

    Args:
        response_data: The response data to validate
        expected_command: The command that generated this response (optional)

    Returns:
        Comprehensive validation result with both JSON Schema and Pydantic results
    """
    validator = get_validator()
    return validator.comprehensive_response_validation(response_data, expected_command)
