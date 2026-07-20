// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestLevel2Mux8to1.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"
#include "Tests/QuickShell/QuickCpuTestUtils.h"

using TestUtils::inputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct Mux8to1Fixture {
    std::unique_ptr<QuickCircuitBuilder> builder;
    IC *ic = nullptr;
    InputSwitch *inputs[8] = {};
    InputSwitch *selectBit[3] = {};
    Led *output = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        builder = std::make_unique<QuickCircuitBuilder>();

        for (int i = 0; i < 8; ++i) {
            inputs[i] = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
            inputs[i]->setOn(i % 2 == 0);
        }
        for (int i = 0; i < 3; ++i) {
            selectBit[i] = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
        }
        output = static_cast<Led *>(builder->addOwnedElement(new Led()));

        ic = static_cast<IC *>(builder->addOwnedElement(loadBuildingBlockIC("level2_mux_8to1.panda")));

        for (int i = 0; i < 8; ++i) {
            builder->connect(inputs[i], 0, ic, QString("Data[%1]").arg(i));
        }
        for (int i = 0; i < 3; ++i) {
            builder->connect(selectBit[i], 0, ic, QString("Sel[%1]").arg(i));
        }
        builder->connect(ic, "Output", output, 0);

        sim = builder->initSimulation();
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
    QCOMPARE(inputStatus(f.output), expectedOutput);
}

// Active-high Enable (74153-style strobe): Enable=0 forces the output low;
// Enable=1 passes the selected input. Built standalone with Enable wired so the
// shared fixture (Enable unconnected → defaulted high) is untouched.
void TestLevel2MUX8To1::testEnableGating()
{
    auto builder = std::make_unique<QuickCircuitBuilder>();

    InputSwitch *data[8] = {};
    for (auto &d : data) {
        d = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
    }
    InputSwitch *sel[3] = {};
    for (auto &s : sel) {
        s = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
    }
    auto *en = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
    auto *out = static_cast<Led *>(builder->addOwnedElement(new Led()));

    auto *ic = static_cast<IC *>(builder->addOwnedElement(loadBuildingBlockIC("level2_mux_8to1.panda")));
    for (int i = 0; i < 8; ++i) {
        builder->connect(data[i], 0, ic, QString("Data[%1]").arg(i));
    }
    for (int i = 0; i < 3; ++i) {
        builder->connect(sel[i], 0, ic, QString("Sel[%1]").arg(i));
    }
    builder->connect(en, 0, ic, "Enable");
    builder->connect(ic, "Output", out, 0);
    auto *sim = builder->initSimulation();

    // Select Data[5]=1 (Sel=101); Enable low → output forced low.
    for (int i = 0; i < 8; ++i) {
        data[i]->setOn(i == 5);
    }
    for (int i = 0; i < 3; ++i) {
        sel[i]->setOn((5 >> i) & 1);
    }
    en->setOn(false);
    sim->update();
    QVERIFY(!inputStatus(out));

    // Enable high → output follows the selected Data[5]=1.
    en->setOn(true);
    sim->update();
    QVERIFY(inputStatus(out));
}
