// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Logic/TestElementLogicErrors.h"

#include <QtTest>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "Tests/Common/TestUtils.h"

using TestUtils::initSrc;
using TestUtils::initElm;

// ============================================================
// Robustness Tests — Error Conditions and Boundary Behavior
// ============================================================

void TestElementLogicErrors::testInvalidInputPortIndex()
{
    And gate;
    gate.setInputSize(4);
    InputVcc in0, in1, in2, in3;
    initElm(gate); initSrc(in0); initSrc(in1); initSrc(in2); initSrc(in3);

    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);
    gate.connectPredecessor(2, &in2, 0);
    gate.connectPredecessor(3, &in3, 0);

    in0.setOutputValue(true);
    in1.setOutputValue(true);
    in2.setOutputValue(true);
    in3.setOutputValue(true);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), Status::Active);

    in3.setOutputValue(false);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), Status::Inactive);
}

void TestElementLogicErrors::testNullPredecessor()
{
    And andGate;
    InputVcc input;
    initElm(andGate); initSrc(input);
    andGate.connectPredecessor(0, &input, 0);

    input.setOutputValue(true);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), Status::Unknown);

    input.setOutputValue(false);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), Status::Unknown);

    Or orGate;
    initElm(orGate);
    orGate.connectPredecessor(0, &input, 0);

    input.setOutputValue(true);
    orGate.updateLogic();
    QCOMPARE(orGate.outputValue(), Status::Unknown);

    input.setOutputValue(false);
    orGate.updateLogic();
    QCOMPARE(orGate.outputValue(), Status::Unknown);
}

void TestElementLogicErrors::testDisconnectedInput()
{
    And andGate;
    initElm(andGate);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), Status::Unknown);

    Or orGate;
    initElm(orGate);
    orGate.updateLogic();
    QCOMPARE(orGate.outputValue(), Status::Unknown);

    Not notGate;
    initElm(notGate);
    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), Status::Unknown);
}

void TestElementLogicErrors::testMultipleConnectionsSamePort()
{
    And gate;
    InputVcc first, second, other;
    initElm(gate); initSrc(first); initSrc(second); initSrc(other);

    gate.connectPredecessor(0, &first, 0);
    gate.connectPredecessor(1, &other, 0);
    gate.connectPredecessor(0, &second, 0);  // overwrites port 0

    first.setOutputValue(true);
    second.setOutputValue(false);
    other.setOutputValue(true);

    gate.updateLogic();
    QCOMPARE(gate.outputValue(), Status::Inactive);

    second.setOutputValue(true);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), Status::Active);

    first.setOutputValue(false);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), Status::Active);
}

void TestElementLogicErrors::testRapidStateChanges()
{
    And andGate;
    InputVcc input1, input2;
    initElm(andGate); initSrc(input1); initSrc(input2);

    andGate.connectPredecessor(0, &input1, 0);
    andGate.connectPredecessor(1, &input2, 0);

    for (int i = 0; i < 1000; ++i) {
        input1.setOutputValue(i % 2 == 0);
        input2.setOutputValue(i % 3 == 0);
        andGate.updateLogic();
        bool expected = (i % 2 == 0) && (i % 3 == 0);
        QCOMPARE(andGate.outputValue(), expected ? Status::Active : Status::Inactive);
    }
}

void TestElementLogicErrors::testUnconnectedGate()
{
    And gate;
    initElm(gate);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), Status::Unknown);

    Not notGate;
    initElm(notGate);
    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), Status::Unknown);
}

void TestElementLogicErrors::testDeepCascading()
{
    InputVcc in1, in2, in3, in4;
    And stage1, stage2, stage3;
    initSrc(in1); initSrc(in2); initSrc(in3); initSrc(in4);
    initElm(stage1); initElm(stage2); initElm(stage3);

    stage1.connectPredecessor(0, &in1, 0);
    stage1.connectPredecessor(1, &in2, 0);
    stage2.connectPredecessor(0, &stage1, 0);
    stage2.connectPredecessor(1, &in3, 0);
    stage3.connectPredecessor(0, &stage2, 0);
    stage3.connectPredecessor(1, &in4, 0);

    in1.setOutputValue(true);
    in2.setOutputValue(true);
    in3.setOutputValue(true);
    in4.setOutputValue(true);
    stage1.updateLogic();
    stage2.updateLogic();
    stage3.updateLogic();
    QCOMPARE(stage3.outputValue(), Status::Active);

    in1.setOutputValue(false);
    stage1.updateLogic();
    stage2.updateLogic();
    stage3.updateLogic();
    QCOMPARE(stage1.outputValue(), Status::Inactive);
    QCOMPARE(stage2.outputValue(), Status::Inactive);
    QCOMPARE(stage3.outputValue(), Status::Inactive);

    in1.setOutputValue(true);
    in3.setOutputValue(false);
    stage1.updateLogic();
    stage2.updateLogic();
    stage3.updateLogic();
    QCOMPARE(stage1.outputValue(), Status::Active);
    QCOMPARE(stage2.outputValue(), Status::Inactive);
    QCOMPARE(stage3.outputValue(), Status::Inactive);
}

void TestElementLogicErrors::testInvalidOutputPortIndex()
{
    And andGate;
    Or orGate;
    orGate.setInputSize(3);
    Not notGate;
    initElm(andGate); initElm(orGate); initElm(notGate);

    QCOMPARE(andGate.simOutputSize(), 1);
    QCOMPARE(orGate.simOutputSize(), 1);
    QCOMPARE(notGate.simOutputSize(), 1);

    QCOMPARE(andGate.outputValue(0), Status::Inactive);
    QCOMPARE(orGate.outputValue(0), Status::Inactive);
    QCOMPARE(notGate.outputValue(0), Status::Inactive);

    // Multi-output source
    InputVcc input;
    input.initSimulationVectors(0, 4);
    input.setOutputValue(0, Status::Active);
    QCOMPARE(input.simOutputSize(), 4);
    QCOMPARE(input.outputValue(0), Status::Active);
    QCOMPARE(input.outputValue(3), Status::Inactive);

    input.setOutputValue(1, true);
    input.setOutputValue(3, true);
    QCOMPARE(input.outputValue(0), Status::Active);
    QCOMPARE(input.outputValue(1), Status::Active);
    QCOMPARE(input.outputValue(2), Status::Inactive);
    QCOMPARE(input.outputValue(3), Status::Active);
}

void TestElementLogicErrors::testSelfLoopOscillation()
{
    Not notGate;
    initElm(notGate);
    notGate.connectPredecessor(0, &notGate, 0);

    QCOMPARE(notGate.outputValue(), Status::Inactive);

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), Status::Active);

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), Status::Inactive);

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), Status::Active);
}

void TestElementLogicErrors::testInputValueBoundary()
{
    And gate;
    gate.setInputSize(4);
    InputVcc in0, in1, in2, in3;
    initElm(gate); initSrc(in0); initSrc(in1); initSrc(in2); initSrc(in3);

    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);
    gate.connectPredecessor(2, &in2, 0);
    gate.connectPredecessor(3, &in3, 0);

    for (int combo = 0; combo < 16; ++combo) {
        in0.setOutputValue((combo & 0x1) != 0);
        in1.setOutputValue((combo & 0x2) != 0);
        in2.setOutputValue((combo & 0x4) != 0);
        in3.setOutputValue((combo & 0x8) != 0);
        gate.updateLogic();
        QCOMPARE(gate.outputValue(), (combo == 15) ? Status::Active : Status::Inactive);
    }
}

void TestElementLogicErrors::testGateWithZeroInputs()
{
    And andGate;
    andGate.setInputSize(0);
    andGate.initSimulationVectors(0, 1);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), Status::Active);

    Or orGate;
    orGate.setInputSize(0);
    orGate.initSimulationVectors(0, 1);
    orGate.updateLogic();
    QCOMPARE(orGate.outputValue(), Status::Inactive);
}

void TestElementLogicErrors::testInvalidPropagatesChain()
{
    And andGate;
    Not notGate;
    InputVcc input;
    initElm(andGate); initElm(notGate); initSrc(input);

    andGate.connectPredecessor(0, &input, 0);
    // port 1 is left null → AND will produce Unknown
    notGate.connectPredecessor(0, &andGate, 0);

    input.setOutputValue(true);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), Status::Unknown);

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), Status::Unknown);

    input.setOutputValue(false);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), Status::Unknown);

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), Status::Unknown);
}

void TestElementLogicErrors::testBoolOverloadMapsToStatus()
{
    InputVcc src;
    src.initSimulationVectors(0, 2);

    src.setOutputValue(true);
    QCOMPARE(src.outputValue(0), Status::Active);

    src.setOutputValue(false);
    QCOMPARE(src.outputValue(0), Status::Inactive);

    src.setOutputValue(1, true);
    QCOMPARE(src.outputValue(1), Status::Active);

    src.setOutputValue(1, false);
    QCOMPARE(src.outputValue(1), Status::Inactive);

    QVERIFY(src.outputValue(0) != Status::Unknown);
    QVERIFY(src.outputValue(1) != Status::Unknown);
}

