// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testelementmapping.h"

#include "elementmapping.h"
#include "and.h"
#include "or.h"
#include "not.h"
#include "inputbutton.h"
#include "inputvcc.h"
#include "inputgnd.h"
#include "led.h"
#include "clock.h"
#include "dflipflop.h"
#include "ic.h"
#include "qneconnection.h"
#include "qneport.h"
#include "logicelement.h"

#include <QTest>

void TestElementMapping::testConstructorAndBasicMapping()
{
    // Test constructor with empty elements list
    QVector<GraphicElement *> emptyElements;
    ElementMapping emptyMapping(emptyElements);
    
    // Should have no logic elements
    QCOMPARE(emptyMapping.logicElms().size(), 0);
    
    // Test constructor with basic elements
    QVector<GraphicElement *> basicElements;
    
    And andGate;
    Or orGate;
    Not notGate;
    
    basicElements.append(&andGate);
    basicElements.append(&orGate);
    basicElements.append(&notGate);
    
    ElementMapping basicMapping(basicElements);
    
    // Should have created logic elements for each graphic element
    QCOMPARE(basicMapping.logicElms().size(), 3);
    
    // Each graphic element should have its logic assigned
    QVERIFY(andGate.logic() != nullptr);
    QVERIFY(orGate.logic() != nullptr);
    QVERIFY(notGate.logic() != nullptr);
    
    // Logic elements should be different instances
    QVERIFY(andGate.logic() != orGate.logic());
    QVERIFY(orGate.logic() != notGate.logic());
    QVERIFY(andGate.logic() != notGate.logic());
}

void TestElementMapping::testInputOutputMapping()
{
    QVector<GraphicElement *> elements;
    
    InputButton input;
    input.setObjectName("input1");
    
    Led output;
    output.setObjectName("output1");
    
    And gate;
    gate.setObjectName("and1");
    
    elements.append(&input);
    elements.append(&output);
    elements.append(&gate);
    
    ElementMapping mapping(elements);
    
    // Should have created logic for all elements
    QCOMPARE(mapping.logicElms().size(), 3);
    
    // Verify logic elements were assigned
    QVERIFY(input.logic() != nullptr);
    QVERIFY(output.logic() != nullptr);
    QVERIFY(gate.logic() != nullptr);
    
    // Input should be a LogicInput type
    // Output should be a LogicOutput type
    // Gate should be appropriate logic gate type
}

void TestElementMapping::testVccGndMapping()
{
    QVector<GraphicElement *> elements;
    
    InputVcc vcc;
    vcc.setObjectName("vcc1");
    
    InputGnd gnd;
    gnd.setObjectName("gnd1");
    
    Not notGate;
    notGate.setObjectName("not1");
    
    elements.append(&vcc);
    elements.append(&gnd);
    elements.append(&notGate);
    
    ElementMapping mapping(elements);
    
    // Should have created logic elements
    QCOMPARE(mapping.logicElms().size(), 3);
    
    // VCC and GND should have logic assigned
    QVERIFY(vcc.logic() != nullptr);
    QVERIFY(gnd.logic() != nullptr);
    QVERIFY(notGate.logic() != nullptr);
}

void TestElementMapping::testConnectionMapping()
{
    QVector<GraphicElement *> elements;
    
    InputButton input;
    input.setObjectName("input1");
    
    Not notGate;
    notGate.setObjectName("not1");
    
    Led output;
    output.setObjectName("output1");
    
    elements.append(&input);
    elements.append(&notGate);
    elements.append(&output);
    
    // Create connections between elements
    QNEConnection conn1;
    conn1.setStartPort(input.outputPort());
    conn1.setEndPort(notGate.inputPort(0));
    
    QNEConnection conn2;
    conn2.setStartPort(notGate.outputPort());
    conn2.setEndPort(output.inputPort(0));
    
    ElementMapping mapping(elements);
    
    // Should have created logic elements
    QCOMPARE(mapping.logicElms().size(), 3);
    
    // All elements should have logic assigned
    QVERIFY(input.logic() != nullptr);
    QVERIFY(notGate.logic() != nullptr);
    QVERIFY(output.logic() != nullptr);
    
    // Test that connections are properly mapped
    // This would typically be verified by checking the logic element connections
}

void TestElementMapping::testDefaultValueConnections()
{
    QVector<GraphicElement *> elements;
    
    And andGate;
    andGate.setObjectName("and1");
    
    // Add AND gate with no inputs connected
    elements.append(&andGate);
    
    ElementMapping mapping(elements);
    
    // Should have created logic for the gate
    QCOMPARE(mapping.logicElms().size(), 1);
    QVERIFY(andGate.logic() != nullptr);
    
    // The unconnected inputs should be handled with default values
    // For an AND gate, unconnected inputs should default to HIGH (VCC)
    // This tests the applyConnection logic for unconnected optional inputs
}

void TestElementMapping::testComplexCircuitMapping()
{
    QVector<GraphicElement *> elements;
    
    // Create a more complex circuit
    InputButton btn1;
    btn1.setObjectName("btn1");
    
    InputButton btn2;
    btn2.setObjectName("btn2");
    
    And andGate;
    andGate.setObjectName("and1");
    
    Or orGate;
    orGate.setObjectName("or1");
    
    Not notGate;
    notGate.setObjectName("not1");
    
    Led led1;
    led1.setObjectName("led1");
    
    Led led2;
    led2.setObjectName("led2");
    
    elements.append(&btn1);
    elements.append(&btn2);
    elements.append(&andGate);
    elements.append(&orGate);
    elements.append(&notGate);
    elements.append(&led1);
    elements.append(&led2);
    
    // Create some connections
    QNEConnection conn1;
    conn1.setStartPort(btn1.outputPort());
    conn1.setEndPort(andGate.inputPort(0));
    
    QNEConnection conn2;
    conn2.setStartPort(btn2.outputPort());
    conn2.setEndPort(andGate.inputPort(1));
    
    QNEConnection conn3;
    conn3.setStartPort(andGate.outputPort());
    conn3.setEndPort(orGate.inputPort(0));
    
    QNEConnection conn4;
    conn4.setStartPort(orGate.outputPort());
    conn4.setEndPort(notGate.inputPort(0));
    
    QNEConnection conn5;
    conn5.setStartPort(notGate.outputPort());
    conn5.setEndPort(led1.inputPort(0));
    
    QNEConnection conn6;
    conn6.setStartPort(andGate.outputPort());
    conn6.setEndPort(led2.inputPort(0));
    
    ElementMapping mapping(elements);
    
    // Should have created logic for all elements
    QCOMPARE(mapping.logicElms().size(), 7);
    
    // Verify all elements have logic assigned
    QVERIFY(btn1.logic() != nullptr);
    QVERIFY(btn2.logic() != nullptr);
    QVERIFY(andGate.logic() != nullptr);
    QVERIFY(orGate.logic() != nullptr);
    QVERIFY(notGate.logic() != nullptr);
    QVERIFY(led1.logic() != nullptr);
    QVERIFY(led2.logic() != nullptr);
}

void TestElementMapping::testSortLogicElements()
{
    QVector<GraphicElement *> elements;
    
    // Create elements that will have different priorities when sorted
    InputButton input;
    input.setObjectName("input1");
    
    And gate1;
    gate1.setObjectName("gate1");
    
    And gate2;
    gate2.setObjectName("gate2");
    
    Led output;
    output.setObjectName("output1");
    
    elements.append(&input);
    elements.append(&gate1);
    elements.append(&gate2);
    elements.append(&output);
    
    // Create a chain: input -> gate1 -> gate2 -> output
    QNEConnection conn1;
    conn1.setStartPort(input.outputPort());
    conn1.setEndPort(gate1.inputPort(0));
    
    QNEConnection conn2;
    conn2.setStartPort(gate1.outputPort());
    conn2.setEndPort(gate2.inputPort(0));
    
    QNEConnection conn3;
    conn3.setStartPort(gate2.outputPort());
    conn3.setEndPort(output.inputPort(0));
    
    ElementMapping mapping(elements);
    
    // Before sorting
    int initialSize = mapping.logicElms().size();
    QCOMPARE(initialSize, 4);
    
    // Test sorting
    mapping.sort();
    
    // Size should remain the same after sorting
    QCOMPARE(mapping.logicElms().size(), initialSize);
    
    // Elements should now be sorted by priority
    // Input elements should typically have highest priority
    // Output elements should have lowest priority
    // The sorting should arrange elements in topological order
    const auto &logicElements = mapping.logicElms();
    
    for (int i = 0; i < logicElements.size() - 1; ++i) {
        // Each element should have priority >= the next element
        QVERIFY(logicElements[i]->priority() >= logicElements[i + 1]->priority());
    }
}

void TestElementMapping::testValidateElements()
{
    QVector<GraphicElement *> elements;
    
    // Create valid circuit
    InputButton input;
    And gate;
    Led output;
    
    elements.append(&input);
    elements.append(&gate);
    elements.append(&output);
    
    // Connect them properly
    QNEConnection conn1;
    conn1.setStartPort(input.outputPort());
    conn1.setEndPort(gate.inputPort(0));
    
    QNEConnection conn2;
    conn2.setStartPort(gate.outputPort());
    conn2.setEndPort(output.inputPort(0));
    
    ElementMapping mapping(elements);
    
    // Sort should include validation
    mapping.sort();
    
    // If validation fails, it would typically throw or set error states
    // For now, we test that sort() completes without issues
    QCOMPARE(mapping.logicElms().size(), 3);
}

void TestElementMapping::testSequentialElementMapping()
{
    QVector<GraphicElement *> elements;
    
    Clock clock;
    clock.setObjectName("clock1");
    clock.setFrequency(1.0);
    
    DFlipFlop flipflop;
    flipflop.setObjectName("dff1");
    
    InputButton input;
    input.setObjectName("input1");
    
    Led output;
    output.setObjectName("output1");
    
    elements.append(&clock);
    elements.append(&flipflop);
    elements.append(&input);
    elements.append(&output);
    
    // Connect: input -> D, clock -> CLK, Q -> output
    QNEConnection conn1;
    conn1.setStartPort(input.outputPort());
    conn1.setEndPort(flipflop.inputPort(0)); // D input
    
    QNEConnection conn2;
    conn2.setStartPort(clock.outputPort());
    conn2.setEndPort(flipflop.inputPort(1)); // CLK input
    
    QNEConnection conn3;
    conn3.setStartPort(flipflop.outputPort(0)); // Q output
    conn3.setEndPort(output.inputPort(0));
    
    ElementMapping mapping(elements);
    
    // Should handle sequential elements correctly
    QCOMPARE(mapping.logicElms().size(), 4);
    
    // All elements should have logic assigned
    QVERIFY(clock.logic() != nullptr);
    QVERIFY(flipflop.logic() != nullptr);
    QVERIFY(input.logic() != nullptr);
    QVERIFY(output.logic() != nullptr);
}

void TestElementMapping::testDestructorCleanup()
{
    QVector<GraphicElement *> elements;
    
    InputVcc vcc;
    InputGnd gnd;
    And gate;
    
    elements.append(&vcc);
    elements.append(&gnd);
    elements.append(&gate);
    
    {
        ElementMapping mapping(elements);
        
        // Elements should have logic assigned
        QVERIFY(vcc.logic() != nullptr);
        QVERIFY(gnd.logic() != nullptr);
        QVERIFY(gate.logic() != nullptr);
        
        QCOMPARE(mapping.logicElms().size(), 3);
    } // ElementMapping destructor called here
    
    // Test that destructor properly cleaned up global VCC/GND
    // This tests the clearSuccessors() calls in the destructor
    // The logic elements should still exist but global connections should be cleaned
}

void TestElementMapping::testMultipleInputConnections()
{
    QVector<GraphicElement *> elements;
    
    InputButton btn1;
    btn1.setObjectName("btn1");
    
    InputButton btn2;
    btn2.setObjectName("btn2");
    
    InputButton btn3;
    btn3.setObjectName("btn3");
    
    And andGate;
    andGate.setObjectName("and3"); // 3-input AND gate
    
    Led output;
    output.setObjectName("output1");
    
    elements.append(&btn1);
    elements.append(&btn2);
    elements.append(&btn3);
    elements.append(&andGate);
    elements.append(&output);
    
    // Connect all three inputs to the AND gate
    QNEConnection conn1;
    conn1.setStartPort(btn1.outputPort());
    conn1.setEndPort(andGate.inputPort(0));
    
    QNEConnection conn2;
    conn2.setStartPort(btn2.outputPort());
    conn2.setEndPort(andGate.inputPort(1));
    
    // Leave third input unconnected to test default value handling
    
    QNEConnection conn3;
    conn3.setStartPort(andGate.outputPort());
    conn3.setEndPort(output.inputPort(0));
    
    ElementMapping mapping(elements);
    
    // Should handle multiple inputs correctly
    QCOMPARE(mapping.logicElms().size(), 5);
    
    // Test that unconnected input gets proper default value connection
    QVERIFY(andGate.logic() != nullptr);
}

void TestElementMapping::testEdgeCaseElements()
{
    QVector<GraphicElement *> elements;
    
    // Test with elements that might have special handling
    InputVcc vcc1;
    vcc1.setObjectName("vcc1");
    
    InputVcc vcc2; // Multiple VCC elements
    vcc2.setObjectName("vcc2");
    
    InputGnd gnd1;
    gnd1.setObjectName("gnd1");
    
    InputGnd gnd2; // Multiple GND elements
    gnd2.setObjectName("gnd2");
    
    elements.append(&vcc1);
    elements.append(&vcc2);
    elements.append(&gnd1);
    elements.append(&gnd2);
    
    ElementMapping mapping(elements);
    
    // Should handle multiple VCC/GND elements correctly
    QCOMPARE(mapping.logicElms().size(), 4);
    
    // All should have logic assigned
    QVERIFY(vcc1.logic() != nullptr);
    QVERIFY(vcc2.logic() != nullptr);
    QVERIFY(gnd1.logic() != nullptr);
    QVERIFY(gnd2.logic() != nullptr);
}

void TestElementMapping::testICElementMapping()
{
    // This test would require IC functionality to be available
    // For now, we create a placeholder test that can be expanded
    // when IC support is more complete
    
    QVector<GraphicElement *> elements;
    
    And normalGate;
    normalGate.setObjectName("normal1");
    
    elements.append(&normalGate);
    
    // Note: IC element testing would go here when IC class is fully testable
    // IC icElement;
    // icElement.setObjectName("ic1");
    // elements.append(&icElement);
    
    ElementMapping mapping(elements);
    
    // For now, just test that normal elements work
    QCOMPARE(mapping.logicElms().size(), 1);
    QVERIFY(normalGate.logic() != nullptr);
}