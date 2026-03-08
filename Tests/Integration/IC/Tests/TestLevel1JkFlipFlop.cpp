// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel1JkFlipFlop.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

// ============================================================
// JK Flip-Flop IC Tests
// ============================================================
//
// Inputs: J, K, Clock, Preset (active LOW), Clear (active LOW)
// Outputs: Q, Q_bar
//
// Truth table (on rising clock edge):
//   J=0, K=0 -> Hold (Q unchanged)
//   J=0, K=1 -> Reset (Q=0)
//   J=1, K=0 -> Set (Q=1)
//   J=1, K=1 -> Toggle (Q inverts)
//
// Async controls (active LOW, override clock):
//   Preset=0 -> Q=1 (asynchronous set)
//   Clear=0  -> Q=0 (asynchronous clear)

struct JkFlipFlopFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *jIn = nullptr, *kIn = nullptr, *clockIn = nullptr;
    InputSwitch *presetIn = nullptr, *clearIn = nullptr;
    Led *ledQ = nullptr, *ledQBar = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        jIn = new InputSwitch();
        kIn = new InputSwitch();
        clockIn = new InputSwitch();
        presetIn = new InputSwitch();
        clearIn = new InputSwitch();
        ledQ = new Led();
        ledQBar = new Led();

        builder.add(jIn, kIn, clockIn, presetIn, clearIn, ledQ, ledQBar);

        ic = loadBuildingBlockIC("level1_jk_flip_flop.panda");
        builder.add(ic);

        builder.connect(jIn, 0, ic, "J");
        builder.connect(kIn, 0, ic, "K");
        builder.connect(clockIn, 0, ic, "Clock");
        builder.connect(presetIn, 0, ic, "Preset");
        builder.connect(clearIn, 0, ic, "Clear");
        builder.connect(ic, "Q", ledQ, 0);
        builder.connect(ic, "Q_bar", ledQBar, 0);

        presetIn->setOn(true);
        clearIn->setOn(true);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    void clockPulse()
    {
        clockIn->setOn(true);
        sim->update();
        clockIn->setOn(false);
        sim->update();
    }

    bool q() { return getInputStatus(ledQ); }
    bool qBar() { return getInputStatus(ledQBar); }

    void resetToKnownState()
    {
        jIn->setOn(false);
        kIn->setOn(true);
        presetIn->setOn(true);
        clearIn->setOn(true);
        clockPulse();
    }
};

static std::unique_ptr<JkFlipFlopFixture> s_level1JkFlipFlop;

void TestLevel1JKFlipFlop::initTestCase()
{
    s_level1JkFlipFlop = std::make_unique<JkFlipFlopFixture>();
    QVERIFY(s_level1JkFlipFlop->build());
}

void TestLevel1JKFlipFlop::cleanupTestCase()
{
    s_level1JkFlipFlop.reset();
}

void TestLevel1JKFlipFlop::cleanup()
{
    if (s_level1JkFlipFlop && s_level1JkFlipFlop->sim) {
        s_level1JkFlipFlop->sim->restart();
        s_level1JkFlipFlop->sim->update();
    }
}

void TestLevel1JKFlipFlop::testResetAndSet()
{
    auto &f = *s_level1JkFlipFlop;
    f.resetToKnownState();

    // Reset: J=0, K=1 -> Q=0
    f.jIn->setOn(false);
    f.kIn->setOn(true);
    f.clockPulse();
    QCOMPARE(f.q(), false);
    QCOMPARE(f.qBar(), true);

    // Confirm Q stays 0 on another reset pulse
    f.clockPulse();
    QCOMPARE(f.q(), false);
    QCOMPARE(f.qBar(), true);

    // Set: J=1, K=0 -> Q=1
    f.jIn->setOn(true);
    f.kIn->setOn(false);
    f.clockPulse();
    QCOMPARE(f.q(), true);
    QCOMPARE(f.qBar(), false);

    // Confirm Q stays 1 on another set pulse
    f.clockPulse();
    QCOMPARE(f.q(), true);
    QCOMPARE(f.qBar(), false);

    // Reset again: J=0, K=1 -> Q=0
    f.jIn->setOn(false);
    f.kIn->setOn(true);
    f.clockPulse();
    QCOMPARE(f.q(), false);
    QCOMPARE(f.qBar(), true);
}

void TestLevel1JKFlipFlop::testHold()
{
    auto &f = *s_level1JkFlipFlop;
    f.resetToKnownState();

    // First reset to known state: Q=0
    f.jIn->setOn(false);
    f.kIn->setOn(true);
    f.clockPulse();
    QCOMPARE(f.q(), false);

    // Hold: J=0, K=0 -> Q stays 0
    f.jIn->setOn(false);
    f.kIn->setOn(false);
    f.clockPulse();
    QCOMPARE(f.q(), false);
    QCOMPARE(f.qBar(), true);

    // Set to Q=1
    f.jIn->setOn(true);
    f.kIn->setOn(false);
    f.clockPulse();
    QCOMPARE(f.q(), true);

    // Hold: J=0, K=0 -> Q stays 1
    f.jIn->setOn(false);
    f.kIn->setOn(false);
    f.clockPulse();
    QCOMPARE(f.q(), true);
    QCOMPARE(f.qBar(), false);
}

void TestLevel1JKFlipFlop::testToggle()
{
    auto &f = *s_level1JkFlipFlop;
    f.resetToKnownState();

    // Reset to known state: Q=0
    f.jIn->setOn(false);
    f.kIn->setOn(true);
    f.clockPulse();
    QCOMPARE(f.q(), false);

    // Toggle: J=1, K=1 -> Q flips to 1
    f.jIn->setOn(true);
    f.kIn->setOn(true);
    f.clockPulse();
    QCOMPARE(f.q(), true);
    QCOMPARE(f.qBar(), false);

    // Toggle again: J=1, K=1 -> Q flips back to 0
    f.clockPulse();
    QCOMPARE(f.q(), false);
    QCOMPARE(f.qBar(), true);

    // Toggle once more: Q=1
    f.clockPulse();
    QCOMPARE(f.q(), true);
    QCOMPARE(f.qBar(), false);
}

void TestLevel1JKFlipFlop::testAsyncPreset()
{
    auto &f = *s_level1JkFlipFlop;
    f.resetToKnownState();

    // Reset to known state: Q=0
    f.jIn->setOn(false);
    f.kIn->setOn(true);
    f.clockPulse();
    QCOMPARE(f.q(), false);

    // Async preset: Preset=0 -> Q forced to 1 without clock
    f.presetIn->setOn(false);
    f.sim->update();
    QCOMPARE(f.q(), true);
    QCOMPARE(f.qBar(), false);

    // Release preset: Q should hold at 1
    f.presetIn->setOn(true);
    f.sim->update();
    QCOMPARE(f.q(), true);
}

void TestLevel1JKFlipFlop::testAsyncClear()
{
    auto &f = *s_level1JkFlipFlop;
    f.resetToKnownState();

    // Initialize to known state: reset first, then set
    f.jIn->setOn(false);
    f.kIn->setOn(true);
    f.clockPulse();
    QCOMPARE(f.q(), false);

    // Now set: Q=1
    f.jIn->setOn(true);
    f.kIn->setOn(false);
    f.clockPulse();
    QCOMPARE(f.q(), true);

    // Async clear: Clear=0 -> Q forced to 0 without clock
    f.clearIn->setOn(false);
    f.sim->update();
    QCOMPARE(f.q(), false);
    QCOMPARE(f.qBar(), true);

    // Release clear: Q should hold at 0
    f.clearIn->setOn(true);
    f.sim->update();
    QCOMPARE(f.q(), false);
}

void TestLevel1JKFlipFlop::testPresetClearOverrideClock()
{
    auto &f = *s_level1JkFlipFlop;
    f.resetToKnownState();

    // Set J=1, K=0 so a clock edge would normally set Q=1
    f.jIn->setOn(true);
    f.kIn->setOn(false);

    // But assert Clear=0 -> Q forced to 0 regardless
    f.clearIn->setOn(false);
    f.clockPulse();
    QCOMPARE(f.q(), false);
    QCOMPARE(f.qBar(), true);

    // Release clear, assert Preset=0 -> Q forced to 1
    f.clearIn->setOn(true);
    f.presetIn->setOn(false);
    f.sim->update();
    QCOMPARE(f.q(), true);
    QCOMPARE(f.qBar(), false);

    // Release preset, normal operation resumes
    f.presetIn->setOn(true);
    f.sim->update();
    QCOMPARE(f.q(), true);
}

