// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Shared geometric/layout constants used across layers.
 */

#pragma once

namespace Constants {

/// Scene grid unit in pixels. Elements snap to gridSize/2, ports are laid out on
/// the half-grid, and the scene background dots are spaced by gridSize.
inline constexpr int gridSize = 16;

} // namespace Constants
