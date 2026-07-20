// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestLevel4ShiftRegisterPiso.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"
#include "Tests/QuickShell/QuickCpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::inputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct ShiftRegisterPisoFixture {
    std::unique_ptr<QuickCircuitBuilder> builder;
    IC *ic = nullptr;
    InputSwitch *clk = nullptr, *load = nullptr;
    InputSwitch *dataIn[4] = {};
    Led *serialOut = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        builder = std::make_unique<QuickCircuitBuilder>();

        clk = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
        load = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));

        ic = static_cast<IC *>(builder->addOwnedElement(loadBuildingBlockIC("level4_shift_register_piso.panda")));

        builder->connect(clk, 0, ic, "Clock");
        builder->connect(load, 0, ic, "Load");

        for (int i = 0; i < 4; ++i) {
            dataIn[i] = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
            builder->connect(dataIn[i], 0, ic, QString("D%1").arg(i));
        }

        serialOut = static_cast<Led *>(builder->addOwnedElement(new Led()));
        builder->connect(ic, "SOUT", serialOut, 0);

        sim = builder->initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<ShiftRegisterPisoFixture> s_level4ShiftRegisterPiso;

void TestLevel4ShiftRegisterPISO::initTestCase()
{
    s_level4ShiftRegisterPiso = std::make_unique<ShiftRegisterPisoFixture>();
    QVERIFY(s_level4ShiftRegisterPiso->build());
}

void TestLevel4ShiftRegisterPISO::cleanupTestCase()
{
    s_level4ShiftRegisterPiso.reset();
}

void TestLevel4ShiftRegisterPISO::cleanup()
{
    if (s_level4ShiftRegisterPiso && s_level4ShiftRegisterPiso->sim) {
        s_level4ShiftRegisterPiso->sim->restart();
        s_level4ShiftRegisterPiso->sim->update();
    }
}

void TestLevel4ShiftRegisterPISO::testShiftRegisterPISO_data()
{
    QTest::addColumn<int>("parallelLoadValue");
    QTest::addColumn<QVector<bool>>("expectedSerialOutput");

    // Load 0001, shift out: 1 then 0 then 0 then 0
    QTest::newRow("shift out 0001") << 0x1 << QVector<bool>({true, false, false, false});

    // Load 1010, shift out: 0 then 1 then 0 then 1
    QTest::newRow("shift out 1010") << 0xA << QVector<bool>({false, true, false, true});

    // Load 1111, shift out: 1 then 1 then 1 then 1
    QTest::newRow("shift out 1111") << 0xF << QVector<bool>({true, true, true, true});

    // Load 0101, shift out: 1 then 0 then 1 then 0
    QTest::newRow("shift out 0101") << 0x5 << QVector<bool>({true, false, true, false});

    // Load 1000, shift out: 0 then 0 then 0 then 1
    QTest::newRow("shift out 1000") << 0x8 << QVector<bool>({false, false, false, true});
}

void TestLevel4ShiftRegisterPISO::testShiftRegisterPISO()
{
    QFETCH(int, parallelLoadValue);
    QFETCH(QVector<bool>, expectedSerialOutput);

    auto &f = *s_level4ShiftRegisterPiso;

    f.clk->setOn(false);
    f.load->setOn(true);
    f.sim->update();

    for (int i = 0; i < 4; ++i) {
        f.dataIn[i]->setOn((parallelLoadValue >> i) & 1);
    }
    f.sim->update();

    clockCycle(f.sim, f.clk);

    f.load->setOn(false);
    f.sim->update();

    for (int bitPos = 0; bitPos < expectedSerialOutput.size(); ++bitPos) {
        bool currentBit = inputStatus(f.serialOut);
        QCOMPARE(currentBit, expectedSerialOutput[bitPos]);
        clockCycle(f.sim, f.clk);
    }
}
