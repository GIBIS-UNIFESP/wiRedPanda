// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5LoadableCounter4bit.h"

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
// Loadable Counter IC Helper
// ============================================================

struct LoadableCounterSetup {
    InputSwitch clk;
    InputSwitch load;
    InputSwitch dataIn[4];
    Led counterOut[4];
};

static Simulation *buildLoadableCounterCircuit(WorkSpace &workspace, LoadableCounterSetup &s)
{
    CircuitBuilder builder(workspace.scene());

    IC *lcIC = loadBuildingBlockIC("level5_loadable_counter_4bit.panda");

    builder.add(&s.clk, &s.load, lcIC);
    for (int i = 0; i < 4; ++i) {
        builder.add(&s.dataIn[i], &s.counterOut[i]);
    }

    builder.connect(&s.clk, 0, lcIC, "CLK");
    builder.connect(&s.load, 0, lcIC, "Load");
    for (int i = 0; i < 4; ++i) {
        builder.connect(&s.dataIn[i], 0, lcIC, QString("D[%1]").arg(i));
    }
    for (int i = 0; i < 4; ++i) {
        builder.connect(lcIC, QString("Q[%1]").arg(i), &s.counterOut[i], 0);
    }

    return builder.initSimulation();
}

static void loadCounterValue(Simulation *simulation, LoadableCounterSetup &s, int value)
{
    s.clk.setOn(false);
    s.load.setOn(true);
    for (int i = 0; i < 4; ++i) {
        s.dataIn[i].setOn((value >> i) & 1);
    }
    simulation->update();
    clockCycle(simulation, &s.clk);
}

static int readCounterValue(LoadableCounterSetup &s)
{
    return readMultiBitOutput(QVector<GraphicElement *>({
        &s.counterOut[0], &s.counterOut[1], &s.counterOut[2], &s.counterOut[3]
    })) & 0xF;
}

void TestLevel5LoadableCounter4Bit::initTestCase()
{
}

void TestLevel5LoadableCounter4Bit::cleanup()
{
}

void TestLevel5LoadableCounter4Bit::testLoadableCounter_data()
{
    QTest::addColumn<int>("loadValue");
    QTest::addColumn<int>("countCycles");
    QTest::addColumn<int>("expectedValue");

    QTest::newRow("load_zero_count3") << 0 << 3 << 3;
    QTest::newRow("load_5_count4") << 5 << 4 << 9;
    QTest::newRow("load_10_count5") << 10 << 5 << 15;
    QTest::newRow("load_14_wrap") << 14 << 3 << 1;
}

void TestLevel5LoadableCounter4Bit::testLoadableCounter()
{
    QFETCH(int, loadValue);
    QFETCH(int, countCycles);
    QFETCH(int, expectedValue);

    WorkSpace workspace;
    LoadableCounterSetup s;
    auto *simulation = buildLoadableCounterCircuit(workspace, s);
    if (!simulation) {
        QFAIL("Failed to load loadable counter IC");
        return;
    }

    // Load the initial value
    loadCounterValue(simulation, s, loadValue);
    QCOMPARE(readCounterValue(s), loadValue & 0xF);

    // Switch to count mode
    s.load.setOn(false);
    simulation->update();

    // Run counter for specified cycles
    for (int cycle = 0; cycle < countCycles; ++cycle) {
        clockCycle(simulation, &s.clk);
    }

    QCOMPARE(readCounterValue(s), expectedValue & 0xF);
}

// ============================================================
// Binary Counter Boundary Transition Tests
// Tests ALL transitions: 0->1, 1->2, 2->3, ..., 14->15, 15->0
// ============================================================
void TestLevel5LoadableCounter4Bit::testBinaryCounterBoundaryTransitions_data()
{
    QTest::addColumn<int>("startValue");
    QTest::addColumn<int>("expectedNextValue");

    // Test all 16 boundary transitions
    QTest::newRow("transition 0->1") << 0 << 1;
    QTest::newRow("transition 1->2") << 1 << 2;
    QTest::newRow("transition 2->3") << 2 << 3;
    QTest::newRow("transition 3->4") << 3 << 4;
    QTest::newRow("transition 4->5") << 4 << 5;
    QTest::newRow("transition 5->6") << 5 << 6;
    QTest::newRow("transition 6->7") << 6 << 7;
    QTest::newRow("transition 7->8") << 7 << 8;
    QTest::newRow("transition 8->9") << 8 << 9;
    QTest::newRow("transition 9->10") << 9 << 10;
    QTest::newRow("transition 10->11") << 10 << 11;
    QTest::newRow("transition 11->12") << 11 << 12;
    QTest::newRow("transition 12->13") << 12 << 13;
    QTest::newRow("transition 13->14") << 13 << 14;
    QTest::newRow("transition 14->15") << 14 << 15;
    QTest::newRow("transition 15->0 (wrap)") << 15 << 0;
}

void TestLevel5LoadableCounter4Bit::testBinaryCounterBoundaryTransitions()
{
    QFETCH(int, startValue);
    QFETCH(int, expectedNextValue);

    WorkSpace workspace;
    LoadableCounterSetup s;
    auto *simulation = buildLoadableCounterCircuit(workspace, s);
    if (!simulation) {
        QFAIL("Failed to load loadable counter IC");
        return;
    }

    // Load start value
    loadCounterValue(simulation, s, startValue);
    QCOMPARE(readCounterValue(s), startValue & 0xF);

    // Switch to count mode and clock once
    s.load.setOn(false);
    simulation->update();
    clockCycle(simulation, &s.clk);

    QCOMPARE(readCounterValue(s), expectedNextValue & 0xF);
}

// ============================================================
// Binary Counter Timing Edge Case Tests
// Tests rapid consecutive transitions with timing assertions
// ============================================================
void TestLevel5LoadableCounter4Bit::testBinaryCounterTimingEdgeCases_data()
{
    QTest::addColumn<int>("startValue");
    QTest::addColumn<int>("expectedAfterFirstEdge");
    QTest::addColumn<int>("expectedAfterSecondEdge");
    QTest::addColumn<int>("expectedAfterThirdEdge");

    QTest::newRow("timing rapid 0->1->2->3") << 0 << 1 << 2 << 3;
    QTest::newRow("timing rapid 3->4->5->6") << 3 << 4 << 5 << 6;
    QTest::newRow("timing rapid 7->8->9->10") << 7 << 8 << 9 << 10;
    QTest::newRow("timing rapid 12->13->14->15") << 12 << 13 << 14 << 15;
    QTest::newRow("timing rapid 14->15->0->1 (wrap)") << 14 << 15 << 0 << 1;
}

void TestLevel5LoadableCounter4Bit::testBinaryCounterTimingEdgeCases()
{
    QFETCH(int, startValue);
    QFETCH(int, expectedAfterFirstEdge);
    QFETCH(int, expectedAfterSecondEdge);
    QFETCH(int, expectedAfterThirdEdge);

    WorkSpace workspace;
    LoadableCounterSetup s;
    auto *simulation = buildLoadableCounterCircuit(workspace, s);
    if (!simulation) {
        QFAIL("Failed to load loadable counter IC");
        return;
    }

    // Load start value
    loadCounterValue(simulation, s, startValue);
    QCOMPARE(readCounterValue(s), startValue & 0xF);

    // Switch to count mode
    s.load.setOn(false);
    simulation->update();

    // First transition
    clockCycle(simulation, &s.clk);
    int valueAfterFirst = readCounterValue(s);

    // Hold phase: extended settle to check for glitches
    simulation->update();
    QCOMPARE(readCounterValue(s), valueAfterFirst);

    // Second transition
    clockCycle(simulation, &s.clk);
    int valueAfterSecond = readCounterValue(s);

    // Third transition
    clockCycle(simulation, &s.clk);
    int valueAfterThird = readCounterValue(s);

    // Verify all three transitions are correct
    QCOMPARE(valueAfterFirst, expectedAfterFirstEdge & 0xF);
    QCOMPARE(valueAfterSecond, expectedAfterSecondEdge & 0xF);
    QCOMPARE(valueAfterThird, expectedAfterThirdEdge & 0xF);
}

// ============================================================
// Hold Behavior Extended - test counter stability across many consecutive cycles
// ============================================================
void TestLevel5LoadableCounter4Bit::testBinaryCounterHoldBehavior_data()
{
    QTest::addColumn<int>("startValue");
    QTest::addColumn<int>("cycleCount");
    QTest::addColumn<QString>("description");

    QTest::newRow("from 0, 5 cycles")
        << 0 << 5
        << "From 0x0: Count 0->1->2->3->4->5";

    QTest::newRow("from 0, full wrap")
        << 0 << 16
        << "From 0x0: Count 0->1->...->15->0";

    QTest::newRow("from 0, 32 cycles")
        << 0 << 32
        << "From 0x0: Two complete wraps (32 cycles)";

    QTest::newRow("from 5, cross wrap")
        << 5 << 16
        << "From 0x5: Count 5->6->...->15->0->1->...->4";

    QTest::newRow("from 12, 20 cycles")
        << 12 << 20
        << "From 0xC: Extended counting across wrap";

    QTest::newRow("from 0, extended")
        << 0 << 100
        << "From 0x0: Extended stability (100 cycles = 6+ wraps)";
}

void TestLevel5LoadableCounter4Bit::testBinaryCounterHoldBehavior()
{
    QFETCH(int, startValue);
    QFETCH(int, cycleCount);

    WorkSpace workspace;
    LoadableCounterSetup s;
    auto *simulation = buildLoadableCounterCircuit(workspace, s);
    if (!simulation) {
        QFAIL("Failed to load loadable counter IC");
        return;
    }

    // Load start value
    loadCounterValue(simulation, s, startValue);
    QCOMPARE(readCounterValue(s), startValue & 0xF);

    // Switch to count mode
    s.load.setOn(false);
    simulation->update();

    // Verify counter sequence through all cycles
    int expectedValue = startValue & 0xF;

    for (int cycle = 0; cycle < cycleCount; ++cycle) {
        clockCycle(simulation, &s.clk);
        expectedValue = (expectedValue + 1) & 0xF;

        QCOMPARE(readCounterValue(s), expectedValue & 0xF);
    }
}
