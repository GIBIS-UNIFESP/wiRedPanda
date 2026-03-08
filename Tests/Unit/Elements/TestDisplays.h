// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestDisplays : public QObject
{
    Q_OBJECT

private slots:
    // Display_7 Tests (3 tests)
    void testDisplay7Configuration();
    void testDisplay7SegmentControl();
    void testDisplay7PortNames();

    // Display_14 Tests (2 tests)
    void testDisplay14Configuration();
    void testDisplay14SegmentControl();

    // Display_16 Tests (2 tests)
    void testDisplay16Configuration();
    void testDisplay16SegmentControl();

    // Color Tests (1 test)
    void testDisplayColors();

    // Integration Tests (2 tests)
    void testDisplay7InCircuit();
    void testDisplayWithDecoder();

    // Basic Display7 test (migrated from testelements)
    void testDisplay7();
};

