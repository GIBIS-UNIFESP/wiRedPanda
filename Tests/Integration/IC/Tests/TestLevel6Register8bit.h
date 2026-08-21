// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel6Register8Bit : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    // Functional Tests -- split from one ~175-line test8BitRegister() bundling 6 unrelated
    // scenarios into separate functions so a failure in one doesn't hide the rest.
    void testRegisterResetToZero();
    void testRegisterWriteAndHoldWhenDisabled();
    void testRegisterReenableAfterHold();
    void testRegisterWriteAllOnesAndHoldAfter();
    void testRegisterIndividualBitMapping();
    void testRegisterMidRangeValues();
    void test8BitRegisterSequential();
    void test8BitRegisterEdgeCases();

    // Structure Tests
    void test8BitRegisterStructure();
};
