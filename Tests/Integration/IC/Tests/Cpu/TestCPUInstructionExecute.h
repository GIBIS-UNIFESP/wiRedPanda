// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestCPUInstructionExecute : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testInstructionExecute();
    void testInstructionExecuteArithmetic();
    void testInstructionExecuteArithmetic_data();
    void testInstructionExecuteFlags();
    void testInstructionExecuteFlags_data();
    void testInstructionExecuteLogical();
    void testInstructionExecuteLogical_data();
    void testInstructionExecute_data();
};

