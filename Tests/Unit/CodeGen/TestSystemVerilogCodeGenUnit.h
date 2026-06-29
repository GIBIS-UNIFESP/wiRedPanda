// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestSystemVerilogCodeGenUnit : public QObject
{
    Q_OBJECT

private slots:

    void testAndGateCircuit();
    void testOrNotCircuit();
    void testMultiGateChain();
    void testMuxCircuit();
    void testDemuxCircuit();
    void testEmptyScene();

    // Regression: F22 — constructor must throw on unwritable output path
    void testUnwritablePathThrows();
    // Regression: F19 — every TruthTable output gets emitted, not just 0
    void testTruthTableMultiOutput();
    // Regression: F23 — rotary positions become module inputs, not floating wires
    void testRotaryInputsDeclared();
};
