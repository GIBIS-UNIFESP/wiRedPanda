// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4BinaryCounter4bit.h"

#include <QFile>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

struct BinaryCounter4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *clk = nullptr;
    InputSwitch *ce = nullptr, *load = nullptr, *clear = nullptr;
    InputSwitch *data[4] = {};
    Led *counterOut[4] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        clk = new InputSwitch();
        ce = new InputSwitch();
        load = new InputSwitch();
        clear = new InputSwitch();
        builder.add(clk, ce, load, clear);
        for (int i = 0; i < 4; ++i) {
            data[i] = new InputSwitch();
            builder.add(data[i]);
        }

        ic = loadBuildingBlockIC("level4_binary_counter_4bit.panda");
        builder.add(ic);

        for (int i = 0; i < 4; ++i) {
            counterOut[i] = new Led();
            builder.add(counterOut[i]);
            builder.connect(ic, QString("Q%1").arg(i), counterOut[i], 0);
        }
        builder.connect(clk, 0, ic, "Clock");
        builder.connect(ce, 0, ic, "CountEnable");
        builder.connect(load, 0, ic, "Load");
        builder.connect(clear, 0, ic, "Reset");
        for (int i = 0; i < 4; ++i) {
            builder.connect(data[i], 0, ic, QString("Data[%1]").arg(i));
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    // Free-running tie-off: count enabled, no load, no clear, data cleared.
    void tieFreeRun()
    {
        ce->setOn(true);
        load->setOn(false);
        clear->setOn(false);
        for (int i = 0; i < 4; ++i) {
            data[i]->setOn(false);
        }
    }

    int readCount()
    {
        return readMultiBitOutput(QVector<GraphicElement *>({
            counterOut[0], counterOut[1], counterOut[2], counterOut[3]
        })) & 0xF;
    }

    void asyncClear()
    {
        clear->setOn(true);
        sim->update();
        clear->setOn(false);
        sim->update();
    }
};

static std::unique_ptr<BinaryCounter4bitFixture> s_level4BinaryCounter4bit;

void TestLevel4BinaryCounter4Bit::initTestCase()
{
    s_level4BinaryCounter4bit = std::make_unique<BinaryCounter4bitFixture>();
    QVERIFY(s_level4BinaryCounter4bit->build());
}

void TestLevel4BinaryCounter4Bit::cleanupTestCase()
{
    s_level4BinaryCounter4bit.reset();
}

void TestLevel4BinaryCounter4Bit::cleanup()
{
    if (s_level4BinaryCounter4bit && s_level4BinaryCounter4bit->sim) {
        s_level4BinaryCounter4bit->sim->restart();
        s_level4BinaryCounter4bit->sim->update();
    }
}

void TestLevel4BinaryCounter4Bit::testBinaryCounter_data()
{
    QTest::addColumn<int>("numCycles");
    QTest::addColumn<int>("expectedFinalCount");

    // Count up 1 cycle
    QTest::newRow("count 1 cycle") << 1 << 1;

    // Count up 3 cycles
    QTest::newRow("count 3 cycles") << 3 << 3;

    // Count up 8 cycles
    QTest::newRow("count 8 cycles") << 8 << 8;

    // Count up 15 cycles
    QTest::newRow("count 15 cycles") << 15 << 15;

    // Count up 16 cycles (wrap to 0)
    QTest::newRow("count 16 cycles (wrap)") << 16 << 0;

    // Count up 17 cycles (wrap to 1)
    QTest::newRow("count 17 cycles") << 17 << 1;
}

void TestLevel4BinaryCounter4Bit::testBinaryCounter()
{
    QFETCH(int, numCycles);
    QFETCH(int, expectedFinalCount);

    auto &f = *s_level4BinaryCounter4bit;

    f.tieFreeRun();
    f.clk->setOn(false);
    f.sim->update();

    // The counter now has a Clear input — use it for a deterministic 0 start
    // instead of the old warm-up-edge hack. After clear, each clockCycle
    // increments by exactly one, so the loop count equals the final count.
    f.asyncClear();

    for (int cycle = 0; cycle < numCycles; ++cycle) {
        clockCycle(f.sim, f.clk);
    }

    QCOMPARE(f.readCount(), expectedFinalCount & 0xF);
}

// Asynchronous Clear zeroes the counter immediately (no clock edge needed).
void TestLevel4BinaryCounter4Bit::testClear()
{
    auto &f = *s_level4BinaryCounter4bit;
    f.tieFreeRun();
    f.asyncClear();

    for (int i = 0; i < 5; ++i) {
        clockCycle(f.sim, f.clk);
    }
    QCOMPARE(f.readCount(), 5);

    // Assert Clear without a clock edge — count drops to 0 asynchronously.
    f.clear->setOn(true);
    f.sim->update();
    QCOMPARE(f.readCount(), 0);
    f.clear->setOn(false);
    f.sim->update();
}

// CountEnable=0 holds the value across clock edges; =1 resumes counting.
void TestLevel4BinaryCounter4Bit::testCountEnable()
{
    auto &f = *s_level4BinaryCounter4bit;
    f.tieFreeRun();
    f.asyncClear();

    for (int i = 0; i < 5; ++i) {
        clockCycle(f.sim, f.clk);
    }
    QCOMPARE(f.readCount(), 5);

    // Pause: several edges with CountEnable low must not change the count.
    f.ce->setOn(false);
    f.sim->update();
    for (int i = 0; i < 4; ++i) {
        clockCycle(f.sim, f.clk);
    }
    QCOMPARE(f.readCount(), 5);

    // Resume.
    f.ce->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    QCOMPARE(f.readCount(), 6);
}

// Synchronous parallel Load captures Data on the next rising edge, then the
// counter continues from the loaded value.
void TestLevel4BinaryCounter4Bit::testParallelLoad()
{
    auto &f = *s_level4BinaryCounter4bit;
    f.tieFreeRun();
    f.asyncClear();

    // Load 0xA = 1010.
    f.data[0]->setOn(false);
    f.data[1]->setOn(true);
    f.data[2]->setOn(false);
    f.data[3]->setOn(true);
    f.load->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);   // synchronous load on this edge
    QCOMPARE(f.readCount(), 0xA);

    // Release load and continue counting from 0xA.
    f.load->setOn(false);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    QCOMPARE(f.readCount(), 0xB);
}
