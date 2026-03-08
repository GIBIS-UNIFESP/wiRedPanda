// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2Mux8to1.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/Cpu/CpuCommon.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel2MUX8To1::initTestCase()
{
    // Initialize test environment
}

void TestLevel2MUX8To1::cleanup()
{
    // Clean up after each test
}

// ============================================================
// 8-to-1 Multiplexer Tests
// ============================================================

void TestLevel2MUX8To1::testMultiplexer8to1_data()
{
    QTest::addColumn<int>("selectValue");

    for (int i = 0; i < 8; i++) {
        QTest::newRow(qPrintable(QString("select_%1").arg(i))) << i;
    }
}

void TestLevel2MUX8To1::testMultiplexer8to1()
{
    QFETCH(int, selectValue);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Create 8 input switches with alternating pattern
    InputSwitch inputs[8];
    for (int i = 0; i < 8; i++) {
        inputs[i].setOn(i % 2 == 0);  // Inputs: 1, 0, 1, 0, 1, 0, 1, 0
        builder.add(&inputs[i]);
    }

    InputSwitch selectBit0, selectBit1, selectBit2;
    Led output;

    builder.add(&selectBit0, &selectBit1, &selectBit2, &output);

    // Load Multiplexer8to1 IC (using level2_mux_8to1 - functionally identical to level3)
    IC *muxIC = loadBuildingBlockIC("level2_mux_8to1.panda");
    builder.add(muxIC);

    // Connect data inputs to mux IC using semantic labels
    for (int i = 0; i < 8; i++) {
        builder.connect(&inputs[i], 0, muxIC, QString("Data[%1]").arg(i));
    }

    // Connect select inputs
    builder.connect(&selectBit0, 0, muxIC, "Sel[0]");
    builder.connect(&selectBit1, 0, muxIC, "Sel[1]");
    builder.connect(&selectBit2, 0, muxIC, "Sel[2]");

    // Connect output
    builder.connect(muxIC, "Output", &output, 0);

    auto *simulation = builder.initSimulation();

    // Initialize select bits to OFF with settlement
    selectBit0.setOff();
    selectBit1.setOff();
    selectBit2.setOff();
    simulation->update();

    // Set select bits for target value
    bool bit0 = (selectValue >> 0) & 1;
    bool bit1 = (selectValue >> 1) & 1;
    bool bit2 = (selectValue >> 2) & 1;
    selectBit0.setOn(bit0);
    selectBit1.setOn(bit1);
    selectBit2.setOn(bit2);
    simulation->update();

    // Verify selected input appears at output
    bool expectedOutput = inputs[selectValue].isOn();
    QCOMPARE(TestUtils::getInputStatus(&output), expectedOutput);
}

