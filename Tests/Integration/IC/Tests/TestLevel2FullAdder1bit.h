// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel2FullAdder1Bit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // Parameterized Full Adder truth table tests
    void testFullAdder_data();
    void testFullAdder();

    // CPU-level Full Adder functional test
    void testCpuFullAdder_data();
    void testCpuFullAdder();

    // Structure and port validation test
    void testStructure();
};
