// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4Register4bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel4Register4Bit::initTestCase()
{
}

void TestLevel4Register4Bit::cleanup()
{
}

void TestLevel4Register4Bit::test4BitRegister_data()
{
    QTest::addColumn<int>("initialValue");
    QTest::addColumn<int>("dataToLoad");
    QTest::addColumn<bool>("enable");
    QTest::addColumn<int>("expectedAfterLoad");

    // Test loading different values
    QTest::newRow("load 0 with enable") << 0xF << 0x0 << true << 0x0;
    QTest::newRow("load 5 with enable") << 0x0 << 0x5 << true << 0x5;
    QTest::newRow("load F with enable") << 0x0 << 0xF << true << 0xF;
    QTest::newRow("load 3 with enable") << 0xA << 0x3 << true << 0x3;

    // Test hold when enable is false
    QTest::newRow("hold with enable=0") << 0x7 << 0x0 << false << 0x7;
    QTest::newRow("hold 5 with enable=0") << 0x5 << 0xA << false << 0x5;

    // Test various bit patterns
    QTest::newRow("load 1010") << 0x0 << 0xA << true << 0xA;
    QTest::newRow("load 0101") << 0xF << 0x5 << true << 0x5;
}

void TestLevel4Register4Bit::test4BitRegister()
{
    QFETCH(int, initialValue);
    QFETCH(int, dataToLoad);
    QFETCH(bool, enable);
    QFETCH(int, expectedAfterLoad);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Input controls
    InputSwitch clk;
    InputSwitch en;
    InputSwitch dataIn[4];

    // Output LEDs
    Led dataOut[4];

    // Load register IC
    IC *regIC = loadBuildingBlockIC("level4_register_4bit.panda");

    // Add elements
    builder.add(&clk, &en, regIC);
    for (int i = 0; i < 4; ++i) {
        builder.add(&dataIn[i], &dataOut[i]);
    }

    // Connect clock and enable
    builder.connect(&clk, 0, regIC, "CLK");
    builder.connect(&en, 0, regIC, "EN");

    // Connect data inputs and outputs
    for (int i = 0; i < 4; ++i) {
        builder.connect(&dataIn[i], 0, regIC, QString("D%1").arg(i));
        builder.connect(regIC, QString("Q%1").arg(i), &dataOut[i], 0);
    }

    auto *simulation = builder.initSimulation();

    // Set initial state - always enable initially to load the initial value
    clk.setOn(false);
    en.setOn(true);
    simulation->update();

    // Load initial value into register
    for (int i = 0; i < 4; ++i) {
        dataIn[i].setOn((initialValue >> i) & 1);
    }
    simulation->update();

    // Complete clock pulse to load initial value
    clockCycle(simulation, &clk);

    // Now set the enable value for the test
    en.setOn(enable);
    simulation->update();

    // Now set data to load
    for (int i = 0; i < 4; ++i) {
        dataIn[i].setOn((dataToLoad >> i) & 1);
    }
    simulation->update();

    // Complete clock pulse - enable signal controls data path
    clockCycle(simulation, &clk);

    // Read final output
    int result = readMultiBitOutput(QVector<GraphicElement *>({
        &dataOut[0], &dataOut[1], &dataOut[2], &dataOut[3]
    }));

    QCOMPARE(result, expectedAfterLoad);
}
