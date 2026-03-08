// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel1DFlipFlop.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct DFlipFlopFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *dataIn = nullptr, *clockIn = nullptr, *presetIn = nullptr, *clearIn = nullptr;
    Led *ledQ = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        dataIn = new InputSwitch();
        clockIn = new InputSwitch();
        presetIn = new InputSwitch();
        clearIn = new InputSwitch();
        ledQ = new Led();

        builder.add(dataIn, clockIn, presetIn, clearIn, ledQ);

        presetIn->setOn(true);
        clearIn->setOn(true);

        ic = loadBuildingBlockIC("level1_d_flip_flop.panda");
        builder.add(ic);

        builder.connect(dataIn, 0, ic, "D");
        builder.connect(clockIn, 0, ic, "Clock");
        builder.connect(presetIn, 0, ic, "Preset");
        builder.connect(clearIn, 0, ic, "Clear");
        builder.connect(ic, "Q", ledQ, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<DFlipFlopFixture> s_level1DFlipFlop;

void TestLevel1DFlipFlop::initTestCase()
{
    s_level1DFlipFlop = std::make_unique<DFlipFlopFixture>();
    QVERIFY(s_level1DFlipFlop->build());
}

void TestLevel1DFlipFlop::cleanupTestCase()
{
    s_level1DFlipFlop.reset();
}

void TestLevel1DFlipFlop::cleanup()
{
    if (s_level1DFlipFlop && s_level1DFlipFlop->sim) {
        s_level1DFlipFlop->sim->restart();
        s_level1DFlipFlop->sim->update();
    }
}

// ============================================================
// D Flip-Flop IC Tests - Sequential Design
// ============================================================

// D Flip-Flop: Edge-triggered master-slave configuration
// Inputs: D (Data), Clock
// Outputs: Q, Q_bar
// Behavior:
// - Output Q captures D value on rising edge of clock
// - Output changes on clock rising edge only
// - During clock low or high: Q remains stable
//
// This test uses sequential design - single circuit with multiple steps
// to test edge-triggered capture behavior.
void TestLevel1DFlipFlop::testDFlipFlopSequential()
{
    auto &f = *s_level1DFlipFlop;

    // Step 1: Initialize with D=0, Clock pulse to establish Q=0
    f.dataIn->setOn(false);
    f.clockIn->setOn(true);
    f.presetIn->setOn(true);  // Preset inactive (HIGH)
    f.clearIn->setOn(true);   // Clear inactive (HIGH)
    f.sim->update();
    f.clockIn->setOn(false);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), false);  // Q=0 after reset pulse

    // Step 2: Set D=1 while Clock=0 -> Q should not change yet
    f.dataIn->setOn(true);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), false);  // Q still 0 (clock low)

    // Step 3: Pulse clock high -> Q should capture D=1
    f.clockIn->setOn(true);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), true);   // Q=1 after clock rising edge

    // Step 4: Set D=0 while Clock=1 -> Q should hold at 1
    f.dataIn->setOn(false);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), true);   // Q held at 1 (clock still high)

    // Step 5: Clock goes low -> Q should still hold at 1
    f.clockIn->setOn(false);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), true);   // Q held at 1

    // Step 6: D=0 while Clock=0 -> no change yet
    // (already set in step 4)
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), true);   // Q still 1

    // Step 7: Clock rises again -> Q captures D=0
    f.clockIn->setOn(true);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), false);  // Q=0 after clock rising edge

    // Step 8: Clock low -> Q holds at 0
    f.clockIn->setOn(false);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), false);  // Q held at 0
}

