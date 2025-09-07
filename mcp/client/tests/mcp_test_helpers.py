#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Helper Functions and Decorators for MCP Test Suite

Common utilities to reduce code duplication and provide consistent test patterns.
"""

import functools
import time
from typing import Any, Callable, Dict, List, Optional, Tuple, TYPE_CHECKING, cast, TypeVar, ParamSpec

from beartype import beartype
from mcp_output import MCPOutput

# Import for runtime use
from mcp_runner import MCPTestRunner

if TYPE_CHECKING:
    pass  # All imports now done at runtime

# Type variables for generic decorators
P = ParamSpec('P')
T = TypeVar('T')
R = TypeVar('R')

# Global output handler
test_output = MCPOutput(verbose=False)  # TODO: Get verbose from config


# ==================== TEST DECORATORS ====================


def timing_test(threshold: float, test_name: str = "") -> Callable[[Callable[P, R]], Callable[P, R]]:
    """Decorator for timing-based performance tests

    Args:
        threshold: Maximum allowed time in seconds
        test_name: Optional custom test name for reporting

    Usage:
        @timing_test(2.0, "Circuit Creation")
        def test_create_circuit(self):
            # Test implementation
    """

    def decorator(func: Callable[..., Any]) -> Callable[..., Any]:
        @functools.wraps(func)
        def wrapper(self: Any, *args: Any, **kwargs: Any) -> Any:
            display_name = test_name or func.__name__
            start_time = time.time()
            result = func(self, *args, **kwargs)
            elapsed = time.time() - start_time

            # Report timing results using the same pattern as original tests
            if elapsed > threshold:
                print(f"❌ {display_name} slow: {elapsed:.3f}s (expected ≤{threshold:.1f}s)")
                self.test_results.total += 1
                self.test_results.failed += 1
            else:
                test_output.success(f"✅ {display_name} fast: {elapsed:.3f}s")
                self.test_results.total += 1
                self.test_results.passed += 1

            return result

        return wrapper

    return decorator


def circuit_test(circuit_spec: Dict[str, Any], cleanup_name: str = "") -> Callable[[Callable[P, R]], Callable[P, R]]:
    """Decorator to auto-setup/cleanup circuits for tests

    Args:
        circuit_spec: Circuit specification dictionary
        cleanup_name: Name for cleanup (defaults to function name)

    Usage:
        @circuit_test(BASIC_AND_CIRCUIT)
        def test_and_logic(self, mapping):
            # Test implementation with mapping available
    """

    def decorator(func: Callable[..., Any]) -> Callable[..., Any]:
        @functools.wraps(func)
        def wrapper(self: Any, *args: Any, **kwargs: Any) -> Any:
            cleanup_id = cleanup_name or func.__name__

            # Setup circuit
            resp = self.send_command("new_circuit", {})
            if not self.assert_success(resp, f"Setup for {func.__name__}"):
                return False

            mapping = self.create_circuit_from_spec(circuit_spec)

            try:
                # Run the actual test with mapping as first parameter
                result = func(self, mapping, *args, **kwargs)
            finally:
                # Always cleanup
                self.cleanup_circuit(cleanup_id)

            return result

        return wrapper

    return decorator


def test_context(context_name: str = "") -> Callable[[Callable[P, R]], Callable[P, R]]:
    """Decorator to automatically set test context for image export

    Args:
        context_name: Context name (defaults to function name)

    Usage:
        @test_context("logic_gate_testing")
        def test_and_gate(self):
            # Test implementation
    """

    def decorator(func: Callable[..., Any]) -> Callable[..., Any]:
        @functools.wraps(func)
        def wrapper(self: Any, *args: Any, **kwargs: Any) -> Any:
            test_context_id = context_name or func.__name__
            self.set_test_context(test_context_id)
            return func(self, *args, **kwargs)

        return wrapper

    return decorator


# ==================== COMMON TEST PATTERNS ====================


@beartype
def standard_test_header(category: str, test_name: str) -> None:
    """Print standardized test headers

    Args:
        category: Test category name
        test_name: Specific test name

    Standard format for test headers like:
    print("\\n=== Basic Protocol Test ===")
    """
    print(f"\n=== {category}: {test_name} ===")


@beartype
def element_creation_helper(element_type: str, x: int, y: int, label: str = "") -> Dict[str, Any]:
    """Standardized element creation parameters

    Args:
        element_type: Type of element to create
        x: X coordinate
        y: Y coordinate
        label: Optional element label

    Returns:
        Dictionary with element creation parameters
    """
    return {
        "type": element_type,
        "x": x,
        "y": y,
        "label": label or f"{element_type}_auto",
    }


@beartype
async def run_truth_table_test(test_runner: MCPTestRunner, inputs: List[int], output: int, truth_table: List[Tuple[bool, ...]], gate_name: str) -> bool:
    """Automated truth table validation

    Args:
        test_runner: Test runner instance with send_command/assert_success methods
        inputs: List of input element IDs
        output: Output element ID
        truth_table: List of (input_vals..., expected_output) tuples
        gate_name: Name of gate being tested for reporting

    Returns:
        True if all truth table entries pass, False otherwise

    Automated truth table validation for logic gates
    """
    success_count = 0
    total_tests = len(truth_table)

    for i, test_case in enumerate(truth_table):
        # Split inputs and expected output
        input_values = test_case[:-1]
        expected_output = test_case[-1]

        # Set input values
        all_inputs_set = True
        for input_id, value in zip(inputs, input_values):
            resp = await test_runner.send_command("set_input_value", {"element_id": input_id, "value": value})
            if not resp.success:
                all_inputs_set = False
                break

        if not all_inputs_set:
            print(f"❌ {gate_name} truth table test {i+1}: Failed to set inputs")
            continue

        # Small delay for propagation
        time.sleep(0.1)

        # Check output
        resp = await test_runner.send_command("get_output_value", {"element_id": output})
        if resp.success and resp.result:
            actual_output = resp.result.get("value")
            if actual_output == expected_output:
                test_output.success(f"✅ {gate_name} truth table test {i+1}: {input_values} → {actual_output}")
                success_count += 1
            else:
                print(f"❌ {gate_name} truth table test {i+1}: {input_values} → {actual_output} (expected {expected_output})")
        else:
            print(f"❌ {gate_name} truth table test {i+1}: Failed to get output")

    success_rate = (success_count / total_tests * 100) if total_tests > 0 else 0
    print(f"📊 {gate_name} truth table: {success_count}/{total_tests} passed ({success_rate:.1f}%)")

    return success_count == total_tests


@beartype
def measure_operation_timing(operation: Callable[[], T], threshold: float, test_name: str, runner: MCPTestRunner) -> Tuple[bool, float]:
    """Standard timing measurement with validation

    Args:
        operation: Callable that performs the operation to time
        threshold: Maximum allowed time in seconds
        test_name: Name for reporting
        runner: Test runner for result tracking

    Returns:
        Tuple of (success, elapsed_time)

    Standard timing measurement with validation
    """
    start_time = time.time()
    operation()
    elapsed = time.time() - start_time

    # Use same reporting pattern as original tests
    if elapsed > threshold:
        print(f"❌ {test_name} slow: {elapsed:.3f}s (expected ≤{threshold:.1f}s)")
        runner.test_results.total += 1
        runner.test_results.failed += 1
        success = False
    else:
        test_output.success(f"✅ {test_name} fast: {elapsed:.3f}s")
        runner.test_results.total += 1
        runner.test_results.passed += 1
        success = True

    return success, elapsed


# ==================== CIRCUIT CREATION HELPERS ====================


@beartype
def create_and_verify_circuit(test_runner: MCPTestRunner, circuit_spec: Dict[str, Any], expected_count: int, test_name: str) -> Optional[Dict[int, int]]:
    """Create circuit and verify element count

    Args:
        test_runner: Test runner instance
        circuit_spec: Circuit specification
        expected_count: Expected number of elements
        test_name: Test name for reporting

    Returns:
        Element mapping if successful, None otherwise

    Create circuit and verify element count
    """
    mapping = cast(Dict[int, int], test_runner.create_circuit_from_spec(circuit_spec))

    if len(mapping) == expected_count:
        test_output.success(f"✅ {test_name} circuit created: {len(mapping)} elements")
        test_runner.test_results.total += 1
        test_runner.test_results.passed += 1
        return mapping

    print(f"❌ {test_name} circuit creation failed: {len(mapping)}/{expected_count} elements")
    test_runner.test_results.total += 1
    test_runner.test_results.failed += 1
    return None


@beartype
async def setup_simulation_test(test_runner: MCPTestRunner, circuit_spec: Dict[str, Any], expected_count: int, test_name: str) -> Optional[Dict[int, int]]:
    """Standard pattern for setting up simulation tests

    Args:
        test_runner: Test runner instance
        circuit_spec: Circuit specification
        expected_count: Expected element count
        test_name: Test name for reporting

    Returns:
        Element mapping if successful, None otherwise

    Standard pattern for setting up simulation tests
    """
    # Create circuit
    mapping = create_and_verify_circuit(test_runner, circuit_spec, expected_count, test_name)
    if not mapping:
        return None

    # Start simulation
    resp = await test_runner.send_command("simulation_control", {"action": "start"})
    if await test_runner.assert_success(resp, f"Start {test_name} simulation"):
        return mapping

    print(f"❌ {test_name} simulation failed to start")
    test_runner.test_results.total += 1
    test_runner.test_results.failed += 1
    return None


# ==================== VALIDATION HELPERS ====================


@beartype
async def validate_element_creation_batch(test_runner: MCPTestRunner, element_types: List[str], base_x: int = 100, base_y: int = 100, spacing: int = 100) -> Dict[str, bool]:
    """Validate creation of multiple element types

    Args:
        test_runner: Test runner instance
        element_types: List of element types to test
        base_x: Starting X coordinate
        base_y: Starting Y coordinate
        spacing: Spacing between elements

    Returns:
        Dictionary mapping element types to success/failure

    Validate creation of multiple element types
    """
    results = {}

    for i, element_type in enumerate(element_types):
        params = element_creation_helper(
            element_type,
            base_x + (i * spacing),
            base_y,
            f"Test_{element_type}"
        )
        resp = await test_runner.send_command("create_element", params)
        element_id = await test_runner.validate_element_creation_response(resp, f"Create {element_type}")

        results[element_type] = element_id is not None

    return results


# ==================== PERFORMANCE TEST HELPERS ====================


@beartype
def run_performance_benchmark(test_runner: MCPTestRunner, operation_name: str, operation: Callable[[], T], iterations: int = 5, threshold: float = 1.0) -> Dict[str, Any]:
    """Run performance benchmark with statistics

    Args:
        test_runner: Test runner instance
        operation_name: Name for reporting
        operation: Callable to benchmark
        iterations: Number of iterations to run
        threshold: Maximum allowed average time

    Returns:
        Dictionary with timing statistics

    Standard timing measurement with validation
    """
    times = []

    for i in range(iterations):
        start_time = time.time()
        try:
            operation()
            elapsed = time.time() - start_time
            times.append(elapsed)
        except Exception as e:
            print(f"❌ {operation_name} iteration {i+1} failed: {e}")
            times.append(float("inf"))

    if times:
        avg_time = sum(times) / len(times)
        max_time = max(times)
        min_time = min(times)

        # Report results using original test patterns
        if avg_time <= threshold:
            test_output.success(f"✅ {operation_name} performance: avg {avg_time:.3f}s, max {max_time:.3f}s")
            test_runner.test_results.total += 1
            test_runner.test_results.passed += 1
        else:
            print(f"❌ {operation_name} performance: avg {avg_time:.3f}s (expected ≤{threshold:.1f}s)")
            test_runner.test_results.total += 1
            test_runner.test_results.failed += 1
        return {
            "avg_time": avg_time,
            "max_time": max_time,
            "min_time": min_time,
            "iterations": len(times),
            "success": avg_time <= threshold
        }

    print(f"❌ {operation_name} benchmark failed: no valid measurements")
    test_runner.test_results.total += 1
    test_runner.test_results.failed += 1
    return {"success": False}


# ==================== ERROR HANDLING HELPERS ====================


@beartype
async def test_invalid_commands(test_runner: MCPTestRunner, invalid_commands: List[Dict[str, Any]], category_name: str) -> int:
    """Test multiple invalid commands and expect failures

    Args:
        test_runner: Test runner instance
        invalid_commands: List of {"method": str, "params": dict} entries (JSON-RPC 2.0 format)
        category_name: Category name for reporting

    Returns:
        Number of commands that properly failed

    Test multiple invalid commands and expect failures
    """
    success_count = 0

    for test_case in invalid_commands:
        resp = await test_runner.send_command(test_case["method"], test_case["params"])
        if await test_runner.assert_failure(resp, f"{category_name}: {test_case['method']}"):
            success_count += 1

    return success_count
