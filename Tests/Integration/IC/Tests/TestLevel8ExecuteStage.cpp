// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel8ExecuteStage.h"

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

struct ExecuteStageFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> operandAInputs;
    QVector<InputSwitch *> operandBInputs;
    QVector<InputSwitch *> aluopInputs;
    QVector<Led *> resultLeds;
    Led *zeroLed = nullptr;
    Led *signLed = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level8_execute_stage.panda");
        builder.add(ic);

        for (int i = 0; i < 8; i++) {
            auto *a = new InputSwitch(); builder.add(a); operandAInputs.append(a);
            auto *b = new InputSwitch(); builder.add(b); operandBInputs.append(b);
            auto *led = new Led(); builder.add(led); resultLeds.append(led);
        }
        for (int i = 0; i < 3; i++) {
            auto *sw = new InputSwitch(); builder.add(sw); aluopInputs.append(sw);
        }
        zeroLed = new Led(); builder.add(zeroLed);
        signLed = new Led(); builder.add(signLed);

        for (int i = 0; i < 8; i++) {
            builder.connect(operandAInputs[i], 0, ic, QString("OperandA[%1]").arg(i));
            builder.connect(operandBInputs[i], 0, ic, QString("OperandB[%1]").arg(i));
            builder.connect(ic, QString("Result[%1]").arg(i), resultLeds[i], 0);
        }
        for (int i = 0; i < 3; i++) {
            builder.connect(aluopInputs[i], 0, ic, QString("ALUOp[%1]").arg(i));
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

static std::unique_ptr<ExecuteStageFixture> s_level8ExecuteStage;

void TestLevel8ExecuteStage::initTestCase()
{
    s_level8ExecuteStage = std::make_unique<ExecuteStageFixture>();
    QVERIFY(s_level8ExecuteStage->build());
}

void TestLevel8ExecuteStage::cleanupTestCase()
{
    s_level8ExecuteStage.reset();
}

void TestLevel8ExecuteStage::cleanup()
{
    if (s_level8ExecuteStage && s_level8ExecuteStage->sim) {
        s_level8ExecuteStage->sim->restart();
        s_level8ExecuteStage->sim->update();
    }
}

void TestLevel8ExecuteStage::testExecuteStage_data()
{
    QTest::addColumn<int>("operandA");
    QTest::addColumn<int>("operandB");
    QTest::addColumn<int>("aluOp");
    QTest::addColumn<int>("expectedResult");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedSign");

    QTest::newRow("ADD 0x42 + 0x33 = 0x75") << 0x42 << 0x33 << 0 << 0x75 << false << false;
    QTest::newRow("ADD 0xFF + 0x01 = 0x00 (zero)") << 0xFF << 0x01 << 0 << 0x00 << true << false;
    QTest::newRow("ADD 0x80 + 0x80 = 0x00 (zero, sign)") << 0x80 << 0x80 << 0 << 0x00 << true << false;

    QTest::newRow("SUB 0x50 - 0x30 = 0x20") << 0x50 << 0x30 << 1 << 0x20 << false << false;
    QTest::newRow("SUB 0x42 - 0x42 = 0x00 (zero)") << 0x42 << 0x42 << 1 << 0x00 << true << false;
    QTest::newRow("SUB 0x30 - 0x50 = 0xE0 (sign)") << 0x30 << 0x50 << 1 << 0xE0 << false << true;

    QTest::newRow("AND 0xF0 & 0x0F = 0x00") << 0xF0 << 0x0F << 2 << 0x00 << true << false;
    QTest::newRow("AND 0xFF & 0x42 = 0x42") << 0xFF << 0x42 << 2 << 0x42 << false << false;
    QTest::newRow("AND 0xAA & 0x55 = 0x00") << 0xAA << 0x55 << 2 << 0x00 << true << false;

    QTest::newRow("OR 0xF0 | 0x0F = 0xFF") << 0xF0 << 0x0F << 3 << 0xFF << false << true;
    QTest::newRow("OR 0x42 | 0x00 = 0x42") << 0x42 << 0x00 << 3 << 0x42 << false << false;
    QTest::newRow("OR 0x00 | 0x00 = 0x00 (zero)") << 0x00 << 0x00 << 3 << 0x00 << true << false;

    QTest::newRow("XOR 0xFF ^ 0xFF = 0x00") << 0xFF << 0xFF << 4 << 0x00 << true << false;
    QTest::newRow("XOR 0xFF ^ 0x00 = 0xFF") << 0xFF << 0x00 << 4 << 0xFF << false << true;
    QTest::newRow("XOR 0xAA ^ 0x55 = 0xFF") << 0xAA << 0x55 << 4 << 0xFF << false << true;

    QTest::newRow("NOT ~0x00 = 0xFF") << 0x00 << 0x00 << 5 << 0xFF << false << true;
    QTest::newRow("NOT ~0xFF = 0x00") << 0xFF << 0x00 << 5 << 0x00 << true << false;
    QTest::newRow("NOT ~0x42 = 0xBD") << 0x42 << 0x00 << 5 << 0xBD << false << true;

    QTest::newRow("SHL 0x42 << 1 = 0x84") << 0x42 << 0x00 << 6 << 0x84 << false << true;
    QTest::newRow("SHL 0x01 << 1 = 0x02") << 0x01 << 0x00 << 6 << 0x02 << false << false;
    QTest::newRow("SHL 0x80 << 1 = 0x00 (overflow)") << 0x80 << 0x00 << 6 << 0x00 << true << false;

    QTest::newRow("SHR 0x84 >> 1 = 0x42") << 0x84 << 0x00 << 7 << 0x42 << false << false;
    QTest::newRow("SHR 0xFF >> 1 = 0x7F") << 0xFF << 0x00 << 7 << 0x7F << false << false;
    QTest::newRow("SHR 0x01 >> 1 = 0x00 (zero)") << 0x01 << 0x00 << 7 << 0x00 << true << false;
}

void TestLevel8ExecuteStage::testExecuteStage()
{
    QFETCH(int, operandA);
    QFETCH(int, operandB);
    QFETCH(int, aluOp);
    QFETCH(int, expectedResult);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedSign);

    auto &f = *s_level8ExecuteStage;

    setMultiBitInput(f.operandAInputs, operandA);
    setMultiBitInput(f.operandBInputs, operandB);
    setMultiBitInput(f.aluopInputs, aluOp);
    f.sim->update();

    QCOMPARE(f.readResult(), expectedResult);
    QCOMPARE(getInputStatus(f.zeroLed), expectedZero);
    QCOMPARE(getInputStatus(f.signLed), expectedSign);
}

void TestLevel8ExecuteStage::testInputPortIsolation_data()
{
    QTest::addColumn<int>("bitPosition");
    for (int i = 0; i < 8; ++i) {
        QTest::newRow(QString("input_bit_%1").arg(i).toLatin1()) << i;
    }
}

void TestLevel8ExecuteStage::testInputPortIsolation()
{
    QFETCH(int, bitPosition);

    auto &f = *s_level8ExecuteStage;

    // AND (ALUOp 010): a one-hot OperandA gated against OperandB = 0xFF must
    // yield a one-hot result at the same position (no bit-lane cross-wiring).
    setMultiBitInput(f.operandBInputs, 0xFF);
    for (int i = 0; i < 8; ++i) {
        f.operandAInputs[i]->setOn(i == bitPosition);
    }
    setMultiBitInput(f.aluopInputs, 2);

    f.sim->update();

    QCOMPARE(f.readResult(), 1 << bitPosition);
}

void TestLevel8ExecuteStage::testOutputPortIsolation_data()
{
    QTest::addColumn<int>("bitPosition");
    for (int i = 0; i < 8; ++i) {
        QTest::newRow(QString("output_bit_%1").arg(i).toLatin1()) << i;
    }
}

void TestLevel8ExecuteStage::testOutputPortIsolation()
{
    QFETCH(int, bitPosition);

    auto &f = *s_level8ExecuteStage;

    // OR (ALUOp 011) with OperandB = 0x00: a one-hot OperandA drives exactly one
    // result bit; every other result bit must stay low.
    setMultiBitInput(f.operandBInputs, 0x00);
    for (int i = 0; i < 8; ++i) {
        f.operandAInputs[i]->setOn(i == bitPosition);
    }
    setMultiBitInput(f.aluopInputs, 3);

    f.sim->update();

    for (int i = 0; i < 8; ++i) {
        QCOMPARE(getInputStatus(f.resultLeds[i]), i == bitPosition);
    }
}

void TestLevel8ExecuteStage::testExecuteStageStructure()
{
    auto &f = *s_level8ExecuteStage;

    QVERIFY(f.ic != nullptr);
    // OperandA[8] + OperandB[8] + ALUOp[3] (F33: the unused Clock element
    // and dead Reset switch were removed — the stage is purely combinational)
    QCOMPARE(f.ic->inputSize(), 19);
    QCOMPARE(f.ic->outputSize(), 10);
}
