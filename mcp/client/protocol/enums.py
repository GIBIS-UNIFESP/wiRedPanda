#!/usr/bin/env python3
"""
MCP Protocol Enumerations

This module provides backward compatibility by re-exporting enums
from the domain package. Use domain.enums directly for new code.
"""

# Import enums from domain package
from domain.enums import ElementType, SimulationAction, ImageExportFormat, WaveformExportFormat

# Re-export for backward compatibility
__all__ = ["ElementType", "SimulationAction", "ImageExportFormat", "WaveformExportFormat"]
