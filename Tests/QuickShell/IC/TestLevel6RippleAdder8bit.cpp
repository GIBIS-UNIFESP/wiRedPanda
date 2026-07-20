// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestLevel6RippleAdder8bit.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"
#include "Tests/QuickShell/QuickCpuTestUtils.h"

using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using CPUTestUtils::loadBuildingBlockIC;

struct RippleAdder8bitFixture {
    std::unique_ptr<QuickCircuitBuilder> builder;
    IC *ic = nullptr;
    InputSwitch *a[8] = {};
    InputSwitch *b[8] = {};
    InputSwitch *carryIn = nullptr;
    Led *sum[8] = {};
    Led *carryOut = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        builder = std::make_unique<QuickCircuitBuilder>();

        ic = static_cast<IC *>(builder->addOwnedElement(loadBuildingBlockIC("level6_ripple_adder_8bit.panda")));

        for (int i = 0; i < 8; i++) {
            a[i] = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
            a[i]->setLabel(QString("A[%1]").arg(i));

            b[i] = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
            b[i]->setLabel(QString("B[%1]").arg(i));
        }

        carryIn = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
        carryIn->setLabel("CarryIn");

        for (int i = 0; i < 8; i++) {
            sum[i] = static_cast<Led *>(builder->addOwnedElement(new Led()));
            sum[i]->setLabel(QString("Sum[%1]").arg(i));
        }

        carryOut = static_cast<Led *>(builder->addOwnedElement(new Led()));
        carryOut->setLabel("CarryOut");

        for (int i = 0; i < 8; i++) {
            builder->connect(a[i], 0, ic, QString("A[%1]").arg(i));
            builder->connect(b[i], 0, ic, QString("B[%1]").arg(i));
        }
        builder->connect(carryIn, 0, ic, "CarryIn");

        for (int i = 0; i < 8; i++) {
            builder->connect(ic, QString("Sum[%1]").arg(i), sum[i], 0);
        }
        builder->connect(ic, "CarryOut", carryOut, 0);

        sim = builder->initSimulation();
        sim->update();
        return true;
    }

    int readSum()
    {
        QVector<GraphicElement *> sum_vec;
        for (int i = 0; i < 8; i++) {
            sum_vec.append(sum[i]);
        }
        return readMultiBitOutput(sum_vec, 0);
    }

    int readCarryOut()
    {
        auto port = carryOut->inputPort(0);
        return (port && port->status() == Status::Active) ? 1 : 0;
    }
};

static std::unique_ptr<RippleAdder8bitFixture> s_level6RippleAdder8bit;

void TestLevel6RippleAdder8Bit::initTestCase()
{
    s_level6RippleAdder8bit = std::make_unique<RippleAdder8bitFixture>();
    QVERIFY(s_level6RippleAdder8bit->build());
}

void TestLevel6RippleAdder8Bit::cleanupTestCase()
{
    s_level6RippleAdder8bit.reset();
}

void TestLevel6RippleAdder8Bit::cleanup()
{
    if (s_level6RippleAdder8bit && s_level6RippleAdder8bit->sim) {
        s_level6RippleAdder8bit->sim->restart();
        s_level6RippleAdder8bit->sim->update();
    }
}

void TestLevel6RippleAdder8Bit::test8BitAdder() {
    auto &f = *s_level6RippleAdder8bit;

    f.sim->update();

    struct TestCase {
        int a_val;
        int b_val;
        int cin_val;
        int sum_expected;
        int cout_expected;
        const char* description;
    };

    TestCase cases[] = {
        {5, 3, 0, 8, 0, "5 + 3 = 8"},
        {0, 0, 0, 0, 0, "0 + 0 = 0"},
        {255, 1, 0, 0, 1, "255 + 1 = 0 (overflow)"},
        {128, 127, 0, 255, 0, "128 + 127 = 255"},
        {255, 255, 0, 254, 1, "255 + 255 = 254 (overflow)"},
        {100, 50, 0, 150, 0, "100 + 50 = 150"},
        {15, 1, 0, 16, 0, "15 + 1 = 16"},
        {200, 100, 0, 44, 1, "200 + 100 = 44 (overflow)"},
        {5, 3, 1, 9, 0, "5 + 3 + Cin=1 = 9"},
        {0, 0, 1, 1, 0, "0 + 0 + Cin=1 = 1 (carry-in only)"},
        {255, 0, 1, 0, 1, "255 + 0 + Cin=1 = 0 (tests all bits propagation)"},
        {127, 127, 1, 255, 0, "127 + 127 + Cin=1 = 255"},
        {255, 254, 1, 254, 1, "255 + 254 + Cin=1 = 254 (overflow)"},
        {128, 127, 1, 0, 1, "128 + 127 + Cin=1 = 0 (ripple chain)"},
    };

    for (size_t test_idx = 0; test_idx < sizeof(cases) / sizeof(cases[0]); test_idx++) {
        const auto &testCase = cases[test_idx];

        QVector<InputSwitch *> a_vec(f.a, f.a + 8);
        setMultiBitInput(a_vec, testCase.a_val);

        QVector<InputSwitch *> b_vec(f.b, f.b + 8);
        setMultiBitInput(b_vec, testCase.b_val);

        f.carryIn->setOn(testCase.cin_val != 0);

        f.sim->update();

        int sum_actual = f.readSum();
        int cout_actual = f.readCarryOut();

        QCOMPARE(sum_actual, testCase.sum_expected);
        QCOMPARE(cout_actual, testCase.cout_expected);
    }
}

void TestLevel6RippleAdder8Bit::test8BitAdderStructure() {
    auto &f = *s_level6RippleAdder8bit;

    QVERIFY(f.ic != nullptr);

    QCOMPARE(f.ic->inputSize(), 17);
    QCOMPARE(f.ic->outputSize(), 9);
}
