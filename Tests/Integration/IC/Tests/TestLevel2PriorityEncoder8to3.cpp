// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2PriorityEncoder8to3.h"

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel2PriorityEncoder8To3::initTestCase()
{
}

void TestLevel2PriorityEncoder8To3::cleanup()
{
}

void TestLevel2PriorityEncoder8To3::test8to3PriorityEncoder_data()
{
    QTest::addColumn<int>("inputMask");        // Which input lines are asserted (bit i = I_i)
    QTest::addColumn<int>("expectedOutput");   // Expected 3-bit output (0-7)

    // Test all single-input cases: Output encodes the index of the asserted input
    QTest::newRow("I0 alone") << (1 << 0) << 0;
    QTest::newRow("I1 alone") << (1 << 1) << 1;
    QTest::newRow("I2 alone") << (1 << 2) << 2;
    QTest::newRow("I3 alone") << (1 << 3) << 3;
    QTest::newRow("I4 alone") << (1 << 4) << 4;
    QTest::newRow("I5 alone") << (1 << 5) << 5;
    QTest::newRow("I6 alone") << (1 << 6) << 6;
    QTest::newRow("I7 alone") << (1 << 7) << 7;

    // Test multiple inputs - highest priority (largest index) wins
    QTest::newRow("I0+I1: I1 wins") << 0b00000011 << 1;
    QTest::newRow("I0+I2: I2 wins") << 0b00000101 << 2;
    QTest::newRow("I0+I3: I3 wins") << 0b00001001 << 3;
    QTest::newRow("I0+I4: I4 wins") << 0b00010001 << 4;
    QTest::newRow("I2+I5: I5 wins") << 0b00100100 << 5;
    QTest::newRow("I0-I3: I3 wins") << 0b00001111 << 3;
    QTest::newRow("I4-I7: I7 wins") << 0b11110000 << 7;
    QTest::newRow("all set: I7 wins") << 0b11111111 << 7;
}

void TestLevel2PriorityEncoder8To3::test8to3PriorityEncoder()
{
    QFETCH(int, inputMask);
    QFETCH(int, expectedOutput);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // 8 input switches for I0-I7
    InputSwitch inputs[8];

    // 3 output LEDs for O0, O1, O2 (representing bits 0, 1, 2)
    Led outLeds[3];

    // Add all inputs and outputs
    for (int i = 0; i < 8; ++i) {
        builder.add(&inputs[i]);
    }
    for (int i = 0; i < 3; ++i) {
        builder.add(&outLeds[i]);
    }

    // Set input states according to inputMask
    for (int i = 0; i < 8; ++i) {
        inputs[i].setOn((inputMask >> i) & 1);
    }

    // Load PriorityEncoder8to3 IC
    IC *encoderIC = loadBuildingBlockIC("level2_priority_encoder_8to3.panda");
    builder.add(encoderIC);

    // Connect 8 input switches to encoder IC data inputs
    for (int i = 0; i < 8; ++i) {
        builder.connect(&inputs[i], 0, encoderIC, QString("data[%1]").arg(i));
    }

    // Connect encoder outputs to LED outputs
    for (int i = 0; i < 3; ++i) {
        builder.connect(encoderIC, QString("addr[%1]").arg(i), &outLeds[i], 0);
    }

    auto *simulation = builder.initSimulation();
    simulation->update();

    // Read output bits and construct 3-bit result
    bool bit0 = getInputStatus(&outLeds[0]);
    bool bit1 = getInputStatus(&outLeds[1]);
    bool bit2 = getInputStatus(&outLeds[2]);

    int actualOutput = (static_cast<int>(bit2) << 2) | (static_cast<int>(bit1) << 1) | static_cast<int>(bit0);

    QCOMPARE(actualOutput, expectedOutput);
}
