// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6RippleAdder8bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using CPUTestUtils::loadBuildingBlockIC;

/**
 * @brief Build test circuit for 8-bit Adder
 *
 * Creates a simulation with:
 * - 8 input switches for A[0..7]
 * - 8 input switches for B[0..7]
 * - 1 input switch for CarryIn
 * - 8-bit Adder IC
 * - 8 output LEDs for Sum[0..7]
 * - 1 output LED for CarryOut
 */
void build8BitAdderTest(WorkSpace* workspace,
                        InputSwitch* a[8],
                        InputSwitch* b[8],
                        InputSwitch* &carryIn,
                        Led* sum[8],
                        Led* &carryOut,
                        Simulation*& sim) {
    CircuitBuilder builder(workspace->scene());

    // Create 8-bit A input switches
    for (int i = 0; i < 8; i++) {
        a[i] = new InputSwitch();

        builder.add(a[i]);

        a[i]->setLabel(QString("A[%1]").arg(i));
        a[i]->setPos(50 + (i % 4) * 60, 100 + (i / 4) * 80);
    }

    // Create 8-bit B input switches
    for (int i = 0; i < 8; i++) {
        b[i] = new InputSwitch();

        builder.add(b[i]);

        b[i]->setLabel(QString("B[%1]").arg(i));
        b[i]->setPos(320 + (i % 4) * 60, 100 + (i / 4) * 80);
    }

    // Create CarryIn input
    carryIn = new InputSwitch();
    builder.add(carryIn);
    carryIn->setLabel("CarryIn");
    carryIn->setPos(590, 100);

    // Load and add 8-bit Adder IC
    IC *adder = loadBuildingBlockIC("level6_ripple_adder_8bit.panda");
    builder.add(adder);

    // Create 8-bit Sum output LEDs
    for (int i = 0; i < 8; i++) {
        sum[i] = new Led();
        builder.add(sum[i]);
        sum[i]->setLabel(QString("Sum[%1]").arg(i));
        sum[i]->setPos(200 + i * 100, 500);
    }

    // Create CarryOut output LED
    carryOut = new Led();
    builder.add(carryOut);
    carryOut->setLabel("CarryOut");
    carryOut->setPos(1000, 500);

    // Connect inputs to IC using semantic port labels
    for (int i = 0; i < 8; i++) {
        builder.connect(a[i], 0, adder, QString("A[%1]").arg(i));
        builder.connect(b[i], 0, adder, QString("B[%1]").arg(i));
    }
    builder.connect(carryIn, 0, adder, "CarryIn");

    // Connect IC outputs to LEDs using semantic port labels
    for (int i = 0; i < 8; i++) {
        builder.connect(adder, QString("Sum[%1]").arg(i), sum[i], 0);
    }
    builder.connect(adder, "CarryOut", carryOut, 0);

    sim = builder.initSimulation();
}

/**
 * Test: 8-bit Adder with various arithmetic operations
 *
 * Test cases:
 * 1. Simple addition: 5 + 3 = 8
 * 2. Zero addition: 0 + 0 = 0
 * 3. Overflow: 255 + 1 = 0 with CarryOut=1
 * 4. Large addition: 128 + 127 = 255
 * 5. Max values: 255 + 255 = 254 with CarryOut=1
 */
void TestLevel6RippleAdder8Bit::test8BitAdder() {
    auto workspace = std::make_unique<WorkSpace>();
    InputSwitch *a[8], *b[8], *carryIn;
    Led *sum[8], *carryOut;
    Simulation *sim = nullptr;

    build8BitAdderTest(workspace.get(), a, b, carryIn, sum, carryOut, sim);

    // Allow circuit to settle
    sim->update();

    struct TestCase {
        int a_val;
        int b_val;
        int cin_val;
        int sum_expected;
        int cout_expected;
        const char* description;
    };

    TestCase cases[] = {
        // Original test cases (Cin=0)
        {5, 3, 0, 8, 0, "5 + 3 = 8"},
        {0, 0, 0, 0, 0, "0 + 0 = 0"},
        {255, 1, 0, 0, 1, "255 + 1 = 0 (overflow)"},
        {128, 127, 0, 255, 0, "128 + 127 = 255"},
        {255, 255, 0, 254, 1, "255 + 255 = 254 (overflow)"},
        {100, 50, 0, 150, 0, "100 + 50 = 150"},
        {15, 1, 0, 16, 0, "15 + 1 = 16"},
        {200, 100, 0, 44, 1, "200 + 100 = 44 (overflow)"},

        // New carry-in test cases (Cin=1)
        {5, 3, 1, 9, 0, "5 + 3 + Cin=1 = 9"},
        {0, 0, 1, 1, 0, "0 + 0 + Cin=1 = 1 (carry-in only)"},
        {255, 0, 1, 0, 1, "255 + 0 + Cin=1 = 0 (tests all bits propagation)"},
        {127, 127, 1, 255, 0, "127 + 127 + Cin=1 = 255"},
        {255, 254, 1, 254, 1, "255 + 254 + Cin=1 = 254 (overflow)"},
        {128, 127, 1, 0, 1, "128 + 127 + Cin=1 = 0 (ripple chain)"},
    };

    // Test all 14 cases - comprehensive adder validation (8 original + 6 carry-in variants)
    for (size_t test_idx = 0; test_idx < sizeof(cases) / sizeof(cases[0]); test_idx++) {
        const auto &testCase = cases[test_idx];

        // Set 8-bit inputs A
        QVector<InputSwitch *> a_vec(a, a + 8);
        setMultiBitInput(a_vec, testCase.a_val);

        // Set 8-bit inputs B
        QVector<InputSwitch *> b_vec(b, b + 8);
        setMultiBitInput(b_vec, testCase.b_val);

        // Set carry in
        carryIn->setOn(testCase.cin_val != 0);

        // Allow circuit to settle
        sim->update();

        // Read 8-bit sum output
        QVector<GraphicElement *> sum_vec;
        for (int i = 0; i < 8; i++) {
            sum_vec.append(sum[i]);
        }
        int sum_actual = readMultiBitOutput(sum_vec, 0);

        // Read carry out
        auto carryOutPort = carryOut->inputPort(0);
        int cout_actual = (carryOutPort && carryOutPort->status() == Status::Active) ? 1 : 0;

        QCOMPARE(sum_actual, testCase.sum_expected);
        QCOMPARE(cout_actual, testCase.cout_expected);
    }
}

/**
 * Test: Verify 8-bit Adder IC structure and port count
 *
 * Validates:
 * - IC has 17 input ports (8 A + 8 B + 1 CarryIn)
 * - IC has 9 output ports (8 Sum + 1 CarryOut)
 * - IC can be loaded and instantiated
 */
void TestLevel6RippleAdder8Bit::test8BitAdderStructure() {
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *adder = nullptr;
    try {
        adder = loadBuildingBlockIC("level6_ripple_adder_8bit.panda");
    } catch (const std::runtime_error &e) {
        QFAIL(e.what());
        return;
    }

    builder.add(adder);

    // Verify IC has been loaded
    QVERIFY(adder != nullptr);

    // Create dummy connections to verify port access
    InputSwitch *a = new InputSwitch();
    builder.add(a);

    Led *sum = new Led();
    builder.add(sum);

    // Verify IC port structure
    QCOMPARE(adder->inputSize(), 17);   // 8 A inputs + 8 B inputs + 1 CarryIn
    QCOMPARE(adder->outputSize(), 9);   // 8 Sum outputs + 1 CarryOut

    // Verify port connections work (don't throw)
    try {
        builder.connect(a, 0, adder, "A[0]");             // Connect A[0] input
        builder.connect(adder, "Sum[0]", sum, 0);       // Connect Sum[0] output
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}
