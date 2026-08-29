// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/SimulationBlocker.h"

#include "App/Core/Common.h"
#include "App/Core/SentryHelpers.h"
#include "App/Simulation/Simulation.h"

SimulationBlocker::SimulationBlocker(Simulation *simulation)
    : m_simulation(simulation)
{
    qCDebug(zero) << "Stopping.";

    // Only flag for restart if the simulation was actually running — avoids
    // accidentally starting a simulation that was intentionally paused.
    if (m_simulation->isRunning()) {
        m_restart = true;
        m_simulation->stop();

        // Only when a blocker genuinely stops a running simulation, and only on the way
        // in. The WIREDPANDA-H2 trail showed the stop/start rhythm around a crash was the
        // most informative signal it had, but a crumb per construction -- most of which
        // are no-ops on an already-stopped simulation, and both ends of every scope --
        // would bury that signal in the 100-entry buffer instead of preserving it.
        sentryBreadcrumb("simulation", QStringLiteral("Blocked (was running)"));
    }
}

SimulationBlocker::~SimulationBlocker()
{
    qCDebug(zero) << "Releasing.";

    // RAII resume: restores the timer exactly when the blocking scope exits,
    // even if an exception was thrown during the blocked operation.
    if (m_restart) {
        m_simulation->start();
    }
}
