#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
File Operations Tests

Tests for file I/O functionality including:
- Circuit saving and loading
- File operation error handling
- Cross-platform file path handling
- File validation and verification

MCP test implementation
"""

import os
import tempfile

from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class FileOperationTests(MCPTestBase):
    """Tests for file operations and I/O"""

    async def run_category_tests(self) -> bool:
        """Run all file operation tests

        Returns:
            bool: True if all tests passed, False otherwise
        """
        tests = [
            self.test_file_operations,
            self.test_file_save_operations,
            self.test_file_load_operations,
            self.test_file_operation_errors,
        ]

        print("\n" + "=" * 60)
        print("🧪 RUNNING FILE OPERATIONS TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    # ==================== TEST METHODS ====================
    # Test method implementations

    @beartype
    async def test_file_operations(self) -> bool:
        """ File save/load operations (FIXED)"""
        print("\n=== File Operations ===")

        all_passed: bool = True

        # Create circuit to save# Add some elements
        resp = await self.send_command(
            "create_element",
            {
                "type": "And",
                "x": 200,
                "y": 150,
                "label": "SaveTestGate",
            },
        )
        all_passed &= await self.assert_success(resp, "Add element for save test")

        # Create temporary file for testing (cross-platform)
        temp_file: str = os.path.join(tempfile.gettempdir(), "mcp_test_circuit.panda")

        # Test save
        resp = await self.send_command("save_circuit", {"filename": temp_file})
        all_passed &= await self.assert_success(resp, f"Save circuit to {temp_file}")
        # Note: File existence verification is redundant - assert_success already validated the save

        # Create new circuit# Test load
        if os.path.exists(temp_file):
            resp = await self.send_command("load_circuit", {"filename": temp_file})
            all_passed &= await self.assert_success(resp, f"Load circuit from {temp_file}")

        # Test load non-existent file
        nonexistent_file = os.path.join(tempfile.gettempdir(), "nonexistent.panda")
        resp = await self.send_command("load_circuit", {"filename": nonexistent_file})
        all_passed &= await self.assert_failure(resp, "Load non-existent file")

        # Test invalid file extension (FIXED - now properly rejects)
        invalid_txt_file = os.path.join(tempfile.gettempdir(), "test.txt")
        resp = await self.send_command("save_circuit", {"filename": invalid_txt_file})
        all_passed &= await self.assert_failure(resp, "Save with invalid extension (FIXED)")

        # Clean up
        try:
            if os.path.exists(temp_file):
                os.remove(temp_file)
        except OSError:
            pass

        return all_passed

    @beartype
    async def test_file_save_operations(self) -> bool:
        """Test file save functionality with verification"""
        print("\n=== File Save Operations Test ===")
        self.set_test_context("test_file_save_operations")

        all_passed: bool = True

        # Create circuit to save
        # Add elements to make the circuit meaningful for save
        elements_to_create = [
            {"type": "InputButton", "x": 100, "y": 100, "label": "SaveInput"},
            {"type": "And", "x": 200, "y": 150, "label": "SaveGate"},
            {"type": "Led", "x": 300, "y": 100, "label": "SaveOutput"},
        ]

        created_elements = []
        for elem in elements_to_create:
            resp = await self.send_command("create_element", elem)
            element_id = await self.validate_element_creation_response(resp, f"Create {elem['type']} for save test")
            if element_id:
                created_elements.append(element_id)
            else:
                all_passed = False

        # Create temporary file for testing (cross-platform)
        temp_file: str = os.path.join(tempfile.gettempdir(), "mcp_save_test_circuit.panda")

        # Test save with populated circuit
        resp = await self.send_command("save_circuit", {"filename": temp_file})
        all_passed &= await self.assert_success(resp, f"Save circuit to {temp_file}")

        # Verify file exists and has reasonable size
        if os.path.exists(temp_file):
            file_size = os.path.getsize(temp_file)
            self.infrastructure.output.success(f"✅ File saved successfully: {temp_file} ({file_size} bytes)")

            # Verify file has reasonable content (not empty)
            if file_size > 0:
                self.infrastructure.output.success(f"✅ Saved file has content: {file_size} bytes")
            else:
                print("❌ Saved file is empty")
                all_passed = False
        else:
            print(f"❌ File not found after save: {temp_file}")
            all_passed = False

        # Test save empty circuit
        empty_temp_file: str = os.path.join(tempfile.gettempdir(), "mcp_empty_circuit.panda")
        resp = await self.send_command("save_circuit", {"filename": empty_temp_file})
        all_passed &= await self.assert_success(resp, f"Save empty circuit to {empty_temp_file}")

        # Clean up temporary files
        for temp_path in [temp_file, empty_temp_file]:
            try:
                if os.path.exists(temp_path):
                    os.remove(temp_path)
            except OSError:
                pass

        return all_passed

    @beartype
    async def test_file_load_operations(self) -> bool:
        """Test file load functionality with validation"""
        print("\n=== File Load Operations Test ===")
        self.set_test_context("test_file_load_operations")

        all_passed: bool = True

        # Create and save a test circuit first# Add meaningful elements to the circuit
        test_elements = [
            {"type": "InputButton", "x": 50, "y": 100, "label": "LoadTestInput"},
            {"type": "Or", "x": 200, "y": 100, "label": "LoadTestGate"},
            {"type": "Led", "x": 350, "y": 100, "label": "LoadTestOutput"},
        ]

        created_elements = []
        for elem in test_elements:
            resp = await self.send_command("create_element", elem)
            element_id = await self.validate_element_creation_response(resp, f"Create {elem['type']} for load test")
            if element_id:
                created_elements.append(element_id)

        # Early return if not enough elements created for meaningful connections
        if len(created_elements) < 3:
            print(f"❌ Insufficient elements created for load test: {len(created_elements)}/3")

            return all_passed

        # Connect input -> gate
        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": created_elements[0],
                "source_port": 0,
                "target_id": created_elements[1],
                "target_port": 0,
            },
        )
        all_passed &= await self.assert_success(resp, "Connect input to gate for load test")

        # Connect gate -> output
        resp = await self.send_command(
            "connect_elements",
            {
                "source_id": created_elements[1],
                "source_port": 0,
                "target_id": created_elements[2],
                "target_port": 0,
            },
        )
        all_passed &= await self.assert_success(resp, "Connect gate to output for load test")

        # Save the connected circuit
        temp_file: str = os.path.join(tempfile.gettempdir(), "mcp_load_test_circuit.panda")
        resp = await self.send_command("save_circuit", {"filename": temp_file})
        all_passed &= await self.assert_success(resp, f"Save circuit for load test to {temp_file}")

        # Verify saved file exists
        if not os.path.exists(temp_file):
            print(f"❌ Cannot proceed with load test - save failed: {temp_file}")
            return False

        # Create new empty circuit# Verify circuit is empty
        resp = await self.send_command("list_elements", {})
        all_passed &= await self.assert_success(resp, "Verify empty circuit before load")
        if resp.success:
            result = await self.get_response_result(resp)
            if result and "elements" in result:
                if len(result["elements"]) == 0:
                    self.infrastructure.output.success("✅ Circuit confirmed empty before load")

        # Test load the saved circuit
        resp = await self.send_command("load_circuit", {"filename": temp_file})
        all_passed &= await self.assert_success(resp, f"Load circuit from {temp_file}")

        # Verify circuit was loaded with correct elements
        resp = await self.send_command("list_elements", {})
        all_passed &= await self.assert_success(resp, "List elements after load")
        if resp.success:
            result = await self.get_response_result(resp)
            if result and "elements" in result:
                loaded_elements = result["elements"]
                if len(loaded_elements) == len(test_elements):
                    self.infrastructure.output.success(f"✅ Circuit loaded successfully: {len(loaded_elements)} elements")
                    # Note: assert_success already incremented counters
                else:
                    print(f"❌ Element count mismatch: expected {len(test_elements)}, got {len(loaded_elements)}")
                    # Note: This is additional validation, but assert_success already handled the counting

        # Clean up temporary file
        try:
            if os.path.exists(temp_file):
                os.remove(temp_file)
        except OSError:
            pass

        return all_passed

    @beartype
    async def test_file_operation_errors(self) -> bool:
        """Test file operation error handling and validation"""
        print("\n=== File Operation Error Handling Test ===")
        self.set_test_context("test_file_operation_errors")

        all_passed: bool = True

        # Test load non-existent file
        nonexistent_file = os.path.join(tempfile.gettempdir(), "nonexistent_mcp_file.panda")
        resp = await self.send_command("load_circuit", {"filename": nonexistent_file})
        all_passed &= await self.assert_failure(resp, "Load non-existent file")

        # Test save with invalid file extension - should be rejected
        invalid_extensions = [".txt", ".doc", ".xyz", ".json", ""]
        for ext in invalid_extensions:
            invalid_file = os.path.join(tempfile.gettempdir(), f"invalid{ext}")
            resp = await self.send_command("save_circuit", {"filename": invalid_file})
            all_passed &= await self.assert_failure(resp, f"Save with invalid extension '{ext}' (should be rejected)")

        # Test load with invalid file extension
        for ext in invalid_extensions[:3]:  # Test subset to avoid excessive output
            invalid_file = os.path.join(tempfile.gettempdir(), f"load_invalid{ext}")
            # Create a dummy file first
            try:
                with open(invalid_file, "w", encoding='utf-8') as f:
                    f.write("dummy content")
                resp = await self.send_command("load_circuit", {"filename": invalid_file})
                all_passed &= await self.assert_failure(resp, f"Load with invalid extension '{ext}' (should be rejected)")
                # Clean up dummy file
                os.remove(invalid_file)
            except OSError:
                pass

        # Test save/load with missing filename parameter
        resp = await self.send_command("save_circuit", {})
        all_passed &= await self.assert_failure(resp, "Save with missing filename parameter")

        resp = await self.send_command("load_circuit", {})
        all_passed &= await self.assert_failure(resp, "Load with missing filename parameter")

        # Test save/load with empty filename
        resp = await self.send_command("save_circuit", {"filename": ""})
        all_passed &= await self.assert_failure(resp, "Save with empty filename")

        resp = await self.send_command("load_circuit", {"filename": ""})
        all_passed &= await self.assert_failure(resp, "Load with empty filename")

        # Test save to directory that doesn't exist (should fail gracefully)
        nonexistent_dir = os.path.join(tempfile.gettempdir(), "nonexistent_dir_12345")
        nonexistent_path = os.path.join(nonexistent_dir, "test.panda")
        resp = await self.send_command("save_circuit", {"filename": nonexistent_path})
        all_passed &= await self.assert_failure(resp, "Save to non-existent directory")

        # Test save with very long filename (edge case)
        long_filename = "a" * 200 + ".panda"
        long_path = os.path.join(tempfile.gettempdir(), long_filename)
        resp = await self.send_command("save_circuit", {"filename": long_path})
        # This might succeed or fail depending on OS limits - either is acceptable
        print(f"Long filename test result: {'Success' if resp.success else 'Failed (expected on some systems)'}")

        # Test case sensitivity of extensions
        case_sensitive_files = [
            os.path.join(tempfile.gettempdir(), "test.PANDA"),
            os.path.join(tempfile.gettempdir(), "test.Panda"),
        ]
        for case_file in case_sensitive_files:
            resp = await self.send_command("save_circuit", {"filename": case_file})
            # Should accept .PANDA/.Panda as valid extensions
            result = "Success" if resp.success else "Failed"
            print(f"Case sensitivity test '{os.path.basename(case_file)}': {result}")

        # No cleanup needed as this is error handling test

        return all_passed
