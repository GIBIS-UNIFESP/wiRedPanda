// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4ShiftRegisterPiso.h"

#include <QFile>
#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel4ShiftRegisterPISO::initTestCase()
{
}

void TestLevel4ShiftRegisterPISO::cleanup()
{
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

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Input controls
    InputSwitch clk;
    InputSwitch load;
    InputSwitch dataIn[4];

    // Serial output
    Led serialOut;

    // Load PISO shift register IC
    IC *pisoIC = loadBuildingBlockIC("level4_shift_register_piso.panda");

    // Add elements
    builder.add(&clk, &load, pisoIC);
    for (int i = 0; i < 4; ++i) {
        builder.add(&dataIn[i]);
    }
    builder.add(&serialOut);

    // Connect inputs
    builder.connect(&clk, 0, pisoIC, "CLK");
    builder.connect(&load, 0, pisoIC, "LOAD");
    for (int i = 0; i < 4; ++i) {
        builder.connect(&dataIn[i], 0, pisoIC, QString("D%1").arg(i));
    }

    // Connect serial output
    builder.connect(pisoIC, "SOUT", &serialOut, 0);

    auto *simulation = builder.initSimulation();

    clk.setOn(false);
    load.setOn(true);
    simulation->update();

    // Load parallel data
    for (int i = 0; i < 4; ++i) {
        dataIn[i].setOn((parallelLoadValue >> i) & 1);
    }
    simulation->update();

    // Complete clock pulse to load parallel data
    clockCycle(simulation, &clk);

    // Now shift out serially
    load.setOn(false);
    simulation->update();

    for (int bitPos = 0; bitPos < expectedSerialOutput.size(); ++bitPos) {
        bool currentBit = getInputStatus(&serialOut);

        // Verify this bit
        QCOMPARE(currentBit, expectedSerialOutput[bitPos]);

        // Complete clock pulse to shift next bit
        clockCycle(simulation, &clk);
    }
}
