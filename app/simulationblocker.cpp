// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simulationblocker.h"

#include "clock.h"
#include "common.h"
#include "simulationcontroller.h"

SimulationBlocker::SimulationBlocker(SimulationController *simController)
    : m_simController(simController)
{
    qCDebug(zero) << QObject::tr("Stopping.");
    if (m_simController->isRunning()) {
        m_restart = true;
        m_simController->stop();
        Clock::reset = true;
        Clock::pause = true;
    }
}

SimulationBlocker::~SimulationBlocker()
{
    qCDebug(zero) << QObject::tr("Releasing.");
    if (m_restart) {
        m_simController->start();
        Clock::pause = false;
    }
}
