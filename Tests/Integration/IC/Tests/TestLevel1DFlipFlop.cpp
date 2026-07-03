// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel1DFlipFlop.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::inputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct DFlipFlopFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *dataIn = nullptr, *clockIn = nullptr, *presetIn = nullptr, *clearIn = nullptr;
    Led *ledQ = nullptr, *ledQBar = nullptr;
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
        ledQBar = new Led();

        builder.add(dataIn, clockIn, presetIn, clearIn, ledQ, ledQBar);

        presetIn->setOn(true);
        clearIn->setOn(true);

        ic = loadBuildingBlockIC("level1_d_flip_flop.panda");
        builder.add(ic);

        builder.connect(dataIn, 0, ic, "D");
        builder.connect(clockIn, 0, ic, "Clock");
        builder.connect(presetIn, 0, ic, "Preset");
        builder.connect(clearIn, 0, ic, "Clear");
        builder.connect(ic, "Q", ledQ, 0);
        builder.connect(ic, "Q_bar", ledQBar, 0);

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
    QCOMPARE(inputStatus(f.ledQ), false);     // Q=0 after reset pulse
    QCOMPARE(inputStatus(f.ledQBar), true);   // Q_bar is the complement

    // Step 2: Set D=1 while Clock=0 -> Q should not change yet
    f.dataIn->setOn(true);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), false);     // Q still 0 (clock low)
    QCOMPARE(inputStatus(f.ledQBar), true);

    // Step 3: Pulse clock high -> Q should capture D=1
    f.clockIn->setOn(true);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), true);      // Q=1 after clock rising edge
    QCOMPARE(inputStatus(f.ledQBar), false);

    // Step 4: Set D=0 while Clock=1 -> Q should hold at 1
    f.dataIn->setOn(false);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), true);      // Q held at 1 (clock still high)
    QCOMPARE(inputStatus(f.ledQBar), false);

    // Step 5: Clock goes low -> Q should still hold at 1
    f.clockIn->setOn(false);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), true);      // Q held at 1
    QCOMPARE(inputStatus(f.ledQBar), false);

    // Step 6: D=0 while Clock=0 -> no change yet
    // (already set in step 4)
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), true);      // Q still 1
    QCOMPARE(inputStatus(f.ledQBar), false);

    // Step 7: Clock rises again -> Q captures D=0
    f.clockIn->setOn(true);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), false);     // Q=0 after clock rising edge
    QCOMPARE(inputStatus(f.ledQBar), true);

    // Step 8: Clock low -> Q holds at 0
    f.clockIn->setOn(false);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), false);     // Q held at 0
    QCOMPARE(inputStatus(f.ledQBar), true);
}

// Async Preset/Clear must override a clock edge that would otherwise capture the
// opposite value (active-LOW controls win over the synchronous data path).
void TestLevel1DFlipFlop::testPresetClearOverrideClock()
{
    auto &f = *s_level1DFlipFlop;

    // Start cleared: Q=0.
    f.presetIn->setOn(true);
    f.clearIn->setOn(true);
    f.dataIn->setOn(false);
    f.clockIn->setOn(true);
    f.sim->update();
    f.clockIn->setOn(false);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), false);

    // D=1 would set Q=1 on the rising edge, but assert Clear=0: Q stays forced 0.
    f.dataIn->setOn(true);
    f.clearIn->setOn(false);
    f.clockIn->setOn(true);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), false);
    QCOMPARE(inputStatus(f.ledQBar), true);
    f.clockIn->setOn(false);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), false);

    // Release Clear, assert Preset=0: Q forced to 1 regardless of D=1/clock.
    f.clearIn->setOn(true);
    f.presetIn->setOn(false);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), true);
    QCOMPARE(inputStatus(f.ledQBar), false);

    // Release Preset, D=0: a rising edge now captures D normally (Q=0).
    // The master samples D while Clock=LOW, so settle the low phase before the edge.
    f.presetIn->setOn(true);
    f.dataIn->setOn(false);
    f.clockIn->setOn(false);
    f.sim->update();
    f.clockIn->setOn(true);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), false);
    QCOMPARE(inputStatus(f.ledQBar), true);
}

// Async Preset/Clear must override the data path even while Clock=1 with the
// master latch holding the opposite value (7474 semantics — F56: before the
// fix, Preset/Clear were injected into the slave latch only, so this
// scenario drove both slave NORs low: the invalid Q=Q_bar=0 state).
void TestLevel1DFlipFlop::testAsyncPresetClearUnderClockHigh()
{
    auto &f = *s_level1DFlipFlop;

    // Establish Q=0 with the master holding 0, then keep Clock HIGH
    f.presetIn->setOn(true);
    f.clearIn->setOn(true);
    f.dataIn->setOn(false);
    f.clockIn->setOn(false);
    f.sim->update();
    f.clockIn->setOn(true);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), false);

    // Assert Preset (active LOW) while Clock=1: Q must be forced to 1
    f.presetIn->setOn(false);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), true);
    QCOMPARE(inputStatus(f.ledQBar), false);

    // Release Preset with Clock still HIGH: Q must hold at 1 (the master was
    // forced too, so the open slave keeps transferring 1)
    f.presetIn->setOn(true);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), true);
    QCOMPARE(inputStatus(f.ledQBar), false);

    // Symmetric: establish Q=1 with the master holding 1, Clock HIGH
    f.dataIn->setOn(true);
    f.clockIn->setOn(false);
    f.sim->update();
    f.clockIn->setOn(true);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), true);

    // Assert Clear (active LOW) while Clock=1: Q must be forced to 0
    f.clearIn->setOn(false);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), false);
    QCOMPARE(inputStatus(f.ledQBar), true);

    // Release Clear with Clock still HIGH: Q must hold at 0
    f.clearIn->setOn(true);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), false);
    QCOMPARE(inputStatus(f.ledQBar), true);
}

// Asserting Preset AND Clear simultaneously is the documented "don't" input. It
// is deterministic here, not undefined: both async paths drive their injection
// ORs HIGH, forcing both slave NORs LOW -> Q=Q_bar=0. This pins that defined
// contention output and confirms a clean resolution when one control is
// released (the surviving control wins).
void TestLevel1DFlipFlop::testPresetClearBothAsserted()
{
    auto &f = *s_level1DFlipFlop;

    // Idle clock; assert both Preset=0 and Clear=0
    f.dataIn->setOn(false);
    f.clockIn->setOn(false);
    f.presetIn->setOn(false);
    f.clearIn->setOn(false);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), false);     // both forced low
    QCOMPARE(inputStatus(f.ledQBar), false);

    // Release Clear (Preset still asserted): Preset wins -> Q=1
    f.clearIn->setOn(true);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), true);
    QCOMPARE(inputStatus(f.ledQBar), false);

    // Re-assert both, then release Preset: Clear wins -> Q=0
    f.clearIn->setOn(false);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), false);
    QCOMPARE(inputStatus(f.ledQBar), false);
    f.presetIn->setOn(true);
    f.sim->update();
    QCOMPARE(inputStatus(f.ledQ), false);
    QCOMPARE(inputStatus(f.ledQBar), true);
}
