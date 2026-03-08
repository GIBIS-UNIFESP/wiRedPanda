// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>

class TestConnectionValidity : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // Connection Validity Tests (8 tests)
    void testConnectionStatusValid();
    void testConnectionStatusInvalid();
    void testPortDeletionDeletesConnection();
    void testMultipleConnectionsStatus();
    void testPortValidityWithConnections();
    void testConnectionWithDisconnectedPorts();
    void testInputPortWithMultipleConnections();
    void testOutputPortWithMultipleConnections();

private:
    QTemporaryDir m_tempDir;
};

