// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel9SingleCycleCPU8Bit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testCPUStructure();
    void testCPUSimpleExecution_data();
    void testCPUSimpleExecution();
    void testCPUReset();
    void testCPUInstructionVisible();
    void testCPUFlagsExposed();
    void testResultRegisterReadable();
    void testFlagSemanticCorrectness();
};
