// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7ExecutionDatapath.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using TestUtils::setMultiBitInput;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

struct ExecutionDatapathFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> operandAInputs;
    QVector<InputSwitch *> operandBInputs;
    QVector<InputSwitch *> opcodeInputs;
    QVector<Led *> resultLeds;
    Led *zeroLed = nullptr;
    Led *signLed = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level7_execution_datapath.panda");
        builder.add(ic);

        for (int i = 0; i < 8; i++) {
            auto *a = new InputSwitch(); builder.add(a); operandAInputs.append(a);
            auto *b = new InputSwitch(); builder.add(b); operandBInputs.append(b);
            auto *led = new Led(); builder.add(led); resultLeds.append(led);
        }
        for (int i = 0; i < 3; i++) {
            auto *sw = new InputSwitch(); builder.add(sw); opcodeInputs.append(sw);
        }

        zeroLed = new Led(); builder.add(zeroLed);
        signLed = new Led(); builder.add(signLed);

        for (int i = 0; i < 8; i++) {
            builder.connect(operandAInputs[i], 0, ic, QString("OperandA[%1]").arg(i));
            builder.connect(operandBInputs[i], 0, ic, QString("OperandB[%1]").arg(i));
            builder.connect(ic, QString("Result[%1]").arg(i), resultLeds[i], 0);
        }
        for (int i = 0; i < 3; i++) {
            builder.connect(opcodeInputs[i], 0, ic, QString("OpCode[%1]").arg(i));
        }
        builder.connect(ic, "Zero", zeroLed, 0);
        builder.connect(ic, "Sign", signLed, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readResult()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(resultLeds.begin(), resultLeds.end()), 0);
    }
};

static std::unique_ptr<ExecutionDatapathFixture> s_level7ExecDatapath;

void TestLevel7ExecutionDatapath::initTestCase()
{
    s_level7ExecDatapath = std::make_unique<ExecutionDatapathFixture>();
    QVERIFY(s_level7ExecDatapath->build());
}

void TestLevel7ExecutionDatapath::cleanupTestCase()
{
    s_level7ExecDatapath.reset();
}

void TestLevel7ExecutionDatapath::cleanup()
{
    if (s_level7ExecDatapath && s_level7ExecDatapath->sim) {
        s_level7ExecDatapath->sim->restart();
        s_level7ExecDatapath->sim->update();
    }
}

void TestLevel7ExecutionDatapath::testExecutionDatapath_data()
{
    QTest::addColumn<int>("operandA");
    QTest::addColumn<int>("operandB");
    QTest::addColumn<int>("opCode");
    QTest::addColumn<int>("expectedResult");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedSign");

    QTest::newRow("ADD: 0x50 + 0x30 = 0x80") << 0x50 << 0x30 << 0 << 0x80 << false << true;
    QTest::newRow("ADD: 0x10 + 0x20 = 0x30") << 0x10 << 0x20 << 0 << 0x30 << false << false;
    QTest::newRow("ADD: 0xFF + 0x01 = 0x00") << 0xFF << 0x01 << 0 << 0x00 << true << false;

    QTest::newRow("SUB: 0x50 - 0x30 = 0x20") << 0x50 << 0x30 << 1 << 0x20 << false << false;
    QTest::newRow("SUB: 0x30 - 0x50 = 0xE0") << 0x30 << 0x50 << 1 << 0xE0 << false << true;
    QTest::newRow("SUB: 0x50 - 0x50 = 0x00") << 0x50 << 0x50 << 1 << 0x00 << true << false;

    QTest::newRow("AND: 0xF0 & 0x0F = 0x00") << 0xF0 << 0x0F << 2 << 0x00 << true << false;
    QTest::newRow("AND: 0xF0 & 0xF0 = 0xF0") << 0xF0 << 0xF0 << 2 << 0xF0 << false << true;

    QTest::newRow("OR: 0xF0 | 0x0F = 0xFF") << 0xF0 << 0x0F << 3 << 0xFF << false << true;
    QTest::newRow("OR: 0x00 | 0x00 = 0x00") << 0x00 << 0x00 << 3 << 0x00 << true << false;

    QTest::newRow("XOR: 0xAA ^ 0x55 = 0xFF") << 0xAA << 0x55 << 4 << 0xFF << false << true;
    QTest::newRow("XOR: 0xAA ^ 0xAA = 0x00") << 0xAA << 0xAA << 4 << 0x00 << true << false;

    QTest::newRow("NOT: ~0xAA = 0x55") << 0xAA << 0x00 << 5 << 0x55 << false << false;
    QTest::newRow("NOT: ~0x7F = 0x80") << 0x7F << 0x00 << 5 << 0x80 << false << true;
    QTest::newRow("NOT: ~0xFF = 0x00") << 0xFF << 0x00 << 5 << 0x00 << true << false;

    QTest::newRow("SHL: 0x40 << 1 = 0x80") << 0x40 << 0x00 << 6 << 0x80 << false << true;
    QTest::newRow("SHL: 0x01 << 1 = 0x02") << 0x01 << 0x00 << 6 << 0x02 << false << false;

    QTest::newRow("SHR: 0x80 >> 1 = 0x40") << 0x80 << 0x00 << 7 << 0x40 << false << false;
    QTest::newRow("SHR: 0x01 >> 1 = 0x00") << 0x01 << 0x00 << 7 << 0x00 << true << false;
}

void TestLevel7ExecutionDatapath::testExecutionDatapath()
{
    QFETCH(int, operandA);
    QFETCH(int, operandB);
    QFETCH(int, opCode);
    QFETCH(int, expectedResult);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedSign);

    auto &f = *s_level7ExecDatapath;

    setMultiBitInput(f.operandAInputs, operandA);
    setMultiBitInput(f.operandBInputs, operandB);
    setMultiBitInput(f.opcodeInputs, opCode);
    f.sim->update();

    int result = f.readResult();
    bool zero = getInputStatus(f.zeroLed, 0);
    bool sign = getInputStatus(f.signLed, 0);

    QCOMPARE(result, expectedResult);
    QCOMPARE(zero, expectedZero);
    QCOMPARE(sign, expectedSign);
}

void TestLevel7ExecutionDatapath::testExecutionDatapathStructure()
{
    auto &f = *s_level7ExecDatapath;

    QVERIFY(f.ic != nullptr);

    QCOMPARE(f.ic->inputSize(), 19);
    QCOMPARE(f.ic->outputSize(), 10);
}

