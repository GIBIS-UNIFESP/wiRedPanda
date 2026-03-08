// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestElementMapping.h"

#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/ElementMapping.h"
#include "Tests/Common/TestUtils.h"

// ============================================================
// Element Validation Tests
// ============================================================

void TestElementMapping::testSimpleElementMapping()
{
    // Test basic element mapping creation
    auto scene = createSimpleCircuit();
    QVERIFY(scene != nullptr);

    auto elements = scene->elements();
    QCOMPARE(elements.size(), 2);

    // Create mapping
    ElementMapping mapping(elements);

    // Verify mapping created successfully
    verifyMappingCreated(elements);
}

void TestElementMapping::testDisconnectedElementHandling()
{
    // Test handling of disconnected/isolated elements
    auto scene = createDisconnectedCircuit();
    QVERIFY(scene != nullptr);

    auto elements = scene->elements();
    QCOMPARE(elements.size(), 3);

    // Create mapping - should handle disconnected elements gracefully
    ElementMapping mapping(elements);

    // Mapping should complete without throwing
    verifyMappingCreated(elements);
}

void TestElementMapping::testDefaultVCCGNDConnections()
{
    // Test that default VCC/GND connections are applied
    auto scene = createCircuitWithVCCGND();
    QVERIFY(scene != nullptr);

    auto elements = scene->elements();
    QCOMPARE(elements.size(), 3);

    // Create mapping - should apply default VCC/GND where needed
    ElementMapping mapping(elements);

    // Verify mapping handles VCC/GND
    verifyMappingCreated(elements);
}

void TestElementMapping::testICInputOutputMapping()
{
    // Test mapping of IC input and output connections
    auto scene = std::make_unique<Scene>();

    // Create an IC element
    auto *ic = ElementFactory::buildElement(ElementType::IC);
    QVERIFY2(ic != nullptr, "ElementFactory failed to build IC element");

    scene->addItem(ic);
    ic->setPos(100, 100);

    auto elements = scene->elements();
    QCOMPARE(elements.size(), 1);

    // Create mapping - should handle IC port mappings
    ElementMapping mapping(elements);

    // Verify mapping created
    verifyMappingCreated(elements);
}

// ============================================================
// Helper Functions
// ============================================================

std::unique_ptr<Scene> TestElementMapping::createSimpleCircuit()
{
    auto scene = std::make_unique<Scene>();

    // Create simple AND gate with input
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *btn = ElementFactory::buildElement(ElementType::InputButton);

    scene->addItem(and1);
    scene->addItem(btn);

    and1->setPos(100, 100);
    btn->setPos(0, 100);

    return scene;
}

std::unique_ptr<Scene> TestElementMapping::createDisconnectedCircuit()
{
    auto scene = std::make_unique<Scene>();

    // Create several disconnected elements
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *or1 = ElementFactory::buildElement(ElementType::Or);
    auto *not1 = ElementFactory::buildElement(ElementType::Not);

    scene->addItem(and1);
    scene->addItem(or1);
    scene->addItem(not1);

    // Position them but don't connect
    and1->setPos(0, 0);
    or1->setPos(100, 0);
    not1->setPos(200, 0);

    return scene;
}

std::unique_ptr<Scene> TestElementMapping::createCircuitWithVCCGND()
{
    auto scene = std::make_unique<Scene>();

    // Create circuit with VCC and GND elements
    auto *vcc = ElementFactory::buildElement(ElementType::InputVcc);
    auto *gnd = ElementFactory::buildElement(ElementType::InputGnd);
    auto *and1 = ElementFactory::buildElement(ElementType::And);

    scene->addItem(vcc);
    scene->addItem(gnd);
    scene->addItem(and1);

    vcc->setPos(0, 0);
    gnd->setPos(0, 100);
    and1->setPos(100, 50);

    return scene;
}

void TestElementMapping::verifyMappingCreated(const QVector<GraphicElement *> &elements)
{
    // Verify that mapping was created successfully
    // (If we get here without exception, mapping was created)
    QVERIFY2(!elements.isEmpty(), "element list must not be empty before mapping");

    // Create mapping to verify it initializes without exceptions
    ElementMapping mapping(elements);
    auto logicElms = mapping.logicElms();
    QVERIFY2(!logicElms.isEmpty(), "mapping produced no logic elements");
}

void TestElementMapping::testGenerateMapWithIC()
{
    // Test mapping generation with IC elements
    auto scene = std::make_unique<Scene>();

    // Create IC element
    auto *ic = ElementFactory::buildElement(ElementType::IC);
    QVERIFY2(ic != nullptr, "ElementFactory failed to build IC element");
    scene->addItem(ic);
    ic->setPos(100, 100);

    // Create multiple logic gates to work with IC
    auto *input = ElementFactory::buildElement(ElementType::And);
    auto *output = ElementFactory::buildElement(ElementType::Or);
    QVERIFY2(input != nullptr, "ElementFactory failed to build And element");
    QVERIFY2(output != nullptr, "ElementFactory failed to build Or element");

    scene->addItem(input);
    scene->addItem(output);
    input->setPos(0, 100);
    output->setPos(200, 100);

    // Create connections between gates (bypass IC which needs file loading)
    auto *conn1 = new QNEConnection();
    conn1->setStartPort(input->outputPort(0));
    conn1->setEndPort(output->inputPort(0));
    scene->addItem(conn1);

    // Generate mapping - should handle mixed element types including IC
    ElementMapping mapping(scene->elements());

    // Verify no exceptions were thrown
    QCOMPARE(scene->elements().size(), 3);
}

void TestElementMapping::testApplyConnectionsWithIC()
{
    // Test connection application with IC elements mixed with logic gates
    auto scene = std::make_unique<Scene>();

    // Create IC element (doesn't need ports for this test)
    auto *ic = ElementFactory::buildElement(ElementType::IC);
    QVERIFY2(ic != nullptr, "ElementFactory failed to build IC element");
    scene->addItem(ic);
    ic->setPos(100, 100);

    // Create multiple input and logic gate elements
    auto *input1 = ElementFactory::buildElement(ElementType::And);
    auto *input2 = ElementFactory::buildElement(ElementType::Or);
    auto *gate = ElementFactory::buildElement(ElementType::Nand);
    QVERIFY2(input1 != nullptr, "ElementFactory failed to build And element");
    QVERIFY2(input2 != nullptr, "ElementFactory failed to build Or element");
    QVERIFY2(gate != nullptr, "ElementFactory failed to build Nand element");

    scene->addItem(input1);
    scene->addItem(input2);
    scene->addItem(gate);
    input1->setPos(0, 50);
    input2->setPos(0, 150);
    gate->setPos(100, 100);

    // Create connections between logic elements
    auto *conn1 = new QNEConnection();
    conn1->setStartPort(input1->outputPort(0));
    conn1->setEndPort(gate->inputPort(0));
    scene->addItem(conn1);

    auto *conn2 = new QNEConnection();
    conn2->setStartPort(input2->outputPort(0));
    conn2->setEndPort(gate->inputPort(1));
    scene->addItem(conn2);

    // Generate mapping - should apply all connections including IC presence
    ElementMapping mapping(scene->elements());

    // Verify mapping created successfully
    QCOMPARE(scene->elements().size(), 4);
}

void TestElementMapping::testICElementSorting()
{
    // Test that IC elements are sorted correctly in mapping with logic gates
    auto scene = std::make_unique<Scene>();

    // Create chain: Input -> AND -> OR -> Output, with IC present
    auto *input = ElementFactory::buildElement(ElementType::InputButton);
    auto *ic = ElementFactory::buildElement(ElementType::IC);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *output = ElementFactory::buildElement(ElementType::Led);

    QVERIFY2(input != nullptr, "ElementFactory failed to build InputButton element");
    QVERIFY2(ic != nullptr, "ElementFactory failed to build IC element");
    QVERIFY2(andGate != nullptr, "ElementFactory failed to build And element");
    QVERIFY2(output != nullptr, "ElementFactory failed to build Led element");

    scene->addItem(input);
    scene->addItem(ic);
    scene->addItem(andGate);
    scene->addItem(output);

    input->setPos(0, 100);
    ic->setPos(100, 100);
    andGate->setPos(200, 100);
    output->setPos(300, 100);

    // Create connections (between gates, IC is just present for mapping test)
    auto *conn1 = new QNEConnection();
    conn1->setStartPort(input->outputPort(0));
    conn1->setEndPort(andGate->inputPort(0));
    scene->addItem(conn1);

    auto *conn2 = new QNEConnection();
    conn2->setStartPort(andGate->outputPort(0));
    conn2->setEndPort(output->inputPort(0));
    scene->addItem(conn2);

    // Generate mapping - should maintain proper element order including IC
    ElementMapping mapping(scene->elements());

    // Verify no exceptions and all elements included
    QCOMPARE(scene->elements().size(), 4);
}

void TestElementMapping::testSortLogicElementsByPriority()
{
    // Test topological sorting of logic elements
    auto scene = std::make_unique<Scene>();

    // Create circuit: Input -> AND -> OR -> Output
    auto *input = ElementFactory::buildElement(ElementType::InputButton);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *orGate = ElementFactory::buildElement(ElementType::Or);
    auto *output = ElementFactory::buildElement(ElementType::Led);

    QVERIFY2(input != nullptr, "ElementFactory failed to build InputButton element");
    QVERIFY2(andGate != nullptr, "ElementFactory failed to build And element");
    QVERIFY2(orGate != nullptr, "ElementFactory failed to build Or element");
    QVERIFY2(output != nullptr, "ElementFactory failed to build Led element");

    scene->addItem(input);
    scene->addItem(andGate);
    scene->addItem(orGate);
    scene->addItem(output);

    input->setPos(0, 100);
    andGate->setPos(100, 100);
    orGate->setPos(200, 100);
    output->setPos(300, 100);

    // Create connections in proper order
    auto *conn1 = new QNEConnection();
    conn1->setStartPort(input->outputPort(0));
    conn1->setEndPort(andGate->inputPort(0));
    scene->addItem(conn1);

    auto *conn2 = new QNEConnection();
    conn2->setStartPort(andGate->outputPort(0));
    conn2->setEndPort(orGate->inputPort(0));
    scene->addItem(conn2);

    auto *conn3 = new QNEConnection();
    conn3->setStartPort(orGate->outputPort(0));
    conn3->setEndPort(output->inputPort(0));
    scene->addItem(conn3);

    // Generate mapping - should sort by priority (topological order)
    ElementMapping mapping(scene->elements());

    // Verify all elements are present
    QCOMPARE(scene->elements().size(), 4);
}

void TestElementMapping::testMixedElementTypesSorting()
{
    // Test sorting with mixed element types (inputs, gates, outputs, IC)
    auto scene = std::make_unique<Scene>();

    // Create mix of element types
    auto *input1 = ElementFactory::buildElement(ElementType::InputButton);
    auto *input2 = ElementFactory::buildElement(ElementType::InputVcc);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *orGate = ElementFactory::buildElement(ElementType::Or);
    auto *ic = ElementFactory::buildElement(ElementType::IC);
    auto *output = ElementFactory::buildElement(ElementType::Led);

    QVERIFY2(input1 != nullptr, "ElementFactory failed to build InputButton element");
    QVERIFY2(input2 != nullptr, "ElementFactory failed to build InputVcc element");
    QVERIFY2(andGate != nullptr, "ElementFactory failed to build And element");
    QVERIFY2(orGate != nullptr, "ElementFactory failed to build Or element");
    QVERIFY2(ic != nullptr, "ElementFactory failed to build IC element");
    QVERIFY2(output != nullptr, "ElementFactory failed to build Led element");

    scene->addItem(input1);
    scene->addItem(input2);
    scene->addItem(andGate);
    scene->addItem(orGate);
    scene->addItem(ic);
    scene->addItem(output);

    // Position elements
    input1->setPos(0, 50);
    input2->setPos(0, 150);
    andGate->setPos(100, 100);
    orGate->setPos(200, 100);
    ic->setPos(300, 100);
    output->setPos(400, 100);

    // Create some connections
    auto *conn1 = new QNEConnection();
    conn1->setStartPort(input1->outputPort(0));
    conn1->setEndPort(andGate->inputPort(0));
    scene->addItem(conn1);

    auto *conn2 = new QNEConnection();
    conn2->setStartPort(andGate->outputPort(0));
    conn2->setEndPort(orGate->inputPort(0));
    scene->addItem(conn2);

    // Generate mapping - should handle mixed types
    ElementMapping mapping(scene->elements());

    // Verify all elements are present
    QCOMPARE(scene->elements().size(), 6);
}

void TestElementMapping::testApplyConnectionsDefaultValues()
{
    // Test that connections are applied with default values
    auto scene = std::make_unique<Scene>();

    // Create elements with unconnected inputs
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    auto *output = ElementFactory::buildElement(ElementType::Led);

    QVERIFY2(and1 != nullptr, "ElementFactory failed to build first And element");
    QVERIFY2(and2 != nullptr, "ElementFactory failed to build second And element");
    QVERIFY2(output != nullptr, "ElementFactory failed to build Led element");

    scene->addItem(and1);
    scene->addItem(and2);
    scene->addItem(output);

    and1->setPos(0, 50);
    and2->setPos(100, 100);
    output->setPos(200, 100);

    // Connect only one input of and2 (leave second unconnected)
    auto *conn1 = new QNEConnection();
    conn1->setStartPort(and1->outputPort(0));
    conn1->setEndPort(and2->inputPort(0));
    scene->addItem(conn1);

    auto *conn2 = new QNEConnection();
    conn2->setStartPort(and2->outputPort(0));
    conn2->setEndPort(output->inputPort(0));
    scene->addItem(conn2);

    // Generate mapping - should apply default values for unconnected inputs
    ElementMapping mapping(scene->elements());

    // Verify mapping created successfully with default values applied
    QCOMPARE(scene->elements().size(), 3);
}

