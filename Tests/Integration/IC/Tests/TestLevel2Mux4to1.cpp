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

struct Mux4to1Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *inputs[4] = {};
    InputSwitch *selectBit0 = nullptr, *selectBit1 = nullptr;
    Led *output = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 4; ++i) {
            inputs[i] = new InputSwitch();
            inputs[i]->setOn(i % 2 == 0);  // Inputs: 1, 0, 1, 0
            builder.add(inputs[i]);
        }

        selectBit0 = new InputSwitch();
        selectBit1 = new InputSwitch();
        output = new Led();

        builder.add(selectBit0, selectBit1, output);

        ic = loadBuildingBlockIC("level2_mux_4to1.panda");
        builder.add(ic);

        builder.connect(inputs[0], 0, ic, "Data[0]");
        builder.connect(inputs[1], 0, ic, "Data[1]");
        builder.connect(inputs[2], 0, ic, "Data[2]");
        builder.connect(inputs[3], 0, ic, "Data[3]");
        builder.connect(selectBit0, 0, ic, "Sel[0]");
        builder.connect(selectBit1, 0, ic, "Sel[1]");
        builder.connect(ic, "Output", output, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Mux4to1Fixture> s_level2Mux4to1;

void TestLevel2MUX4To1::initTestCase()
{
    s_level2Mux4to1 = std::make_unique<Mux4to1Fixture>();
    QVERIFY(s_level2Mux4to1->build());
}

void TestLevel2MUX4To1::cleanupTestCase()
{
    s_level2Mux4to1.reset();
}

void TestLevel2MUX4To1::cleanup()
{
    if (s_level2Mux4to1 && s_level2Mux4to1->sim) {
        s_level2Mux4to1->sim->restart();
        s_level2Mux4to1->sim->update();
    }
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

    auto &f = *s_level2Mux4to1;

    // Reset input pattern
    for (int i = 0; i < 4; ++i) {
        f.inputs[i]->setOn(i % 2 == 0);
    }
    f.selectBit0->setOff();
    f.selectBit1->setOff();
    f.sim->update();

    if (selectValue == -1 || selectValue == -2) {
        for (int s = 0; s < 4; ++s) {
            f.selectBit0->setOn((s >> 0) & 1);
            f.selectBit1->setOn((s >> 1) & 1);
            f.sim->update();

            bool expectedInput = f.inputs[s]->isOn();
            bool actualOutput = getInputStatus(f.output);
            QCOMPARE(actualOutput, expectedInput);
        }
    } else {
        f.selectBit0->setOn((selectValue >> 0) & 1);
        f.selectBit1->setOn((selectValue >> 1) & 1);
        f.sim->update();

        bool expectedInput = f.inputs[selectValue]->isOn();
        bool actualOutput = getInputStatus(f.output);
        QCOMPARE(actualOutput, expectedInput);
    }
}

// Active-high Enable (74153-style strobe): Enable=0 forces the output low;
// Enable=1 passes the selected input. Built standalone with Enable wired so the
// shared fixture (Enable unconnected → defaulted high) is untouched.
void TestLevel2MUX4To1::testEnableGating()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    InputSwitch *data[4] = {};
    for (auto &d : data) {
        d = new InputSwitch();
        builder.add(d);
    }
    auto *s0 = new InputSwitch();
    auto *s1 = new InputSwitch();
    auto *en = new InputSwitch();
    auto *out = new Led();
    builder.add(s0, s1, en, out);

    auto *ic = loadBuildingBlockIC("level2_mux_4to1.panda");
    builder.add(ic);
    for (int i = 0; i < 4; ++i) {
        builder.connect(data[i], 0, ic, QString("Data[%1]").arg(i));
    }
    builder.connect(s0, 0, ic, "Sel[0]");
    builder.connect(s1, 0, ic, "Sel[1]");
    builder.connect(en, 0, ic, "Enable");
    builder.connect(ic, "Output", out, 0);
    auto *sim = builder.initSimulation();

    // Select Data[2]=1 (Sel=10); Enable low → output forced low.
    for (int i = 0; i < 4; ++i) {
        data[i]->setOn(i == 2);
    }
    s0->setOn(false);
    s1->setOn(true);
    en->setOn(false);
    sim->update();
    QVERIFY(!getInputStatus(out));

    // Enable high → output follows the selected Data[2]=1.
    en->setOn(true);
    sim->update();
    QVERIFY(getInputStatus(out));
}
