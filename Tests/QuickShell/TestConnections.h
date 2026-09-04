// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/**
 * \class TestConnections
 * \brief Domain-layer subset of Tests/Unit/Wiring/TestConnections.cpp.
 *
 * \details ConnectionManager-based rejection tests are not included here: ConnectionManager.cpp
 * is listed under CMakeSources.cmake's WIDGETS_ONLY_SOURCES, so it isn't compiled into
 * wiredpanda_lib at all. Calling it -- its interactive wire-dragging instance API
 * (startFromOutput()/startFromInput()/tryComplete()/hasEditedConnection(), which takes a
 * Scene* in its constructor) or its static isConnectionAllowed()/connectionRejectionReason()
 * methods -- from a Quick test would compile against the lightweight, Scene-forward-declaring
 * header but fail to link.
 */
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

    // Connection validation: only the non-ConnectionManager case ports here
    void testValidOutputToInput();

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
