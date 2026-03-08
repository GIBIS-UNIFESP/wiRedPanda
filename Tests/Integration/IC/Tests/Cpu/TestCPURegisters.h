// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestCPURegisters : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testFlagRegister();
    void testFlagRegister_data();
    void testInstructionRegister();
    void testInstructionRegister_data();
    void testSingleGatedRegister();
};

