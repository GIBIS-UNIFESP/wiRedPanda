// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel3Register1bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

struct Register1bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *data = nullptr, *clock = nullptr, *writeEnable = nullptr, *reset = nullptr;
    Led *q = nullptr, *notQ = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        data = new InputSwitch();
        clock = new InputSwitch();
        writeEnable = new InputSwitch();
        reset = new InputSwitch();
        q = new Led();
        notQ = new Led();

        builder.add(data, clock, writeEnable, reset, q, notQ);

        ic = loadBuildingBlockIC("level3_register_1bit.panda");
        builder.add(ic);

        builder.connect(data, 0, ic, "Data");
        builder.connect(clock, 0, ic, "Clock");
        builder.connect(writeEnable, 0, ic, "WriteEnable");
        builder.connect(reset, 0, ic, "Reset");
        builder.connect(ic, "Q", q, 0);
        builder.connect(ic, "NotQ", notQ, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Register1bitFixture> s_level3Register1bit;

void TestLevel3Register1Bit::initTestCase()
{
    s_level3Register1bit = std::make_unique<Register1bitFixture>();
    QVERIFY(s_level3Register1bit->build());
}

void TestLevel3Register1Bit::cleanupTestCase()
{
    s_level3Register1bit.reset();
}

void TestLevel3Register1Bit::cleanup()
{
    if (s_level3Register1bit && s_level3Register1bit->sim) {
        s_level3Register1bit->sim->restart();
        s_level3Register1bit->sim->update();
    }
}

/**
 * Test: 1-bit Register with write, hold, and reset operations
 *
 * Test scenarios:
 * 1. Write 1 on clock edge
 * 2. Hold value when WriteEnable=0
 * 3. Reset to 0 (async)
 * 4. Write 0 then read Q and NotQ
 */
void TestLevel3Register1Bit::test1BitRegister() {
    auto &f = *s_level3Register1bit;
    auto *data = f.data;
    auto *clock = f.clock;
    auto *writeEnable = f.writeEnable;
    auto *reset = f.reset;
    auto *q = f.q;
    auto *notQ = f.notQ;
    auto *sim = f.sim;

    // Test 1: Reset to 0
    reset->setOn(true);             // Reset=TRUE → NOT gate inverts it → Clear=FALSE (active-low assert)
    sim->update();
    // After reset: Q=0, NotQ=1
    QCOMPARE(TestUtils::getInputStatus(q), false);
    QCOMPARE(TestUtils::getInputStatus(notQ), true);

    // Test 2: Write 1 on clock edge
    reset->setOn(false);            // De-assert reset (false = not resetting)
    data->setOn(true);              // Set data = 1
    writeEnable->setOn(true);       // Enable writing
    sim->update();

    // Pulse clock
    clockCycle(sim, clock);

    QCOMPARE(TestUtils::getInputStatus(q), 1);
    QCOMPARE(TestUtils::getInputStatus(notQ), 0);

    // Test 3: Hold value when WriteEnable=0
    data->setOn(false);             // Change data to 0

    writeEnable->setOn(false);      // Disable writing (hold)

    sim->update();
    int q_after_hold = TestUtils::getInputStatus(q);
    int notq_after_hold = TestUtils::getInputStatus(notQ);

    // IMPORTANT: Hold should preserve value - Q should STILL be 1, not change
    QCOMPARE(q_after_hold, 1);  // Verify hold preserves Q value
    QCOMPARE(notq_after_hold, 0);

    clockCycle(sim, clock);

    QCOMPARE(TestUtils::getInputStatus(q), 1);
    QCOMPARE(TestUtils::getInputStatus(notQ), 0);

    // Test 4: Write 0 on clock edge
    data->setOn(false);             // Set data = 0
    writeEnable->setOn(true);       // Enable writing
    sim->update();

    clockCycle(sim, clock);

    QCOMPARE(TestUtils::getInputStatus(q), 0);
    QCOMPARE(TestUtils::getInputStatus(notQ), 1);

    // Test 5: Reset overrides everything
    data->setOn(true);

    writeEnable->setOn(true);

    reset->setOn(true);             // Assert reset (triggers through NOT gate)

    sim->update();

    QCOMPARE(TestUtils::getInputStatus(q), 0);
    QCOMPARE(TestUtils::getInputStatus(notQ), 1);

    // Test 5b: De-assert reset and see what happens
    reset->setOn(false);

    sim->update();

    // Now try to write 1
    data->setOn(true);
    writeEnable->setOn(true);

    sim->update();

    // Explicitly ensure clock is LOW for a clean rising edge
    if (clock->isOn()) {
        clock->setOn(false);
        sim->update();
    }

    clockCycle(sim, clock);

    QCOMPARE(TestUtils::getInputStatus(q), 1);  // Verify write after reset actually worked
}

/**
 * Test: Verify 1-bit Register IC structure and port count
 *
 * Validates:
 * - IC has 4 input ports (Data, Clock, WriteEnable, Reset)
 * - IC has 2 output ports (Q, NotQ)
 * - IC can be loaded and instantiated
 */
void TestLevel3Register1Bit::test1BitRegisterStructure() {
    auto &f = *s_level3Register1bit;

    QVERIFY(f.ic != nullptr);
    QCOMPARE(f.ic->inputSize(), 4);   // 4 input ports: Data, Clock, WriteEnable, Reset
    QCOMPARE(f.ic->outputSize(), 2);  // 2 output ports: Q, NotQ
}

