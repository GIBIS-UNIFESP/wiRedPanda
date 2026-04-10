// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief RAII guard that disables the visual refresh throttle while in scope.
 */

#pragma once

#include <QObject>

class Simulation;

/**
 * \class SimulationThrottleDisabler
 * \brief RAII guard that disables the visual refresh throttle for the duration of its scope.
 *
 * \details The simulation throttle skips phases 3–4 (port-status updates) most ticks
 * as a live-UI performance optimisation.  Batch operations like the BeWavedDolphin
 * waveform sweep must read exact output values after every step, so the throttle
 * must not apply.
 *
 * Example:
 * \code
 * {
 *     SimulationThrottleDisabler noThrottle(simulation);
 *     for (int col = 0; col < length; ++col) {
 *         simulation->update(); // always runs all 4 phases
 *     }
 * } // throttle re-enabled here
 * \endcode
 */
class SimulationThrottleDisabler
{
public:
    /// Disables the visual refresh throttle on \a simulation.
    explicit SimulationThrottleDisabler(Simulation *simulation);

    /// Re-enables the visual refresh throttle.
    ~SimulationThrottleDisabler();

private:
    Q_DISABLE_COPY(SimulationThrottleDisabler)

    Simulation *m_simulation; ///< The guarded simulation instance.
};

