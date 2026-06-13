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
    InputSwitch *enable = nullptr;
    Led *output = nullptr;
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
            selectBit[i] = new InputSwitch();
            builder.add(selectBit[i]);
        }
        enable = new InputSwitch();
        builder.add(enable);
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
        builder.connect(enable, 0, ic, "Enable");
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
    QTest::addColumn<int>("select");
    QTest::addColumn<bool>("x");       // value driven onto the selected line
    QTest::addColumn<bool>("enabled");

    // For each select line, drive that line = X and ALL OTHERS = ~X (enable=1).
    // Output must equal X — proves the line routes AND that no other line leaks.
    for (int s = 0; s < 8; ++s) {
        QTest::newRow(qPrintable(QString("sel %1, x=0").arg(s))) << s << false << true;
        QTest::newRow(qPrintable(QString("sel %1, x=1").arg(s))) << s << true << true;
    }
    // enable=0 (strobe): output forced 0 regardless of the selected line value.
    for (int s = 0; s < 8; ++s) {
        QTest::newRow(qPrintable(QString("sel %1, disabled").arg(s))) << s << true << false;
    }
}

void TestLevel2MUX8To1::testMultiplexer8to1()
{
    QFETCH(int, select);
    QFETCH(bool, x);
    QFETCH(bool, enabled);

    auto &f = *s_level2Mux8to1;

    for (int i = 0; i < 8; ++i) {
        f.inputs[i]->setOn(i == select ? x : !x);
    }
    for (int i = 0; i < 3; ++i) {
        f.selectBit[i]->setOn((select >> i) & 1);
    }
    f.enable->setOn(enabled);
    f.sim->update();

    const bool expected = enabled ? x : false;
    QCOMPARE(getInputStatus(f.output), expected);
}
