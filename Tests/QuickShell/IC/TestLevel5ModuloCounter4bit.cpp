// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestLevel5ModuloCounter4bit.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"
#include "Tests/QuickShell/QuickCpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::inputStatus;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

struct ModuloCounter4bitFixture {
    std::unique_ptr<QuickCircuitBuilder> builder;
    IC *ic = nullptr;
    InputSwitch *clk = nullptr;
    InputSwitch *moduloIn[4] = {};
    Led *counterOut[4] = {};
    Led *overflowOut = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        builder = std::make_unique<QuickCircuitBuilder>();

        clk = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));

        ic = static_cast<IC *>(builder->addOwnedElement(loadBuildingBlockIC("level5_modulo_counter_4bit.panda")));

        builder->connect(clk, 0, ic, "Clock");

        for (int i = 0; i < 4; ++i) {
            moduloIn[i] = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
            counterOut[i] = static_cast<Led *>(builder->addOwnedElement(new Led()));
            builder->connect(moduloIn[i], 0, ic, QString("Modulo[%1]").arg(i));
            builder->connect(ic, QString("Q[%1]").arg(i), counterOut[i], 0);
        }
        overflowOut = static_cast<Led *>(builder->addOwnedElement(new Led()));
        builder->connect(ic, "Overflow", overflowOut, 0);

        sim = builder->initSimulation();
        sim->update();
        return true;
    }

    void resetCounter()
    {
        clk->setOn(false);
        sim->update();
        clockCycle(sim, clk);
    }

    int readValue()
    {
        return readMultiBitOutput(QVector<GraphicElement *>({
            counterOut[0], counterOut[1], counterOut[2], counterOut[3]
        })) & 0xF;
    }
};

static std::unique_ptr<ModuloCounter4bitFixture> s_level5ModuloCounter4bit;

void TestLevel5ModuloCounter4Bit::initTestCase()
{
    s_level5ModuloCounter4bit = std::make_unique<ModuloCounter4bitFixture>();
    QVERIFY(s_level5ModuloCounter4bit->build());
}

void TestLevel5ModuloCounter4Bit::cleanupTestCase()
{
    s_level5ModuloCounter4bit.reset();
}

void TestLevel5ModuloCounter4Bit::cleanup()
{
    if (s_level5ModuloCounter4bit && s_level5ModuloCounter4bit->sim) {
        s_level5ModuloCounter4bit->sim->restart();
        s_level5ModuloCounter4bit->sim->update();
    }
}

void TestLevel5ModuloCounter4Bit::testModuloCounter_data()
{
    QTest::addColumn<int>("modulo");
    QTest::addColumn<int>("cycles");
    QTest::addColumn<int>("expectedPattern");

    QTest::newRow("mod5_cycle6") << 5 << 6 << 1;
    QTest::newRow("mod5_complete") << 5 << 5 << 0;
    QTest::newRow("mod10_to_9") << 10 << 9 << 9;
    QTest::newRow("mod10_wrap") << 10 << 11 << 1;
}

void TestLevel5ModuloCounter4Bit::testModuloCounter()
{
    QFETCH(int, modulo);
    QFETCH(int, cycles);
    QFETCH(int, expectedPattern);

    auto &f = *s_level5ModuloCounter4bit;

    for (int i = 0; i < 4; ++i) {
        f.moduloIn[i]->setOn((modulo >> i) & 1);
    }
    f.sim->update();

    f.resetCounter();

    for (int cycle = 0; cycle < cycles; ++cycle) {
        clockCycle(f.sim, f.clk);
    }

    QCOMPARE(f.readValue(), expectedPattern & 0xF);
}

// The Overflow output (= comparator Equal = "next value would reach Modulo")
// must be asserted exactly when the count holds Modulo-1, the cycle before the
// wrap, and clear at every other count. Walk two full mod-5 periods and check
// the invariant at each step — robust to the counter's warm-up offset since it
// correlates Overflow against the actually-observed count.
void TestLevel5ModuloCounter4Bit::testOverflow()
{
    auto &f = *s_level5ModuloCounter4bit;

    const int modulo = 5;
    for (int i = 0; i < 4; ++i) {
        f.moduloIn[i]->setOn((modulo >> i) & 1);
    }
    f.sim->update();
    f.resetCounter();

    bool sawTerminalCount = false;
    for (int step = 0; step < 12; ++step) {
        const int q = f.readValue();
        const bool overflow = inputStatus(f.overflowOut);
        QCOMPARE(overflow, q == modulo - 1);
        sawTerminalCount = sawTerminalCount || overflow;
        clockCycle(f.sim, f.clk);
    }
    // Sanity: across two periods the terminal-count pulse must have occurred.
    QVERIFY(sawTerminalCount);
}
