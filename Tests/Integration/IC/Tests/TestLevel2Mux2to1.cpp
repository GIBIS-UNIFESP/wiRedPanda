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

struct Mux2to1Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *in0 = nullptr, *in1 = nullptr, *select = nullptr;
    Led *output = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        in0 = new InputSwitch();
        in1 = new InputSwitch();
        select = new InputSwitch();
        output = new Led();

        builder.add(in0, in1, select, output);

        ic = loadBuildingBlockIC("level2_mux_2to1.panda");
        builder.add(ic);

        builder.connect(in0, 0, ic, "Data[0]");
        builder.connect(in1, 0, ic, "Data[1]");
        builder.connect(select, 0, ic, "Sel[0]");
        builder.connect(ic, "Output", output, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Mux2to1Fixture> s_level2Mux2to1;

void TestLevel2MUX2To1::initTestCase()
{
    s_level2Mux2to1 = std::make_unique<Mux2to1Fixture>();
    QVERIFY(s_level2Mux2to1->build());
}

void TestLevel2MUX2To1::cleanupTestCase()
{
    s_level2Mux2to1.reset();
}

void TestLevel2MUX2To1::cleanup()
{
    if (s_level2Mux2to1 && s_level2Mux2to1->sim) {
        s_level2Mux2to1->sim->restart();
        s_level2Mux2to1->sim->update();
    }
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

    auto &f = *s_level2Mux2to1;

    f.in0->setOn(in0Value);
    f.in1->setOn(in1Value);
    f.select->setOn(selectValue);
    f.sim->update();

    QCOMPARE(getInputStatus(f.output), expectedOutput);
}

