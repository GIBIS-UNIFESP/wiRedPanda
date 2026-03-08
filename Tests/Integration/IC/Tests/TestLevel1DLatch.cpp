// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel1DLatch.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel1DLatch::initTestCase()
{
    // Initialize test environment
}

void TestLevel1DLatch::cleanup()
{
    // Clean up after each test
}

// ============================================================
// D Latch IC Tests - Sequential Design
// ============================================================

// D Latch: Gated SR latch with data input and enable signal
// Inputs: D (Data), Enable
// Outputs: Q, Q_bar
// Behavior:
// - When Enable=1: Q follows D input (transparent)
// - When Enable=0: Q holds last value (latched)
//
// This test uses sequential design - single circuit with multiple steps
// to properly test both transparent and hold modes.
void TestLevel1DLatch::testDLatchSequential()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch dataIn, enableIn;
    Led ledQ;

    builder.add(&dataIn, &enableIn, &ledQ);

    // Load D Latch IC
    IC *dlatchIC = loadBuildingBlockIC("level1_d_latch.panda");
    builder.add(dlatchIC);

    // Connect inputs to IC using semantic port labels
    builder.connect(&dataIn, 0, dlatchIC, "D");
    builder.connect(&enableIn, 0, dlatchIC, "Enable");

    // Connect IC output to LED
    builder.connect(dlatchIC, "Q", &ledQ, 0);

    auto *simulation = builder.initSimulation();

    // Step 1: Enable=1, D=0 -> Q should follow D (Q=0)
    dataIn.setOn(false);
    enableIn.setOn(true);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), false);  // Q follows D=0

    // Step 2: Enable=1, D=1 -> Q should follow D (Q=1)
    dataIn.setOn(true);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), true);   // Q follows D=1

    // Step 3: Enable=0, D=1 -> Q should hold (Q=1, even though D=1)
    enableIn.setOn(false);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), true);   // Q held at 1

    // Step 4: Enable=0, D=0 -> Q should still hold previous (Q=1)
    dataIn.setOn(false);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), true);   // Q still held at 1

    // Step 5: Enable=1 again -> Q should follow D (Q=0)
    enableIn.setOn(true);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), false);  // Q follows D=0

    // Step 6: Enable=0 -> Q should hold (Q=0)
    enableIn.setOn(false);
    simulation->update();
    QCOMPARE(getInputStatus(&ledQ), false);  // Q held at 0
}

