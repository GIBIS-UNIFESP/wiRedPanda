
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class Simulation;

class SimulationBlocker
{
public:
    explicit SimulationBlocker(Simulation *simulation);
    ~SimulationBlocker();

private:
    Q_DISABLE_COPY(SimulationBlocker)

    Simulation *m_simulation;
    bool m_restart = false;
};
