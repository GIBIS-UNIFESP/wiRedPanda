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

struct DLatchFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *dataIn = nullptr, *enableIn = nullptr;
    Led *ledQ = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        dataIn = new InputSwitch();
        enableIn = new InputSwitch();
        ledQ = new Led();

        builder.add(dataIn, enableIn, ledQ);

        ic = loadBuildingBlockIC("level1_d_latch.panda");
        builder.add(ic);

        builder.connect(dataIn, 0, ic, "D");
        builder.connect(enableIn, 0, ic, "Enable");
        builder.connect(ic, "Q", ledQ, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<DLatchFixture> s_level1DLatch;

void TestLevel1DLatch::initTestCase()
{
    s_level1DLatch = std::make_unique<DLatchFixture>();
    QVERIFY(s_level1DLatch->build());
}

void TestLevel1DLatch::cleanupTestCase()
{
    s_level1DLatch.reset();
}

void TestLevel1DLatch::cleanup()
{
    if (s_level1DLatch && s_level1DLatch->sim) {
        s_level1DLatch->sim->restart();
        s_level1DLatch->sim->update();
    }
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
    auto &f = *s_level1DLatch;

    // Step 1: Enable=1, D=0 -> Q should follow D (Q=0)
    f.dataIn->setOn(false);
    f.enableIn->setOn(true);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), false);  // Q follows D=0

    // Step 2: Enable=1, D=1 -> Q should follow D (Q=1)
    f.dataIn->setOn(true);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), true);   // Q follows D=1

    // Step 3: Enable=0, D=1 -> Q should hold (Q=1, even though D=1)
    f.enableIn->setOn(false);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), true);   // Q held at 1

    // Step 4: Enable=0, D=0 -> Q should still hold previous (Q=1)
    f.dataIn->setOn(false);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), true);   // Q still held at 1

    // Step 5: Enable=1 again -> Q should follow D (Q=0)
    f.enableIn->setOn(true);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), false);  // Q follows D=0

    // Step 6: Enable=0 -> Q should hold (Q=0)
    f.enableIn->setOn(false);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), false);  // Q held at 0
}

