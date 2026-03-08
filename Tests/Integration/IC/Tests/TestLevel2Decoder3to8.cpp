// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2Decoder3to8.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel2Decoder3To8::initTestCase()
{
    // Initialize test environment
}

void TestLevel2Decoder3To8::cleanup()
{
    // Clean up after each test
}

// ============================================================
// 3-to-8 Decoder Tests
// ============================================================

void TestLevel2Decoder3To8::test3to8Decoder_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("expectedActive");

    // For address N, output N should be active (1), others inactive (0)
    QTest::newRow("address 0") << 0 << 0;
    QTest::newRow("address 1") << 1 << 1;
    QTest::newRow("address 2") << 2 << 2;
    QTest::newRow("address 3") << 3 << 3;
    QTest::newRow("address 4") << 4 << 4;
    QTest::newRow("address 5") << 5 << 5;
    QTest::newRow("address 6") << 6 << 6;
    QTest::newRow("address 7") << 7 << 7;
}

void TestLevel2Decoder3To8::test3to8Decoder()
{
    QFETCH(int, address);
    QFETCH(int, expectedActive);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swAddr[3];
    Led ledOut[8];

    for (int i = 0; i < 3; ++i) {
        builder.add(&swAddr[i]);
    }
    for (int i = 0; i < 8; ++i) {
        builder.add(&ledOut[i]);
    }

    // Load Decoder3to8 IC
    IC *decoderIC = loadBuildingBlockIC("level2_decoder_3to8.panda");
    builder.add(decoderIC);

    // Set address bits
    for (int i = 0; i < 3; ++i) {
        swAddr[i].setOn((address >> i) & 1);
    }

    // Connect address inputs to decoder IC using semantic labels
    builder.connect(&swAddr[0], 0, decoderIC, "addr[0]");
    builder.connect(&swAddr[1], 0, decoderIC, "addr[1]");
    builder.connect(&swAddr[2], 0, decoderIC, "addr[2]");

    // Connect decoder outputs using semantic labels
    for (int i = 0; i < 8; ++i) {
        builder.connect(decoderIC, QString("out[%1]").arg(i), &ledOut[i], 0);
    }

    auto *simulation = builder.initSimulation();
    simulation->update();

    // Verify only the expected output is active
    for (int i = 0; i < 8; ++i) {
        bool expected = (i == expectedActive);
        QCOMPARE(getInputStatus(&ledOut[i]), expected);
    }
}
