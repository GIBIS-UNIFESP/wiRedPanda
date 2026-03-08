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

void TestLevel1DFlipFlop::initTestCase()
{
    // Initialize test environment
}

void TestLevel1DFlipFlop::cleanup()
{
    // Clean up after each test
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
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch dataIn, clockIn, presetIn, clearIn;
    Led ledQ;

    builder.add(&dataIn, &clockIn, &presetIn, &clearIn, &ledQ);

    // Set Preset and Clear to inactive (HIGH)
    presetIn.setOn(true);
    clearIn.setOn(true);

    // Load D Flip-Flop IC
    IC *dffIC = loadBuildingBlockIC("level1_d_flip_flop.panda");
    builder.add(dffIC);

    // Connect inputs to IC using semantic port labels
    builder.connect(&dataIn, 0, dffIC, "D");
    builder.connect(&clockIn, 0, dffIC, "Clock");
    builder.connect(&presetIn, 0, dffIC, "Preset");
    builder.connect(&clearIn, 0, dffIC, "Clear");

    // Connect IC output to LED
    builder.connect(dffIC, "Q", &ledQ, 0);

    auto *simulation = builder.initSimulation();

    // Step 1: Initialize with D=0, Clock pulse to establish Q=0
    dataIn.setOn(false);
    clockIn.setOn(true);
    presetIn.setOn(true);  // Preset inactive (HIGH)
    clearIn.setOn(true);   // Clear inactive (HIGH)
    simulation->update();
    clockIn.setOn(false);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), false);  // Q=0 after reset pulse

    // Step 2: Set D=1 while Clock=0 -> Q should not change yet
    dataIn.setOn(true);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), false);  // Q still 0 (clock low)

    // Step 3: Pulse clock high -> Q should capture D=1
    clockIn.setOn(true);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), true);   // Q=1 after clock rising edge

    // Step 4: Set D=0 while Clock=1 -> Q should hold at 1
    dataIn.setOn(false);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), true);   // Q held at 1 (clock still high)

    // Step 5: Clock goes low -> Q should still hold at 1
    clockIn.setOn(false);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), true);   // Q held at 1

    // Step 6: D=0 while Clock=0 -> no change yet
    // (already set in step 4)
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), true);   // Q still 1

    // Step 7: Clock rises again -> Q captures D=0
    clockIn.setOn(true);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), false);  // Q=0 after clock rising edge

    // Step 8: Clock low -> Q holds at 0
    clockIn.setOn(false);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), false);  // Q held at 0
}
