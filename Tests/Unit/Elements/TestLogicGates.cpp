// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestLogicGates.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "Tests/Common/TestUtils.h"

void TestLogicGates::testAnd()
{
    Element elm{ElementType::And};
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::And);
}

void TestLogicGates::testOr()
{
    Element elm{ElementType::Or};
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Or);
}

void TestLogicGates::testNot()
{
    Element elm{ElementType::Not};
    QCOMPARE(elm.inputSize(), 1);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 1);
    QCOMPARE(elm.maxInputSize(), 1);
    QCOMPARE(elm.elementType(), ElementType::Not);
}

void TestLogicGates::testNand()
{
    Element elm{ElementType::Nand};
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Nand);
}

void TestLogicGates::testNor()
{
    Element elm{ElementType::Nor};
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Nor);
}

void TestLogicGates::testXor()
{
    Element elm{ElementType::Xor};
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Xor);
}

void TestLogicGates::testXnor()
{
    Element elm{ElementType::Xnor};
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
    Element andGate{ElementType::And};
    Led led;

    builder.add(&sw1, &sw2, &andGate, &led);
    builder.connect(&sw1, 0, &andGate, 0);
    builder.connect(&sw2, 0, &andGate, 1);
    builder.connect(&andGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Test AND truth table
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Inactive);  // 0 AND 0 = 0

    sw1.setOn(false); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Inactive);  // 0 AND 1 = 0

    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Inactive);  // 1 AND 0 = 0

    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Active);   // 1 AND 1 = 1
}

void TestLogicGates::testOrGateBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Element orGate{ElementType::Or};
    Led led;

    builder.add(&sw1, &sw2, &orGate, &led);
    builder.connect(&sw1, 0, &orGate, 0);
    builder.connect(&sw2, 0, &orGate, 1);
    builder.connect(&orGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Test OR truth table
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Inactive);  // 0 OR 0 = 0

    sw1.setOn(false); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Active);   // 0 OR 1 = 1

    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Active);   // 1 OR 0 = 1

    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Active);   // 1 OR 1 = 1
}

void TestLogicGates::testNotGateBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Element notGate{ElementType::Not};
    Led led;

    builder.add(&sw, &notGate, &led);
    builder.connect(&sw, 0, &notGate, 0);
    builder.connect(&notGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Test NOT behavior
    sw.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Active);   // NOT 0 = 1

    sw.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Inactive);  // NOT 1 = 0
}

void TestLogicGates::testNandGateBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Element nandGate{ElementType::Nand};
    Led led;

    builder.add(&sw1, &sw2, &nandGate, &led);
    builder.connect(&sw1, 0, &nandGate, 0);
    builder.connect(&sw2, 0, &nandGate, 1);
    builder.connect(&nandGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Test NAND truth table
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Active);   // 0 NAND 0 = 1

    sw1.setOn(false); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Active);   // 0 NAND 1 = 1

    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Active);   // 1 NAND 0 = 1

    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Inactive);  // 1 NAND 1 = 0
}

void TestLogicGates::testNorGateBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Element norGate{ElementType::Nor};
    Led led;

    builder.add(&sw1, &sw2, &norGate, &led);
    builder.connect(&sw1, 0, &norGate, 0);
    builder.connect(&sw2, 0, &norGate, 1);
    builder.connect(&norGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Test NOR truth table
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Active);   // 0 NOR 0 = 1

    sw1.setOn(false); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Inactive);  // 0 NOR 1 = 0

    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Inactive);  // 1 NOR 0 = 0

    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Inactive);  // 1 NOR 1 = 0
}

void TestLogicGates::testXorGateBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Element xorGate{ElementType::Xor};
    Led led;

    builder.add(&sw1, &sw2, &xorGate, &led);
    builder.connect(&sw1, 0, &xorGate, 0);
    builder.connect(&sw2, 0, &xorGate, 1);
    builder.connect(&xorGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Test XOR truth table
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Inactive);  // 0 XOR 0 = 0

    sw1.setOn(false); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Active);   // 0 XOR 1 = 1

    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Active);   // 1 XOR 0 = 1

    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Inactive);  // 1 XOR 1 = 0
}

void TestLogicGates::testXnorGateBehavior()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Element xnorGate{ElementType::Xnor};
    Led led;

    builder.add(&sw1, &sw2, &xnorGate, &led);
    builder.connect(&sw1, 0, &xnorGate, 0);
    builder.connect(&sw2, 0, &xnorGate, 1);
    builder.connect(&xnorGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Test XNOR truth table
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Active);   // 0 XNOR 0 = 1

    sw1.setOn(false); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Inactive);  // 0 XNOR 1 = 0

    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Inactive);  // 1 XNOR 0 = 0

    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(TestUtils::inputStatus(&led), Status::Active);   // 1 XNOR 1 = 1
}
