/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

class SimulationController;

class SCStop
{
public:
    explicit SCStop(SimulationController *sc);
    ~SCStop();

    void release();

private:
    SimulationController *m_sc;
    bool m_restart = false;
};

