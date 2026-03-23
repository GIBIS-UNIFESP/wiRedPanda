// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Logic/TestElementLogicErrors.h"

#include <QtTest>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"

static void initErrSrc(GraphicElement &elm) { elm.initSimulationVectors(0, 1); }
static void initErrElm(GraphicElement &elm) { elm.initSimulationVectors(elm.inputSize(), elm.outputSize()); }

// ============================================================
// Robustness Tests — Error Conditions and Boundary Behavior
// ============================================================

void TestElementLogicErrors::testInvalidInputPortIndex()
{
    And gate;
    gate.setInputSize(4);
    InputVcc in0, in1, in2, in3;
    initErrElm(gate); initErrSrc(in0); initErrSrc(in1); initErrSrc(in2); initErrSrc(in3);

    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);
    gate.connectPredecessor(2, &in2, 0);
    gate.connectPredecessor(3, &in3, 0);

    in0.setOutputValue(true);
    in1.setOutputValue(true);
    in2.setOutputValue(true);
    in3.setOutputValue(true);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), true);

    in3.setOutputValue(false);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), false);
}

void TestElementLogicErrors::testNullPredecessor()
{
    // In the bool-based simulation, null predecessors default to false.
    And andGate;
    InputVcc input;
    initErrElm(andGate); initErrSrc(input);
    andGate.connectPredecessor(0, &input, 0);
    // port 1 left null -> defaults to false

    input.setOutputValue(true);
    andGate.updateLogic();
    // AND(true, false) = false
    QCOMPARE(andGate.outputValue(), false);

    input.setOutputValue(false);
    andGate.updateLogic();
    // AND(false, false) = false
    QCOMPARE(andGate.outputValue(), false);

    Or orGate;
    initErrElm(orGate);
    orGate.connectPredecessor(0, &input, 0);
    // port 1 left null -> defaults to false

    input.setOutputValue(true);
    orGate.updateLogic();
    // OR(true, false) = true
    QCOMPARE(orGate.outputValue(), true);

    input.setOutputValue(false);
    orGate.updateLogic();
    // OR(false, false) = false
    QCOMPARE(orGate.outputValue(), false);
}

void TestElementLogicErrors::testDisconnectedInput()
{
    // All inputs null -> default to false.
    And andGate;
    initErrElm(andGate);
    andGate.updateLogic();
    // AND(false, false) = false
    QCOMPARE(andGate.outputValue(), false);

    Or orGate;
    initErrElm(orGate);
    orGate.updateLogic();
    // OR(false, false) = false
    QCOMPARE(orGate.outputValue(), false);

    Not notGate;
    initErrElm(notGate);
    notGate.updateLogic();
    // NOT(false) = true
    QCOMPARE(notGate.outputValue(), true);
}

void TestElementLogicErrors::testMultipleConnectionsSamePort()
{
    And gate;
    InputVcc first, second, other;
    initErrElm(gate); initErrSrc(first); initErrSrc(second); initErrSrc(other);

    gate.connectPredecessor(0, &first, 0);
    gate.connectPredecessor(1, &other, 0);
    gate.connectPredecessor(0, &second, 0);  // overwrites port 0

    first.setOutputValue(true);
    second.setOutputValue(false);
    other.setOutputValue(true);

    gate.updateLogic();
    QCOMPARE(gate.outputValue(), false);

    second.setOutputValue(true);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), true);

    first.setOutputValue(false);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), true);
}

void TestElementLogicErrors::testRapidStateChanges()
{
    And andGate;
    InputVcc input1, input2;
    initErrElm(andGate); initErrSrc(input1); initErrSrc(input2);

    andGate.connectPredecessor(0, &input1, 0);
    andGate.connectPredecessor(1, &input2, 0);

    for (int i = 0; i < 1000; ++i) {
        input1.setOutputValue(i % 2 == 0);
        input2.setOutputValue(i % 3 == 0);
        andGate.updateLogic();
        bool expected = (i % 2 == 0) && (i % 3 == 0);
        QCOMPARE(andGate.outputValue(), expected);
    }
}

void TestElementLogicErrors::testUnconnectedGate()
{
    // All inputs null -> default to false.
    And gate;
    initErrElm(gate);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), false);

    Not notGate;
    initErrElm(notGate);
    notGate.updateLogic();
    // NOT(false) = true
    QCOMPARE(notGate.outputValue(), true);
}

void TestElementLogicErrors::testDeepCascading()
{
    InputVcc in1, in2, in3, in4;
    And stage1, stage2, stage3;
    initErrSrc(in1); initErrSrc(in2); initErrSrc(in3); initErrSrc(in4);
    initErrElm(stage1); initErrElm(stage2); initErrElm(stage3);

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
    QCOMPARE(stage3.outputValue(), true);

    in1.setOutputValue(false);
    stage1.updateLogic();
    stage2.updateLogic();
    stage3.updateLogic();
    QCOMPARE(stage1.outputValue(), false);
    QCOMPARE(stage2.outputValue(), false);
    QCOMPARE(stage3.outputValue(), false);

    in1.setOutputValue(true);
    in3.setOutputValue(false);
    stage1.updateLogic();
    stage2.updateLogic();
    stage3.updateLogic();
    QCOMPARE(stage1.outputValue(), true);
    QCOMPARE(stage2.outputValue(), false);
    QCOMPARE(stage3.outputValue(), false);
}

void TestElementLogicErrors::testInvalidOutputPortIndex()
{
    And andGate;
    Or orGate;
    orGate.setInputSize(3);
    Not notGate;
    initErrElm(andGate); initErrElm(orGate); initErrElm(notGate);

    QCOMPARE(andGate.simOutputSize(), 1);
    QCOMPARE(orGate.simOutputSize(), 1);
    QCOMPARE(notGate.simOutputSize(), 1);

    QCOMPARE(andGate.outputValue(0), false);
    QCOMPARE(orGate.outputValue(0), false);
    QCOMPARE(notGate.outputValue(0), false);

    // InputVcc with 4 simulation outputs: port 0 defaults to true (VCC),
    // ports 1-3 have no corresponding graphic ports so they default to false.
    InputVcc input;
    input.initSimulationVectors(0, 4);
    QCOMPARE(input.simOutputSize(), 4);
    QCOMPARE(input.outputValue(0), true);
    QCOMPARE(input.outputValue(3), false);

    // Each output is independently settable
    input.setOutputValue(1, true);
    input.setOutputValue(3, true);
    QCOMPARE(input.outputValue(0), true);
    QCOMPARE(input.outputValue(1), true);
    QCOMPARE(input.outputValue(2), false);
    QCOMPARE(input.outputValue(3), true);
}

void TestElementLogicErrors::testConnectionCycles()
{
    Not notGate;
    initErrElm(notGate);
    notGate.connectPredecessor(0, &notGate, 0);

    QCOMPARE(notGate.outputValue(), false);

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), true);

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), false);

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), true);
}

void TestElementLogicErrors::testInputValueBoundary()
{
    And gate;
    gate.setInputSize(4);
    InputVcc in0, in1, in2, in3;
    initErrElm(gate); initErrSrc(in0); initErrSrc(in1); initErrSrc(in2); initErrSrc(in3);

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
        QCOMPARE(gate.outputValue(), combo == 15);
    }
}

void TestElementLogicErrors::testGateWithZeroInputs()
{
    And andGate;
    andGate.setInputSize(0);
    andGate.initSimulationVectors(0, 1);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), true);

    Or orGate;
    orGate.setInputSize(0);
    orGate.initSimulationVectors(0, 1);
    orGate.updateLogic();
    QCOMPARE(orGate.outputValue(), false);
}

