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

struct Mux8to1Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *inputs[8] = {};
    InputSwitch *selectBit[3] = {};
    Led *output = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 8; ++i) {
            inputs[i] = new InputSwitch();
            inputs[i]->setOn(i % 2 == 0);
            builder.add(inputs[i]);
        }
        for (int i = 0; i < 3; ++i) {
            selectBit[i] = new InputSwitch();
            builder.add(selectBit[i]);
        }
        output = new Led();
        builder.add(output);

        ic = loadBuildingBlockIC("level2_mux_8to1.panda");
        builder.add(ic);

        for (int i = 0; i < 8; ++i) {
            builder.connect(inputs[i], 0, ic, QString("Data[%1]").arg(i));
        }
        for (int i = 0; i < 3; ++i) {
            builder.connect(selectBit[i], 0, ic, QString("Sel[%1]").arg(i));
        }
        builder.connect(ic, "Output", output, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Mux8to1Fixture> s_level2Mux8to1;

void TestLevel2MUX8To1::initTestCase()
{
    s_level2Mux8to1 = std::make_unique<Mux8to1Fixture>();
    QVERIFY(s_level2Mux8to1->build());
}

void TestLevel2MUX8To1::cleanupTestCase()
{
    s_level2Mux8to1.reset();
}

void TestLevel2MUX8To1::cleanup()
{
    if (s_level2Mux8to1 && s_level2Mux8to1->sim) {
        s_level2Mux8to1->sim->restart();
        s_level2Mux8to1->sim->update();
    }
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

    auto &f = *s_level2Mux8to1;

    for (int i = 0; i < 8; ++i) {
        f.inputs[i]->setOn(i % 2 == 0);
    }
    for (int i = 0; i < 3; ++i) {
        f.selectBit[i]->setOn((selectValue >> i) & 1);
    }
    f.sim->update();

    bool expectedOutput = f.inputs[selectValue]->isOn();
    QCOMPARE(getInputStatus(f.output), expectedOutput);
}

