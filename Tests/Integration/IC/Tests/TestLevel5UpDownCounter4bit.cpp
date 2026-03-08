// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5UpDownCounter4bit.h"

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

// ============================================================
// Up/Down Counter IC Helper
// ============================================================

struct UpDownCounterSetup {
    InputSwitch clk;
    InputSwitch dir;
    InputSwitch en;
    Led counterOut[4];
};

static Simulation *buildUpDownCounterCircuit(WorkSpace &workspace, UpDownCounterSetup &s)
{
    CircuitBuilder builder(workspace.scene());

    IC *udcIC = loadBuildingBlockIC("level5_up_down_counter_4bit.panda");

    builder.add(&s.clk, &s.dir, &s.en, udcIC);
    for (int i = 0; i < 4; ++i) {
        builder.add(&s.counterOut[i]);
    }

    builder.connect(&s.clk, 0, udcIC, "CLK");
    builder.connect(&s.dir, 0, udcIC, "Direction");
    builder.connect(&s.en, 0, udcIC, "Enable");
    for (int i = 0; i < 4; ++i) {
        builder.connect(udcIC, QString("Q[%1]").arg(i), &s.counterOut[i], 0);
    }

    return builder.initSimulation();
}

static int readCounterValue(UpDownCounterSetup &s)
{
    return readMultiBitOutput(QVector<GraphicElement *>({
        &s.counterOut[0], &s.counterOut[1], &s.counterOut[2], &s.counterOut[3]
    })) & 0xF;
}

static void initializeCounterToValue(Simulation *simulation, UpDownCounterSetup &s, int targetValue)
{
    // IC D flip-flops initialize to Q=1 (NOR latch default), so counter starts at 0xF.
    // One clock cycle advances 0xF -> 0x0 to reset the counter.
    s.clk.setOn(false);
    s.en.setOn(true);
    s.dir.setOn(true);  // Up direction
    simulation->update();
    clockCycle(simulation, &s.clk);

    // Count up to target value from 0
    for (int i = 0; i < targetValue; ++i) {
        clockCycle(simulation, &s.clk);
    }
}

void TestLevel5UpDownCounter4Bit::initTestCase()
{
}

void TestLevel5UpDownCounter4Bit::cleanup()
{
}

void TestLevel5UpDownCounter4Bit::testUpDownCounter_data()
{
    QTest::addColumn<bool>("direction");
    QTest::addColumn<bool>("enable");
    QTest::addColumn<int>("cycles");
    QTest::addColumn<int>("expectedValue");

    // Counter always initializes to 0, then counts up or down
    // Direction: 1 = up, 0 = down
    // Enable: 1 = count, 0 = hold
    QTest::newRow("count_up") << true << true << 3 << 3;           // 0 + 3 = 3
    QTest::newRow("count_up_large") << true << true << 4 << 4;     // 0 + 4 = 4
    QTest::newRow("count_down") << false << true << 3 << 13;       // 0 - 3 (wrap) = 13 (0xD)
    QTest::newRow("count_down_by_2") << false << true << 2 << 14;  // 0 - 2 (wrap) = 14 (0xE)
    QTest::newRow("hold_when_disabled") << true << false << 5 << 0; // Enable=0: stays at 0 despite 5 cycles
    QTest::newRow("hold_mixed") << false << false << 3 << 0;        // Enable=0: stays at 0 regardless of direction
}

void TestLevel5UpDownCounter4Bit::testUpDownCounter()
{
    QFETCH(bool, direction);
    QFETCH(bool, enable);
    QFETCH(int, cycles);
    QFETCH(int, expectedValue);

    WorkSpace workspace;
    UpDownCounterSetup s;
    auto *simulation = buildUpDownCounterCircuit(workspace, s);
    if (!simulation) {
        QFAIL("Failed to load up/down counter IC");
        return;
    }

    // Initialize counter to 0 (from default 0xF)
    initializeCounterToValue(simulation, s, 0);

    // Set test parameters
    s.dir.setOn(direction);
    s.en.setOn(enable);
    simulation->update();

    // Run counter for specified cycles
    for (int cycle = 0; cycle < cycles; ++cycle) {
        clockCycle(simulation, &s.clk);
    }

    QCOMPARE(readCounterValue(s), expectedValue & 0xF);
}

// ============================================================
// Up/Down Counter From Value Tests
// Tests counting from a non-zero initial value by first counting up
// ============================================================
void TestLevel5UpDownCounter4Bit::testUpDownCounterFromValue_data()
{
    QTest::addColumn<int>("initialValue");
    QTest::addColumn<bool>("direction");
    QTest::addColumn<int>("cycles");
    QTest::addColumn<int>("expectedValue");

    QTest::newRow("count_up_from_5") << 5 << true << 4 << 9;
    QTest::newRow("count_down_from_5") << 5 << false << 3 << 2;
    QTest::newRow("count_down_to_zero") << 2 << false << 2 << 0;
}

void TestLevel5UpDownCounter4Bit::testUpDownCounterFromValue()
{
    QFETCH(int, initialValue);
    QFETCH(bool, direction);
    QFETCH(int, cycles);
    QFETCH(int, expectedValue);

    WorkSpace workspace;
    UpDownCounterSetup s;
    auto *simulation = buildUpDownCounterCircuit(workspace, s);
    if (!simulation) {
        QFAIL("Failed to load up/down counter IC");
        return;
    }

    // Initialize counter to initialValue (from default 0xF)
    initializeCounterToValue(simulation, s, initialValue);

    // Set actual test parameters
    s.dir.setOn(direction);
    s.en.setOn(true);
    simulation->update();

    // Run counter for specified cycles
    for (int cycle = 0; cycle < cycles; ++cycle) {
        clockCycle(simulation, &s.clk);
    }

    QCOMPARE(readCounterValue(s), expectedValue & 0xF);
}
