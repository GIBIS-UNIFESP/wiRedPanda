// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2PriorityMux3to1.h"

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel2PriorityMUX3To1::initTestCase()
{
}

void TestLevel2PriorityMUX3To1::cleanup()
{
}

void TestLevel2PriorityMUX3To1::testPriorityMux3to1()
{
    InputSwitch *data[3];
    InputSwitch *sel[2];
    Led *output;

    for (int i = 0; i < 3; i++) {
        data[i] = new InputSwitch();
    }
    for (int i = 0; i < 2; i++) {
        sel[i] = new InputSwitch();
    }
    output = new Led();

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create IC instance and load priority mux definition
    IC *mux = loadBuildingBlockIC("level2_priority_mux_3to1.panda");
    builder.add(mux);

    // Connect data inputs by label (position-independent)
    for (int i = 0; i < 3; i++) {
        builder.add(data[i]);
        builder.connect(data[i], 0, mux, QString("data%1").arg(i));
    }

    // Connect select inputs by label (position-independent)
    for (int i = 0; i < 2; i++) {
        builder.add(sel[i]);
        builder.connect(sel[i], 0, mux, QString("sel%1").arg(i));
    }

    // Connect IC output to external LED
    builder.add(output);
    builder.connect(mux, "out", output, 0);

    auto *simulation = builder.initSimulation();

    // Test truth table for 3-way priority multiplexer
    // Logic: output = sel0 ? data0 : (sel1 ? data1 : data2)

    struct TestCase {
        int sel0, sel1, data0, data1, data2, expected;
        const char *name;
    };

    TestCase tests[] = {
        // sel0=0, sel1=0 → output = data2
        {0, 0, 0, 0, 0, 0, "sel0=0, sel1=0, data2=0 → 0"},
        {0, 0, 1, 1, 1, 1, "sel0=0, sel1=0, data2=1 → 1"},
        {0, 0, 0, 0, 1, 1, "sel0=0, sel1=0, data2=1 (ignore data0, data1) → 1"},

        // sel0=0, sel1=1 → output = data1
        {0, 1, 0, 0, 0, 0, "sel0=0, sel1=1, data1=0 → 0"},
        {0, 1, 0, 1, 0, 1, "sel0=0, sel1=1, data1=1 → 1"},
        {0, 1, 1, 1, 0, 1, "sel0=0, sel1=1, data1=1 (ignore data2) → 1"},

        // sel0=1 → output = data0 (highest priority)
        {1, 0, 0, 0, 0, 0, "sel0=1, data0=0 (ignore sel1, data1, data2) → 0"},
        {1, 0, 1, 0, 0, 1, "sel0=1, data0=1 (ignore sel1, data1, data2) → 1"},
        {1, 1, 1, 0, 0, 1, "sel0=1, data0=1 (sel0 overrides sel1) → 1"},
    };

    for (int testIdx = 0; testIdx < 9; ++testIdx) {
        const auto &test = tests[testIdx];

        // Set inputs
        data[0]->setOn(test.data0);
        data[1]->setOn(test.data1);
        data[2]->setOn(test.data2);
        sel[0]->setOn(test.sel0);
        sel[1]->setOn(test.sel1);

        simulation->update();

        // Check output (LED input port receives the driven value)
        bool result = getInputStatus(output, 0);

        QCOMPARE(result, (bool)test.expected);
    }
}
