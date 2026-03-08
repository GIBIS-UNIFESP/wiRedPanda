// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLogicElementsErrors : public QObject
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
    void testConnectionCycles();
    void testInputValueBoundary();
    void testGateWithZeroInputs();
};

