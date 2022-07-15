// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simulationblocker.h"

#include "clock.h"
#include "common.h"
#include "simulation.h"

SimulationBlocker::SimulationBlocker(Simulation *simulation)
    : m_simulation(simulation)
{
    qCDebug(zero) << QObject::tr("Stopping.");
    if (m_simulation->isRunning()) {
        m_restart = true;
        m_simulation->stop();
    }
}

SimulationBlocker::~SimulationBlocker()
{
    qCDebug(zero) << QObject::tr("Releasing.");
    if (m_restart) {
        m_simulation->start();
    }
}
