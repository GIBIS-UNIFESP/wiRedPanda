// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/SimulationBlocker.h"

#include "App/Core/Common.h"
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

