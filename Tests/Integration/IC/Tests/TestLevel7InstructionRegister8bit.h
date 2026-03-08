// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel7InstructionRegister8Bit : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    // Functional Tests
    void testInstructionRegister8Bit();

    // Structure Tests
    void testInstructionRegister8BitStructure();
};

