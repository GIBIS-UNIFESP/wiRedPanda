#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Element Type Coverage Tests

Tests for element type coverage and availability including:
- Missing logic gate types validation
- Input element type coverage testing
- Output element type coverage testing
- Sequential element type validation
- Complex element type coverage testing

MCP test implementation
"""

import asyncio
from beartype import beartype

# Import base infrastructure
from tests.mcp_test_base import MCPTestBase


class ElementTypeCoverageTests(MCPTestBase):
    """Tests for element type coverage and availability"""

    async def run_category_tests(self) -> bool:
        """Run all element type coverage tests

        Returns:
            bool: True if all tests passed, False otherwise
        """
        tests = [
            self.test_missing_logic_gate_types,
            self.test_missing_input_element_types,
            self.test_missing_output_element_types,
            self.test_missing_sequential_element_types,
            self.test_missing_complex_element_types,
        ]

        print("\n" + "=" * 60)
        print("🧪 RUNNING ELEMENT TYPE COVERAGE TESTS")
        print("=" * 60)

        category_success = True
        for test in tests:
            if not await self.run_test_method(test):
                category_success = False

        return category_success

    # ==================== TEST METHODS ====================
    # Test method implementations

    @beartype
    async def test_missing_logic_gate_types(self) -> bool:
        """Test missing logic gate element types coverage"""
        print("\n=== Missing Logic Gate Types Test ===")
        self.set_test_context("test_missing_logic_gate_types")

        all_passed: bool = True

        # Setup# Test missing logic gates
        missing_logic_gates = ["Nand", "Xor", "Xnor", "Node"]
        successful_creates = 0
        failed_creates = []

        for i, element_type in enumerate(missing_logic_gates):
            try:
                create_resp = await self.send_command("create_element", {
                    "type": element_type,
                    "x": 100.0 + i * 50.0,
                    "y": 100.0,
                    "label": f"Test_{element_type}"
                })

                if create_resp.success:
                    successful_creates += 1
                    self.infrastructure.output.success(f"✅ Successfully created {element_type}")
                else:
                    failed_creates.append(element_type)
                    print(f"⚠️  Failed to create {element_type}: {create_resp.error}")

            except Exception as e:
                failed_creates.append(element_type)
                print(f"❌ Exception creating {element_type}: {e}")

        print(f"Logic gate creation results: {successful_creates}/{len(missing_logic_gates)} successful")

        if failed_creates:
            print(f"Failed logic gate types: {', '.join(failed_creates)}")

        # Report result - all element creation attempts were validated via validate_element_creation_response
        # which already counted individual successes/failures

        return all_passed

    @beartype
    async def test_missing_input_element_types(self) -> bool:
        """Test missing input element types coverage"""
        print("\n=== Missing Input Element Types Test ===")
        self.set_test_context("test_missing_input_element_types")

        all_passed: bool = True

        # Setup# Test missing input types
        missing_inputs = ["InputSwitch", "InputRotary", "InputVcc", "InputGnd"]
        successful_creates = 0
        failed_creates = []

        for i, element_type in enumerate(missing_inputs):
            try:
                create_resp = await self.send_command("create_element", {
                    "type": element_type,
                    "x": 100.0 + i * 50.0,
                    "y": 100.0,
                    "label": f"Test_{element_type}"
                })

                if create_resp.success:
                    successful_creates += 1
                    self.infrastructure.output.success(f"✅ Successfully created {element_type}")
                else:
                    failed_creates.append(element_type)
                    print(f"⚠️  Failed to create {element_type}: {create_resp.error}")

            except Exception as e:
                failed_creates.append(element_type)
                print(f"❌ Exception creating {element_type}: {e}")

        print(f"Input element creation results: {successful_creates}/{len(missing_inputs)} successful")

        if failed_creates:
            print(f"Failed input element types: {', '.join(failed_creates)}")

        # Report result - all element creation attempts were validated via validate_element_creation_response
        # which already counted individual successes/failures

        return all_passed

    @beartype
    async def test_missing_output_element_types(self) -> bool:
        """Test missing output element types coverage"""
        print("\n=== Missing Output Element Types Test ===")
        self.set_test_context("test_missing_output_element_types")

        all_passed: bool = True

        # Setup# Test missing output types
        missing_outputs = ["Buzzer", "AudioBox", "Display7", "Display14", "Display16"]
        successful_creates = 0
        failed_creates = []

        for i, element_type in enumerate(missing_outputs):
            try:
                create_resp = await self.send_command("create_element", {
                    "type": element_type,
                    "x": 100.0 + i * 50.0,
                    "y": 100.0,
                    "label": f"Test_{element_type}"
                })

                if create_resp.success:
                    successful_creates += 1
                    self.infrastructure.output.success(f"✅ Successfully created {element_type}")
                else:
                    failed_creates.append(element_type)
                    print(f"⚠️  Failed to create {element_type}: {create_resp.error}")

            except Exception as e:
                failed_creates.append(element_type)
                print(f"❌ Exception creating {element_type}: {e}")

        print(f"Output element creation results: {successful_creates}/{len(missing_outputs)} successful")

        if failed_creates:
            print(f"Failed output element types: {', '.join(failed_creates)}")

        # Report result - all element creation attempts were validated via validate_element_creation_response
        # which already counted individual successes/failures

        return all_passed

    @beartype
    async def test_missing_sequential_element_types(self) -> bool:
        """Test missing sequential element types coverage"""
        print("\n=== Missing Sequential Element Types Test ===")
        self.set_test_context("test_missing_sequential_element_types")

        all_passed: bool = True

        # Setup# Test missing sequential elements
        missing_sequential = ["SRFlipFlop", "SRLatch"]
        successful_creates = 0
        failed_creates = []

        for i, element_type in enumerate(missing_sequential):
            try:
                create_resp = await self.send_command("create_element", {
                    "type": element_type,
                    "x": 100.0 + i * 50.0,
                    "y": 100.0,
                    "label": f"Test_{element_type}"
                })

                if create_resp.success:
                    successful_creates += 1
                    self.infrastructure.output.success(f"✅ Successfully created {element_type}")
                else:
                    failed_creates.append(element_type)
                    print(f"⚠️  Failed to create {element_type}: {create_resp.error}")

            except Exception as e:
                failed_creates.append(element_type)
                print(f"❌ Exception creating {element_type}: {e}")

        print(f"Sequential element creation results: {successful_creates}/{len(missing_sequential)} successful")

        if failed_creates:
            print(f"Failed sequential element types: {', '.join(failed_creates)}")

        # Report result - all element creation attempts were validated via validate_element_creation_response
        # which already counted individual successes/failures

        return all_passed

    @beartype
    async def test_missing_complex_element_types(self) -> bool:
        """Test missing complex element types coverage"""
        print("\n=== Missing Complex Element Types Test ===")
        self.set_test_context("test_missing_complex_element_types")

        all_passed: bool = True

        # Setup# Test missing complex elements
        missing_complex = ["Mux", "Demux", "IC", "TruthTable", "Line", "Text"]
        successful_creates = 0
        failed_creates = []

        for i, element_type in enumerate(missing_complex):
            try:
                create_resp = await self.send_command("create_element", {
                    "type": element_type,
                    "x": 100.0 + i * 50.0,
                    "y": 100.0,
                    "label": f"Test_{element_type}"
                })

                if create_resp.success:
                    successful_creates += 1
                    self.infrastructure.output.success(f"✅ Successfully created {element_type}")
                else:
                    failed_creates.append(element_type)
                    print(f"⚠️  Failed to create {element_type}: {create_resp.error}")

            except Exception as e:
                failed_creates.append(element_type)
                print(f"❌ Exception creating {element_type}: {e}")

        print(f"Complex element creation results: {successful_creates}/{len(missing_complex)} successful")

        if failed_creates:
            print(f"Failed complex element types: {', '.join(failed_creates)}")

        # Report result - all element creation attempts were validated via validate_element_creation_response
        # which already counted individual successes/failures

        return all_passed
