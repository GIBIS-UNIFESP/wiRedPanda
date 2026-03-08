// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestsWithoutPanda/TestRamCell1bit.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "Tests/Common/TestUtils.h"

using TestUtils::clockCycle;

// ============================================================
// RAM Cell circuit fixture
// ============================================================

struct RamCellFixture {
    WorkSpace ws;
    InputSwitch dataIn, writeEnable, clock;
    And writeControlGate;
    Mux cellDataMux;
    DFlipFlop storage;
    Led dataOut;
    Simulation *sim = nullptr;

    RamCellFixture()
    {
        CircuitBuilder builder(ws.scene());
        builder.add(&dataIn, &writeEnable, &clock, &dataOut);
        builder.add(&writeControlGate, &cellDataMux, &storage);

        // Stage 1: Write control gate (both inputs tied to WE)
        builder.connect(&writeEnable, 0, &writeControlGate, 0);
        builder.connect(&writeEnable, 0, &writeControlGate, 1);

        // Stage 2: Data mux — hold (Q) when WE=0, write (dataIn) when WE=1
        builder.connect(&storage, 0, &cellDataMux, 0);           // Q (hold when WE=0)
        builder.connect(&dataIn, 0, &cellDataMux, 1);            // dataIn (write when WE=1)
        builder.connect(&writeControlGate, 0, &cellDataMux, 2);  // select = WE

        builder.connect(&cellDataMux, 0, &storage, 0);
        builder.connect(&clock, 0, &storage, 1);
        builder.connect(&storage, 0, &dataOut, 0);

        sim = builder.initSimulation();
        sim->update();
    }
};

// ============================================================
// Individual test cases (previously a switch-dispatch in one function)
// ============================================================

void TestRAMCell1Bit::testWriteZeroReadZero()
{
    RamCellFixture f;
    f.dataIn.setOn(false);
    f.writeEnable.setOn(true);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), false);
}

void TestRAMCell1Bit::testWriteOneReadOne()
{
    RamCellFixture f;
    f.dataIn.setOn(true);
    f.writeEnable.setOn(true);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), true);
}

void TestRAMCell1Bit::testWriteOneHoldWeDisabled()
{
    RamCellFixture f;

    // Write 1
    f.dataIn.setOn(true);
    f.writeEnable.setOn(true);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), true);

    // Hold: disable WE, change data — stored value must not change
    f.dataIn.setOn(false);
    f.writeEnable.setOn(false);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), true);
}

void TestRAMCell1Bit::testWriteZeroHoldWeDisabled()
{
    RamCellFixture f;

    // Write 0
    f.dataIn.setOn(false);
    f.writeEnable.setOn(true);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), false);

    // Hold: disable WE, change data — stored value must not change
    f.dataIn.setOn(true);
    f.writeEnable.setOn(false);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), false);
}

void TestRAMCell1Bit::testWriteOneMultipleHolds()
{
    RamCellFixture f;

    // Write 1
    f.dataIn.setOn(true);
    f.writeEnable.setOn(true);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), true);

    // Multiple hold cycles with alternating data on the line
    f.writeEnable.setOn(false);
    for (int i = 0; i < 3; ++i) {
        f.dataIn.setOn(i % 2);
        f.sim->update();
        clockCycle(f.sim, &f.clock);
        f.sim->update();
        QCOMPARE(TestUtils::getInputStatus(&f.dataOut), true);
    }
}

void TestRAMCell1Bit::testWeDisabledFromStart()
{
    RamCellFixture f;

    // Try to write 1 with WE disabled — cell should remain 0
    f.dataIn.setOn(true);
    f.writeEnable.setOn(false);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), false);
}

void TestRAMCell1Bit::testRapidWeToggle()
{
    RamCellFixture f;

    // Write 1
    f.dataIn.setOn(true);
    f.writeEnable.setOn(true);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), true);

    // Hold cycle
    f.writeEnable.setOn(false);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), true);

    // Write 0
    f.dataIn.setOn(false);
    f.writeEnable.setOn(true);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), false);
}

void TestRAMCell1Bit::testDataChangeWithoutWe()
{
    RamCellFixture f;

    // Write 1
    f.dataIn.setOn(true);
    f.writeEnable.setOn(true);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), true);

    // Change data but disable WE — stored value must hold
    f.dataIn.setOn(false);
    f.writeEnable.setOn(false);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), true);
}

void TestRAMCell1Bit::testHoldMultipleCycles()
{
    RamCellFixture f;

    // Write 1
    f.dataIn.setOn(true);
    f.writeEnable.setOn(true);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), true);

    // 5 hold cycles with data=0 on the line
    f.writeEnable.setOn(false);
    f.dataIn.setOn(false);
    for (int i = 0; i < 5; ++i) {
        f.sim->update();
        clockCycle(f.sim, &f.clock);
        f.sim->update();
        QCOMPARE(TestUtils::getInputStatus(&f.dataOut), true);
    }
}

void TestRAMCell1Bit::testWriteZeroThenOneWithHold()
{
    RamCellFixture f;

    // Write 0
    f.dataIn.setOn(false);
    f.writeEnable.setOn(true);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), false);

    // Hold cycle
    f.writeEnable.setOn(false);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), false);

    // Write 1
    f.dataIn.setOn(true);
    f.writeEnable.setOn(true);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), true);

    // Hold cycle
    f.writeEnable.setOn(false);
    f.sim->update();
    clockCycle(f.sim, &f.clock);
    f.sim->update();
    QCOMPARE(TestUtils::getInputStatus(&f.dataOut), true);
}

