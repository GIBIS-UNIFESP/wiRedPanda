// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scstop.h"
#include "clock.h"
#include "common.h"
#include "simulationcontroller.h"

SCStop::SCStop(SimulationController *sc)
    : m_sc(sc)
{
    qCDebug(zero) << "Stoping.";
    if (m_sc->isRunning()) {
        m_restart = true;
        m_sc->stop();
        Clock::reset = true;
        Clock::pause = true;
    }
}

void SCStop::release()
{
    qCDebug(zero) << "Releasing.";
    if (m_restart) {
        m_sc->start();
        Clock::pause = false;
    }
}
SCStop::~SCStop()
{
    release();
}
