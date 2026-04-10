// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/SimulationThrottleDisabler.h"

#include "App/Core/Common.h"
#include "App/Simulation/Simulation.h"

SimulationThrottleDisabler::SimulationThrottleDisabler(Simulation *simulation)
    : m_simulation(simulation)
{
    qCDebug(zero) << "Disabling visual throttle.";
    m_simulation->setVisualThrottleEnabled(false);
}

SimulationThrottleDisabler::~SimulationThrottleDisabler()
{
    qCDebug(zero) << "Re-enabling visual throttle.";
    m_simulation->setVisualThrottleEnabled(true);
}

