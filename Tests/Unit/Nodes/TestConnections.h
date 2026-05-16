// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestConnections : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // Basic lifecycle (4 tests)
    void testCreateConnection();
    void testSetPorts();
    void testDisconnectPorts();
    void testOtherPort();

    // Connection validation (5 tests)
    void testValidOutputToInput();
    void testRejectOutputToOutput();
    void testRejectInputToInput();
    void testRejectSelfConnection();
    void testRejectDuplicateConnection();

    // Port capacity (4 tests)
    void testSingleInputConnection();
    void testMultipleInputInvalid();
    void testOutputFanOut();
    void testRequiredPortValidation();

    // Status propagation (3 tests)
    void testStatusPropagation();
    void testMultiConnectionStatus();
    void testInvalidPortStatus();

    // Integration (2 tests)
    void testConnectionPositionUpdate();
    void testPortIsConnected();
};
