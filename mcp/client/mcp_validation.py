#!/usr/bin/env python3
"""
MCPValidation: Response validation and assertion helpers for MCP testing

This module provides validation methods for MCP (Model Context Protocol)
testing including response validation, element verification, and assertion helpers.
"""

import asyncio
from typing import TYPE_CHECKING, Any, Dict, Optional, cast

from beartype import beartype
# Import pydantic models
from mcp_models import MCPResponse, TestResults

# Import for runtime use
from mcp_infrastructure import MCPInfrastructure
from mcp_organizer import MCPTestOrganizer

if TYPE_CHECKING:
    pass  # All imports now done at runtime


class MCPValidation:
    """Validation methods: Response validation, element verification, assertion helpers"""

    def __init__(self, test_results: TestResults, infrastructure: MCPInfrastructure, organizer: Optional[MCPTestOrganizer] = None) -> None:
        self.test_results = test_results
        self.infrastructure = infrastructure
        self.organizer = organizer

    @beartype
    async def assert_success(self, response: MCPResponse, test_name: str) -> bool:
        """Assert command was successful with proper response validation"""
        # Response is already validated by pydantic MCPResponse model
        if not response.success:
            error = response.error or "Unknown error"
            self.test_results.errors.append(f"{test_name}: {error}")
            print(f"❌ {test_name}: {error}")
            return False

        self.infrastructure.output.success(f"✅ {test_name}")
        return True

    @beartype
    async def get_response_result(self, response: MCPResponse) -> Optional[Dict[str, Any]]:
        """Get the result data from a successful response"""
        return response.result if response.success else None

    async def assert_success_with_image(self, response: MCPResponse, test_name: str, test_context: str = "") -> bool:
        """Assert command was successful and export circuit image"""
        success = await self.assert_success(response, test_name)
        if success and self.organizer and self.organizer.export_images:
            # Export circuit image after successful operation
            image_name = f"{test_context}_{test_name}" if test_context else test_name
            print(f"   🖼️  Capturing circuit state: {image_name}")
            await self.organizer.export_circuit_image(image_name, self.infrastructure)
        return success

    @beartype
    async def assert_failure(self, response: MCPResponse, test_name: str) -> bool:
        """Assert command was expected to fail"""
        if response.success:
            self.test_results.errors.append(f"{test_name}: Expected failure but got success")
            print(f"❌ {test_name}: Expected failure but got success")
            return False

        self.infrastructure.output.success(f"✅ {test_name} (expected failure)")
        return True

    @beartype
    async def validate_element_id(self, element_id: Any, test_name: str) -> bool:
        """Validate element ID is a positive integer"""
        if not isinstance(element_id, int) or element_id <= 0:
            print(f"❌ {test_name}: Invalid element_id: {element_id}")
            self.test_results.errors.append(f"{test_name}: Invalid element_id: {element_id}")
            return False
        return True

    @beartype
    async def validate_element_creation_response(self, response: MCPResponse, test_name: str) -> Optional[int]:
        """Validate element creation response and return element_id (or None if validation fails)"""
        if not await self.assert_success(response, test_name):
            return None

        result = await self.get_response_result(response)
        if not result or "element_id" not in result:
            print(f"❌ {test_name}: Missing element_id in response")
            self.test_results.errors.append(f"{test_name}: Missing element_id")
            return None

        element_id = result["element_id"]
        if await self.validate_element_id(element_id, f"{test_name} ID validation"):
            return cast(int, element_id)
        return None

    @beartype
    async def verify_connection_exists(self, source_id: int, target_id: int, test_name: str) -> bool:
        """Verify a connection exists between two elements"""
        try:
            resp = await self.infrastructure.send_command("list_connections", {})
            if not await self.assert_success(resp, f"{test_name} - list connections"):
                return False

            result = await self.get_response_result(resp)
            if not result or "connections" not in result:
                return False

            # Look for the actual connection
            connections = result["connections"]
            for conn in connections:
                if (conn.get("source_id") == source_id and
                    conn.get("target_id") == target_id):
                    self.infrastructure.output.success(f"✅ {test_name}: Connection verified ({source_id} -> {target_id})")
                    return True

            print(f"❌ {test_name}: Connection not found ({source_id} -> {target_id})")
            return False

        except (ValueError, KeyError, AttributeError, TypeError) as e:
            print(f"❌ {test_name}: Connection verification failed: {e}")
            return False

    @beartype
    async def verify_output_value(self, element_id: int, expected_value: bool, test_name: str) -> bool:
        """Verify output value matches expected value"""
        try:
            resp = await self.infrastructure.send_command("get_output_value", {"element_id": element_id})
            if not await self.assert_success(resp, f"{test_name} - get output value"):
                return False

            result = await self.get_response_result(resp)
            if not result:
                return False

            actual_value = result.get("value")
            if actual_value == expected_value:
                self.infrastructure.output.success(f"✅ {test_name}: Output value correct ({actual_value})")
                return True

            print(f"❌ {test_name}: Expected {expected_value}, got {actual_value}")
            self.test_results.errors.append(f"{test_name}: Value mismatch - expected {expected_value}, got {actual_value}")
            return False
        except (ValueError, KeyError, AttributeError, TypeError) as e:
            print(f"❌ {test_name}: Value verification failed: {e}")
            self.test_results.errors.append(f"{test_name}: Value verification error - {e}")
            return False
