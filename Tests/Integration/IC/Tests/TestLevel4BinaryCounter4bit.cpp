// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4BinaryCounter4bit.h"

#include <QFile>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel4BinaryCounter4Bit::initTestCase()
{
}

void TestLevel4BinaryCounter4Bit::cleanup()
{
}

void TestLevel4BinaryCounter4Bit::testBinaryCounter_data()
{
    QTest::addColumn<int>("numCycles");
    QTest::addColumn<int>("expectedFinalCount");

    // Count up 1 cycle
    QTest::newRow("count 1 cycle") << 1 << 1;

    // Count up 3 cycles
    QTest::newRow("count 3 cycles") << 3 << 3;

    // Count up 8 cycles
    QTest::newRow("count 8 cycles") << 8 << 8;

    // Count up 15 cycles
    QTest::newRow("count 15 cycles") << 15 << 15;

    // Count up 16 cycles (wrap to 0)
    QTest::newRow("count 16 cycles (wrap)") << 16 << 0;

    // Count up 17 cycles (wrap to 1)
    QTest::newRow("count 17 cycles") << 17 << 1;
}

void TestLevel4BinaryCounter4Bit::testBinaryCounter()
{
    QFETCH(int, numCycles);
    QFETCH(int, expectedFinalCount);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Inputs
    InputSwitch clk, rst;

    // Output LEDs
    Led counterOut[4];

    // Load IC
    IC *counterIC = loadBuildingBlockIC("level4_binary_counter_4bit.panda");

    // Add elements
    builder.add(&clk, &rst, counterIC);
    for (int i = 0; i < 4; ++i) {
        builder.add(&counterOut[i]);
    }

    // Connect inputs
    builder.connect(&clk, 0, counterIC, "CLK");
    builder.connect(&rst, 0, counterIC, "RST");

    // Connect counter outputs
    for (int i = 0; i < 4; ++i) {
        QString outputLabel = QString("Q%1").arg(i);
        builder.connect(counterIC, outputLabel, &counterOut[i], 0);
    }

    auto *simulation = builder.initSimulation();

    // Reset counter to 0 (RST is active-LOW)
    clk.setOn(false);
    rst.setOn(false);           // Assert reset
    simulation->update();
    rst.setOn(true);            // Release reset
    simulation->update();

    // Run counter for specified cycles
    for (int cycle = 0; cycle < numCycles; ++cycle) {
        TestUtils::clockCycle(simulation, &clk);
    }

    // Read counter value
    int result = TestUtils::readMultiBitOutput(QVector<GraphicElement *>({
        &counterOut[0], &counterOut[1], &counterOut[2], &counterOut[3]
    }));

    QCOMPARE(result & 0xF, expectedFinalCount & 0xF);
}
