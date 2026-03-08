// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5ModuloCounter4bit.h"

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

void TestLevel5ModuloCounter4Bit::initTestCase()
{
}

void TestLevel5ModuloCounter4Bit::cleanup()
{
}

// Helper to initialize modulo counter from its default state
static void resetModuloCounter(Simulation *simulation, InputSwitch &clk)
{
    // IC D flip-flops initialize to Q=1 (NOR latch default), so counter starts at 0xF.
    // One clock cycle advances 0xF -> 0x0 to reset the counter.
    clk.setOn(false);
    simulation->update();
    clockCycle(simulation, &clk);
}

void TestLevel5ModuloCounter4Bit::testModuloCounter_data()
{
    QTest::addColumn<int>("modulo");
    QTest::addColumn<int>("cycles");
    QTest::addColumn<int>("expectedPattern");

    QTest::newRow("mod5_cycle6") << 5 << 6 << 1;
    QTest::newRow("mod5_complete") << 5 << 5 << 0;
    QTest::newRow("mod10_to_9") << 10 << 9 << 9;
    QTest::newRow("mod10_wrap") << 10 << 11 << 1;
}

void TestLevel5ModuloCounter4Bit::testModuloCounter()
{
    QFETCH(int, modulo);
    QFETCH(int, cycles);
    QFETCH(int, expectedPattern);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Input controls
    InputSwitch clk;
    InputSwitch moduloIn[4];

    // Output LEDs
    Led counterOut[4];
    Led overflowOut;

    // Load modulo counter IC
    IC *mcIC = loadBuildingBlockIC("level5_modulo_counter_4bit.panda");

    // Add elements
    builder.add(&clk, mcIC);
    for (int i = 0; i < 4; ++i) {
        builder.add(&moduloIn[i], &counterOut[i]);
    }
    builder.add(&overflowOut);

    // Connect inputs
    builder.connect(&clk, 0, mcIC, "CLK");
    for (int i = 0; i < 4; ++i) {
        builder.connect(&moduloIn[i], 0, mcIC, QString("Modulo[%1]").arg(i));
    }

    // Connect outputs
    for (int i = 0; i < 4; ++i) {
        builder.connect(mcIC, QString("Q[%1]").arg(i), &counterOut[i], 0);
    }
    builder.connect(mcIC, "Overflow", &overflowOut, 0);

    auto *simulation = builder.initSimulation();

    // Set modulo value
    for (int i = 0; i < 4; ++i) {
        moduloIn[i].setOn((modulo >> i) & 1);
    }
    simulation->update();

    // Reset counter from default 0xF to 0
    resetModuloCounter(simulation, clk);

    // Run counter for specified cycles
    for (int cycle = 0; cycle < cycles; ++cycle) {
        clockCycle(simulation, &clk);
    }

    // Read counter value
    int result = readMultiBitOutput(QVector<GraphicElement *>({
        &counterOut[0], &counterOut[1], &counterOut[2], &counterOut[3]
    }));

    QCOMPARE(result & 0xF, expectedPattern & 0xF);
}

