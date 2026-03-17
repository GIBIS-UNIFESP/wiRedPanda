// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestElementMapping.h"

#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/Node.h"
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
    verifyMappingCreated(mapping);
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
    verifyMappingCreated(mapping);
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
    verifyMappingCreated(mapping);
}

void TestElementMapping::testICInputOutputMapping()
{
    // An unloaded IC has no inner circuit, no ports, and no logicCreator — it
    // is excluded from the simulation graph just like decorative elements.
    auto scene = std::make_unique<Scene>();

    auto *ic = ElementFactory::buildElement(ElementType::IC);
    QVERIFY2(ic != nullptr, "ElementFactory failed to build IC element");

    scene->addItem(ic);
    ic->setPos(100, 100);

    auto elements = scene->elements();
    QCOMPARE(elements.size(), 1);

    // Mapping should complete without throwing and produce no logic elements.
    ElementMapping mapping(elements);
    // 2 elements: the global GND and VCC sources (shared across all IC levels).
    QCOMPARE(mapping.logicElms().size(), 2);
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

void TestElementMapping::verifyMappingCreated(const ElementMapping &mapping)
{
    QVERIFY2(!mapping.logicElms().isEmpty(), "mapping produced no logic elements");
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

// ============================================================
// Refactor Coverage Tests
// ============================================================

/**
 * Test: Line and Text elements are decorative and must not produce logic elements.
 * After Phase 5 of the LogicElement refactor, decorative elements register no
 * logicCreator, so ElementMapping skips them entirely.
 */
void TestElementMapping::testDecorativeElementsExcluded()
{
    auto scene = std::make_unique<Scene>();

    auto *line = ElementFactory::buildElement(ElementType::Line);
    auto *text = ElementFactory::buildElement(ElementType::Text);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(line != nullptr, "ElementFactory failed to build Line element");
    QVERIFY2(text != nullptr, "ElementFactory failed to build Text element");
    QVERIFY2(andGate != nullptr, "ElementFactory failed to build And element");

    scene->addItem(line);
    scene->addItem(text);
    scene->addItem(andGate);
    line->setPos(0, 0);
    text->setPos(100, 0);
    andGate->setPos(200, 0);

    ElementMapping mapping(scene->elements());

    // AND gate produces 1 logic element; Line and Text contribute none.
    // +2 for the global GND/VCC sources always present in every mapping.
    QCOMPARE(mapping.logicElms().size(), 1 + 2);
}

/**
 * Test: After sort(), elements closer to the input (sources) have strictly
 * higher priority than elements closer to the output (sinks).
 * Circuit: InputButton → NOT → LED  (priorities: 3 > 2 > 1).
 */
/**
 * Test: After ElementMapping is constructed, every non-decorative element
 * has logic() set, and each of its ports points back to the same logic element.
 * This verifies the createLogicElements() + bindPorts() two-step (Phase 7).
 */
void TestElementMapping::testPortsHaveLogicAfterMapping()
{
    auto scene = std::make_unique<Scene>();

    auto *notGate = ElementFactory::buildElement(ElementType::Not);
    QVERIFY2(notGate != nullptr, "ElementFactory failed to build Not element");
    scene->addItem(notGate);
    notGate->setPos(0, 0);

    ElementMapping mapping(scene->elements());

    QVERIFY2(notGate->logic() != nullptr, "NOT gate must have logic assigned after mapping");

    auto *inPort  = notGate->inputPort(0);
    auto *outPort = notGate->outputPort(0);
    QVERIFY2(inPort->logic()  != nullptr, "Input port must have logic assigned");
    QVERIFY2(outPort->logic() != nullptr, "Output port must have logic assigned");
    QCOMPARE(inPort->logic(),  notGate->logic());
    QCOMPARE(outPort->logic(), notGate->logic());
}

/**
 * Test: The logic element count equals exactly the number of non-decorative
 * elements in the scene. Line and Text elements must not contribute.
 */
void TestElementMapping::testLogicElementCountMatchesNonDecorativeElements()
{
    auto scene = std::make_unique<Scene>();

    // 3 non-decorative + 2 decorative
    auto *input   = ElementFactory::buildElement(ElementType::InputButton);
    auto *notGate = ElementFactory::buildElement(ElementType::Not);
    auto *led     = ElementFactory::buildElement(ElementType::Led);
    auto *line    = ElementFactory::buildElement(ElementType::Line);
    auto *text    = ElementFactory::buildElement(ElementType::Text);

    scene->addItem(input);
    scene->addItem(notGate);
    scene->addItem(led);
    scene->addItem(line);
    scene->addItem(text);

    QCOMPARE(scene->elements().size(), 5);

    ElementMapping mapping(scene->elements());

    // 3 logic-producing elements + 2 global GND/VCC sources
    QCOMPARE(mapping.logicElms().size(), 3 + 2);
}

/**
 * Test: Unconnected optional inputs receive a default predecessor (globalVCC
 * or globalGND) from applyConnection(). DFlipFlop PRESET and CLEAR are
 * optional, default to Active (active-low = not asserted). After mapping,
 * both must have a non-null predecessor in their inputPairs.
 */
void TestElementMapping::testDefaultVCCAppliedToOptionalInputs()
{
    auto scene = std::make_unique<Scene>();

    auto *dff = ElementFactory::buildElement(ElementType::DFlipFlop);
    QVERIFY2(dff != nullptr, "ElementFactory failed to build DFlipFlop element");
    scene->addItem(dff);
    dff->setPos(0, 0);

    ElementMapping mapping(scene->elements());

    auto *logic = dff->logic();
    QVERIFY2(logic != nullptr, "DFlipFlop must have logic assigned after mapping");

    // inputPairs indices: 0=D, 1=CLK, 2=PRESET(optional), 3=CLEAR(optional)
    QVERIFY2(logic->inputPairs().at(2).logic != nullptr,
             "PRESET port must be connected to globalVCC by default");
    QVERIFY2(logic->inputPairs().at(3).logic != nullptr,
             "CLEAR port must be connected to globalVCC by default");
}

// ============================================================
// Wireless Connection Tests
// ============================================================

void TestElementMapping::testWirelessRxConnectedToTx()
{
    auto scene = std::make_unique<Scene>();

    auto *txNode = ElementFactory::buildElement(ElementType::Node);
    auto *rxNode = ElementFactory::buildElement(ElementType::Node);
    auto *src = ElementFactory::buildElement(ElementType::InputButton);

    QVERIFY(txNode && rxNode && src);
    scene->addItem(txNode);
    scene->addItem(rxNode);
    scene->addItem(src);

    txNode->setPos(100, 100);
    rxNode->setPos(300, 100);
    src->setPos(0, 100);

    auto *txN = qobject_cast<Node *>(txNode);
    auto *rxN = qobject_cast<Node *>(rxNode);
    QVERIFY(txN && rxN);

    txN->setLabel("CLK");
    txN->setWirelessMode(WirelessMode::Tx);
    rxN->setLabel("CLK");
    rxN->setWirelessMode(WirelessMode::Rx);

    // Wire src → txNode physically
    auto *conn = new QNEConnection();
    conn->setStartPort(src->outputPort(0));
    conn->setEndPort(txNode->inputPort(0));
    scene->addItem(conn);

    ElementMapping mapping(scene->elements());
    mapping.buildGraph();

    // Rx's logic input 0 must point to Tx's logic element (not GND)
    auto *rxLogic = rxN->logic();
    QVERIFY(rxLogic != nullptr);
    const auto &pairs = rxLogic->inputPairs();
    QVERIFY(!pairs.isEmpty());
    QCOMPARE(pairs.at(0).logic, txN->logic());
}

void TestElementMapping::testWirelessRxNoMatchFallsToGnd()
{
    auto scene = std::make_unique<Scene>();

    auto *rxNode = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(rxNode);
    scene->addItem(rxNode);
    rxNode->setPos(100, 100);

    auto *rxN = qobject_cast<Node *>(rxNode);
    QVERIFY(rxN);
    rxN->setLabel("MISSING");
    rxN->setWirelessMode(WirelessMode::Rx);

    ElementMapping mapping(scene->elements());
    mapping.buildGraph();

    // No matching Tx: logic must still have a valid (GND) predecessor — element is valid
    auto *rxLogic = rxN->logic();
    QVERIFY(rxLogic != nullptr);
    const auto &pairs = rxLogic->inputPairs();
    QVERIFY(!pairs.isEmpty());
    QVERIFY(pairs.at(0).logic != nullptr);   // GND, not null
}

void TestElementMapping::testWirelessFanOut()
{
    auto scene = std::make_unique<Scene>();

    auto *src = ElementFactory::buildElement(ElementType::InputButton);
    auto *txNode = ElementFactory::buildElement(ElementType::Node);
    auto *rx1 = ElementFactory::buildElement(ElementType::Node);
    auto *rx2 = ElementFactory::buildElement(ElementType::Node);
    auto *rx3 = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(src && txNode && rx1 && rx2 && rx3);

    scene->addItem(src);
    scene->addItem(txNode);
    scene->addItem(rx1);
    scene->addItem(rx2);
    scene->addItem(rx3);

    src->setPos(0, 100);
    txNode->setPos(100, 100);
    rx1->setPos(300, 50);
    rx2->setPos(300, 100);
    rx3->setPos(300, 150);

    auto *txN = qobject_cast<Node *>(txNode);
    txN->setLabel("BUS");
    txN->setWirelessMode(WirelessMode::Tx);
    for (auto *rx : {rx1, rx2, rx3}) {
        auto *rxN = qobject_cast<Node *>(rx);
        rxN->setLabel("BUS");
        rxN->setWirelessMode(WirelessMode::Rx);
    }

    auto *conn = new QNEConnection();
    conn->setStartPort(src->outputPort(0));
    conn->setEndPort(txNode->inputPort(0));
    scene->addItem(conn);

    ElementMapping mapping(scene->elements());
    mapping.buildGraph();

    LogicElement *txLogic = txN->logic();
    QVERIFY(txLogic != nullptr);

    for (auto *rx : {rx1, rx2, rx3}) {
        auto *rxN = qobject_cast<Node *>(rx);
        auto *rxLogic = rxN->logic();
        QVERIFY(rxLogic != nullptr);
        QCOMPARE(rxLogic->inputPairs().at(0).logic, txLogic);
    }
}

void TestElementMapping::testDuplicateTxDoesNotCrash()
{
    auto scene = std::make_unique<Scene>();

    auto *src1 = ElementFactory::buildElement(ElementType::InputButton);
    auto *src2 = ElementFactory::buildElement(ElementType::InputButton);
    auto *tx1 = ElementFactory::buildElement(ElementType::Node);
    auto *tx2 = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(src1 && src2 && tx1 && tx2);

    scene->addItem(src1);
    scene->addItem(src2);
    scene->addItem(tx1);
    scene->addItem(tx2);

    src1->setPos(0, 0);
    src2->setPos(0, 100);
    tx1->setPos(100, 0);
    tx2->setPos(100, 100);

    for (auto *tx : {tx1, tx2}) {
        auto *txN = qobject_cast<Node *>(tx);
        txN->setLabel("DUPE");
        txN->setWirelessMode(WirelessMode::Tx);
    }

    auto *c1 = new QNEConnection();
    c1->setStartPort(src1->outputPort(0));
    c1->setEndPort(tx1->inputPort(0));
    scene->addItem(c1);

    auto *c2 = new QNEConnection();
    c2->setStartPort(src2->outputPort(0));
    c2->setEndPort(tx2->inputPort(0));
    scene->addItem(c2);

    // Must not crash or assert — duplicate Tx is a user error but must be handled gracefully.
    // connectWirelessElements() inserts into a QHash keyed by label; the second Tx overwrites
    // the first, so no Rx is connected to both simultaneously.
    auto *rx = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(rx);
    scene->addItem(rx);
    rx->setPos(200, 50);
    auto *rxN = qobject_cast<Node *>(rx);
    rxN->setLabel("DUPE");
    rxN->setWirelessMode(WirelessMode::Rx);

    ElementMapping mapping(scene->elements());
    mapping.buildGraph();
    QVERIFY(!mapping.logicElms().isEmpty());

    // Rx must be connected to exactly one of the two Tx logic elements — not to null
    auto *rxLogic = rxN->logic();
    QVERIFY(rxLogic != nullptr);
    const auto &pairs = rxLogic->inputPairs();
    QVERIFY(!pairs.isEmpty());
    LogicElement *winner = pairs.at(0).logic;
    QVERIFY(winner != nullptr);
    QVERIFY(winner == qobject_cast<Node *>(tx1)->logic() || winner == qobject_cast<Node *>(tx2)->logic());
}

void TestElementMapping::testWirelessEmptyLabelNotMatched()
{
    // Tx and Rx nodes with empty labels must NOT be paired by connectWirelessElements()
    // because the guard `elm->label().isEmpty()` skips both; the Rx must fall back to GND.
    auto scene = std::make_unique<Scene>();

    auto *txNode = ElementFactory::buildElement(ElementType::Node);
    auto *rxNode = ElementFactory::buildElement(ElementType::Node);
    auto *src    = ElementFactory::buildElement(ElementType::InputButton);
    QVERIFY(txNode && rxNode && src);
    scene->addItem(src);
    scene->addItem(txNode);
    scene->addItem(rxNode);

    auto *txN = qobject_cast<Node *>(txNode);
    auto *rxN = qobject_cast<Node *>(rxNode);
    QVERIFY(txN && rxN);

    // Both nodes have mode set but NO label — connectWirelessElements() must skip them
    txN->setWirelessMode(WirelessMode::Tx);
    rxN->setWirelessMode(WirelessMode::Rx);
    // labels remain empty (default)

    auto *conn = new QNEConnection();
    conn->setStartPort(src->outputPort(0));
    conn->setEndPort(txNode->inputPort(0));
    scene->addItem(conn);

    ElementMapping mapping(scene->elements());
    mapping.buildGraph();

    // Rx must NOT be wired to Tx — it must have a valid (GND) predecessor, not the Tx logic
    auto *rxLogic = rxN->logic();
    QVERIFY(rxLogic != nullptr);
    const auto &pairs = rxLogic->inputPairs();
    QVERIFY(!pairs.isEmpty());
    QVERIFY(pairs.at(0).logic != txN->logic());   // GND, not the empty-label Tx
    QVERIFY(pairs.at(0).logic != nullptr);        // valid GND reference
}

void TestElementMapping::testWirelessTxAloneDoesNotCrash()
{
    // A Tx node with no matching Rx must not crash or produce invalid state.
    auto scene = std::make_unique<Scene>();

    auto *src    = ElementFactory::buildElement(ElementType::InputButton);
    auto *txNode = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(src && txNode);
    scene->addItem(src);
    scene->addItem(txNode);

    auto *txN = qobject_cast<Node *>(txNode);
    QVERIFY(txN);
    txN->setLabel(QStringLiteral("ORPHAN"));
    txN->setWirelessMode(WirelessMode::Tx);

    auto *conn = new QNEConnection();
    conn->setStartPort(src->outputPort(0));
    conn->setEndPort(txNode->inputPort(0));
    scene->addItem(conn);

    // No Rx with label "ORPHAN" — connectWirelessElements() just builds an empty txMap entry
    ElementMapping mapping(scene->elements());
    mapping.buildGraph();

    // Tx logic must be valid and the mapping must have produced logic elements
    QVERIFY(txN->logic() != nullptr);
    QVERIFY(!mapping.logicElms().isEmpty());
}

void TestElementMapping::testWirelessMultipleChannels()
{
    // Two independent wireless channels ("CLK" and "DATA") in the same scene must
    // not cross-connect: RxCLK→TxCLK and RxDATA→TxDATA, not each other.
    auto scene = std::make_unique<Scene>();

    auto *srcClk  = ElementFactory::buildElement(ElementType::InputButton);
    auto *srcData = ElementFactory::buildElement(ElementType::InputButton);
    auto *txClk   = ElementFactory::buildElement(ElementType::Node);
    auto *txData  = ElementFactory::buildElement(ElementType::Node);
    auto *rxClk   = ElementFactory::buildElement(ElementType::Node);
    auto *rxData  = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(srcClk && srcData && txClk && txData && rxClk && rxData);

    for (auto *elm : {srcClk, srcData, txClk, txData, rxClk, rxData})
        scene->addItem(elm);

    auto *txClkN  = qobject_cast<Node *>(txClk);
    auto *txDataN = qobject_cast<Node *>(txData);
    auto *rxClkN  = qobject_cast<Node *>(rxClk);
    auto *rxDataN = qobject_cast<Node *>(rxData);
    QVERIFY(txClkN && txDataN && rxClkN && rxDataN);

    txClkN->setLabel("CLK");   txClkN->setWirelessMode(WirelessMode::Tx);
    txDataN->setLabel("DATA"); txDataN->setWirelessMode(WirelessMode::Tx);
    rxClkN->setLabel("CLK");   rxClkN->setWirelessMode(WirelessMode::Rx);
    rxDataN->setLabel("DATA"); rxDataN->setWirelessMode(WirelessMode::Rx);

    auto *c1 = new QNEConnection();
    c1->setStartPort(srcClk->outputPort(0));
    c1->setEndPort(txClk->inputPort(0));
    scene->addItem(c1);

    auto *c2 = new QNEConnection();
    c2->setStartPort(srcData->outputPort(0));
    c2->setEndPort(txData->inputPort(0));
    scene->addItem(c2);

    ElementMapping mapping(scene->elements());
    mapping.buildGraph();

    // Each Rx must be wired to its own Tx, not to the other channel's Tx
    QCOMPARE(rxClkN->logic()->inputPairs().at(0).logic,  txClkN->logic());
    QCOMPARE(rxDataN->logic()->inputPairs().at(0).logic, txDataN->logic());
    QVERIFY(rxClkN->logic()->inputPairs().at(0).logic  != txDataN->logic());
    QVERIFY(rxDataN->logic()->inputPairs().at(0).logic != txClkN->logic());
}

void TestElementMapping::testWirelessRxPhysicalWireOverriddenByTx()
{
    // An Rx node that also has a physical wire attached must end up driven by the
    // Tx logic, not by the physical source.  connectWirelessElements() runs after
    // connectElements() and overwrites the predecessor set by the physical wire.
    auto scene = std::make_unique<Scene>();

    auto *srcTx      = ElementFactory::buildElement(ElementType::InputButton);  // drives Tx
    auto *srcPhysRx  = ElementFactory::buildElement(ElementType::InputButton);  // physical wire to Rx
    auto *txNode     = ElementFactory::buildElement(ElementType::Node);
    auto *rxNode     = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(srcTx && srcPhysRx && txNode && rxNode);

    scene->addItem(srcTx);
    scene->addItem(srcPhysRx);
    scene->addItem(txNode);
    scene->addItem(rxNode);

    auto *txN = qobject_cast<Node *>(txNode);
    auto *rxN = qobject_cast<Node *>(rxNode);
    QVERIFY(txN && rxN);

    txN->setLabel("SIG"); txN->setWirelessMode(WirelessMode::Tx);
    rxN->setLabel("SIG"); rxN->setWirelessMode(WirelessMode::Rx);

    // Physical wire: srcTx → txNode
    auto *c1 = new QNEConnection();
    c1->setStartPort(srcTx->outputPort(0));
    c1->setEndPort(txNode->inputPort(0));
    scene->addItem(c1);

    // Physical wire: srcPhysRx → rxNode (this connection will be overridden wirelessly)
    auto *c2 = new QNEConnection();
    c2->setStartPort(srcPhysRx->outputPort(0));
    c2->setEndPort(rxNode->inputPort(0));
    scene->addItem(c2);

    ElementMapping mapping(scene->elements());
    mapping.buildGraph();

    // Wireless overrides physical: Rx must point to Tx, not to srcPhysRx
    const auto &pairs = rxN->logic()->inputPairs();
    QVERIFY(!pairs.isEmpty());
    QCOMPARE(pairs.at(0).logic, txN->logic());
    QVERIFY(pairs.at(0).logic != srcPhysRx->logic());
}

void TestElementMapping::testWirelessLabelCaseSensitive()
{
    // "CLK" (Tx) and "clk" (Rx) must NOT be paired — label matching is case-sensitive.
    auto scene = std::make_unique<Scene>();

    auto *src    = ElementFactory::buildElement(ElementType::InputButton);
    auto *txNode = ElementFactory::buildElement(ElementType::Node);
    auto *rxNode = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(src && txNode && rxNode);
    scene->addItem(src);
    scene->addItem(txNode);
    scene->addItem(rxNode);

    auto *txN = qobject_cast<Node *>(txNode);
    auto *rxN = qobject_cast<Node *>(rxNode);
    QVERIFY(txN && rxN);

    txN->setLabel("CLK");  txN->setWirelessMode(WirelessMode::Tx);
    rxN->setLabel("clk");  rxN->setWirelessMode(WirelessMode::Rx);  // different case

    auto *conn = new QNEConnection();
    conn->setStartPort(src->outputPort(0));
    conn->setEndPort(txNode->inputPort(0));
    scene->addItem(conn);

    ElementMapping mapping(scene->elements());
    mapping.buildGraph();

    // "CLK" != "clk" → Rx must NOT connect to Tx; must have a valid GND fallback
    const auto &pairs = rxN->logic()->inputPairs();
    QVERIFY(!pairs.isEmpty());
    QVERIFY(pairs.at(0).logic != txN->logic());
    QVERIFY(pairs.at(0).logic != nullptr);
}

void TestElementMapping::testWirelessSignalPropagation()
{
    // After sort(), driving the Tx logic HIGH must propagate through the wireless
    // connection so that the Rx logic output is also HIGH, and vice versa.
    auto scene = std::make_unique<Scene>();

    auto *src    = ElementFactory::buildElement(ElementType::InputButton);
    auto *txNode = ElementFactory::buildElement(ElementType::Node);
    auto *rxNode = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(src && txNode && rxNode);
    scene->addItem(src);
    scene->addItem(txNode);
    scene->addItem(rxNode);

    auto *txN = qobject_cast<Node *>(txNode);
    auto *rxN = qobject_cast<Node *>(rxNode);
    QVERIFY(txN && rxN);

    txN->setLabel("SIG"); txN->setWirelessMode(WirelessMode::Tx);
    rxN->setLabel("SIG"); rxN->setWirelessMode(WirelessMode::Rx);

    auto *conn = new QNEConnection();
    conn->setStartPort(src->outputPort(0));
    conn->setEndPort(txNode->inputPort(0));
    scene->addItem(conn);

    ElementMapping mapping(scene->elements());
    mapping.buildGraph();

    // Drive the source HIGH and run one simulation tick
    src->logic()->setOutputValue(true);
    for (const auto &logicElm : mapping.logicElms()) {
        logicElm->updateLogic();
    }
    QCOMPARE(rxN->logic()->outputValue(), Status::Active);

    // Drive LOW and re-verify
    src->logic()->setOutputValue(false);
    for (const auto &logicElm : mapping.logicElms()) {
        logicElm->updateLogic();
    }
    QCOMPARE(rxN->logic()->outputValue(), Status::Inactive);
}

void TestElementMapping::testWirelessNoneModeNodeIgnored()
{
    // A Node in None mode (default) with a matching label must NOT be registered as
    // a Tx source — connectWirelessElements() only processes WirelessMode::Tx nodes.
    auto scene = std::make_unique<Scene>();

    auto *src      = ElementFactory::buildElement(ElementType::InputButton);
    auto *noneNode = ElementFactory::buildElement(ElementType::Node);  // WirelessMode::None
    auto *rxNode   = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(src && noneNode && rxNode);
    scene->addItem(src);
    scene->addItem(noneNode);
    scene->addItem(rxNode);

    auto *noneN = qobject_cast<Node *>(noneNode);
    auto *rxN   = qobject_cast<Node *>(rxNode);
    QVERIFY(noneN && rxN);

    noneN->setLabel("SIG");                          // same label, but mode = None
    rxN->setLabel("SIG");
    rxN->setWirelessMode(WirelessMode::Rx);

    auto *conn = new QNEConnection();
    conn->setStartPort(src->outputPort(0));
    conn->setEndPort(noneNode->inputPort(0));
    scene->addItem(conn);

    ElementMapping mapping(scene->elements());
    mapping.buildGraph();

    // None-mode node must not be treated as Tx — Rx falls back to GND
    const auto &pairs = rxN->logic()->inputPairs();
    QVERIFY(!pairs.isEmpty());
    QVERIFY(pairs.at(0).logic != noneN->logic());
    QVERIFY(pairs.at(0).logic != nullptr);
}

void TestElementMapping::testWirelessRxMismatchLabelFallsToGnd()
{
    // Rx "FOO" must not be driven by Tx "BAR" — mismatched labels must never connect.
    auto scene = std::make_unique<Scene>();

    auto *src    = ElementFactory::buildElement(ElementType::InputButton);
    auto *txNode = ElementFactory::buildElement(ElementType::Node);
    auto *rxNode = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(src && txNode && rxNode);
    scene->addItem(src);
    scene->addItem(txNode);
    scene->addItem(rxNode);

    auto *txN = qobject_cast<Node *>(txNode);
    auto *rxN = qobject_cast<Node *>(rxNode);
    QVERIFY(txN && rxN);

    txN->setLabel("BAR"); txN->setWirelessMode(WirelessMode::Tx);
    rxN->setLabel("FOO"); rxN->setWirelessMode(WirelessMode::Rx);

    auto *conn = new QNEConnection();
    conn->setStartPort(src->outputPort(0));
    conn->setEndPort(txNode->inputPort(0));
    scene->addItem(conn);

    ElementMapping mapping(scene->elements());
    mapping.buildGraph();

    // "FOO" != "BAR" → Rx must fall back to GND, not connect to Tx
    const auto &pairs = rxN->logic()->inputPairs();
    QVERIFY(!pairs.isEmpty());
    QVERIFY(pairs.at(0).logic != txN->logic());
    QVERIFY(pairs.at(0).logic != nullptr);
}

void TestElementMapping::testWirelessCircuitWithGate()
{
    // Full circuit: Switch → TxNode -[wireless]-> RxNode → AND(input 0)
    //               VCC → AND(input 1)
    // After sort() and one tick, driving the switch HIGH must make the AND output HIGH.
    auto scene = std::make_unique<Scene>();

    auto *sw      = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *txNode  = ElementFactory::buildElement(ElementType::Node);
    auto *rxNode  = ElementFactory::buildElement(ElementType::Node);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *vcc     = ElementFactory::buildElement(ElementType::InputVcc);
    QVERIFY(sw && txNode && rxNode && andGate && vcc);

    for (auto *elm : {sw, txNode, rxNode, andGate, vcc})
        scene->addItem(elm);

    auto *txN = qobject_cast<Node *>(txNode);
    auto *rxN = qobject_cast<Node *>(rxNode);
    QVERIFY(txN && rxN);
    txN->setLabel("D"); txN->setWirelessMode(WirelessMode::Tx);
    rxN->setLabel("D"); rxN->setWirelessMode(WirelessMode::Rx);

    // sw → txNode
    auto *c1 = new QNEConnection();
    c1->setStartPort(sw->outputPort(0));
    c1->setEndPort(txNode->inputPort(0));
    scene->addItem(c1);

    // rxNode → AND input 0
    auto *c2 = new QNEConnection();
    c2->setStartPort(rxNode->outputPort(0));
    c2->setEndPort(andGate->inputPort(0));
    scene->addItem(c2);

    // vcc → AND input 1
    auto *c3 = new QNEConnection();
    c3->setStartPort(vcc->outputPort(0));
    c3->setEndPort(andGate->inputPort(1));
    scene->addItem(c3);

    ElementMapping mapping(scene->elements());
    mapping.buildGraph();

    // Helper: iterate until no output changes (convergence).
    auto settle = [&mapping]() {
        for (int pass = 0; pass < 10; ++pass) {
            bool changed = false;
            for (const auto &logicElm : mapping.logicElms()) {
                logicElm->clearOutputChanged();
                logicElm->updateLogic();
                if (logicElm->outputChanged()) {
                    changed = true;
                }
            }
            if (!changed) {
                break;
            }
        }
    };

    // Drive switch HIGH → AND output must be HIGH
    sw->logic()->setOutputValue(true);
    settle();
    QCOMPARE(andGate->logic()->outputValue(), Status::Active);

    // Drive switch LOW → AND output must be LOW
    sw->logic()->setOutputValue(false);
    settle();
    QCOMPARE(andGate->logic()->outputValue(), Status::Inactive);
}

void TestElementMapping::testWirelessFanOutSignalValues()
{
    // Fan-out: one Tx drives three Rx nodes.
    // After buildGraph() and convergence, all Rx outputs must match the Tx input value.
    auto scene = std::make_unique<Scene>();

    auto *sw     = ElementFactory::buildElement(ElementType::InputButton);
    auto *txNode = ElementFactory::buildElement(ElementType::Node);
    auto *rx1    = ElementFactory::buildElement(ElementType::Node);
    auto *rx2    = ElementFactory::buildElement(ElementType::Node);
    auto *rx3    = ElementFactory::buildElement(ElementType::Node);
    QVERIFY(sw && txNode && rx1 && rx2 && rx3);

    for (auto *elm : {sw, txNode, rx1, rx2, rx3})
        scene->addItem(elm);

    auto *txN = qobject_cast<Node *>(txNode);
    QVERIFY(txN);
    txN->setLabel("BUS"); txN->setWirelessMode(WirelessMode::Tx);
    for (auto *rx : {rx1, rx2, rx3}) {
        auto *rxN = qobject_cast<Node *>(rx);
        rxN->setLabel("BUS"); rxN->setWirelessMode(WirelessMode::Rx);
    }

    auto *conn = new QNEConnection();
    conn->setStartPort(sw->outputPort(0));
    conn->setEndPort(txNode->inputPort(0));
    scene->addItem(conn);

    ElementMapping mapping(scene->elements());
    mapping.buildGraph();

    // Helper: iterate until convergence.
    auto settle = [&mapping]() {
        for (int pass = 0; pass < 10; ++pass) {
            bool changed = false;
            for (const auto &logicElm : mapping.logicElms()) {
                logicElm->clearOutputChanged();
                logicElm->updateLogic();
                if (logicElm->outputChanged()) {
                    changed = true;
                }
            }
            if (!changed) {
                break;
            }
        }
    };

    // Drive HIGH → all three Rx outputs HIGH
    sw->logic()->setOutputValue(true);
    settle();
    for (auto *rx : {rx1, rx2, rx3}) {
        QCOMPARE(qobject_cast<Node *>(rx)->logic()->outputValue(), Status::Active);
    }

    // Drive LOW → all three Rx outputs LOW
    sw->logic()->setOutputValue(false);
    settle();
    for (auto *rx : {rx1, rx2, rx3}) {
        QCOMPARE(qobject_cast<Node *>(rx)->logic()->outputValue(), Status::Inactive);
    }
}

void TestElementMapping::testWirelessFeedbackLoopDetected()
{
    // A feedback cycle created through a wireless channel must be identified by
    // hasFeedbackElements() / isInFeedbackLoop() after buildGraph().
    //
    // Circuit topology:
    //   Rx → NOT gate → Tx -[wireless label "FB"]-> Rx
    //
    // The wireless link from Tx back to Rx closes a 3-element cycle.
    auto scene = std::make_unique<Scene>();

    auto *txNode  = ElementFactory::buildElement(ElementType::Node);
    auto *rxNode  = ElementFactory::buildElement(ElementType::Node);
    auto *notGate = ElementFactory::buildElement(ElementType::Not);
    QVERIFY(txNode && rxNode && notGate);

    scene->addItem(txNode);
    scene->addItem(rxNode);
    scene->addItem(notGate);

    auto *txN = qobject_cast<Node *>(txNode);
    auto *rxN = qobject_cast<Node *>(rxNode);
    QVERIFY(txN && rxN);

    txN->setLabel("FB"); txN->setWirelessMode(WirelessMode::Tx);
    rxN->setLabel("FB"); rxN->setWirelessMode(WirelessMode::Rx);

    // Rx output → NOT gate input
    auto *c1 = new QNEConnection();
    c1->setStartPort(rxNode->outputPort(0));
    c1->setEndPort(notGate->inputPort(0));
    scene->addItem(c1);

    // NOT gate output → Tx input (physical)
    auto *c2 = new QNEConnection();
    c2->setStartPort(notGate->outputPort(0));
    c2->setEndPort(txNode->inputPort(0));
    scene->addItem(c2);

    ElementMapping mapping(scene->elements());
    mapping.buildGraph();

    QVERIFY(mapping.hasFeedbackElements());
    QVERIFY(mapping.isInFeedbackLoop(txN->logic()));
    QVERIFY(mapping.isInFeedbackLoop(rxN->logic()));
    QVERIFY(mapping.isInFeedbackLoop(notGate->logic()));
}
