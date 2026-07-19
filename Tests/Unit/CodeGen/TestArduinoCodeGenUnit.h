// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestArduinoCodeGenUnit : public QObject
{
    Q_OBJECT

private slots:

    void testAndGateCircuit();
    void testMultiGateCircuit();
    void testMuxCircuit();
    void testDemuxCircuit();
    void testEmptyScene();

    // Regression: F19 — every TruthTable output gets emitted, not just 0
    void testTruthTableMultiOutput();

    void testSRLatchCircuit();
    void testGenerateTestbenchDeclaresSequentialState();
    void testTooManyInputRotaryPositionsThrowsPinOverflow();
    void testGenerateTestbenchUnwritablePathThrows();
    void testTruthTableInputTiedToVccResolvesToHigh();
    void testGenerateTestbenchNonSequentialSettleLoop();
    void testDemuxWithFourOutputsUsesTwoSelectLines();
};
