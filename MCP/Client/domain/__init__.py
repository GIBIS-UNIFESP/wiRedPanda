#!/usr/bin/env python3
"""
Domain Layer

This package contains domain-specific models, enums, and data structures
for the MCP circuit simulation system. These are the business objects
that represent circuit elements, connections, and related concepts.
"""

# Re-export enums
from .enums import ElementType, ImageExportFormat, SimulationAction, WaveformExportFormat

# Re-export domain models
from .models import (
    CircuitSpecification,
    ConnectionSpec,
    ElementInfo,
    ElementSpec,
    ICInfo,
    StateTableEntry,
    TruthTableEntry,
)

__all__ = [
    # Enums
    "ElementType",
    "SimulationAction",
    "ImageExportFormat",
    "WaveformExportFormat",
    # Domain models
    "ElementInfo",
    "ICInfo",
    "ElementSpec",
    "ConnectionSpec",
    "CircuitSpecification",
    "TruthTableEntry",
    "StateTableEntry",
]
