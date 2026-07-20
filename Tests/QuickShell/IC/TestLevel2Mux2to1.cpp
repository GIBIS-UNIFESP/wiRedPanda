// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestLevel2Mux2to1.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"
#include "Tests/QuickShell/QuickCpuTestUtils.h"

using TestUtils::inputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct Mux2to1Fixture {
    std::unique_ptr<QuickCircuitBuilder> builder;
    IC *ic = nullptr;
    InputSwitch *in0 = nullptr, *in1 = nullptr, *select = nullptr;
    Led *output = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        builder = std::make_unique<QuickCircuitBuilder>();

        in0 = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
        in1 = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
        select = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
        output = static_cast<Led *>(builder->addOwnedElement(new Led()));

        ic = static_cast<IC *>(builder->addOwnedElement(loadBuildingBlockIC("level2_mux_2to1.panda")));

        builder->connect(in0, 0, ic, "Data[0]");
        builder->connect(in1, 0, ic, "Data[1]");
        builder->connect(select, 0, ic, "Sel[0]");
        builder->connect(ic, "Output", output, 0);

        sim = builder->initSimulation();
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

    QCOMPARE(inputStatus(f.output), expectedOutput);
}

// Active-high Enable (74153-style strobe): Enable=0 forces the output low;
// Enable=1 passes the selected input. Built standalone with Enable wired so the
// shared fixture (Enable unconnected → defaulted high) is untouched.
void TestLevel2MUX2To1::testEnableGating()
{
    auto builder = std::make_unique<QuickCircuitBuilder>();

    auto *d0 = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
    auto *d1 = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
    auto *sel = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
    auto *en = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
    auto *out = static_cast<Led *>(builder->addOwnedElement(new Led()));

    auto *ic = static_cast<IC *>(builder->addOwnedElement(loadBuildingBlockIC("level2_mux_2to1.panda")));
    builder->connect(d0, 0, ic, "Data[0]");
    builder->connect(d1, 0, ic, "Data[1]");
    builder->connect(sel, 0, ic, "Sel[0]");
    builder->connect(en, 0, ic, "Enable");
    builder->connect(ic, "Output", out, 0);
    auto *sim = builder->initSimulation();

    // Select Data[1]=1; Enable low → output forced low.
    d0->setOn(false);
    d1->setOn(true);
    sel->setOn(true);
    en->setOn(false);
    sim->update();
    QVERIFY(!inputStatus(out));

    // Enable high → output follows the selected Data[1]=1.
    en->setOn(true);
    sim->update();
    QVERIFY(inputStatus(out));
}
