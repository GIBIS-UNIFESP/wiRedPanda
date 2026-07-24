// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestSimulationUnit : public QObject
{
    Q_OBJECT

private slots:

    void testSimulationWithNoElements();
    void testAddRemoveClockDuringSimulation();
    void testElementRemovalMidSimulation();

    // Regression: B22 — Simulation::start/stop must not emit Sentry breadcrumbs
    void testSimulationStartStopNoBreadcrumbsB22();

    // Regression: Phase 3 must refresh output-port visuals even with no wire attached
    void testUnconnectedOutputPortVisualUpdates();

    void testInitializeReturnsFalseWithNoHost();
    void testInitializeSkipsNullItemsAndFailsWithNoElements();
    void testUpdatePortWithNullPortsAreNoOps();
    void testCollectSequentialElementsSkipsNullElements();
    void testUpdateFlushesPendingVisualsOnLaterIdleTick();
};
