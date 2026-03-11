// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/SimulationBlocker.h"

#include "App/Core/Common.h"
#include "App/Simulation/Simulation.h"

SimulationBlocker::SimulationBlocker(Simulation *simulation)
    : m_simulation(simulation)
{
    qCDebug(zero) << "Stopping.";

    if (m_simulation->isRunning()) {
        m_restart = true;
        m_simulation->stop();
    }
}

SimulationBlocker::~SimulationBlocker()
{
    qCDebug(zero) << "Releasing.";

    if (m_restart) {
        m_simulation->start();
    }
}

