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

struct SrLatchFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *setIn = nullptr, *resetIn = nullptr;
    Led *ledQ = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        setIn = new InputSwitch();
        resetIn = new InputSwitch();
        ledQ = new Led();

        builder.add(setIn, resetIn, ledQ);

        ic = loadBuildingBlockIC("level1_sr_latch.panda");
        builder.add(ic);

        builder.connect(setIn, 0, ic, "S");
        builder.connect(resetIn, 0, ic, "R");
        builder.connect(ic, "Q", ledQ, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<SrLatchFixture> s_level1SrLatch;

void TestLevel1SRLatch::initTestCase()
{
    s_level1SrLatch = std::make_unique<SrLatchFixture>();
    QVERIFY(s_level1SrLatch->build());
}

void TestLevel1SRLatch::cleanupTestCase()
{
    s_level1SrLatch.reset();
}

void TestLevel1SRLatch::cleanup()
{
    if (s_level1SrLatch && s_level1SrLatch->sim) {
        s_level1SrLatch->sim->restart();
        s_level1SrLatch->sim->update();
    }
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
    auto &f = *s_level1SrLatch;

    // Step 1: Reset the latch to establish known state Q=0
    f.setIn->setOn(false);
    f.resetIn->setOn(true);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), false);  // Q=0 after reset

    // Step 2: Release reset, verify hold (Q should stay 0)
    f.resetIn->setOn(false);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), false);  // Q=0 held

    // Step 3: Set the latch to Q=1
    f.setIn->setOn(true);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), true);   // Q=1 after set

    // Step 4: Release set, verify hold (Q should stay 1)
    f.setIn->setOn(false);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), true);   // Q=1 held

    // Step 5: Reset again to verify we can change state
    f.resetIn->setOn(true);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), false);  // Q=0 after reset

    // Step 6: Release reset, verify hold again
    f.resetIn->setOn(false);
    f.sim->update();
    QCOMPARE(getInputStatus(f.ledQ), false);  // Q=0 held
}

