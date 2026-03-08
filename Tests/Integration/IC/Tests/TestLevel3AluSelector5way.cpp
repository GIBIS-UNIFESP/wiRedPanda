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

struct AluSelector5wayFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *in[5] = {};
    InputSwitch *sel[3] = {};
    Led *output = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 5; ++i) {
            in[i] = new InputSwitch();
            builder.add(in[i]);
        }
        for (int i = 0; i < 3; ++i) {
            sel[i] = new InputSwitch();
            builder.add(sel[i]);
        }
        output = new Led();
        builder.add(output);

        ic = loadBuildingBlockIC("level3_alu_selector_5way.panda");
        builder.add(ic);

        for (int i = 0; i < 5; ++i) {
            builder.connect(in[i], 0, ic, QString("result%1").arg(i));
        }
        for (int i = 0; i < 3; ++i) {
            builder.connect(sel[i], 0, ic, QString("op%1").arg(i));
        }
        builder.connect(ic, "out", output, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<AluSelector5wayFixture> s_level3AluSelector5way;

void TestLevel3ALUSelector5Way::initTestCase()
{
    s_level3AluSelector5way = std::make_unique<AluSelector5wayFixture>();
    QVERIFY(s_level3AluSelector5way->build());
}

void TestLevel3ALUSelector5Way::cleanupTestCase()
{
    s_level3AluSelector5way.reset();
}

void TestLevel3ALUSelector5Way::cleanup()
{
    if (s_level3AluSelector5way && s_level3AluSelector5way->sim) {
        s_level3AluSelector5way->sim->restart();
        s_level3AluSelector5way->sim->update();
    }
}

// ============================================================
// Test Implementation
// ============================================================

void TestLevel3ALUSelector5Way::testALUSelector5way()
{
    auto &f = *s_level3AluSelector5way;

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

        for (int i = 0; i < 5; i++) {
            f.in[i]->setOn(test.input_values[i]);
        }
        f.sel[0]->setOn(test.sel0);
        f.sel[1]->setOn(test.sel1);
        f.sel[2]->setOn(test.sel2);

        f.sim->update();

        bool result = getInputStatus(f.output, 0);

        QCOMPARE(result, static_cast<bool>(test.expected));
    }
}

