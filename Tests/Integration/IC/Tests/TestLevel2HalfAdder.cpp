// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2HalfAdder.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel2HalfAdder::initTestCase()
{
    // Initialize test environment
}

void TestLevel2HalfAdder::cleanup()
{
    // Clean up after each test
}

// ============================================================
// Half Adder IC Tests
// ============================================================

// Half Adder: A + B -> (Sum, Carry)
// Sum = A XOR B
// Carry = A AND B
void TestLevel2HalfAdder::testHalfAdder_data()
{
    QTest::addColumn<bool>("inputA");
    QTest::addColumn<bool>("inputB");
    QTest::addColumn<bool>("expectedSum");
    QTest::addColumn<bool>("expectedCarry");

    QTest::newRow("0 + 0 = 0, c=0") << false << false << false << false;
    QTest::newRow("0 + 1 = 1, c=0") << false << true << true << false;
    QTest::newRow("1 + 0 = 1, c=0") << true << false << true << false;
    QTest::newRow("1 + 1 = 0, c=1") << true << true << false << true;
}

void TestLevel2HalfAdder::testHalfAdder()
{
    QFETCH(bool, inputA);
    QFETCH(bool, inputB);
    QFETCH(bool, expectedSum);
    QFETCH(bool, expectedCarry);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swA, swB;
    Led ledSum, ledCarry;

    builder.add(&swA, &swB, &ledSum, &ledCarry);

    // Load Half Adder IC
    IC *halfAdderIC = loadBuildingBlockIC("level2_half_adder.panda");
    builder.add(halfAdderIC);

    // Connect inputs to IC using semantic port labels
    // HalfAdder IC input ports: A, B
    builder.connect(&swA, 0, halfAdderIC, "A");
    builder.connect(&swB, 0, halfAdderIC, "B");

    // Connect IC outputs to LEDs using semantic port labels
    // HalfAdder IC output ports: Sum, Carry
    builder.connect(halfAdderIC, "Sum", &ledSum, 0);
    builder.connect(halfAdderIC, "Carry", &ledCarry, 0);

    auto *simulation = builder.initSimulation();

    swA.setOn(inputA);
    swB.setOn(inputB);
    simulation->update();

    QCOMPARE(getInputStatus(&ledSum), expectedSum);
    QCOMPARE(getInputStatus(&ledCarry), expectedCarry);
}

