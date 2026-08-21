// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel3Register1Bit : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    // Functional Tests
    void testRegisterAsyncReset();
    void testRegisterWriteOne();
    void testRegisterHoldWhenWriteDisabled();
    void testRegisterWriteZero();
    void testRegisterResetOverridesWrite();
    void testRegisterWriteAfterResetReleased();

    // Structure Tests
    void test1BitRegisterStructure();
};
