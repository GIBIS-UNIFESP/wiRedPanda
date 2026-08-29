// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/SimulationThrottleDisabler.h"

#include "App/Core/Common.h"
#include "App/Simulation/Simulation.h"

SimulationThrottleDisabler::SimulationThrottleDisabler(Simulation *simulation)
    : m_simulation(simulation)
    , m_wasEnabled(simulation->isVisualThrottleEnabled())
{
    qCDebug(zero) << "Disabling visual throttle.";
    m_simulation->setVisualThrottleEnabled(false);
}

SimulationThrottleDisabler::~SimulationThrottleDisabler()
{
    // RESTORE, do not enable. Setting true unconditionally makes an inner guard undo an outer
    // one: the throttle would come back on inside a scope built to keep it off, and the sweep
    // that scope protects would resume skipping the port-status phases and read stale values.
    qCDebug(zero) << "Restoring visual throttle to" << m_wasEnabled;
    m_simulation->setVisualThrottleEnabled(m_wasEnabled);
}
