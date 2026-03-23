// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel8DecodeStage.h"

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
using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct DecodeStageFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> opcodeInputs;
    InputSwitch *clk = nullptr;
    InputSwitch *reset = nullptr;
    QVector<Led *> aluopLeds;
    Led *regwriteLed = nullptr;
    Led *memreadLed = nullptr;
    Led *memwriteLed = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level8_decode_stage.panda");
        builder.add(ic);

        for (int i = 0; i < 5; i++) {
            auto *sw = new InputSwitch(); builder.add(sw); opcodeInputs.append(sw);
        }
        clk = new InputSwitch(); builder.add(clk);
        reset = new InputSwitch(); builder.add(reset);

        for (int i = 0; i < 3; i++) {
            auto *led = new Led(); builder.add(led); aluopLeds.append(led);
        }
        regwriteLed = new Led(); builder.add(regwriteLed);
        memreadLed = new Led(); builder.add(memreadLed);
        memwriteLed = new Led(); builder.add(memwriteLed);

        for (int i = 0; i < 5; i++) {
            builder.connect(opcodeInputs[i], 0, ic, QString("OpCode[%1]").arg(i));
        }
        builder.connect(clk, 0, ic, "Clock");
        builder.connect(reset, 0, ic, "Reset");

        for (int i = 0; i < 3; i++) {
            builder.connect(ic, QString("ALUOp[%1]").arg(i), aluopLeds[i], 0);
        }
        builder.connect(ic, "RegWrite", regwriteLed, 0);
        builder.connect(ic, "MemRead", memreadLed, 0);
        builder.connect(ic, "MemWrite", memwriteLed, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readALUOp()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(aluopLeds.begin(), aluopLeds.end()), 0);
    }
};

static std::unique_ptr<DecodeStageFixture> s_level8DecodeStage;

void TestLevel8DecodeStage::initTestCase()
{
    s_level8DecodeStage = std::make_unique<DecodeStageFixture>();
    QVERIFY(s_level8DecodeStage->build());
}

void TestLevel8DecodeStage::cleanupTestCase()
{
    s_level8DecodeStage.reset();
}

void TestLevel8DecodeStage::cleanup()
{
    if (s_level8DecodeStage && s_level8DecodeStage->sim) {
        s_level8DecodeStage->sim->restart();
        s_level8DecodeStage->sim->update();
    }
}

void TestLevel8DecodeStage::testDecodeStage_data()
{
    QTest::addColumn<int>("opcode");
    QTest::addColumn<int>("expectedALUOp");
    QTest::addColumn<bool>("expectedRegWrite");
    QTest::addColumn<bool>("expectedMemRead");
    QTest::addColumn<bool>("expectedMemWrite");

    QTest::newRow("ADD (0x00)") << 0x00 << 0 << true << false << false;
    QTest::newRow("SUB (0x01)") << 0x01 << 1 << true << false << false;
    QTest::newRow("AND (0x02)") << 0x02 << 2 << true << false << false;
    QTest::newRow("OR (0x03)") << 0x03 << 3 << true << false << false;
    QTest::newRow("XOR (0x04)") << 0x04 << 4 << true << false << false;
    QTest::newRow("NOT (0x05)") << 0x05 << 5 << true << false << false;
    QTest::newRow("SHL (0x06)") << 0x06 << 6 << true << false << false;
    QTest::newRow("SHR (0x07)") << 0x07 << 7 << true << false << false;
    QTest::newRow("LOAD (0x10)") << 0x10 << 0 << false << true << false;
    QTest::newRow("STORE (0x18)") << 0x18 << 0 << false << false << true;
    QTest::newRow("LOAD-ALUOp5 (0x15)") << 0x15 << 5 << false << true << false;
    QTest::newRow("STORE-ALUOp7 (0x1F)") << 0x1F << 7 << false << false << true;
}

void TestLevel8DecodeStage::testDecodeStage()
{
    QFETCH(int, opcode);
    QFETCH(int, expectedALUOp);
    QFETCH(bool, expectedRegWrite);
    QFETCH(bool, expectedMemRead);
    QFETCH(bool, expectedMemWrite);

    auto &f = *s_level8DecodeStage;

    setMultiBitInput(f.opcodeInputs, opcode);
    f.sim->update();

    QCOMPARE(f.readALUOp(), expectedALUOp);
    QCOMPARE(getInputStatus(f.regwriteLed), expectedRegWrite);
    QCOMPARE(getInputStatus(f.memreadLed), expectedMemRead);
    QCOMPARE(getInputStatus(f.memwriteLed), expectedMemWrite);
}

void TestLevel8DecodeStage::testDecodeStageStructure()
{
    auto &f = *s_level8DecodeStage;

    QVERIFY(f.ic != nullptr);
    QCOMPARE(f.ic->inputSize(), 7);
    QCOMPARE(f.ic->outputSize(), 6);
}

