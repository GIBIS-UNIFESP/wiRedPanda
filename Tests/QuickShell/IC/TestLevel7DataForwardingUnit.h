// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel7DataForwardingUnit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testDataForwardingUnit_data();
    void testDataForwardingUnit();

    // Per-bit lane isolation through the selected mux lane: a one-hot input must
    // forward to a one-hot output (input side), and a one-cold input must leave
    // exactly that output bit low (output side) — proving the 8 per-bit muxes
    // are independent with no bit-to-bit cross-wiring.
    void testInputPortIsolation_data();
    void testInputPortIsolation();
    void testOutputPortIsolation_data();
    void testOutputPortIsolation();

    void testDataForwardingUnitStructure();
};
