// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel5InstructionDecoder4Bit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testInstructionDecoder4Bit_data();
    void testInstructionDecoder4Bit();

    void testInstructionDecoderStructure();
};

