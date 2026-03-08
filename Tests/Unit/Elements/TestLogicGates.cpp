// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestLogicGates.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Nand.h"
#include "App/Element/GraphicElements/Nor.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/Xnor.h"
#include "App/Element/GraphicElements/Xor.h"
#include "Tests/Common/TestUtils.h"

void TestLogicGates::testAnd()
{
    And elm;
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::And);
}

void TestLogicGates::testOr()
{
    Or elm;
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Or);
}

void TestLogicGates::testNot()
{
    Not elm;
    QCOMPARE(elm.inputSize(), 1);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 1);
    QCOMPARE(elm.maxInputSize(), 1);
    QCOMPARE(elm.elementType(), ElementType::Not);
}

void TestLogicGates::testNand()
{
    Nand elm;
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Nand);
}

void TestLogicGates::testNor()
{
    Nor elm;
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Nor);
}

void TestLogicGates::testXor()
{
    Xor elm;
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Xor);
}

void TestLogicGates::testXnor()
{
    Xnor elm;
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Xnor);
}

// ============================================================================
// Behavioral Tests - Verify actual logic computation in circuits
// ============================================================================

void TestLogicGates::testAndGateBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    And andGate;
    Led led;

    builder.add(&sw1, &sw2, &andGate, &led);
    builder.connect(&sw1, 0, &andGate, 0);
    builder.connect(&sw2, 0, &andGate, 1);
    builder.connect(&andGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Test AND truth table
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), false);  // 0 AND 0 = 0

    sw1.setOn(false); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), false);  // 0 AND 1 = 0

    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), false);  // 1 AND 0 = 0

    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);   // 1 AND 1 = 1
}

void TestLogicGates::testOrGateBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Or orGate;
    Led led;

    builder.add(&sw1, &sw2, &orGate, &led);
    builder.connect(&sw1, 0, &orGate, 0);
    builder.connect(&sw2, 0, &orGate, 1);
    builder.connect(&orGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Test OR truth table
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), false);  // 0 OR 0 = 0

    sw1.setOn(false); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);   // 0 OR 1 = 1

    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);   // 1 OR 0 = 1

    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);   // 1 OR 1 = 1
}

void TestLogicGates::testNotGateBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Not notGate;
    Led led;

    builder.add(&sw, &notGate, &led);
    builder.connect(&sw, 0, &notGate, 0);
    builder.connect(&notGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Test NOT behavior
    sw.setOn(false); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);   // NOT 0 = 1

    sw.setOn(true); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), false);  // NOT 1 = 0
}

void TestLogicGates::testNandGateBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Nand nandGate;
    Led led;

    builder.add(&sw1, &sw2, &nandGate, &led);
    builder.connect(&sw1, 0, &nandGate, 0);
    builder.connect(&sw2, 0, &nandGate, 1);
    builder.connect(&nandGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Test NAND truth table
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);   // 0 NAND 0 = 1

    sw1.setOn(false); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);   // 0 NAND 1 = 1

    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);   // 1 NAND 0 = 1

    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), false);  // 1 NAND 1 = 0
}

void TestLogicGates::testNorGateBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Nor norGate;
    Led led;

    builder.add(&sw1, &sw2, &norGate, &led);
    builder.connect(&sw1, 0, &norGate, 0);
    builder.connect(&sw2, 0, &norGate, 1);
    builder.connect(&norGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Test NOR truth table
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);   // 0 NOR 0 = 1

    sw1.setOn(false); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), false);  // 0 NOR 1 = 0

    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), false);  // 1 NOR 0 = 0

    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), false);  // 1 NOR 1 = 0
}

void TestLogicGates::testXorGateBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Xor xorGate;
    Led led;

    builder.add(&sw1, &sw2, &xorGate, &led);
    builder.connect(&sw1, 0, &xorGate, 0);
    builder.connect(&sw2, 0, &xorGate, 1);
    builder.connect(&xorGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Test XOR truth table
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), false);  // 0 XOR 0 = 0

    sw1.setOn(false); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);   // 0 XOR 1 = 1

    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);   // 1 XOR 0 = 1

    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), false);  // 1 XOR 1 = 0
}

void TestLogicGates::testXnorGateBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Xnor xnorGate;
    Led led;

    builder.add(&sw1, &sw2, &xnorGate, &led);
    builder.connect(&sw1, 0, &xnorGate, 0);
    builder.connect(&sw2, 0, &xnorGate, 1);
    builder.connect(&xnorGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Test XNOR truth table
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);   // 0 XNOR 0 = 1

    sw1.setOn(false); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), false);  // 0 XNOR 1 = 0

    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), false);  // 1 XNOR 0 = 0

    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::getInputStatus(&led), true);   // 1 XNOR 1 = 1
}

