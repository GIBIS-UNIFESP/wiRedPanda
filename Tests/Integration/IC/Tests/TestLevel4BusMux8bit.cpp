// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4BusMux8bit.h"

#include <QFile>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel4BusMUX8Bit::initTestCase()
{
}

void TestLevel4BusMUX8Bit::cleanup()
{
}

void TestLevel4BusMUX8Bit::testBusMux8Bit_data()
{
    QTest::addColumn<int>("in0Value");
    QTest::addColumn<int>("in1Value");
    QTest::addColumn<bool>("select");
    QTest::addColumn<int>("expectedOutput");

    // Test select = 0 (choose In0)
    QTest::newRow("sel0_in0=0x00") << 0x00 << 0xFF << false << 0x00;
    QTest::newRow("sel0_in0=0xFF") << 0xFF << 0x00 << false << 0xFF;
    QTest::newRow("sel0_in0=0xAA") << 0xAA << 0x55 << false << 0xAA;

    // Test select = 1 (choose In1)
    QTest::newRow("sel1_in1=0xFF") << 0x00 << 0xFF << true << 0xFF;
    QTest::newRow("sel1_in1=0x55") << 0xAA << 0x55 << true << 0x55;
    QTest::newRow("sel1_in1=0x3C") << 0xC3 << 0x3C << true << 0x3C;

    // Test 8-bit patterns
    QTest::newRow("pattern_10101010_sel0") << 0xAA << 0x55 << false << 0xAA;
    QTest::newRow("pattern_01010101_sel1") << 0xAA << 0x55 << true << 0x55;
}

void TestLevel4BusMUX8Bit::testBusMux8Bit()
{
    QFETCH(int, in0Value);
    QFETCH(int, in1Value);
    QFETCH(bool, select);
    QFETCH(int, expectedOutput);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Input switches for In0, In1, and Select
    InputSwitch in0[8];
    InputSwitch in1[8];
    InputSwitch sel;

    // Output LEDs
    Led out[8];

    // Load Bus Mux 8-bit IC
    IC *muxIC = loadBuildingBlockIC("level4_bus_mux_8bit.panda");

    // Add elements
    builder.add(&sel, muxIC);
    for (int i = 0; i < 8; ++i) {
        builder.add(&in0[i], &in1[i], &out[i]);
    }

    // Connect In0[0..7]
    for (int i = 0; i < 8; ++i) {
        builder.connect(&in0[i], 0, muxIC, QString("In0[%1]").arg(i));
    }

    // Connect In1[0..7]
    for (int i = 0; i < 8; ++i) {
        builder.connect(&in1[i], 0, muxIC, QString("In1[%1]").arg(i));
    }

    // Connect Select
    builder.connect(&sel, 0, muxIC, "Sel");

    // Connect Out[0..7]
    for (int i = 0; i < 8; ++i) {
        builder.connect(muxIC, QString("Out[%1]").arg(i), &out[i], 0);
    }

    auto *simulation = builder.initSimulation();

    // Set input values
    for (int i = 0; i < 8; ++i) {
        in0[i].setOn((in0Value >> i) & 1);
        in1[i].setOn((in1Value >> i) & 1);
    }
    sel.setOn(select);

    simulation->update();

    // Read output
    int result = readMultiBitOutput(QVector<GraphicElement *>({
        &out[0], &out[1], &out[2], &out[3],
        &out[4], &out[5], &out[6], &out[7]
    }));

    QCOMPARE(result, expectedOutput);
}

