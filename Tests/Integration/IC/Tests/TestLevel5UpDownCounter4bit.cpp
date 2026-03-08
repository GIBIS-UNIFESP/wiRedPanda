// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5UpDownCounter4bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

struct UpDownCounter4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *clk = nullptr, *dir = nullptr, *en = nullptr;
    Led *counterOut[4] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        clk = new InputSwitch();
        dir = new InputSwitch();
        en = new InputSwitch();
        builder.add(clk, dir, en);

        ic = loadBuildingBlockIC("level5_up_down_counter_4bit.panda");
        builder.add(ic);

        builder.connect(clk, 0, ic, "CLK");
        builder.connect(dir, 0, ic, "Direction");
        builder.connect(en, 0, ic, "Enable");

        for (int i = 0; i < 4; ++i) {
            counterOut[i] = new Led();
            builder.add(counterOut[i]);
            builder.connect(ic, QString("Q[%1]").arg(i), counterOut[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readValue()
    {
        return readMultiBitOutput(QVector<GraphicElement *>({
            counterOut[0], counterOut[1], counterOut[2], counterOut[3]
        })) & 0xF;
    }

    void initializeToValue(int targetValue)
    {
        clk->setOn(false);
        en->setOn(true);
        dir->setOn(true);
        sim->update();
        clockCycle(sim, clk);

        for (int i = 0; i < targetValue; ++i) {
            clockCycle(sim, clk);
        }
    }
};

static std::unique_ptr<UpDownCounter4bitFixture> s_level5UpDownCounter4bit;

void TestLevel5UpDownCounter4Bit::initTestCase()
{
    s_level5UpDownCounter4bit = std::make_unique<UpDownCounter4bitFixture>();
    QVERIFY(s_level5UpDownCounter4bit->build());
}

void TestLevel5UpDownCounter4Bit::cleanupTestCase()
{
    s_level5UpDownCounter4bit.reset();
}

void TestLevel5UpDownCounter4Bit::cleanup()
{
    if (s_level5UpDownCounter4bit && s_level5UpDownCounter4bit->sim) {
        s_level5UpDownCounter4bit->sim->restart();
        s_level5UpDownCounter4bit->sim->update();
    }
}

void TestLevel5UpDownCounter4Bit::testUpDownCounter_data()
{
    QTest::addColumn<bool>("direction");
    QTest::addColumn<bool>("enable");
    QTest::addColumn<int>("cycles");
    QTest::addColumn<int>("expectedValue");

    // Counter always initializes to 0, then counts up or down
    // Direction: 1 = up, 0 = down
    // Enable: 1 = count, 0 = hold
    QTest::newRow("count_up") << true << true << 3 << 3;           // 0 + 3 = 3
    QTest::newRow("count_up_large") << true << true << 4 << 4;     // 0 + 4 = 4
    QTest::newRow("count_down") << false << true << 3 << 13;       // 0 - 3 (wrap) = 13 (0xD)
    QTest::newRow("count_down_by_2") << false << true << 2 << 14;  // 0 - 2 (wrap) = 14 (0xE)
    QTest::newRow("hold_when_disabled") << true << false << 5 << 0; // Enable=0: stays at 0 despite 5 cycles
    QTest::newRow("hold_mixed") << false << false << 3 << 0;        // Enable=0: stays at 0 regardless of direction
}

void TestLevel5UpDownCounter4Bit::testUpDownCounter()
{
    QFETCH(bool, direction);
    QFETCH(bool, enable);
    QFETCH(int, cycles);
    QFETCH(int, expectedValue);

    auto &f = *s_level5UpDownCounter4bit;

    f.initializeToValue(0);

    f.dir->setOn(direction);
    f.en->setOn(enable);
    f.sim->update();

    for (int cycle = 0; cycle < cycles; ++cycle) {
        clockCycle(f.sim, f.clk);
    }

    QCOMPARE(f.readValue(), expectedValue & 0xF);
}

// ============================================================
// Up/Down Counter From Value Tests
// Tests counting from a non-zero initial value by first counting up
// ============================================================
void TestLevel5UpDownCounter4Bit::testUpDownCounterFromValue_data()
{
    QTest::addColumn<int>("initialValue");
    QTest::addColumn<bool>("direction");
    QTest::addColumn<int>("cycles");
    QTest::addColumn<int>("expectedValue");

    QTest::newRow("count_up_from_5") << 5 << true << 4 << 9;
    QTest::newRow("count_down_from_5") << 5 << false << 3 << 2;
    QTest::newRow("count_down_to_zero") << 2 << false << 2 << 0;
}

void TestLevel5UpDownCounter4Bit::testUpDownCounterFromValue()
{
    QFETCH(int, initialValue);
    QFETCH(bool, direction);
    QFETCH(int, cycles);
    QFETCH(int, expectedValue);

    auto &f = *s_level5UpDownCounter4bit;

    f.initializeToValue(initialValue);

    f.dir->setOn(direction);
    f.en->setOn(true);
    f.sim->update();

    for (int cycle = 0; cycle < cycles; ++cycle) {
        clockCycle(f.sim, f.clk);
    }

    QCOMPARE(f.readValue(), expectedValue & 0xF);
}

