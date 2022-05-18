// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simulationcontrollerstop.h"

#include "clock.h"
#include "common.h"
#include "simulationcontroller.h"

SimulationControllerStop::SimulationControllerStop(SimulationController *simController)
    : m_simController(simController)
{
    qCDebug(zero) << "Stopping.";
    if (m_simController->isRunning()) {
        m_restart = true;
        m_simController->stop();
        Clock::reset = true;
        Clock::pause = true;
    }
}

SimulationControllerStop::~SimulationControllerStop()
{
    qCDebug(zero) << "Releasing.";
    if (m_restart) {
        m_simController->start();
        Clock::pause = false;
    }
}
