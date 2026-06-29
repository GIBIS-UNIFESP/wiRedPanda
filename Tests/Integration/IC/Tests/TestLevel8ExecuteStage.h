// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel8ExecuteStage : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testExecuteStage_data();
    void testExecuteStage();

    // Per-bit lane isolation through the embedded 8-bit ALU.
    void testInputPortIsolation_data();
    void testInputPortIsolation();
    void testOutputPortIsolation_data();
    void testOutputPortIsolation();

    void testExecuteStageStructure();
};
