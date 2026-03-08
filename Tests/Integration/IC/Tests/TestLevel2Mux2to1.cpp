// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2Mux2to1.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel2MUX2To1::initTestCase()
{
    // Initialize test environment
}

void TestLevel2MUX2To1::cleanup()
{
    // Clean up after each test
}

// ============================================================
// 2-to-1 Multiplexer Tests
// ============================================================

void TestLevel2MUX2To1::testMux2to1_data()
{
    QTest::addColumn<bool>("in0Value");
    QTest::addColumn<bool>("in1Value");
    QTest::addColumn<bool>("selectValue");
    QTest::addColumn<bool>("expectedOutput");

    // Test cases: All combinations of 2 inputs and 1 select bit
    // When Sel=0, output In0; when Sel=1, output In1

    // Sel=0 cases (select In0)
    QTest::newRow("In0=0, In1=0, Sel=0") << false << false << false << false;
    QTest::newRow("In0=0, In1=1, Sel=0") << false << true << false << false;
    QTest::newRow("In0=1, In1=0, Sel=0") << true << false << false << true;
    QTest::newRow("In0=1, In1=1, Sel=0") << true << true << false << true;

    // Sel=1 cases (select In1)
    QTest::newRow("In0=0, In1=0, Sel=1") << false << false << true << false;
    QTest::newRow("In0=0, In1=1, Sel=1") << false << true << true << true;
    QTest::newRow("In0=1, In1=0, Sel=1") << true << false << true << false;
    QTest::newRow("In0=1, In1=1, Sel=1") << true << true << true << true;
}

void TestLevel2MUX2To1::testMux2to1()
{
    QFETCH(bool, in0Value);
    QFETCH(bool, in1Value);
    QFETCH(bool, selectValue);
    QFETCH(bool, expectedOutput);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Create input switches
    InputSwitch in0, in1, select;
    Led output;

    builder.add(&in0, &in1, &select, &output);

    in0.setOn(in0Value);
    in1.setOn(in1Value);
    select.setOn(selectValue);

    // Load Mux2to1 IC
    IC *muxIC = loadBuildingBlockIC("level2_mux_2to1.panda");
    builder.add(muxIC);

    // Connect inputs to mux IC (using Mux primitive labels: Data[0], Data[1], Sel[0])
    builder.connect(&in0, 0, muxIC, "Data[0]");
    builder.connect(&in1, 0, muxIC, "Data[1]");
    builder.connect(&select, 0, muxIC, "Sel[0]");

    // Connect output (LED output port labeled "Output")
    builder.connect(muxIC, "Output", &output, 0);

    auto *simulation = builder.initSimulation();
    simulation->update();

    // Verify output matches expected value
    QCOMPARE(TestUtils::getInputStatus(&output), expectedOutput);
}

