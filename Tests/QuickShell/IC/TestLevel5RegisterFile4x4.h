// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestLevel5RegisterFile4X4 : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    // 4x4 Register File IC Tests
    void testRegisterFile4x4_data();
    void testRegisterFile4x4();

    // Read mux debug tests
    void testRegisterFile4x4_debug_readmux_data();
    void testRegisterFile4x4_debug_readmux();

    // Read address tests
    void testRegisterFile4x4_debug_readaddr_data();
    void testRegisterFile4x4_debug_readaddr();

    // Boundary tests (4x4 rows only)
    void testRegisterFileBoundary4x4_data();
    void testRegisterFileBoundary4x4();

    // Timing tests
    void testMemoryTiming_data();
    void testMemoryTiming();

    // Timing edge case tests
    void testRegisterFileTimingEdges_data();
    void testRegisterFileTimingEdges();

    // Both read ports exercised concurrently
    void testDualReadPorts();
};
