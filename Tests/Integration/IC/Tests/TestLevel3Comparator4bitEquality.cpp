// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel3Comparator4bitEquality.h"

#include <QFile>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/Cpu/CpuCommon.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct Comparator4bitEqualityFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *inputA[4] = {}, *inputB[4] = {};
    Led *ledEqual = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level3_comparator_4bit_equality.panda");
        builder.add(ic);

        ledEqual = new Led();
        builder.add(ledEqual);

        for (int i = 0; i < 4; ++i) {
            inputA[i] = new InputSwitch();
            inputB[i] = new InputSwitch();
            builder.add(inputA[i], inputB[i]);
            builder.connect(inputA[i], 0, ic, QString("A[%1]").arg(i));
            builder.connect(inputB[i], 0, ic, QString("B[%1]").arg(i));
        }
        builder.connect(ic, "Equal", ledEqual, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Comparator4bitEqualityFixture> s_level3Comparator4bitEquality;

void TestLevel3Comparator4BitEquality::initTestCase()
{
    s_level3Comparator4bitEquality = std::make_unique<Comparator4bitEqualityFixture>();
    QVERIFY(s_level3Comparator4bitEquality->build());
}

void TestLevel3Comparator4BitEquality::cleanupTestCase()
{
    s_level3Comparator4bitEquality.reset();
}

void TestLevel3Comparator4BitEquality::cleanup()
{
    if (s_level3Comparator4bitEquality && s_level3Comparator4bitEquality->sim) {
        s_level3Comparator4bitEquality->sim->restart();
        s_level3Comparator4bitEquality->sim->update();
    }
}

void TestLevel3Comparator4BitEquality::testComparator4BitEquality_data()
{
    QTest::addColumn<int>("valueA");
    QTest::addColumn<int>("valueB");
    QTest::addColumn<bool>("expectedEqual");

    // Equal cases
    QTest::newRow("eq_0000_0000") << 0 << 0 << true;
    QTest::newRow("eq_1111_1111") << 15 << 15 << true;
    QTest::newRow("eq_0101_0101") << 5 << 5 << true;
    QTest::newRow("eq_1010_1010") << 10 << 10 << true;

    // Not-equal cases
    QTest::newRow("neq_0000_0001") << 0 << 1 << false;
    QTest::newRow("neq_1000_0000") << 8 << 0 << false;
    QTest::newRow("neq_0101_1010") << 5 << 10 << false;
    QTest::newRow("neq_1111_0000") << 15 << 0 << false;
    QTest::newRow("neq_0011_0101") << 3 << 5 << false;
    QTest::newRow("neq_0111_0001") << 7 << 1 << false;
    QTest::newRow("neq_1111_1110") << 15 << 14 << false;
}

void TestLevel3Comparator4BitEquality::testComparator4BitEquality()
{
    QFETCH(int, valueA);
    QFETCH(int, valueB);
    QFETCH(bool, expectedEqual);

    auto &f = *s_level3Comparator4bitEquality;

    for (int i = 0; i < 4; ++i) {
        f.inputA[i]->setOn((valueA >> i) & 1);
        f.inputB[i]->setOn((valueB >> i) & 1);
    }
    f.sim->update();

    QCOMPARE(getInputStatus(f.ledEqual), expectedEqual);
}

