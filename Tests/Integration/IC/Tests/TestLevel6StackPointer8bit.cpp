// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6StackPointer8bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct StackPointer8bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *clk = nullptr;
    InputSwitch *reset = nullptr;
    InputSwitch *load = nullptr;
    InputSwitch *push = nullptr;
    InputSwitch *pop = nullptr;
    InputSwitch *enable = nullptr;
    InputSwitch *loadVal[8] = {};
    Led *sp[8] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level6_stack_pointer_8bit.panda");

        clk = new InputSwitch();
        reset = new InputSwitch();
        load = new InputSwitch();
        push = new InputSwitch();
        pop = new InputSwitch();
        enable = new InputSwitch();

        builder.add(clk, reset, load, push, pop, enable, ic);

        for (int i = 0; i < 8; ++i) {
            loadVal[i] = new InputSwitch();
            sp[i] = new Led();
            builder.add(loadVal[i], sp[i]);
        }

        builder.connect(clk, 0, ic, "Clock");
        builder.connect(reset, 0, ic, "Reset");
        builder.connect(load, 0, ic, "Load");
        builder.connect(push, 0, ic, "Push");
        builder.connect(pop, 0, ic, "Pop");
        builder.connect(enable, 0, ic, "Enable");

        for (int i = 0; i < 8; ++i) {
            builder.connect(loadVal[i], 0, ic, QString("LoadValue[%1]").arg(i));
            builder.connect(ic, QString("SP[%1]").arg(i), sp[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<StackPointer8bitFixture> s_level6StackPtr8bit;

void TestLevel6StackPointer8Bit::initTestCase()
{
    s_level6StackPtr8bit = std::make_unique<StackPointer8bitFixture>();
    QVERIFY(s_level6StackPtr8bit->build());
}

void TestLevel6StackPointer8Bit::cleanupTestCase()
{
    s_level6StackPtr8bit.reset();
}

void TestLevel6StackPointer8Bit::cleanup()
{
    if (s_level6StackPtr8bit && s_level6StackPtr8bit->sim) {
        s_level6StackPtr8bit->sim->restart();
        s_level6StackPtr8bit->sim->update();
    }
}

void TestLevel6StackPointer8Bit::testStackPointer_data()
{
    QTest::addColumn<int>("loadValue");
    QTest::addColumn<int>("expectedValue");

    QTest::newRow("load_zero") << 0x00 << 0x00;
    QTest::newRow("load_0x55") << 0x55 << 0x55;
    QTest::newRow("load_0xAA") << 0xAA << 0xAA;
    QTest::newRow("load_0xFF") << 0xFF << 0xFF;
}

void TestLevel6StackPointer8Bit::testStackPointer()
{
    QFETCH(int, loadValue);
    QFETCH(int, expectedValue);

    auto &f = *s_level6StackPtr8bit;

    // Initialization
    f.clk->setOn(false);
    f.reset->setOn(false);
    f.load->setOn(false);
    f.push->setOn(false);
    f.pop->setOn(false);
    f.enable->setOn(false);
    f.sim->update();

    // Load phase
    f.reset->setOn(false);
    f.load->setOn(true);
    f.enable->setOn(true);

    for (int i = 0; i < 8; ++i) {
        f.loadVal[i]->setOn((loadValue >> i) & 1);
    }

    f.sim->update();

    clockCycle(f.sim, f.clk);

    int readValue = 0;
    for (int i = 0; i < 8; ++i) {
        if (getInputStatus(f.sp[i])) {
            readValue |= (1 << i);
        }
    }

    QCOMPARE(readValue, expectedValue);
}

