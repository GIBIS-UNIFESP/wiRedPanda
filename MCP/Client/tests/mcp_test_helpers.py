#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Helper Functions and Decorators for MCP Test Suite

Common utilities to reduce code duplication and provide consistent test patterns.
"""

import functools
import time
from typing import Any, Callable, Dict, ParamSpec, TypeVar

# Type variables for generic decorators
P = ParamSpec("P")
R = TypeVar("R")


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
                self.infrastructure.output.success(f"✅ {display_name} fast: {elapsed:.3f}s")
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
