// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6StackPointer8bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::inputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct StackPointer8bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *clk = nullptr;
    InputSwitch *reset = nullptr;
    InputSwitch *load = nullptr;
    InputSwitch *push = nullptr;
    InputSwitch *pop = nullptr;
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

        builder.add(clk, reset, load, push, pop, ic);

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

static int readSP()
{
    auto &f = *s_level6StackPtr8bit;
    int value = 0;
    for (int i = 0; i < 8; ++i) {
        if (inputStatus(f.sp[i])) {
            value |= (1 << i);
        }
    }
    return value;
}

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
    f.sim->update();

    // Load phase
    f.reset->setOn(false);
    f.load->setOn(true);

    for (int i = 0; i < 8; ++i) {
        f.loadVal[i]->setOn((loadValue >> i) & 1);
    }

    f.sim->update();

    clockCycle(f.sim, f.clk);

    QCOMPARE(readSP(), expectedValue);
}

void TestLevel6StackPointer8Bit::testStackPointerPushPop()
{
    auto &f = *s_level6StackPtr8bit;

    // Initialization: load 0x10 into SP
    f.clk->setOn(false);
    f.reset->setOn(false);
    f.push->setOn(false);
    f.pop->setOn(false);
    f.load->setOn(true);
    for (int i = 0; i < 8; ++i) {
        f.loadVal[i]->setOn((0x10 >> i) & 1);
    }
    f.sim->update();
    clockCycle(f.sim, f.clk);
    QCOMPARE(readSP(), 0x10);

    // Push decrements SP (stack grows down): 0x10 -> 0x0F -> 0x0E
    f.load->setOn(false);
    f.push->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    QCOMPARE(readSP(), 0x0F);
    clockCycle(f.sim, f.clk);
    QCOMPARE(readSP(), 0x0E);

    // Pop increments SP: 0x0E -> 0x0F
    f.push->setOn(false);
    f.pop->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    QCOMPARE(readSP(), 0x0F);

    // Hold: with no control active, SP keeps its value across clocks
    f.pop->setOn(false);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    QCOMPARE(readSP(), 0x0F);
}

void TestLevel6StackPointer8Bit::testStackPointerReset()
{
    auto &f = *s_level6StackPtr8bit;

    // Load a known value first
    f.clk->setOn(false);
    f.reset->setOn(false);
    f.push->setOn(false);
    f.pop->setOn(false);
    f.load->setOn(true);
    for (int i = 0; i < 8; ++i) {
        f.loadVal[i]->setOn((0x42 >> i) & 1);
    }
    f.sim->update();
    clockCycle(f.sim, f.clk);
    QCOMPARE(readSP(), 0x42);

    // Reset has the highest priority (even with Load still asserted) and
    // initializes the stack pointer to 0xFF (top of descending stack)
    f.reset->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    QCOMPARE(readSP(), 0xFF);
}
