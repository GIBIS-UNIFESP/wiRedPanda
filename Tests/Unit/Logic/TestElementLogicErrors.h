// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestElementLogicErrors : public QObject
{
    Q_OBJECT

private slots:

    // Error Handling Tests
    void testInvalidInputPortIndex();
    void testNullPredecessor();
    void testDisconnectedInput();
    void testMultipleConnectionsSamePort();
    void testRapidStateChanges();
    void testUnconnectedGate();
    void testDeepCascading();
    void testInvalidOutputPortIndex();
    void testSelfLoopOscillation();
    void testInputValueBoundary();
    void testGateWithZeroInputs();
    void testConnectPredecessorOutOfBoundsIsNoOp();
    void testResetSimStateWithMoreSimOutputsThanPorts();

    // Status propagation tests (Phase 4+6)
    void testInvalidPropagatesChain();
    void testBoolOverloadMapsToStatus();

    // Regression: F12 — no fabricated clock edge after an invalid-input period
    void testFlipFlopNoSpuriousEdgeAfterInvalidInputs();
};
