// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestTemporalSimulation : public QObject
{
    Q_OBJECT

public:
    TestTemporalSimulation() = default;

private slots:
    // --- EventQueue unit tests ---
    void testEventQueueOrdering();
    void testEventQueueSameTimeFIFO();
    void testEventQueueClear();

    // --- LogicElement delay ---
    void testDefaultPropagationDelays();
    void testZeroDelayElements();

    // --- Successor tracking ---
    void testSuccessorListsBuilt();

    // --- Mode switching ---
    void testModeDefaultIsFunctional();
    void testSetModeRestartsSimulation();

    // --- Temporal engine: zero delay equivalence ---
    void testZeroDelayAndGate();
    void testZeroDelayCascadedGates();
    void testZeroDelayFeedbackConverges();

    // --- Temporal engine: propagation delays ---
    void testNotGateDelayPropagation();
    void testChainedGatesCumulativeDelay();

    // --- Temporal engine: clock scheduling ---
    void testClockEdgeScheduling();

    // --- Temporal engine: input change detection ---
    void testInputSwitchSchedulesEvent();

    // --- Waveform recorder ---
    void testRecorderWatchAndRecord();
    void testRecorderDeduplication();
    void testRecorderStatusAt();
    void testRecorderIntegration();

    // --- Waveform widget ---
    void testWidgetSizeHint();
};
