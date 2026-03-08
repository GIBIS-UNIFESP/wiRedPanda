// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel9MultiCycleCPU8Bit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testCPUStructure();
    void testCycleCounter_data();
    void testCycleCounter();
    void testCPUReset();
    void testPipelineStageSequence();
    void testInstructionVisibleDuringFetch();
    void testResultRegisterReadable();
    void testInstructionStabilityAcrossPipelineStages();
};

