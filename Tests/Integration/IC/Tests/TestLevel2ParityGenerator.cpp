// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2ParityGenerator.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::inputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct ParityGeneratorFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *dataInputs[4] = {};
    InputSwitch *cascadeIn = nullptr;
    Led *parityOutput = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 4; ++i) {
            dataInputs[i] = new InputSwitch();
            builder.add(dataInputs[i]);
        }
        cascadeIn = new InputSwitch();
        builder.add(cascadeIn);
        parityOutput = new Led();
        builder.add(parityOutput);

        ic = loadBuildingBlockIC("level2_parity_generator.panda");
        builder.add(ic);

        for (int i = 0; i < 4; ++i) {
            builder.connect(dataInputs[i], 0, ic, QString("Data[%1]").arg(i));
        }
        builder.connect(cascadeIn, 0, ic, "CascadeIn");
        builder.connect(ic, "Parity", parityOutput, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<ParityGeneratorFixture> s_level2ParityGenerator;

void TestLevel2ParityGenerator::initTestCase()
{
    s_level2ParityGenerator = std::make_unique<ParityGeneratorFixture>();
    QVERIFY(s_level2ParityGenerator->build());
}

void TestLevel2ParityGenerator::cleanupTestCase()
{
    s_level2ParityGenerator.reset();
}

void TestLevel2ParityGenerator::cleanup()
{
    if (s_level2ParityGenerator && s_level2ParityGenerator->sim) {
        s_level2ParityGenerator->sim->restart();
        s_level2ParityGenerator->sim->update();
    }
}

// ============================================================
// Parity Generator Tests
// ============================================================

void TestLevel2ParityGenerator::testParityGenerator_data()
{
    QTest::addColumn<int>("dataValue");
    QTest::addColumn<bool>("expectedParity");

    // Test all 16 possible 4-bit values
    // Parity = XOR of all bits (even parity: 1 if odd number of 1s, 0 if even)

    QTest::newRow("0000 (0 ones) → parity 0") << 0x0 << false;      // Even number of 1s
    QTest::newRow("0001 (1 ones) → parity 1") << 0x1 << true;       // Odd number of 1s
    QTest::newRow("0010 (1 ones) → parity 1") << 0x2 << true;       // Odd number of 1s
    QTest::newRow("0011 (2 ones) → parity 0") << 0x3 << false;      // Even number of 1s
    QTest::newRow("0100 (1 ones) → parity 1") << 0x4 << true;       // Odd number of 1s
    QTest::newRow("0101 (2 ones) → parity 0") << 0x5 << false;      // Even number of 1s
    QTest::newRow("0110 (2 ones) → parity 0") << 0x6 << false;      // Even number of 1s
    QTest::newRow("0111 (3 ones) → parity 1") << 0x7 << true;       // Odd number of 1s
    QTest::newRow("1000 (1 ones) → parity 1") << 0x8 << true;       // Odd number of 1s
    QTest::newRow("1001 (2 ones) → parity 0") << 0x9 << false;      // Even number of 1s
    QTest::newRow("1010 (2 ones) → parity 0") << 0xA << false;      // Even number of 1s
    QTest::newRow("1011 (3 ones) → parity 1") << 0xB << true;       // Odd number of 1s
    QTest::newRow("1100 (2 ones) → parity 0") << 0xC << false;      // Even number of 1s
    QTest::newRow("1101 (3 ones) → parity 1") << 0xD << true;       // Odd number of 1s
    QTest::newRow("1110 (3 ones) → parity 1") << 0xE << true;       // Odd number of 1s
    QTest::newRow("1111 (4 ones) → parity 0") << 0xF << false;      // Even number of 1s
}

void TestLevel2ParityGenerator::testParityGenerator()
{
    QFETCH(int, dataValue);
    QFETCH(bool, expectedParity);

    auto &f = *s_level2ParityGenerator;

    for (int i = 0; i < 4; ++i) {
        f.dataInputs[i]->setOn((dataValue >> i) & 1);
    }
    f.sim->update();

    QCOMPARE(inputStatus(f.parityOutput), expectedParity);
}

// 74180 cascade-in XORs a chained parity bit into this block's parity, so
// toggling CascadeIn flips the output for any fixed data.
void TestLevel2ParityGenerator::testCascadeIn()
{
    auto &f = *s_level2ParityGenerator;

    // Fixed data 0x1 → tree parity = 1 (odd number of ones).
    for (int i = 0; i < 4; ++i) {
        f.dataInputs[i]->setOn(i == 0);
    }

    f.cascadeIn->setOn(false);
    f.sim->update();
    QCOMPARE(inputStatus(f.parityOutput), true);    // 1 XOR 0 = 1

    f.cascadeIn->setOn(true);
    f.sim->update();
    QCOMPARE(inputStatus(f.parityOutput), false);   // 1 XOR 1 = 0
}
