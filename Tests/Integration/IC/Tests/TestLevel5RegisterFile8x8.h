// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel5RegisterFile8X8 : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    // 8x8 Register File Tests
    void testRegisterFile8x8_data();
    void testRegisterFile8x8();

    // Boundary tests (from test_memory_boundary, 8x8 rows only)
    void testBoundary8x8_data();
    void testBoundary8x8();
};

