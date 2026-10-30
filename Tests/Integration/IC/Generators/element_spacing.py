#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""Element spacing constants for IC generators."""

# Horizontal spacing for elements in a row
HORIZONTAL_GATE_SPACING = 104  # Between logic gate centers horizontally

# Vertical spacing between logic levels
VERTICAL_STAGE_SPACING = 104  # Between stages of logic (input → gates → output)

# The step every placement snaps a position to (half the scene grid, Element::snapToGrid()): rows packed flush
# against a measured size are spaced by a multiple of it, or snapping can pull neighbours into each other
GRID_SNAP_STEP = 8
