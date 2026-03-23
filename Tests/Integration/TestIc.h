// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTest>

class TestIC : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // Port Mapping (2 tests)
    void testICPortMapping();
    void testICEmptyPorts();

    // Nested ICs (2 tests)
    void testICNestedSingleLevel();
    void testICNestedMultiLevel();

    // Logic Generation (1 test)
    void testICLogicGeneration();

    // Serialization (1 test)
    void testICSaveLoad();

    // Integration (4 tests)
    void testICInSimulation();
    void testICStatusPropagation();
    void testICRequiredPorts();
    void testICDefaultValues();

    // Error Handling (1 test)
    void testICMissingFile();

    void testICNestedCircuitPortMapping();
    void testICNestedCircuitSignalPropagation();
    void testICFileDependencyResolution();

    // Migration (4 tests)
    void testICMigrationPreservesConnections();
    void testICFileMigrationCreatesBackup();         // old sub-circuit file gets versioned backup
    void testICFileMigrationUpdatesSubcircuitVersion(); // sub-circuit re-saved at current version
    void testICFileMigrationDisabledSkips();         // no backup when migration disabled
};

