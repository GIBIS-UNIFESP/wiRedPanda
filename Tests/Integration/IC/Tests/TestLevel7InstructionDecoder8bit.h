// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel7InstructionDecoder8Bit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testInstructionDecoder8Bit_data();
    void testInstructionDecoder8Bit();

    void testInstructionDecoder8BitStructure();
};

