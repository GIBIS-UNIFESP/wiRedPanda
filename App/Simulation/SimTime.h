// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Simulation time type for the temporal simulation engine.
 */

#pragma once

#include <cstdint>

/// Simulation time in nanoseconds.
using SimTime = uint64_t;

/// Sentinel value indicating an unset or invalid simulation time.
constexpr SimTime SIM_TIME_UNSET = UINT64_MAX;
