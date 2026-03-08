// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel3AluSelector5way.h"

#include <QFile>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

// ============================================================
// Helper Functions
// ============================================================

void buildALUSelector5way(WorkSpace *workspace,
                          InputSwitch* in[5],
                          InputSwitch* sel[3],
                          Led* output,
                          Simulation*& sim)
{
    CircuitBuilder builder(workspace->scene());

    IC *selector = loadBuildingBlockIC("level3_alu_selector_5way.panda");
    builder.add(selector);

    // Port validation (5 data inputs + 3 select inputs = 8 inputs total)
    if (selector->inputSize() != 8) {
        throw std::runtime_error(QString("ALUSelector5way IC has incorrect input count: got %1, expected 8 (5 data + 3 select)")
                                .arg(selector->inputSize()).toStdString());
    }

    if (selector->outputSize() != 1) {
        throw std::runtime_error(QString("ALUSelector5way IC has incorrect output count: got %1, expected 1")
                                .arg(selector->outputSize()).toStdString());
    }

    // Connect 5 data inputs (ports 0-4)
    for (int i = 0; i < 5; i++) {
        builder.add(in[i]);
        QString portLabel = QString("result%1").arg(i);
        builder.connect(in[i], 0, selector, portLabel);
    }

    // Connect 3 select inputs (ports 5-7)
    for (int i = 0; i < 3; i++) {
        builder.add(sel[i]);
        QString portLabel = QString("op%1").arg(i);
        builder.connect(sel[i], 0, selector, portLabel);
    }

    // Connect output
    builder.add(output);
    builder.connect(selector, "out", output, 0);

    sim = builder.initSimulation();
}

// ============================================================
// Test Setup and Teardown
// ============================================================

void TestLevel3ALUSelector5Way::initTestCase()
{
    // IC file will be loaded by loadBuildingBlockIC() in test methods
}

void TestLevel3ALUSelector5Way::cleanup()
{
    // Clean up after each test
}

// ============================================================
// Test Implementation
// ============================================================

void TestLevel3ALUSelector5Way::testALUSelector5way()
{
    auto workspace = std::make_unique<WorkSpace>();
    InputSwitch *in[5];
    InputSwitch *sel[3];
    Led *output;

    for (int i = 0; i < 5; i++) {
        in[i] = new InputSwitch();
    }
    for (int i = 0; i < 3; i++) {
        sel[i] = new InputSwitch();
    }
    output = new Led();

    Simulation *sim = nullptr;
    buildALUSelector5way(workspace.get(), in, sel, output, sim);

    // Test all 8 possible select combinations
    // IC should output: in[sel[2]:sel[0]]
    // sel[2:0] = 000 → in[0]
    // sel[2:0] = 001 → in[1]
    // sel[2:0] = 010 → in[2]
    // sel[2:0] = 011 → in[3]
    // sel[2:0] = 100 → in[4]

    struct TestCase {
        int sel0, sel1, sel2;
        int input_values[5];  // in[0..4]
        int expected;
        const char *name;
    };

    TestCase tests[] = {
        // Test selecting each input
        {0, 0, 0, {1, 0, 0, 0, 0}, 1, "sel=000 → in[0]=1"},
        {1, 0, 0, {0, 1, 0, 0, 0}, 1, "sel=001 → in[1]=1"},
        {0, 1, 0, {0, 0, 1, 0, 0}, 1, "sel=010 → in[2]=1"},
        {1, 1, 0, {0, 0, 0, 1, 0}, 1, "sel=011 → in[3]=1"},
        {0, 0, 1, {0, 0, 0, 0, 1}, 1, "sel=100 → in[4]=1"},

        // Test with 0 values
        {0, 0, 0, {0, 1, 1, 1, 1}, 0, "sel=000 → in[0]=0"},
        {1, 0, 0, {1, 0, 1, 1, 1}, 0, "sel=001 → in[1]=0"},
        {0, 1, 0, {1, 1, 0, 1, 1}, 0, "sel=010 → in[2]=0"},
    };

    for (int testIdx = 0; testIdx < 8; ++testIdx) {
        const auto &test = tests[testIdx];

        // Set inputs
        for (int i = 0; i < 5; i++) {
            in[i]->setOn(test.input_values[i]);
        }
        sel[0]->setOn(test.sel0);
        sel[1]->setOn(test.sel1);
        sel[2]->setOn(test.sel2);

        sim->update();

        // Check output
        bool result = TestUtils::getInputStatus(output, 0);

        QCOMPARE(result, static_cast<bool>(test.expected));
    }
}

