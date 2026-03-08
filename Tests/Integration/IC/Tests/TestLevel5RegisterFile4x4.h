// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

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

    // Read mux debug tests (from test_memory_readmux)
    void testRegisterFile4x4_debug_readmux_data();
    void testRegisterFile4x4_debug_readmux();

    // Read address tests (from test_memory_readaddr)
    void testRegisterFile4x4_debug_readaddr_data();
    void testRegisterFile4x4_debug_readaddr();

    // Boundary tests (from test_memory_boundary, 4x4 rows only)
    void testRegisterFileBoundary4x4_data();
    void testRegisterFileBoundary4x4();

    // Timing tests (from test_memory_timing)
    void testMemoryTiming_data();
    void testMemoryTiming();

    // Timing edge case tests (from test_memory_timing_edges)
    void testRegisterFileTimingEdges_data();
    void testRegisterFileTimingEdges();
};

