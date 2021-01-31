/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SCSTOP_H
#define SCSTOP_H

class SimulationController;

class SCStop
{
    SimulationController *sc;
    bool restart = false;

public:
    SCStop(SimulationController *sc);
    void release();
    ~SCStop();
};

#endif /* SCSTOP_H */
