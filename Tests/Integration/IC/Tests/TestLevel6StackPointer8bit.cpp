// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6StackPointer8bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel6StackPointer8Bit::initTestCase()
{
}

void TestLevel6StackPointer8Bit::cleanup()
{
}

void TestLevel6StackPointer8Bit::testStackPointer_data()
{
    QTest::addColumn<int>("loadValue");
    QTest::addColumn<int>("expectedValue");

    QTest::newRow("load_zero") << 0x00 << 0x00;
    QTest::newRow("load_0x55") << 0x55 << 0x55;
    QTest::newRow("load_0xAA") << 0xAA << 0xAA;
    QTest::newRow("load_0xFF") << 0xFF << 0xFF;
}

void TestLevel6StackPointer8Bit::testStackPointer()
{
    QFETCH(int, loadValue);
    QFETCH(int, expectedValue);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Control signals
    InputSwitch clk;
    InputSwitch reset;
    InputSwitch load;
    InputSwitch push;
    InputSwitch pop;
    InputSwitch enable;

    // Data inputs
    InputSwitch loadVal[8];

    // Output LEDs
    Led sp[8];

    // Load Stack Pointer IC
    IC *spIC = loadBuildingBlockIC("level6_stack_pointer_8bit.panda");

    // Setup circuit
    builder.add(&clk, &reset, &load, &push, &pop, &enable, spIC);
    for (int i = 0; i < 8; ++i) {
        builder.add(&loadVal[i], &sp[i]);
    }

    // Connect control signals
    builder.connect(&clk, 0, spIC, "Clock");
    builder.connect(&reset, 0, spIC, "Reset");
    builder.connect(&load, 0, spIC, "Load");
    builder.connect(&push, 0, spIC, "Push");
    builder.connect(&pop, 0, spIC, "Pop");
    builder.connect(&enable, 0, spIC, "Enable");

    // Connect load value inputs
    for (int i = 0; i < 8; ++i) {
        builder.connect(&loadVal[i], 0, spIC, QString("LoadValue[%1]").arg(i));
    }

    // Connect output
    for (int i = 0; i < 8; ++i) {
        builder.connect(spIC, QString("SP[%1]").arg(i), &sp[i], 0);
    }

    auto *simulation = builder.initSimulation();

    // INITIALIZATION PHASE: Set control signals to stable state
    // Reset needs time to propagate through IC
    clk.setOn(false);
    reset.setOn(false);  // Disable reset
    load.setOn(false);
    push.setOn(false);
    pop.setOn(false);
    enable.setOn(false);

    // Allow controls to propagate
    simulation->update();

    // LOAD PHASE (from loadable_counter reference pattern):
    // 1. Set clock LOW (already done)
    // 2. Set control signals to desired state (load=true to enable loading)
    reset.setOn(false);  // Disable reset (so load takes priority)
    load.setOn(true);    // Enable load BEFORE setting data
    enable.setOn(true);

    // 3. Set data bits AFTER control signal is enabled
    for (int i = 0; i < 8; ++i) {
        loadVal[i].setOn((loadValue >> i) & 1);
    }

    // 4. Single update() call
    simulation->update();

    // 5. Clock pulse to capture the value
    clockCycle(simulation, &clk);

    // 6. Read output immediately (no extra waiting)
    int readValue = 0;
    for (int i = 0; i < 8; ++i) {
        if (getInputStatus(&sp[i])) {
            readValue |= (1 << i);
        }
    }

    // Verify the loaded value matches expected
    QCOMPARE(readValue, expectedValue);
}
