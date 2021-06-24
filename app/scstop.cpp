// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scstop.h"
#include "common.h"
#include "simulationcontroller.h"

SCStop::SCStop(SimulationController *sc)
    : m_sc(sc)
{
    COMMENT("Stoping!!!!!", 0 );
    if (m_sc->isRunning()) {
        m_restart = true;
        m_sc->stop();
    }
}

void SCStop::release()
{
    COMMENT("Releasing!!!!!", 0 );
    if (m_restart) {
        m_sc->start();
    }
}
SCStop::~SCStop()
{
    release();
}
