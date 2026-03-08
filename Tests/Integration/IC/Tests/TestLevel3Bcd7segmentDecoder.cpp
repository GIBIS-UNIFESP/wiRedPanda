// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel3Bcd7segmentDecoder.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel3BCD7SegmentDecoder::initTestCase()
{
    // Initialize test environment
}

void TestLevel3BCD7SegmentDecoder::cleanup()
{
    // Clean up after each test
}

// ============================================================
// BCD to 7-Segment Decoder Tests
// ============================================================

void TestLevel3BCD7SegmentDecoder::testBCD7SegmentDecoder_data()
{
    QTest::addColumn<int>("bcdValue");
    QTest::addColumn<int>("expectedSegmentPattern");

    // Standard 7-segment patterns for BCD digits 0-9
    // Pattern is a bitmask where bit i represents segment i (a=0, b=1, ..., g=6)
    // Segment layout: a=MSB at pos 6, g=LSB at pos 0
    //    aaa       (a is segment_a, position 6)
    //   f   b      (b is segment_b, position 1)
    //    ggg       (g is segment_g, position 0)
    //   e   c
    //    ddd

    QTest::newRow("digit_0") << 0 << 0x3F;  // a,b,c,d,e,f (6 segments)
    QTest::newRow("digit_1") << 1 << 0x06;  // b,c (2 segments)
    QTest::newRow("digit_2") << 2 << 0x5B;  // a,b,d,e,g (5 segments)
    QTest::newRow("digit_3") << 3 << 0x4F;  // a,b,c,d,g (5 segments)
    QTest::newRow("digit_4") << 4 << 0x66;  // b,c,f,g (4 segments)
    QTest::newRow("digit_5") << 5 << 0x6D;  // a,c,d,f,g (5 segments)
    QTest::newRow("digit_6") << 6 << 0x7D;  // a,c,d,e,f,g (6 segments)
    QTest::newRow("digit_7") << 7 << 0x07;  // a,b,c (3 segments) - standard 7-segment pattern
    QTest::newRow("digit_8") << 8 << 0x7F;  // a,b,c,d,e,f,g (all 7 segments)
    QTest::newRow("digit_9") << 9 << 0x6F;  // a,b,c,d,f,g (6 segments)

    // Invalid BCD inputs (10-15) - behavior is implementation-defined
    // Some decoders blank all segments (0x00), some display partial patterns
    // We test that behavior is consistent (doesn't crash) but don't prescribe expected output
    QTest::newRow("invalid_10") << 10 << -1;  // -1 means any valid output acceptable
    QTest::newRow("invalid_11") << 11 << -1;
    QTest::newRow("invalid_12") << 12 << -1;
    QTest::newRow("invalid_13") << 13 << -1;
    QTest::newRow("invalid_14") << 14 << -1;
    QTest::newRow("invalid_15") << 15 << -1;
}

void TestLevel3BCD7SegmentDecoder::testBCD7SegmentDecoder()
{
    QFETCH(int, bcdValue);
    QFETCH(int, expectedSegmentPattern);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Create input switches for BCD[0-3]
    InputSwitch bcdInputs[4];
    for (int i = 0; i < 4; i++) {
        builder.add(&bcdInputs[i]);
        bcdInputs[i].setOn((bcdValue >> i) & 1);
    }

    // Create output LEDs for 7 segments (a-g)
    Led segmentOutputs[7];
    for (int i = 0; i < 7; i++) {
        builder.add(&segmentOutputs[i]);
    }

    // Load BCD7SegmentDecoder IC
    IC *decoderIC = loadBuildingBlockIC("level3_bcd_7segment_decoder.panda");
    builder.add(decoderIC);

    // Connect BCD inputs to decoder IC using semantic labels
    for (int i = 0; i < 4; i++) {
        builder.connect(&bcdInputs[i], 0, decoderIC, QString("BCD[%1]").arg(i));
    }

    // Connect segment outputs using semantic labels
    const char *segmentLabels[] = {"segment_a", "segment_b", "segment_c", "segment_d",
                                   "segment_e", "segment_f", "segment_g"};
    for (int i = 0; i < 7; i++) {
        builder.connect(decoderIC, segmentLabels[i], &segmentOutputs[i], 0);
    }

    auto *simulation = builder.initSimulation();
    simulation->update();

    // Read actual segment pattern from outputs
    int actualSegmentPattern = 0;
    for (int i = 0; i < 7; i++) {
        if (TestUtils::getInputStatus(&segmentOutputs[i])) {
            actualSegmentPattern |= (1 << i);
        }
    }

    // For valid BCD digits (0-9), verify exact 7-segment pattern matches expected encoding
    // For invalid BCD inputs (10-15), any output is acceptable (implementation-defined behavior)
    if (expectedSegmentPattern >= 0) {
        // Valid BCD digit - must match exact pattern
        QVERIFY2(actualSegmentPattern == expectedSegmentPattern,
                 qPrintable(QString("BCD %1: expected 0x%2, got 0x%3")
                     .arg(bcdValue)
                     .arg(expectedSegmentPattern, 2, 16, QChar('0'))
                     .arg(actualSegmentPattern, 2, 16, QChar('0'))));
    } else {
        // Invalid BCD input (10-15) - just verify it doesn't crash and produces some valid output
        // (0x00 to 0x7F are valid segment patterns)
        QVERIFY2(actualSegmentPattern >= 0 && actualSegmentPattern <= 0x7F,
                 qPrintable(QString("Invalid BCD output pattern: 0x%1").arg(actualSegmentPattern, 2, 16, QChar('0'))));
    }
}

