// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestLevel3Bcd7segmentDecoder.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/QuickShell/IC/CpuTestUtils.h"
#include "Tests/QuickShell/IC/QuickTestUtils.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

using QuickTestUtils::inputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct Bcd7segFixture {
    std::unique_ptr<QuickCircuitBuilder> builder;
    IC *ic = nullptr;
    InputSwitch *bcdInputs[4] = {};
    Led *segmentOutputs[7] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        builder = std::make_unique<QuickCircuitBuilder>();

        for (int i = 0; i < 4; ++i) {
            bcdInputs[i] = new InputSwitch();
            builder->addOwnedElement(bcdInputs[i]);
        }
        for (int i = 0; i < 7; ++i) {
            segmentOutputs[i] = new Led();
            builder->addOwnedElement(segmentOutputs[i]);
        }

        ic = loadBuildingBlockIC("level3_bcd_7segment_decoder.panda");
        builder->addOwnedElement(ic);

        for (int i = 0; i < 4; ++i) {
            builder->connect(bcdInputs[i], 0, ic, QString("BCD[%1]").arg(i));
        }
        const char *segLabels[] = {"segment_a", "segment_b", "segment_c", "segment_d",
                                   "segment_e", "segment_f", "segment_g"};
        for (int i = 0; i < 7; ++i) {
            builder->connect(ic, segLabels[i], segmentOutputs[i], 0);
        }

        sim = builder->initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Bcd7segFixture> s_level3Bcd7seg;

void TestLevel3BCD7SegmentDecoder::initTestCase()
{
    s_level3Bcd7seg = std::make_unique<Bcd7segFixture>();
    QVERIFY(s_level3Bcd7seg->build());
}

void TestLevel3BCD7SegmentDecoder::cleanupTestCase()
{
    s_level3Bcd7seg.reset();
}

void TestLevel3BCD7SegmentDecoder::cleanup()
{
    if (s_level3Bcd7seg && s_level3Bcd7seg->sim) {
        s_level3Bcd7seg->sim->restart();
        s_level3Bcd7seg->sim->update();
    }
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
    // Segment layout: a=LSB at pos 0, g=MSB at pos 6
    //    aaa       (a is segment_a, position 0)
    //   f   b      (b is segment_b, position 1)
    //    ggg       (g is segment_g, position 6)
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

    // Invalid BCD inputs (10-15): this specific circuit (see
    // create_level3_bcd_7segment_decoder.py) has exactly 10 digit-detector AND gates (one per
    // digit 0-9), each requiring an exact 4-bit match on BCD[0-3]; every segment OR gate's
    // inputs are wired only from those 10 detectors. None of 1010..1111 matches any of the 10
    // listed digit patterns, so every detector -- and therefore every segment -- is
    // deterministically off (0x00) for all six invalid codes. Not implementation-defined: a
    // provable consequence of this circuit's own structure, not a guess.
    QTest::newRow("invalid_10") << 10 << 0x00;
    QTest::newRow("invalid_11") << 11 << 0x00;
    QTest::newRow("invalid_12") << 12 << 0x00;
    QTest::newRow("invalid_13") << 13 << 0x00;
    QTest::newRow("invalid_14") << 14 << 0x00;
    QTest::newRow("invalid_15") << 15 << 0x00;
}

void TestLevel3BCD7SegmentDecoder::testBCD7SegmentDecoder()
{
    QFETCH(int, bcdValue);
    QFETCH(int, expectedSegmentPattern);

    auto &f = *s_level3Bcd7seg;

    for (int i = 0; i < 4; ++i) {
        f.bcdInputs[i]->setOn((bcdValue >> i) & 1);
    }
    f.sim->update();

    int actualSegmentPattern = 0;
    for (int i = 0; i < 7; ++i) {
        if (inputStatus(f.segmentOutputs[i])) {
            actualSegmentPattern |= (1 << i);
        }
    }

    // Every row (valid digits 0-9 and invalid codes 10-15 alike) now has a real, exact
    // expected pattern -- see the invalid-code rows' comment above for why 0x00 is provably
    // correct for this circuit, not a placeholder.
    QVERIFY2(actualSegmentPattern == expectedSegmentPattern,
             qPrintable(QString("BCD %1: expected 0x%2, got 0x%3")
                 .arg(bcdValue)
                 .arg(expectedSegmentPattern, 2, 16, QChar('0'))
                 .arg(actualSegmentPattern, 2, 16, QChar('0'))));
}
