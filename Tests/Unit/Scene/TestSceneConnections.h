// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>

class TestSceneConnections : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    // Connection state and query tests (3 tests)
    void testConnectionCountInitiallyZero();
    void testConnectionQueryAfterWiring();
    void testConnectionListRetrieval();

    // Connection creation and lifecycle (4 tests)
    void testConnectionCreationBetweenPorts();
    void testConnectionWithMultiplePorts();
    void testConnectionPreservesPortReferences();
    void testConnectionListUpdatesAfterAddition();

    // Connection validation tests (3 tests)
    void testValidConnectionBetweenInputOutput();
    void testConnectionStatusTracking();
    void testConnectionRemovalUpdatesCounters();

private:
    QTemporaryDir m_tempDir;
};
