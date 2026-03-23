// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4JohnsonCounter4bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel4JohnsonCounter4Bit::initTestCase()
{
}

void TestLevel4JohnsonCounter4Bit::cleanup()
{
}

void TestLevel4JohnsonCounter4Bit::testJohnsonCounter_data()
{
    QTest::addColumn<int>("cycleNumber");
    QTest::addColumn<int>("expectedPattern");

    // Sequence: inverted feedback ring counter
    // 0001→0011→0111→1111→1110→1100→1000→0000→0001
    QTest::newRow("cycle 0") << 0 << 0x1;  // 0001
    QTest::newRow("cycle 1") << 1 << 0x3;  // 0011
    QTest::newRow("cycle 2") << 2 << 0x7;  // 0111
    QTest::newRow("cycle 3") << 3 << 0xF;  // 1111
    QTest::newRow("cycle 4") << 4 << 0xE;  // 1110
    QTest::newRow("cycle 5") << 5 << 0xC;  // 1100
    QTest::newRow("cycle 6") << 6 << 0x8;  // 1000
    QTest::newRow("cycle 7") << 7 << 0x0;  // 0000
}

void TestLevel4JohnsonCounter4Bit::testJohnsonCounter()
{
    QFETCH(int, cycleNumber);
    QFETCH(int, expectedPattern);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Clock and preset inputs
    InputSwitch clk;
    InputSwitch preset;

    // Output LEDs
    Led counterOut[4];

    // Load Johnson counter IC
    IC *johnsonIC = loadBuildingBlockIC("level4_johnson_counter_4bit.panda");

    // Add elements
    builder.add(&clk, &preset, johnsonIC);
    for (int i = 0; i < 4; ++i) {
        builder.add(&counterOut[i]);
    }

    // Connect clock and preset
    builder.connect(&clk, 0, johnsonIC, "CLK");
    builder.connect(&preset, 0, johnsonIC, "PRESET");

    // Connect counter outputs
    for (int i = 0; i < 4; ++i) {
        QString outputLabel = QString("Q%1").arg(i);
        builder.connect(johnsonIC, outputLabel, &counterOut[i], 0);
    }

    auto *simulation = builder.initSimulation();

    // Initialize
    clk.setOn(false);
    preset.setOn(true);  // PRESET = HIGH (inactive)
    simulation->update();

    // Activate preset: sets FF[0] Q=1, clears FF[1-3] Q=0
    preset.setOn(false);  // PRESET = LOW (active)
    simulation->update();

    // Release preset
    preset.setOn(true);  // PRESET = HIGH (inactive)
    simulation->update();

    // Run for cycleNumber clock cycles
    for (int cycle = 0; cycle < cycleNumber; ++cycle) {
        clockCycle(simulation, &clk);
    }

    // Read counter value
    int result = readMultiBitOutput(QVector<GraphicElement *>({
        &counterOut[0], &counterOut[1], &counterOut[2], &counterOut[3]
    }));

    QCOMPARE(result & 0xF, expectedPattern & 0xF);
}

