// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2PriorityMux3to1.h"

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct PriorityMux3to1Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *data[3] = {};
    InputSwitch *sel[2] = {};
    InputSwitch *enable = nullptr;
    Led *output = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 3; ++i) {
            data[i] = new InputSwitch();
            builder.add(data[i]);
        }
        for (int i = 0; i < 2; ++i) {
            sel[i] = new InputSwitch();
            builder.add(sel[i]);
        }
        enable = new InputSwitch();
        builder.add(enable);
        output = new Led();
        builder.add(output);

        ic = loadBuildingBlockIC("level2_priority_mux_3to1.panda");
        builder.add(ic);

        for (int i = 0; i < 3; ++i) {
            builder.connect(data[i], 0, ic, QString("data%1").arg(i));
        }
        for (int i = 0; i < 2; ++i) {
            builder.connect(sel[i], 0, ic, QString("sel%1").arg(i));
        }
        builder.connect(enable, 0, ic, "Enable");
        builder.connect(ic, "out", output, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<PriorityMux3to1Fixture> s_level2PriorityMux3to1;

void TestLevel2PriorityMUX3To1::initTestCase()
{
    s_level2PriorityMux3to1 = std::make_unique<PriorityMux3to1Fixture>();
    QVERIFY(s_level2PriorityMux3to1->build());
}

void TestLevel2PriorityMUX3To1::cleanupTestCase()
{
    s_level2PriorityMux3to1.reset();
}

void TestLevel2PriorityMUX3To1::cleanup()
{
    if (s_level2PriorityMux3to1 && s_level2PriorityMux3to1->sim) {
        s_level2PriorityMux3to1->sim->restart();
        s_level2PriorityMux3to1->sim->update();
    }
}

// Truth table for the 3-way priority multiplexer.
// Logic: out = enable ? (sel0 ? data0 : (sel1 ? data1 : data2)) : 0
void TestLevel2PriorityMUX3To1::testPriorityMux3to1_data()
{
    QTest::addColumn<bool>("sel0");
    QTest::addColumn<bool>("sel1");
    QTest::addColumn<bool>("data0");
    QTest::addColumn<bool>("data1");
    QTest::addColumn<bool>("data2");
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<bool>("expected");

    // sel0=0, sel1=0 → output = data2
    QTest::newRow("sel00 data2=0 -> 0") << false << false << false << false << false << true << false;
    QTest::newRow("sel00 data2=1 -> 1") << false << false << true << true << true << true << true;
    QTest::newRow("sel00 data2=1 (ignore d0,d1)") << false << false << false << false << true << true << true;

    // sel0=0, sel1=1 → output = data1
    QTest::newRow("sel01 data1=0 -> 0") << false << true << false << false << false << true << false;
    QTest::newRow("sel01 data1=1 -> 1") << false << true << false << true << false << true << true;
    QTest::newRow("sel01 data1=1 (ignore d2)") << false << true << true << true << false << true << true;

    // sel0=1 → output = data0 (highest priority)
    QTest::newRow("sel1x data0=0 (ignore rest)") << true << false << false << false << false << true << false;
    QTest::newRow("sel1x data0=1 (ignore rest)") << true << false << true << false << false << true << true;
    QTest::newRow("sel11 data0=1 (sel0 wins)") << true << true << true << false << false << true << true;

    // enable=0 forces the output low regardless of select/data.
    QTest::newRow("disabled sel1 data0=1 -> 0") << true << false << true << false << false << false << false;
    QTest::newRow("disabled sel00 data2=1 -> 0") << false << false << true << true << true << false << false;
}

void TestLevel2PriorityMUX3To1::testPriorityMux3to1()
{
    QFETCH(bool, sel0);
    QFETCH(bool, sel1);
    QFETCH(bool, data0);
    QFETCH(bool, data1);
    QFETCH(bool, data2);
    QFETCH(bool, enabled);
    QFETCH(bool, expected);

    auto &f = *s_level2PriorityMux3to1;

    f.data[0]->setOn(data0);
    f.data[1]->setOn(data1);
    f.data[2]->setOn(data2);
    f.sel[0]->setOn(sel0);
    f.sel[1]->setOn(sel1);
    f.enable->setOn(enabled);
    f.sim->update();

    QCOMPARE(getInputStatus(f.output, 0), expected);
}
