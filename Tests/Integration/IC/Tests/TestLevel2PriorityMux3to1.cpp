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

using TestUtils::inputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct PriorityMux3to1Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *data[3] = {};
    InputSwitch *sel[2] = {};
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

void TestLevel2PriorityMUX3To1::testPriorityMux3to1_data()
{
    QTest::addColumn<int>("sel0");
    QTest::addColumn<int>("sel1");
    QTest::addColumn<int>("data0");
    QTest::addColumn<int>("data1");
    QTest::addColumn<int>("data2");
    QTest::addColumn<int>("expected");

    // Test truth table for 3-way priority multiplexer
    // Logic: output = sel0 ? data0 : (sel1 ? data1 : data2)

    // sel0=0, sel1=0 → output = data2
    QTest::newRow("sel0=0, sel1=0, data2=0 -> 0") << 0 << 0 << 0 << 0 << 0 << 0;
    QTest::newRow("sel0=0, sel1=0, data2=1 -> 1") << 0 << 0 << 1 << 1 << 1 << 1;
    QTest::newRow("sel0=0, sel1=0, data2=1 (ignore data0, data1) -> 1") << 0 << 0 << 0 << 0 << 1 << 1;

    // sel0=0, sel1=1 → output = data1
    QTest::newRow("sel0=0, sel1=1, data1=0 -> 0") << 0 << 1 << 0 << 0 << 0 << 0;
    QTest::newRow("sel0=0, sel1=1, data1=1 -> 1") << 0 << 1 << 0 << 1 << 0 << 1;
    QTest::newRow("sel0=0, sel1=1, data1=1 (ignore data2) -> 1") << 0 << 1 << 1 << 1 << 0 << 1;

    // sel0=1 → output = data0 (highest priority)
    QTest::newRow("sel0=1, data0=0 (ignore sel1, data1, data2) -> 0") << 1 << 0 << 0 << 0 << 0 << 0;
    QTest::newRow("sel0=1, data0=1 (ignore sel1, data1, data2) -> 1") << 1 << 0 << 1 << 0 << 0 << 1;
    QTest::newRow("sel0=1, data0=1 (sel0 overrides sel1) -> 1") << 1 << 1 << 1 << 0 << 0 << 1;
}

void TestLevel2PriorityMUX3To1::testPriorityMux3to1()
{
    auto &f = *s_level2PriorityMux3to1;

    QFETCH(int, sel0);
    QFETCH(int, sel1);
    QFETCH(int, data0);
    QFETCH(int, data1);
    QFETCH(int, data2);
    QFETCH(int, expected);

    f.data[0]->setOn(data0);
    f.data[1]->setOn(data1);
    f.data[2]->setOn(data2);
    f.sel[0]->setOn(sel0);
    f.sel[1]->setOn(sel1);

    f.sim->update();

    bool result = inputStatus(f.output, 0);

    QCOMPARE(result, static_cast<bool>(expected));
}
