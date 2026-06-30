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
    void testEventQueueSameTime();
    void testEventQueueClear();

    // --- Per-element propagation delay ---
    void testDefaultPropagationDelays();
    void testZeroDelayElements();

    // --- Successor graph ---
    void testSuccessorGraphBuilt();

    // --- Functional vs temporal mode ---
    void testFunctionalModeDefault();
    void testTemporalModeAdvancesTime();

    // --- Temporal engine: zero delay equivalence ---
    void testZeroDelayAndGate();
    void testZeroDelayCascadedGates();
    void testZeroDelayFeedbackConverges();

    // --- Temporal engine: propagation delays ---
    void testNotGateDelayPropagation();
    void testChainedGatesCumulativeDelay();

    // --- Temporal engine: delays apply inside ICs (flattened netlist) ---
    void testICInternalDelayPropagation();
    void testWatchedICOutputPinNotOneTickStale();
    void testNestedICDelayAccumulation();
    void testNestedICDelayAccumulationDeepNesting();

    // --- Temporal engine: input change detection ---
    void testInputSwitchSchedulesEvent();

    // --- Waveform recorder ---
    void testRecorderWatchAndRecord();
    void testRecorderWatchSignalDedups();
    void testRecorderDeduplication();
    void testRecorderStatusAt();
    void testRecorderIntegration();
    void testRecorderSubTickResolution();
    void testRecorderStatusAtEmptyTrace();
    void testRecorderRecordAllSkipsNullAndCollapsesSameTime();

    // --- Waveform widget ---
    void testWidgetSizeHint();
    void testWaveformWidgetZoom();
};
