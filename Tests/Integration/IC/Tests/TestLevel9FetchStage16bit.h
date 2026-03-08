// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel9FetchStage16Bit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testFetchStage16Bit_data();
    void testFetchStage16Bit();
    void testPCIncrement();
    void testPCLoadValue();
    void testInstructionFieldsDecoded();
    void testSrcBitsMatchInstruction();
    void testResetClearsPC();
    void testPCWrapAround();
};
