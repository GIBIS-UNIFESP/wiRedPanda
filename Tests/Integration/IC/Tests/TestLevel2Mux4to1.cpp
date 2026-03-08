// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2Mux4to1.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/Cpu/CpuCommon.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel2MUX4To1::initTestCase()
{
    // Initialize test environment
}

void TestLevel2MUX4To1::cleanup()
{
    // Clean up after each test
}

// ============================================================
// 4-to-1 Multiplexer Tests
// ============================================================

void TestLevel2MUX4To1::testMultiplexer4to1_data()
{
    QTest::addColumn<int>("selectValue");

    QTest::newRow("select_0") << 0;
    QTest::newRow("select_1") << 1;
    QTest::newRow("select_2") << 2;
    QTest::newRow("select_3") << 3;
    QTest::newRow("sequential") << -1;
    QTest::newRow("pattern_propagation") << -2;
}

void TestLevel2MUX4To1::testMultiplexer4to1()
{
    QFETCH(int, selectValue);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Create 4 input switches with alternating pattern
    InputSwitch inputs[4];
    for (int i = 0; i < 4; i++) {
        inputs[i].setOn(i % 2 == 0);  // Inputs: 1, 0, 1, 0
        builder.add(&inputs[i]);
    }

    InputSwitch selectBit0, selectBit1;
    Led output;

    builder.add(&selectBit0, &selectBit1, &output);

    // Load Multiplexer4to1 IC
    IC *muxIC = loadBuildingBlockIC("level2_mux_4to1.panda");
    builder.add(muxIC);

    // Connect data inputs to mux IC using semantic labels
    builder.connect(&inputs[0], 0, muxIC, "Data[0]");
    builder.connect(&inputs[1], 0, muxIC, "Data[1]");
    builder.connect(&inputs[2], 0, muxIC, "Data[2]");
    builder.connect(&inputs[3], 0, muxIC, "Data[3]");

    // Connect select inputs
    builder.connect(&selectBit0, 0, muxIC, "Sel[0]");
    builder.connect(&selectBit1, 0, muxIC, "Sel[1]");

    // Connect output
    builder.connect(muxIC, "Output", &output, 0);

    auto *simulation = builder.initSimulation();

    // Initialize select bits to OFF with settlement
    selectBit0.setOff();
    selectBit1.setOff();
    simulation->update();

    // Handle special test cases
    if (selectValue == -1) {
        // Sequential selection: test all 4 inputs in sequence
        for (int s = 0; s < 4; ++s) {
            bool bit0 = (s >> 0) & 1;
            bool bit1 = (s >> 1) & 1;
            selectBit0.setOn(bit0);
            selectBit1.setOn(bit1);
            simulation->update();

            bool expectedInput = inputs[s].isOn();
            bool actualOutput = TestUtils::getInputStatus(&output);
            QCOMPARE(actualOutput, expectedInput);
        }
    } else if (selectValue == -2) {
        // Pattern propagation: verify input patterns appear at output for each select
        for (int s = 0; s < 4; ++s) {
            bool bit0 = (s >> 0) & 1;
            bool bit1 = (s >> 1) & 1;
            selectBit0.setOn(bit0);
            selectBit1.setOn(bit1);
            simulation->update();

            bool expectedInput = inputs[s].isOn();
            bool actualOutput = TestUtils::getInputStatus(&output);
            QCOMPARE(actualOutput, expectedInput);
        }
    } else {
        // Normal case: single select value
        bool bit0 = (selectValue >> 0) & 1;
        bool bit1 = (selectValue >> 1) & 1;

        // Set select bits
        selectBit0.setOn(bit0);
        selectBit1.setOn(bit1);
        simulation->update();

        // Verify selected input appears at output
        bool expectedInput = inputs[selectValue].isOn();
        bool actualOutput = TestUtils::getInputStatus(&output);
        QCOMPARE(actualOutput, expectedInput);
    }
}
