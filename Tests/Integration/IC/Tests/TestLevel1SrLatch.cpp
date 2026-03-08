// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel1SrLatch.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel1SRLatch::initTestCase()
{
    // Initialize test environment
}

void TestLevel1SRLatch::cleanup()
{
    // Clean up after each test
}

// ============================================================
// SR Latch IC Tests - Sequential Design
// ============================================================

// SR Latch: Set-Reset Latch using cross-coupled NOR gates
// Inputs: S (Set), R (Reset)
// Outputs: Q, Q_bar
// Truth table:
// S=1, R=0 -> Q=1 (Set state)
// S=0, R=1 -> Q=0 (Reset state)
// S=0, R=0 -> Q=previous (Hold state)
// S=1, R=1 -> Invalid (undefined behavior)
//
// This test uses sequential design - single circuit with multiple steps
// to properly test state retention (hold) behavior.
void TestLevel1SRLatch::testSRLatchSequential()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch setIn, resetIn;
    Led ledQ;

    builder.add(&setIn, &resetIn, &ledQ);

    // Load SR Latch IC
    IC *srlatchIC = loadBuildingBlockIC("level1_sr_latch.panda");
    builder.add(srlatchIC);

    // Connect inputs to IC using semantic port labels
    builder.connect(&setIn, 0, srlatchIC, "S");
    builder.connect(&resetIn, 0, srlatchIC, "R");

    // Connect IC output to LED using semantic port label
    builder.connect(srlatchIC, "Q", &ledQ, 0);

    auto *simulation = builder.initSimulation();

    // Step 1: Reset the latch to establish known state Q=0
    setIn.setOn(false);
    resetIn.setOn(true);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), false);  // Q=0 after reset

    // Step 2: Release reset, verify hold (Q should stay 0)
    resetIn.setOn(false);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), false);  // Q=0 held

    // Step 3: Set the latch to Q=1
    setIn.setOn(true);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), true);   // Q=1 after set

    // Step 4: Release set, verify hold (Q should stay 1)
    setIn.setOn(false);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), true);   // Q=1 held

    // Step 5: Reset again to verify we can change state
    resetIn.setOn(true);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), false);  // Q=0 after reset

    // Step 6: Release reset, verify hold again
    resetIn.setOn(false);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), false);  // Q=0 held
}
