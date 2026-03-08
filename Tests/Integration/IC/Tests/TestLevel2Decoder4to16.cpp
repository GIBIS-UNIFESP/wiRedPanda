// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2Decoder4to16.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using TestUtils::getOutputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel2Decoder4To16::initTestCase()
{
    // Initialize test environment
}

void TestLevel2Decoder4To16::cleanup()
{
    // Clean up after each test
}

// ============================================================
// 4-to-16 Decoder Tests
// ============================================================

/**
 * Test: 4-to-16 Decoder (4-bit address -> 16 one-hot outputs)
 * Selects one of 16 output lines based on 4-bit address input
 */
void TestLevel2Decoder4To16::test4to16Decoder_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("expectedActive");

    // For address N, output N should be active (1), others inactive (0)
    for (int i = 0; i < 16; ++i) {
        QTest::newRow(QString("address %1").arg(i).toLatin1().data()) << i << i;
    }
}

void TestLevel2Decoder4To16::test4to16Decoder()
{
    QFETCH(int, address);
    QFETCH(int, expectedActive);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swAddr[4];
    Led ledOut[16];

    for (int i = 0; i < 4; ++i) {
        builder.add(&swAddr[i]);
    }
    for (int i = 0; i < 16; ++i) {
        builder.add(&ledOut[i]);
    }

    // Set address bits
    for (int i = 0; i < 4; ++i) {
        bool bit = (address >> i) & 1;
        swAddr[i].setOn(bit);
    }

    // Load Decoder4to16 IC
    IC *decoderIC = loadBuildingBlockIC("level2_decoder_4to16.panda");
    builder.add(decoderIC);

    // Connect address inputs to decoder IC using semantic labels
    builder.connect(&swAddr[0], 0, decoderIC, "addr[0]");
    builder.connect(&swAddr[1], 0, decoderIC, "addr[1]");
    builder.connect(&swAddr[2], 0, decoderIC, "addr[2]");
    builder.connect(&swAddr[3], 0, decoderIC, "addr[3]");

    // Connect decoder outputs using semantic labels (output LEDs)
    for (int i = 0; i < 16; ++i) {
        builder.connect(decoderIC, QString("out[%1]").arg(i), &ledOut[i], 0);
    }

    auto *simulation = builder.initSimulation();
    simulation->update();

    for (int i = 0; i < 16; ++i) {
        bool actual = getInputStatus(&ledOut[i]);
        bool expected = (i == expectedActive);
        QCOMPARE(actual, expected);
    }
}

// ============================================================
// Edge Cases
// ============================================================

void TestLevel2Decoder4To16::test4to16DecoderMaxAddress_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("expectedOutput");

    // Maximum address: 1111 (15) - should produce one-hot output
    QTest::newRow("max address 15") << 15 << 15;
    // Minimum address: 0000 (0) - should produce one-hot output
    QTest::newRow("min address 0") << 0 << 0;
}

void TestLevel2Decoder4To16::test4to16DecoderMaxAddress()
{
    QFETCH(int, address);
    QFETCH(int, expectedOutput);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swAddr[4];
    Led ledOut[16];

    for (int i = 0; i < 4; ++i) {
        builder.add(&swAddr[i]);
        swAddr[i].setOn((address >> i) & 1);
    }

    for (int i = 0; i < 16; ++i) {
        builder.add(&ledOut[i]);
    }

    // Load Decoder4to16 IC
    IC *decoderIC = loadBuildingBlockIC("level2_decoder_4to16.panda");
    builder.add(decoderIC);

    // Connect address inputs to decoder IC using semantic labels
    builder.connect(&swAddr[0], 0, decoderIC, "addr[0]");
    builder.connect(&swAddr[1], 0, decoderIC, "addr[1]");
    builder.connect(&swAddr[2], 0, decoderIC, "addr[2]");
    builder.connect(&swAddr[3], 0, decoderIC, "addr[3]");

    // Connect decoder outputs using semantic labels
    for (int i = 0; i < 16; ++i) {
        builder.connect(decoderIC, QString("out[%1]").arg(i), &ledOut[i], 0);
    }

    auto *simulation = builder.initSimulation();
    simulation->update();

    // Count active outputs
    int activeCount = 0;
    int activeIndex = -1;
    for (int i = 0; i < 16; ++i) {
        if (getInputStatus(&ledOut[i])) {
            activeCount++;
            activeIndex = i;
        }
    }

    // Verify only one output is high and it's the expected one
    QCOMPARE(activeCount, 1);
    QCOMPARE(activeIndex, expectedOutput);
}

