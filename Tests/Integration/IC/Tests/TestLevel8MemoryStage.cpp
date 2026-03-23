// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel8MemoryStage.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

struct MemoryStageFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> addressInputs;
    QVector<InputSwitch *> datainInputs;
    QVector<InputSwitch *> resultInputs;
    InputSwitch *memread = nullptr;
    InputSwitch *memwrite = nullptr;
    InputSwitch *clk = nullptr;
    InputSwitch *reset = nullptr;
    QVector<Led *> dataoutLeds;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level8_memory_stage.panda");
        builder.add(ic);

        for (int i = 0; i < 8; i++) {
            auto *a = new InputSwitch(); builder.add(a); addressInputs.append(a);
            auto *d = new InputSwitch(); builder.add(d); datainInputs.append(d);
            auto *r = new InputSwitch(); builder.add(r); resultInputs.append(r);
            auto *led = new Led(); builder.add(led); dataoutLeds.append(led);
        }

        memread = new InputSwitch(); builder.add(memread);
        memwrite = new InputSwitch(); builder.add(memwrite);
        clk = new InputSwitch(); builder.add(clk);
        reset = new InputSwitch(); builder.add(reset);

        for (int i = 0; i < 8; i++) {
            builder.connect(addressInputs[i], 0, ic, QString("Address[%1]").arg(i));
            builder.connect(datainInputs[i], 0, ic, QString("DataIn[%1]").arg(i));
            builder.connect(resultInputs[i], 0, ic, QString("Result[%1]").arg(i));
            builder.connect(ic, QString("DataOut[%1]").arg(i), dataoutLeds[i], 0);
        }

        builder.connect(memread, 0, ic, "MemRead");
        builder.connect(memwrite, 0, ic, "MemWrite");
        builder.connect(clk, 0, ic, "Clock");
        builder.connect(reset, 0, ic, "Reset");

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readDataOut()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(dataoutLeds.begin(), dataoutLeds.end()), 0);
    }
};

static std::unique_ptr<MemoryStageFixture> s_level8MemoryStage;

void TestLevel8MemoryStage::initTestCase()
{
    s_level8MemoryStage = std::make_unique<MemoryStageFixture>();
    QVERIFY(s_level8MemoryStage->build());
}

void TestLevel8MemoryStage::cleanupTestCase()
{
    s_level8MemoryStage.reset();
}

void TestLevel8MemoryStage::cleanup()
{
    if (s_level8MemoryStage && s_level8MemoryStage->sim) {
        s_level8MemoryStage->sim->restart();
        s_level8MemoryStage->sim->update();
    }
}

void TestLevel8MemoryStage::testMemoryStage_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("dataIn");
    QTest::addColumn<int>("result");
    QTest::addColumn<bool>("memRead");
    QTest::addColumn<bool>("memWrite");
    QTest::addColumn<int>("expectedOut");

    QTest::newRow("passthrough 0x84 (no mem op)") << 0x00 << 0x00 << 0x84 << false << false << 0x84;
    QTest::newRow("passthrough 0x00") << 0xFF << 0x00 << 0x00 << false << false << 0x00;
    QTest::newRow("passthrough 0xAA") << 0x55 << 0xBB << 0xAA << false << false << 0xAA;

    QTest::newRow("write 0x42 to addr 0x00") << 0x00 << 0x42 << 0xFF << false << true << 0xFF;
    QTest::newRow("write 0x55 to addr 0x10") << 0x10 << 0x55 << 0x77 << false << true << 0x77;

    QTest::newRow("read from addr 0x00 (empty)") << 0x00 << 0x00 << 0xFF << true << false << 0x00;
    QTest::newRow("read from addr 0x10 (empty)") << 0x10 << 0x00 << 0xFF << true << false << 0x00;
}

void TestLevel8MemoryStage::testMemoryStage()
{
    QFETCH(int, address);
    QFETCH(int, dataIn);
    QFETCH(int, result);
    QFETCH(bool, memRead);
    QFETCH(bool, memWrite);
    QFETCH(int, expectedOut);

    auto &f = *s_level8MemoryStage;

    setMultiBitInput(f.addressInputs, address);
    setMultiBitInput(f.datainInputs, dataIn);
    setMultiBitInput(f.resultInputs, result);
    f.memread->setOn(memRead);
    f.memwrite->setOn(memWrite);
    f.sim->update();

    if (memWrite) {
        clockCycle(f.sim, f.clk);
    }

    QCOMPARE(f.readDataOut(), expectedOut);
}

void TestLevel8MemoryStage::testMemoryStageStructure()
{
    auto &f = *s_level8MemoryStage;

    QVERIFY(f.ic != nullptr);
    QCOMPARE(f.ic->inputSize(), 28);
    QCOMPARE(f.ic->outputSize(), 8);
}

