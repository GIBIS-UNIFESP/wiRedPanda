// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5BarrelShifter4bit.h"

#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct BarrelShifter4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *swData[4] = {};
    InputSwitch *swShift[2] = {};
    InputSwitch *swDir = nullptr;
    Led *ledOut[4] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level5_barrel_shifter_4bit.panda");
        builder.add(ic);

        for (int i = 0; i < 4; ++i) {
            swData[i] = new InputSwitch();
            ledOut[i] = new Led();
            builder.add(swData[i], ledOut[i]);
            builder.connect(swData[i], 0, ic, QString("Data[%1]").arg(i));
            builder.connect(ic, QString("ShiftedData[%1]").arg(i), ledOut[i], 0);
        }
        for (int i = 0; i < 2; ++i) {
            swShift[i] = new InputSwitch();
            builder.add(swShift[i]);
            builder.connect(swShift[i], 0, ic, QString("ShiftAmount[%1]").arg(i));
        }
        swDir = new InputSwitch();
        builder.add(swDir);
        builder.connect(swDir, 0, ic, "ShiftDirection");

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<BarrelShifter4bitFixture> s_level5BarrelShifter4bit;

void TestLevel5BarrelShifter4Bit::initTestCase()
{
    s_level5BarrelShifter4bit = std::make_unique<BarrelShifter4bitFixture>();
    QVERIFY(s_level5BarrelShifter4bit->build());
}

void TestLevel5BarrelShifter4Bit::cleanupTestCase()
{
    s_level5BarrelShifter4bit.reset();
}

void TestLevel5BarrelShifter4Bit::cleanup()
{
    if (s_level5BarrelShifter4bit && s_level5BarrelShifter4bit->sim) {
        s_level5BarrelShifter4bit->sim->restart();
        s_level5BarrelShifter4bit->sim->update();
    }
}

// Barrel Shifter: 4-bit arithmetic shifter (SHL/SHR)
// ============================================================================
// INPUT/OUTPUT BIT-WIDTH
// ============================================================================
// Inputs:  inputData[3:0] (data to shift), shiftAmount[1:0], shiftRight (direction)
// Output:  result[3:0] (shifted result)
// Total bit-width: 4 data bits + 2 control bits + 1 direction bit = 7 bits
// ============================================================================

void TestLevel5BarrelShifter4Bit::testBarrelShifter_data()
{
    QTest::addColumn<int>("inputData");
    QTest::addColumn<int>("shiftAmount");
    QTest::addColumn<bool>("shiftRight");  // false=left, true=right
    QTest::addColumn<int>("expectedResult");

    // Left shift operations (shiftRight = false)
    QTest::newRow("SHL: 0001 << 0 = 0001") << 1 << 0 << false << 1;
    QTest::newRow("SHL: 0001 << 1 = 0010") << 1 << 1 << false << 2;
    QTest::newRow("SHL: 0001 << 2 = 0100") << 1 << 2 << false << 4;
    QTest::newRow("SHL: 0001 << 3 = 1000") << 1 << 3 << false << 8;
    QTest::newRow("SHL: 0011 << 1 = 0110") << 3 << 1 << false << 6;
    QTest::newRow("SHL: 1111 << 1 = 1110") << 15 << 1 << false << 14;

    // Right shift operations (shiftRight = true)
    QTest::newRow("SHR: 1000 >> 0 = 1000") << 8 << 0 << true << 8;
    QTest::newRow("SHR: 1000 >> 1 = 0100") << 8 << 1 << true << 4;
    QTest::newRow("SHR: 1000 >> 2 = 0010") << 8 << 2 << true << 2;
    QTest::newRow("SHR: 1000 >> 3 = 0001") << 8 << 3 << true << 1;
    QTest::newRow("SHR: 1111 >> 1 = 0111") << 15 << 1 << true << 7;
    QTest::newRow("SHR: 1100 >> 2 = 0011") << 12 << 2 << true << 3;
}

void TestLevel5BarrelShifter4Bit::testBarrelShifter()
{
    QFETCH(int, inputData);
    QFETCH(int, shiftAmount);
    QFETCH(bool, shiftRight);
    QFETCH(int, expectedResult);

    auto &f = *s_level5BarrelShifter4bit;

    for (int i = 0; i < 4; ++i) {
        f.swData[i]->setOn((inputData >> i) & 1);
    }
    for (int i = 0; i < 2; ++i) {
        f.swShift[i]->setOn((shiftAmount >> i) & 1);
    }
    f.swDir->setOn(shiftRight);
    f.sim->update();

    int result = 0;
    for (int i = 0; i < 4; ++i) {
        if (getInputStatus(f.ledOut[i])) {
            result |= (1 << i);
        }
    }

    QCOMPARE(result & 0xF, expectedResult & 0xF);
}

