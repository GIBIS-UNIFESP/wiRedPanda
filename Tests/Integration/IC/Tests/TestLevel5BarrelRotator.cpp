// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5BarrelRotator.h"

#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel5BarrelRotator::initTestCase()
{
    // Initialize test environment
}

void TestLevel5BarrelRotator::cleanup()
{
    // Clean up after each test
}

// ============================================================
// Barrel Rotator Tests
// ============================================================

void TestLevel5BarrelRotator::testBarrelRotator_data()
{
    QTest::addColumn<int>("inputData");
    QTest::addColumn<int>("rotateAmount");
    QTest::addColumn<bool>("direction");
    QTest::addColumn<int>("expectedResult");

    // Left rotation (direction=0): rotate left by amount
    // Right rotation (direction=1): rotate right by amount

    // Test left rotations (direction=0)
    QTest::newRow("0001 rotate left 0 = 0001") << 1 << 0 << false << 1;
    QTest::newRow("0001 rotate left 1 = 0010") << 1 << 1 << false << 2;
    QTest::newRow("0001 rotate left 2 = 0100") << 1 << 2 << false << 4;
    QTest::newRow("0001 rotate left 3 = 1000") << 1 << 3 << false << 8;
    QTest::newRow("0011 rotate left 1 = 0110") << 3 << 1 << false << 6;
    QTest::newRow("1000 rotate left 1 = 0001") << 8 << 1 << false << 1;

    // Test right rotations (direction=1)
    QTest::newRow("1000 rotate right 0 = 1000") << 8 << 0 << true << 8;
    QTest::newRow("1000 rotate right 1 = 0100") << 8 << 1 << true << 4;
    QTest::newRow("1000 rotate right 2 = 0010") << 8 << 2 << true << 2;
    QTest::newRow("1000 rotate right 3 = 0001") << 8 << 3 << true << 1;
    QTest::newRow("0001 rotate right 1 = 1000") << 1 << 1 << true << 8;
    QTest::newRow("0110 rotate right 1 = 0011") << 6 << 1 << true << 3;
}

void TestLevel5BarrelRotator::testBarrelRotator()
{
    QFETCH(int, inputData);
    QFETCH(int, rotateAmount);
    QFETCH(bool, direction);
    QFETCH(int, expectedResult);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Create input switches for Data[0-3]
    InputSwitch dataInputs[4];
    for (int i = 0; i < 4; i++) {
        builder.add(&dataInputs[i]);
        dataInputs[i].setOn((inputData >> i) & 1);
    }

    // Create input switches for Rotate_Amount[0-1]
    InputSwitch rotateAmountInputs[2];
    for (int i = 0; i < 2; i++) {
        builder.add(&rotateAmountInputs[i]);
        rotateAmountInputs[i].setOn((rotateAmount >> i) & 1);
    }

    // Create input switch for Direction
    InputSwitch directionInput;
    builder.add(&directionInput);
    directionInput.setOn(direction);

    // Create output LEDs for Result[0-3]
    Led resultOutputs[4];
    for (int i = 0; i < 4; i++) {
        builder.add(&resultOutputs[i]);
    }

    // Load BarrelRotator IC
    IC *rotatorIC = loadBuildingBlockIC("level5_barrel_rotator.panda");
    builder.add(rotatorIC);

    // Connect data inputs to rotator IC using semantic labels
    for (int i = 0; i < 4; i++) {
        builder.connect(&dataInputs[i], 0, rotatorIC, QString("Data[%1]").arg(i));
    }

    // Connect rotate amount inputs
    for (int i = 0; i < 2; i++) {
        builder.connect(&rotateAmountInputs[i], 0, rotatorIC, QString("Rotate_Amount[%1]").arg(i));
    }

    // Connect direction input
    builder.connect(&directionInput, 0, rotatorIC, "Direction");

    // Connect result outputs
    for (int i = 0; i < 4; i++) {
        builder.connect(rotatorIC, QString("Result[%1]").arg(i), &resultOutputs[i], 0);
    }

    auto *simulation = builder.initSimulation();
    simulation->update();

    // Read result bits from circuit
    int actualResult = 0;
    for (int i = 0; i < 4; i++) {
        if (getInputStatus(&resultOutputs[i])) {
            actualResult |= (1 << i);
        }
    }

    // Compare circuit output with expected result (masked to 4 bits)
    int maskedResult = actualResult & 0x0F;
    int maskedExpected = expectedResult & 0x0F;

    QCOMPARE(maskedResult, maskedExpected);
}
