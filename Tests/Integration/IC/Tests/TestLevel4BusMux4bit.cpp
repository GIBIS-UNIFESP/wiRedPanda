// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4BusMux4bit.h"

#include <QFile>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel4BusMUX4Bit::initTestCase()
{
}

void TestLevel4BusMUX4Bit::cleanup()
{
}

void TestLevel4BusMUX4Bit::testBusMux4Bit_data()
{
    QTest::addColumn<int>("in0Value");
    QTest::addColumn<int>("in1Value");
    QTest::addColumn<bool>("select");
    QTest::addColumn<int>("expectedOutput");

    // Test select = 0 (choose In0)
    QTest::newRow("sel0_in0=0xF") << 0xF << 0x0 << false << 0xF;
    QTest::newRow("sel0_in0=0x5") << 0x5 << 0xA << false << 0x5;
    QTest::newRow("sel0_in0=0x3") << 0x3 << 0xC << false << 0x3;

    // Test select = 1 (choose In1)
    QTest::newRow("sel1_in1=0xA") << 0xF << 0xA << true << 0xA;
    QTest::newRow("sel1_in1=0xC") << 0x0 << 0xC << true << 0xC;
    QTest::newRow("sel1_in1=0x5") << 0x3 << 0x5 << true << 0x5;

    // Test all values
    QTest::newRow("0000_select0") << 0x0 << 0xF << false << 0x0;
    QTest::newRow("1111_select0") << 0xF << 0x0 << false << 0xF;
    QTest::newRow("1010_select1") << 0x0 << 0xA << true << 0xA;
}

void TestLevel4BusMUX4Bit::testBusMux4Bit()
{
    QFETCH(int, in0Value);
    QFETCH(int, in1Value);
    QFETCH(bool, select);
    QFETCH(int, expectedOutput);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Input switches for In0, In1, and Select
    InputSwitch in0[4];
    InputSwitch in1[4];
    InputSwitch sel;

    // Output LEDs
    Led out[4];

    // Load Bus Mux 4-bit IC
    IC *muxIC = loadBuildingBlockIC("level4_bus_mux_4bit.panda");

    // Add elements
    builder.add(&sel, muxIC);
    for (int i = 0; i < 4; ++i) {
        builder.add(&in0[i], &in1[i], &out[i]);
    }

    // Connect In0[0..3]
    for (int i = 0; i < 4; ++i) {
        builder.connect(&in0[i], 0, muxIC, QString("In0[%1]").arg(i));
    }

    // Connect In1[0..3]
    for (int i = 0; i < 4; ++i) {
        builder.connect(&in1[i], 0, muxIC, QString("In1[%1]").arg(i));
    }

    // Connect Select
    builder.connect(&sel, 0, muxIC, "Sel");

    // Connect Out[0..3]
    for (int i = 0; i < 4; ++i) {
        builder.connect(muxIC, QString("Out[%1]").arg(i), &out[i], 0);
    }

    auto *simulation = builder.initSimulation();

    // Set input values
    for (int i = 0; i < 4; ++i) {
        in0[i].setOn((in0Value >> i) & 1);
        in1[i].setOn((in1Value >> i) & 1);
    }
    sel.setOn(select);

    simulation->update();

    // Read output
    int result = readMultiBitOutput(QVector<GraphicElement *>({&out[0], &out[1], &out[2], &out[3]}));

    QCOMPARE(result, expectedOutput);
}
