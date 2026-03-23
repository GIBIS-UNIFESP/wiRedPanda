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

struct BarrelRotatorFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *dataInputs[4] = {};
    InputSwitch *rotateAmountInputs[2] = {};
    InputSwitch *directionInput = nullptr;
    Led *resultOutputs[4] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 4; ++i) {
            dataInputs[i] = new InputSwitch();
            resultOutputs[i] = new Led();
            builder.add(dataInputs[i], resultOutputs[i]);
        }
        for (int i = 0; i < 2; ++i) {
            rotateAmountInputs[i] = new InputSwitch();
            builder.add(rotateAmountInputs[i]);
        }
        directionInput = new InputSwitch();
        builder.add(directionInput);

        ic = loadBuildingBlockIC("level5_barrel_rotator.panda");
        builder.add(ic);

        for (int i = 0; i < 4; ++i) {
            builder.connect(dataInputs[i], 0, ic, QString("Data[%1]").arg(i));
            builder.connect(ic, QString("Result[%1]").arg(i), resultOutputs[i], 0);
        }
        for (int i = 0; i < 2; ++i) {
            builder.connect(rotateAmountInputs[i], 0, ic, QString("Rotate_Amount[%1]").arg(i));
        }
        builder.connect(directionInput, 0, ic, "Direction");

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<BarrelRotatorFixture> s_level5BarrelRotator;

void TestLevel5BarrelRotator::initTestCase()
{
    s_level5BarrelRotator = std::make_unique<BarrelRotatorFixture>();
    QVERIFY(s_level5BarrelRotator->build());
}

void TestLevel5BarrelRotator::cleanupTestCase()
{
    s_level5BarrelRotator.reset();
}

void TestLevel5BarrelRotator::cleanup()
{
    if (s_level5BarrelRotator && s_level5BarrelRotator->sim) {
        s_level5BarrelRotator->sim->restart();
        s_level5BarrelRotator->sim->update();
    }
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

    auto &f = *s_level5BarrelRotator;

    for (int i = 0; i < 4; ++i) {
        f.dataInputs[i]->setOn((inputData >> i) & 1);
    }
    for (int i = 0; i < 2; ++i) {
        f.rotateAmountInputs[i]->setOn((rotateAmount >> i) & 1);
    }
    f.directionInput->setOn(direction);
    f.sim->update();

    int actualResult = 0;
    for (int i = 0; i < 4; ++i) {
        if (getInputStatus(f.resultOutputs[i])) {
            actualResult |= (1 << i);
        }
    }

    QCOMPARE(actualResult & 0xF, expectedResult & 0xF);
}

