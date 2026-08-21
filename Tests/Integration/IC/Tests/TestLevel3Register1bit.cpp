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

// Reset=TRUE -> NOT gate inverts it -> Clear=FALSE (active-low assert), async (no clock needed).
void TestLevel3Register1Bit::testRegisterAsyncReset()
{
    auto &f = *s_level3Register1bit;

    f.reset->setOn(true);
    f.sim->update();

    QCOMPARE(TestUtils::inputStatus(f.q), false);
    QCOMPARE(TestUtils::inputStatus(f.notQ), true);
}

void TestLevel3Register1Bit::testRegisterWriteOne()
{
    auto &f = *s_level3Register1bit;

    f.reset->setOn(false);
    f.data->setOn(true);
    f.writeEnable->setOn(true);
    f.sim->update();

    clockCycle(f.sim, f.clock);

    QCOMPARE(TestUtils::inputStatus(f.q), 1);
    QCOMPARE(TestUtils::inputStatus(f.notQ), 0);
}

void TestLevel3Register1Bit::testRegisterHoldWhenWriteDisabled()
{
    auto &f = *s_level3Register1bit;

    // Write 1 first so there's a non-default value to hold.
    f.reset->setOn(false);
    f.data->setOn(true);
    f.writeEnable->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clock);
    QCOMPARE(TestUtils::inputStatus(f.q), 1);

    // Change data to 0 with WriteEnable disabled: the register must not update.
    f.data->setOn(false);
    f.writeEnable->setOn(false);
    f.sim->update();
    QCOMPARE(TestUtils::inputStatus(f.q), 1);
    QCOMPARE(TestUtils::inputStatus(f.notQ), 0);

    // A clock edge with WriteEnable still disabled must also not update.
    clockCycle(f.sim, f.clock);
    QCOMPARE(TestUtils::inputStatus(f.q), 1);
    QCOMPARE(TestUtils::inputStatus(f.notQ), 0);
}

void TestLevel3Register1Bit::testRegisterWriteZero()
{
    auto &f = *s_level3Register1bit;

    // Write 1 first so writing 0 is a real transition, not a no-op.
    f.reset->setOn(false);
    f.data->setOn(true);
    f.writeEnable->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clock);
    QCOMPARE(TestUtils::inputStatus(f.q), 1);

    f.data->setOn(false);
    f.writeEnable->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clock);

    QCOMPARE(TestUtils::inputStatus(f.q), 0);
    QCOMPARE(TestUtils::inputStatus(f.notQ), 1);
}

void TestLevel3Register1Bit::testRegisterResetOverridesWrite()
{
    auto &f = *s_level3Register1bit;

    // Write 1 first so Reset has a non-zero value to override.
    f.reset->setOn(false);
    f.data->setOn(true);
    f.writeEnable->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clock);
    QCOMPARE(TestUtils::inputStatus(f.q), 1);

    // Assert Reset while Data/WriteEnable still request a write: Reset wins, async.
    f.data->setOn(true);
    f.writeEnable->setOn(true);
    f.reset->setOn(true);
    f.sim->update();

    QCOMPARE(TestUtils::inputStatus(f.q), 0);
    QCOMPARE(TestUtils::inputStatus(f.notQ), 1);
}

void TestLevel3Register1Bit::testRegisterWriteAfterResetReleased()
{
    auto &f = *s_level3Register1bit;

    // Drive Reset, then release it.
    f.reset->setOn(true);
    f.sim->update();
    f.reset->setOn(false);
    f.sim->update();

    // Writing must work normally again once Reset is released.
    f.data->setOn(true);
    f.writeEnable->setOn(true);
    f.sim->update();

    // Ensure clock is LOW first for a clean rising edge.
    if (f.clock->isOn()) {
        f.clock->setOn(false);
        f.sim->update();
    }
    clockCycle(f.sim, f.clock);

    QCOMPARE(TestUtils::inputStatus(f.q), 1);
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
