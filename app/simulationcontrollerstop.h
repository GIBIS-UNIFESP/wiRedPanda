/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

class SimulationController;

class SimulationControllerStop // RAII
{
public:
    explicit SimulationControllerStop(SimulationController *simController);
    ~SimulationControllerStop();

private:
    SimulationController *m_simController;
    bool m_restart = false;
};
