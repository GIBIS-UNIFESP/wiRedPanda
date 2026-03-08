// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7FlagRegister.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::setMultiBitInput;
using TestUtils::readMultiBitOutput;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

struct FlagRegisterFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> flagInputs;
    InputSwitch *load = nullptr;
    InputSwitch *clk = nullptr;
    QVector<Led *> flagOutputs;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level7_flag_register.panda");
        builder.add(ic);

        for (int i = 0; i < 4; i++) {
            auto *sw = new InputSwitch(); builder.add(sw); flagInputs.append(sw);
        }

        load = new InputSwitch(); builder.add(load);
        clk = new InputSwitch(); builder.add(clk);

        for (int i = 0; i < 4; i++) {
            auto *led = new Led(); builder.add(led); flagOutputs.append(led);
        }

        const QStringList flag_names = {"Zero", "Sign", "Carry", "Overflow"};
        for (int i = 0; i < 4; i++) {
            builder.connect(flagInputs[i], 0, ic, QString("FlagIn[%1]_%2").arg(i).arg(flag_names[i]));
            builder.connect(ic, QString("FlagOut[%1]_%2").arg(i).arg(flag_names[i]), flagOutputs[i], 0);
        }

        builder.connect(load, 0, ic, "Load");
        builder.connect(clk, 0, ic, "Clock");

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readFlags()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(flagOutputs.begin(), flagOutputs.end()), 0);
    }
};

static std::unique_ptr<FlagRegisterFixture> s_level7FlagReg;

void TestLevel7FlagRegister::initTestCase()
{
    s_level7FlagReg = std::make_unique<FlagRegisterFixture>();
    QVERIFY(s_level7FlagReg->build());
}

void TestLevel7FlagRegister::cleanupTestCase()
{
    s_level7FlagReg.reset();
}

void TestLevel7FlagRegister::cleanup()
{
    if (s_level7FlagReg && s_level7FlagReg->sim) {
        s_level7FlagReg->sim->restart();
        s_level7FlagReg->sim->update();
    }
}

void TestLevel7FlagRegister::testFlagRegister_data()
{
    QTest::addColumn<int>("flagInput");
    QTest::addColumn<int>("expectedFlagOutput");

    QTest::newRow("load 0000 (no flags set)")  << 0x0 << 0x0;
    QTest::newRow("load 0001 (Zero flag)")     << 0x1 << 0x1;
    QTest::newRow("load 0010 (Sign flag)")     << 0x2 << 0x2;
    QTest::newRow("load 0100 (Carry flag)")    << 0x4 << 0x4;
    QTest::newRow("load 1000 (Overflow flag)") << 0x8 << 0x8;
    QTest::newRow("load 1111 (all flags)")     << 0xF << 0xF;
    QTest::newRow("load 0101 (Z and C)")       << 0x5 << 0x5;
    QTest::newRow("load 1010 (O and S)")       << 0xA << 0xA;
}

void TestLevel7FlagRegister::testFlagRegister()
{
    QFETCH(int, flagInput);
    QFETCH(int, expectedFlagOutput);

    auto &f = *s_level7FlagReg;

    // Set flag inputs and load
    setMultiBitInput(f.flagInputs, flagInput);
    f.load->setOn(true);
    f.sim->update();

    clockCycle(f.sim, f.clk);
    f.sim->update();

    int flagOutput = f.readFlags();
    QCOMPARE(flagOutput, expectedFlagOutput);

    // Test hold path: disable load and verify flags don't change
    f.load->setOn(false);
    f.sim->update();

    for (int i = 0; i < 3; i++) {
        setMultiBitInput(f.flagInputs, (flagInput ^ 0xF));
        f.sim->update();

        clockCycle(f.sim, f.clk);
        f.sim->update();

        int heldFlagOutput = f.readFlags();
        QVERIFY2(heldFlagOutput == expectedFlagOutput,
                 qPrintable(QString("Hold path failed at iteration %1: flags changed when load=0 (got 0x%2, expected 0x%3)")
                     .arg(i + 1)
                     .arg(heldFlagOutput, 2, 16, QChar('0'))
                     .arg(expectedFlagOutput, 2, 16, QChar('0'))));
    }
}

void TestLevel7FlagRegister::testFlagRegisterStructure()
{
    auto &f = *s_level7FlagReg;

    QVERIFY(f.ic != nullptr);

    QCOMPARE(f.ic->inputSize(), 6);
    QCOMPARE(f.ic->outputSize(), 4);
}

