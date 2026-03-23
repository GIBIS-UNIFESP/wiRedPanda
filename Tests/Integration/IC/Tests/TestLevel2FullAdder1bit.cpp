// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2FullAdder1bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;

// ============================================================
// Helper Functions
// ============================================================

// Using consolidated loadBuildingBlockIC from cpu_test_utils

/**
 * @brief Build test circuit for 1-bit Full Adder
 *
 * Creates a simulation with:
 * - 3 input switches (A, B, Cin)
 * - 1-bit Full Adder IC
 * - 2 output LEDs (Sum, Cout)
 *
 * Note: Workspace must be managed by caller to keep elements alive
 */
void buildFullAdderTestCircuit(WorkSpace *workspace,
                                InputSwitch* &a,
                                InputSwitch* &b,
                                InputSwitch* &carryIn,
                                Led* &sum,
                                Led* &carryOut,
                                Simulation* &sim)
{
    CircuitBuilder builder(workspace->scene());

    // Create input switches
    a = new InputSwitch();
    builder.add(a);
    a->setLabel("A");
    a->setPos(50, 100);

    b = new InputSwitch();
    builder.add(b);
    b->setLabel("B");
    b->setPos(50, 150);

    carryIn = new InputSwitch();
    builder.add(carryIn);
    carryIn->setLabel("CarryIn");
    carryIn->setPos(50, 200);

    // Load and add Full Adder IC
    IC *fullAdder = CPUTestUtils::loadBuildingBlockIC("level2_full_adder_1bit.panda");
    builder.add(fullAdder);

    // Create output LEDs
    sum = new Led();
    builder.add(sum);
    sum->setLabel("Sum");
    sum->setPos(400, 100);

    carryOut = new Led();
    builder.add(carryOut);
    carryOut->setLabel("CarryOut");
    carryOut->setPos(400, 200);

    // Connect inputs to IC using semantic port labels
    // FullAdder IC input ports: A, B, Cin
    builder.connect(a, 0, fullAdder, "A");
    builder.connect(b, 0, fullAdder, "B");
    builder.connect(carryIn, 0, fullAdder, "Cin");

    // Connect IC outputs to LEDs using semantic port labels
    // FullAdder IC output ports: Sum, Cout
    builder.connect(fullAdder, "Sum", sum, 0);
    builder.connect(fullAdder, "Cout", carryOut, 0);

    sim = builder.initSimulation();
}

// ============================================================
// Test Implementation
// ============================================================

void TestLevel2FullAdder1Bit::initTestCase()
{
    // Initialize test environment
}

void TestLevel2FullAdder1Bit::cleanup()
{
    // Clean up after each test
}

// ============================================================
// Test: Full Adder Truth Table (Parameterized)
// ============================================================

// Full Adder: A + B + Cin -> (Sum, Cout)
// Uses 2 Half Adders + 1 OR gate
// HA1: A + B -> (s1, c1)
// HA2: s1 + Cin -> (Sum, c2)
// Cout = c1 OR c2
void TestLevel2FullAdder1Bit::testFullAdder_data()
{
    QTest::addColumn<bool>("inputA");
    QTest::addColumn<bool>("inputB");
    QTest::addColumn<bool>("inputCin");
    QTest::addColumn<bool>("expectedSum");
    QTest::addColumn<bool>("expectedCout");

    QTest::newRow("0 + 0 + 0 = 0, c=0") << false << false << false << false << false;
    QTest::newRow("0 + 0 + 1 = 1, c=0") << false << false << true << true << false;
    QTest::newRow("0 + 1 + 0 = 1, c=0") << false << true << false << true << false;
    QTest::newRow("0 + 1 + 1 = 0, c=1") << false << true << true << false << true;
    QTest::newRow("1 + 0 + 0 = 1, c=0") << true << false << false << true << false;
    QTest::newRow("1 + 0 + 1 = 0, c=1") << true << false << true << false << true;
    QTest::newRow("1 + 1 + 0 = 0, c=1") << true << true << false << false << true;
    QTest::newRow("1 + 1 + 1 = 1, c=1") << true << true << true << true << true;
}

void TestLevel2FullAdder1Bit::testFullAdder()
{
    QFETCH(bool, inputA);
    QFETCH(bool, inputB);
    QFETCH(bool, inputCin);
    QFETCH(bool, expectedSum);
    QFETCH(bool, expectedCout);

    auto workspace = std::make_unique<WorkSpace>();
    InputSwitch *a, *b, *carryIn;
    Led *sum, *carryOut;
    Simulation *sim;

    buildFullAdderTestCircuit(workspace.get(), a, b, carryIn, sum, carryOut, sim);

    a->setOn(inputA);
    b->setOn(inputB);
    carryIn->setOn(inputCin);
    sim->update();

    QCOMPARE(getInputStatus(sum), expectedSum);
    QCOMPARE(getInputStatus(carryOut), expectedCout);
}

// ============================================================
// Test: CPU-Level Full Adder with All Truth Table Combinations
// ============================================================

// Truth table for Full Adder:
// A | B | Cin | Sum | Cout
// --+---+-----+-----+------
// 0 | 0 |  0  |  0  |  0
// 0 | 0 |  1  |  1  |  0
// 0 | 1 |  0  |  1  |  0
// 0 | 1 |  1  |  0  |  1
// 1 | 0 |  0  |  1  |  0
// 1 | 0 |  1  |  0  |  1
// 1 | 1 |  0  |  0  |  1
// 1 | 1 |  1  |  1  |  1
void TestLevel2FullAdder1Bit::testCpuFullAdder_data()
{
    QTest::addColumn<int>("inputA");
    QTest::addColumn<int>("inputB");
    QTest::addColumn<int>("inputCin");
    QTest::addColumn<int>("expectedSum");
    QTest::addColumn<int>("expectedCout");

    QTest::newRow("0+0+0=0, Cout=0") << 0 << 0 << 0 << 0 << 0;
    QTest::newRow("0+0+1=1, Cout=0") << 0 << 0 << 1 << 1 << 0;
    QTest::newRow("0+1+0=1, Cout=0") << 0 << 1 << 0 << 1 << 0;
    QTest::newRow("0+1+1=10, Cout=1") << 0 << 1 << 1 << 0 << 1;
    QTest::newRow("1+0+0=1, Cout=0") << 1 << 0 << 0 << 1 << 0;
    QTest::newRow("1+0+1=10, Cout=1") << 1 << 0 << 1 << 0 << 1;
    QTest::newRow("1+1+0=10, Cout=1") << 1 << 1 << 0 << 0 << 1;
    QTest::newRow("1+1+1=11, Cout=1") << 1 << 1 << 1 << 1 << 1;
}

void TestLevel2FullAdder1Bit::testCpuFullAdder()
{
    QFETCH(int, inputA);
    QFETCH(int, inputB);
    QFETCH(int, inputCin);
    QFETCH(int, expectedSum);
    QFETCH(int, expectedCout);

    auto workspace = std::make_unique<WorkSpace>();
    InputSwitch *a, *b, *carryIn;
    Led *sum, *carryOut;
    Simulation *sim;

    buildFullAdderTestCircuit(workspace.get(), a, b, carryIn, sum, carryOut, sim);

    // Allow circuit to settle
    sim->update();

    // Set inputs
    a->setOn(inputA != 0);
    b->setOn(inputB != 0);
    carryIn->setOn(inputCin != 0);

    // Allow circuit to settle
    sim->update();

    // Check outputs - read from LED input port
    auto sumPort = sum->inputPort(0);
    auto carryOutPort = carryOut->inputPort(0);

    int sum_actual = (sumPort && sumPort->status() == Status::Active) ? 1 : 0;
    int cout_actual = (carryOutPort && carryOutPort->status() == Status::Active) ? 1 : 0;

    QCOMPARE(sum_actual, expectedSum);
    QCOMPARE(cout_actual, expectedCout);
}

// ============================================================
// Test: IC Structure and Port Validation
// ============================================================

/**
 * Test: Verify 1-bit Full Adder IC structure and port count
 *
 * Validates:
 * - IC has 3 input ports (A, B, Cin)
 * - IC has 2 output ports (Sum, Cout)
 * - IC can be loaded and instantiated
 * - Port connections work correctly
 */
void TestLevel2FullAdder1Bit::testStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *fullAdder = CPUTestUtils::loadBuildingBlockIC("level2_full_adder_1bit.panda");
    builder.add(fullAdder);

    // Verify IC has been loaded
    QVERIFY(fullAdder != nullptr);

    // Create dummy connections to verify port access
    InputSwitch *a = new InputSwitch();
    builder.add(a);

    Led *sum = new Led();
    builder.add(sum);

    // Verify IC port structure
    QCOMPARE(fullAdder->inputSize(), 3);   // 3 input ports: A, B, Cin
    QCOMPARE(fullAdder->outputSize(), 2);  // 2 output ports: Sum, Cout

    // Verify port connections work (don't throw)
    try {
        builder.connect(a, 0, fullAdder, "A");           // Connect A input
        builder.connect(fullAdder, "Sum", sum, 0);     // Connect Sum output
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}

