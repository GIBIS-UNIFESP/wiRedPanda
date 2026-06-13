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

struct PriorityEncoder8to3Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *inputs[8] = {};
    Led *outLeds[3] = {};
    Led *validLed = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 8; ++i) {
            inputs[i] = new InputSwitch();
            builder.add(inputs[i]);
        }
        for (int i = 0; i < 3; ++i) {
            outLeds[i] = new Led();
            builder.add(outLeds[i]);
        }
        validLed = new Led();
        builder.add(validLed);

        ic = loadBuildingBlockIC("level2_priority_encoder_8to3.panda");
        builder.add(ic);

        for (int i = 0; i < 8; ++i) {
            builder.connect(inputs[i], 0, ic, QString("data[%1]").arg(i));
        }
        for (int i = 0; i < 3; ++i) {
            builder.connect(ic, QString("addr[%1]").arg(i), outLeds[i], 0);
        }
        builder.connect(ic, "valid", validLed, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<PriorityEncoder8to3Fixture> s_level2PriorityEncoder8to3;

void TestLevel2PriorityEncoder8To3::initTestCase()
{
    s_level2PriorityEncoder8to3 = std::make_unique<PriorityEncoder8to3Fixture>();
    QVERIFY(s_level2PriorityEncoder8to3->build());
}

void TestLevel2PriorityEncoder8To3::cleanupTestCase()
{
    s_level2PriorityEncoder8to3.reset();
}

void TestLevel2PriorityEncoder8To3::cleanup()
{
    if (s_level2PriorityEncoder8to3 && s_level2PriorityEncoder8to3->sim) {
        s_level2PriorityEncoder8to3->sim->restart();
        s_level2PriorityEncoder8to3->sim->update();
    }
}

void TestLevel2PriorityEncoder8To3::test8to3PriorityEncoder_data()
{
    QTest::addColumn<int>("inputMask");        // Which input lines are asserted (bit i = I_i)
    QTest::addColumn<int>("expectedOutput");   // Expected 3-bit output (0-7)
    QTest::addColumn<bool>("expectedValid");   // Group-select: any input active?

    // Idle: no input active. addr=000 AND valid=0 — the case the old fixture
    // could not distinguish from "input 0 active" before the valid output existed.
    QTest::newRow("none: idle") << 0b00000000 << 0 << false;

    // Test all single-input cases: Output encodes the index of the asserted input
    QTest::newRow("I0 alone") << (1 << 0) << 0 << true;
    QTest::newRow("I1 alone") << (1 << 1) << 1 << true;
    QTest::newRow("I2 alone") << (1 << 2) << 2 << true;
    QTest::newRow("I3 alone") << (1 << 3) << 3 << true;
    QTest::newRow("I4 alone") << (1 << 4) << 4 << true;
    QTest::newRow("I5 alone") << (1 << 5) << 5 << true;
    QTest::newRow("I6 alone") << (1 << 6) << 6 << true;
    QTest::newRow("I7 alone") << (1 << 7) << 7 << true;

    // Test multiple inputs - highest priority (largest index) wins
    QTest::newRow("I0+I1: I1 wins") << 0b00000011 << 1 << true;
    QTest::newRow("I0+I2: I2 wins") << 0b00000101 << 2 << true;
    QTest::newRow("I0+I3: I3 wins") << 0b00001001 << 3 << true;
    QTest::newRow("I0+I4: I4 wins") << 0b00010001 << 4 << true;
    QTest::newRow("I2+I5: I5 wins") << 0b00100100 << 5 << true;
    QTest::newRow("I0-I3: I3 wins") << 0b00001111 << 3 << true;
    QTest::newRow("I4-I7: I7 wins") << 0b11110000 << 7 << true;
    QTest::newRow("all set: I7 wins") << 0b11111111 << 7 << true;
}

void TestLevel2PriorityEncoder8To3::test8to3PriorityEncoder()
{
    QFETCH(int, inputMask);
    QFETCH(int, expectedOutput);
    QFETCH(bool, expectedValid);

    auto &f = *s_level2PriorityEncoder8to3;

    for (int i = 0; i < 8; ++i) {
        f.inputs[i]->setOn((inputMask >> i) & 1);
    }
    f.sim->update();

    bool bit0 = getInputStatus(f.outLeds[0]);
    bool bit1 = getInputStatus(f.outLeds[1]);
    bool bit2 = getInputStatus(f.outLeds[2]);

    int actualOutput = (static_cast<int>(bit2) << 2) | (static_cast<int>(bit1) << 1) | static_cast<int>(bit0);

    QCOMPARE(actualOutput, expectedOutput);
    QCOMPARE(getInputStatus(f.validLed), expectedValid);
}
