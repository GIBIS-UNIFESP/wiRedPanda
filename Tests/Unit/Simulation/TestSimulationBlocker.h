// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class Scene;
class Simulation;

class TestSimulationBlocker : public QObject {
    Q_OBJECT

public:
    TestSimulationBlocker() = default;

private slots:
    // Test infrastructure

    // RAII Pattern Tests (3 tests)
    void testRAIIPatternStopsSimulation();
    void testRAIIPatternRestartsSimulation();
    void testNestedSimulationBlockers();

private:
    // Helper functions
    Scene *createSimpleCircuit();
};

