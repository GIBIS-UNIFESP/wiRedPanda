/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QObject>

class SimulationController;

class SimulationBlocker
{
public:
    explicit SimulationBlocker(SimulationController *simController);
    ~SimulationBlocker();

private:
    Q_DISABLE_COPY(SimulationBlocker)

    SimulationController *m_simController;
    bool m_restart = false;
};
