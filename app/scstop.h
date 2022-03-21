/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SCSTOP_H
#define SCSTOP_H

class SimulationController;

class SCStop
{
public:
    SCStop(SimulationController *sc);
    void release();
    ~SCStop();

private:
    SimulationController *m_sc;
    bool m_restart = false;
};

#endif /* SCSTOP_H */
