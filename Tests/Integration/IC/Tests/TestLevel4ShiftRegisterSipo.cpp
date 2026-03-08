// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4ShiftRegisterSipo.h"

#include <QFile>
#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel4ShiftRegisterSIPO::initTestCase()
{
}

void TestLevel4ShiftRegisterSIPO::cleanup()
{
}

void TestLevel4ShiftRegisterSIPO::testShiftRegisterSIPO_data()
{
    QTest::addColumn<QVector<bool>>("shiftInSequence");  // Bits to shift in
    QTest::addColumn<int>("expectedOutput");             // Expected parallel output

    // NOTE: SIPO shifts LEFT - first bit in goes to position 0 (LSB)
    // Shift sequence [b0, b1, b2, b3] → output Q3|Q2|Q1|Q0 = [b0, b1, b2, b3]

    // Shift in [0, 0, 0, 1]: first bit 0 to pos0, ... last bit 1 to pos3
    QTest::newRow("shift in 0001") << QVector<bool>({false, false, false, true}) << 0x1;

    // Shift in [1, 0, 0, 0]: first bit 1 to pos0, ... last bit 0 to pos3
    QTest::newRow("shift in 1000") << QVector<bool>({true, false, false, false}) << 0x8;

    // Shift in [0, 1, 0, 1]: first 0 pos0, 1 pos1, 0 pos2, 1 pos3 → 0b0101 = 5
    QTest::newRow("shift in 0101") << QVector<bool>({false, true, false, true}) << 0x5;

    // Shift in [1, 0, 1, 0]: first 1 pos0, 0 pos1, 1 pos2, 0 pos3 → 0b1010 = 10
    QTest::newRow("shift in 1010") << QVector<bool>({true, false, true, false}) << 0xA;

    // Shift in [1, 1, 1, 1]: all ones → 0b1111 = 15
    QTest::newRow("shift in 1111") << QVector<bool>({true, true, true, true}) << 0xF;

    // Shift in [0, 0, 0, 0]: all zeros → 0b0000 = 0
    QTest::newRow("shift in 0000") << QVector<bool>({false, false, false, false}) << 0x0;
}

void TestLevel4ShiftRegisterSIPO::testShiftRegisterSIPO()
{
    QFETCH(QVector<bool>, shiftInSequence);
    QFETCH(int, expectedOutput);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Input controls
    InputSwitch clk;
    InputSwitch serialIn;

    // Output LEDs (parallel output)
    Led dataOut[4];

    // Load SIPO shift register IC
    IC *sipoIC = loadBuildingBlockIC("level4_shift_register_sipo.panda");

    // Add elements
    builder.add(&clk, &serialIn, sipoIC);
    for (int i = 0; i < 4; ++i) {
        builder.add(&dataOut[i]);
    }

    // Connect clock and serial input
    builder.connect(&clk, 0, sipoIC, "CLK");
    builder.connect(&serialIn, 0, sipoIC, "SIN");

    // Connect parallel outputs
    for (int i = 0; i < 4; ++i) {
        builder.connect(sipoIC, QString("Q%1").arg(i), &dataOut[i], 0);
    }

    auto *simulation = builder.initSimulation();

    clk.setOn(false);
    simulation->update();

    // Shift in bits sequentially
    for (int bit = 0; bit < shiftInSequence.size(); ++bit) {
        serialIn.setOn(shiftInSequence[bit]);
        simulation->update();

        // Complete clock pulse (rising + falling edge)
        clockCycle(simulation, &clk);
    }

    // Read parallel output
    int result = readMultiBitOutput(QVector<GraphicElement *>({
        &dataOut[0], &dataOut[1], &dataOut[2], &dataOut[3]
    }));

    QCOMPARE(result, expectedOutput);
}

