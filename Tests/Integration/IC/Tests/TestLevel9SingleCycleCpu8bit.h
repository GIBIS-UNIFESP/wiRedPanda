// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel9SingleCycleCPU8Bit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testCPUStructure();
    void testCPUReset();
    void testPCIncrement();

    void testISA_ADD_data();
    void testISA_ADD();
    void testISA_SUB_data();
    void testISA_SUB();
    void testISA_AND_data();
    void testISA_AND();
    void testISA_OR_data();
    void testISA_OR();
    void testISA_XOR_data();
    void testISA_XOR();
    void testISA_NOT_data();
    void testISA_NOT();
    void testISA_SHL_data();
    void testISA_SHL();
    void testISA_SHR_data();
    void testISA_SHR();

    void testZeroFlag_data();
    void testZeroFlag();
    void testSignFlag_data();
    void testSignFlag();

    void testMultipleInstructions();

    void testISA_STORE();
    void testISA_LOAD();
    void testISA_StoreLoad();
};

