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


from typing import List

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

    # ==================== SHARED HELPER ====================

    async def _test_element_type_group(
        self,
        element_types: List[str],
        group_name: str,
        test_context: str,
        fail_on_error: bool = True,
    ) -> bool:
        """Create each element type in the list and report results.

        Args:
            element_types: List of element type strings to attempt creating.
            group_name: Human-readable group label for print output.
            test_context: Context string passed to set_test_context().
            fail_on_error: If True, failed creates set the return value to False.
        """
        self.set_test_context(test_context)

        all_passed: bool = True
        successful_creates = 0
        failed_creates: List[str] = []

        for i, element_type in enumerate(element_types):
            try:
                create_resp = await self.send_command(
                    "create_element",
                    {"type": element_type, "x": 100.0 + i * 50.0, "y": 100.0, "label": f"Test_{element_type}"},
                )

                if create_resp.success:
                    successful_creates += 1
                    self.infrastructure.output.success(f"✅ Successfully created {element_type}")
                else:
                    failed_creates.append(element_type)
                    print(f"⚠️  Failed to create {element_type}: {create_resp.error}")

            except Exception as e:
                failed_creates.append(element_type)
                print(f"❌ Exception creating {element_type}: {e}")

        print(f"{group_name} creation results: {successful_creates}/{len(element_types)} successful")

        if failed_creates:
            print(f"Failed {group_name.lower()} types: {', '.join(failed_creates)}")
            if fail_on_error:
                all_passed = False

        return all_passed

    # ==================== TEST METHODS ====================

    @beartype
    async def test_missing_logic_gate_types(self) -> bool:
        """Test missing logic gate element types coverage"""
        print("\n=== Missing Logic Gate Types Test ===")
        return await self._test_element_type_group(
            ["Nand", "Xor", "Xnor", "Node"],
            "Logic gate",
            "test_missing_logic_gate_types",
        )

    @beartype
    async def test_missing_input_element_types(self) -> bool:
        """Test missing input element types coverage"""
        print("\n=== Missing Input Element Types Test ===")
        return await self._test_element_type_group(
            ["InputSwitch", "InputRotary", "InputVcc", "InputGnd"],
            "Input element",
            "test_missing_input_element_types",
        )

    @beartype
    async def test_missing_output_element_types(self) -> bool:
        """Test missing output element types coverage"""
        print("\n=== Missing Output Element Types Test ===")
        return await self._test_element_type_group(
            ["Buzzer", "AudioBox", "Display7", "Display14", "Display16"],
            "Output element",
            "test_missing_output_element_types",
        )

    @beartype
    async def test_missing_sequential_element_types(self) -> bool:
        """Test missing sequential element types coverage"""
        print("\n=== Missing Sequential Element Types Test ===")
        return await self._test_element_type_group(
            ["SRFlipFlop", "SRLatch"],
            "Sequential element",
            "test_missing_sequential_element_types",
            fail_on_error=False,
        )

    @beartype
    async def test_missing_complex_element_types(self) -> bool:
        """Test missing complex element types coverage"""
        print("\n=== Missing Complex Element Types Test ===")
        # IC is intentionally excluded: it requires a .panda file and must be created
        # via instantiate_ic, not create_element. See test_simulation_ic_operations.
        return await self._test_element_type_group(
            ["Mux", "Demux", "TruthTable", "Line", "Text"],
            "Complex element",
            "test_missing_complex_element_types",
        )
