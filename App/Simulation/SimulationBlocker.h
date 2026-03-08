// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief RAII guard that temporarily stops the simulation while in scope.
 */

#pragma once

#include <QObject>

class Simulation;

/**
 * \class SimulationBlocker
 * \brief RAII guard that stops the simulation on construction and restarts it on destruction.
 *
 * \details Used to safely perform operations (e.g. loading a file, rebuilding the
 * element map) that must not run while the simulation timer is active.
 *
 * Example:
 * \code
 * {
 *     SimulationBlocker blocker(simulation);
 *     // ... do non-thread-safe circuit modification ...
 * } // simulation restarts here
 * \endcode
 */
class SimulationBlocker
{
public:
    /**
     * \brief Stops \a simulation if it was running and records whether to restart.
     * \param simulation Simulation to block.
     */
    explicit SimulationBlocker(Simulation *simulation);

    /// Restarts the simulation if it was running when this blocker was created.
    ~SimulationBlocker();

private:
    Q_DISABLE_COPY(SimulationBlocker)

    Simulation *m_simulation; ///< The guarded simulation instance.
    bool m_restart = false;   ///< Whether to restart on destruction.
};

