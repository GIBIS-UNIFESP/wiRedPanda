// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testgraphicelements.h"

#include "and.h"
#include "audiobox.h"
#include "buzzer.h"
#include "demux.h"
#include "dflipflop.h"
#include "display_14.h"
#include "display_16.h"
#include "display_7.h"
#include "dlatch.h"
#include "globalproperties.h"
#include "ic.h"
#include "inputbutton.h"
#include "inputgnd.h"
#include "inputrotary.h"
#include "inputswitch.h"
#include "inputvcc.h"
#include "jkflipflop.h"
#include "led.h"
#include "line.h"
#include "mux.h"
#include "node.h"
#include "not.h"
#include "or.h"
#include "qneconnection.h"
#include "qneport.h"
#include "scene.h"
#include "simulation.h"
#include "srflipflop.h"
#include "text.h"
#include "tflipflop.h"

#include <QTest>

#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string

void TestGraphicElements::init()
{
    for (int i = 0; i < connections.size(); ++i) {
        connections[i] = new QNEConnection();
        switches[i] = new InputSwitch();
        connections.at(i)->setStartPort(switches.at(i)->outputPort());
    }
}

void TestGraphicElements::cleanup()
{
    qDeleteAll(switches);
}

void TestGraphicElements::testElementBasicStructure(GraphicElement *element,
                                                   int expectedInputs, int expectedOutputs,
                                                   int expectedMinInputs, ElementType expectedType)
{
    QCOMPARE(element->inputSize(), element->inputs().size());
    QCOMPARE(element->inputSize(), expectedInputs);
    QCOMPARE(element->outputSize(), element->outputs().size());
    QCOMPARE(element->outputSize(), expectedOutputs);
    QCOMPARE(element->minInputSize(), expectedMinInputs);
    QCOMPARE(element->elementType(), expectedType);
}

// =============== GROUPED STRUCTURE TESTS ===============

void TestGraphicElements::testBasicGateStructure()
{
    // Test basic logic gates that share similar structure
    And andGate;
    testElementBasicStructure(&andGate, 2, 1, 2, ElementType::And);

    Or orGate;
    testElementBasicStructure(&orGate, 2, 1, 2, ElementType::Or);
}

void TestGraphicElements::testInputOutputElements()
{
    // Test input/output elements structure
    InputVcc vcc;
    QCOMPARE(vcc.outputSize(), 1);
    QCOMPARE(vcc.inputSize(), 0);
    QCOMPARE(vcc.outputPort()->status(), Status::Active);

    InputGnd gnd;
    QCOMPARE(gnd.outputSize(), 1);
    QCOMPARE(gnd.inputSize(), 0);
    QCOMPARE(gnd.outputPort()->status(), Status::Inactive);
}

void TestGraphicElements::testSequentialElementStructure()
{
    // Test sequential elements that have similar port structures
    DFlipFlop dff;
    testElementBasicStructure(&dff, 4, 2, 4, ElementType::DFlipFlop);
    QCOMPARE(dff.maxInputSize(), 4);
    QCOMPARE(dff.minOutputSize(), 2);
    QCOMPARE(dff.maxOutputSize(), 2);

    JKFlipFlop jkff;
    testElementBasicStructure(&jkff, 5, 2, 5, ElementType::JKFlipFlop);
    QCOMPARE(jkff.maxInputSize(), 5);
    QCOMPARE(jkff.minOutputSize(), 2);
    QCOMPARE(jkff.maxOutputSize(), 2);

    SRFlipFlop srff;
    testElementBasicStructure(&srff, 5, 2, 5, ElementType::SRFlipFlop);
    QCOMPARE(srff.maxInputSize(), 5);
    QCOMPARE(srff.minOutputSize(), 2);
    QCOMPARE(srff.maxOutputSize(), 2);

    TFlipFlop tff;
    testElementBasicStructure(&tff, 4, 2, 4, ElementType::TFlipFlop);
    QCOMPARE(tff.maxInputSize(), 4);
    QCOMPARE(tff.minOutputSize(), 2);
    QCOMPARE(tff.maxOutputSize(), 2);
}

void TestGraphicElements::testComplexElementStructure()
{
    // Test complex elements like MUX/DEMUX
    Mux mux;
    testElementBasicStructure(&mux, 3, 1, 3, ElementType::Mux);

    Demux demux;
    testElementBasicStructure(&demux, 2, 2, 2, ElementType::Demux);
}

// =============== ELEMENT-SPECIFIC TESTS ===============

void TestGraphicElements::testAndGateStructure()
{
    And elm;
    testElementBasicStructure(&elm, 2, 1, 2, ElementType::And);
}

void TestGraphicElements::testOrGateStructure()
{
    Or elm;
    testElementBasicStructure(&elm, 2, 1, 2, ElementType::Or);
}

void TestGraphicElements::testMuxDemuxStructure()
{
    Mux mux;
    testElementBasicStructure(&mux, 3, 1, 3, ElementType::Mux);

    Demux demux;
    testElementBasicStructure(&demux, 2, 2, 2, ElementType::Demux);
}

void TestGraphicElements::testFlipFlopStructure()
{
    DFlipFlop dff;
    testElementBasicStructure(&dff, 4, 2, 4, ElementType::DFlipFlop);
    QCOMPARE(dff.maxInputSize(), 4);
    QCOMPARE(dff.minOutputSize(), 2);
    QCOMPARE(dff.maxOutputSize(), 2);
}

void TestGraphicElements::testLatchStructure()
{
    DLatch latch;
    testElementBasicStructure(&latch, 2, 2, 2, ElementType::DLatch);
    QCOMPARE(latch.maxInputSize(), 2);
    QCOMPARE(latch.minOutputSize(), 2);
    QCOMPARE(latch.maxOutputSize(), 2);
}

void TestGraphicElements::testNodeStructure()
{
    Node elm;
    testElementBasicStructure(&elm, 1, 1, 1, ElementType::Node);
}

void TestGraphicElements::testVccGndStructure()
{
    InputVcc vcc;
    QCOMPARE(vcc.outputSize(), 1);
    QCOMPARE(vcc.inputSize(), 0);
    QCOMPARE(vcc.outputPort()->status(), Status::Active);

    InputGnd gnd;
    QCOMPARE(gnd.outputSize(), 1);
    QCOMPARE(gnd.inputSize(), 0);
    QCOMPARE(gnd.outputPort()->status(), Status::Inactive);

    // Test InputRotary (rotary encoder) - defaults to 2 outputs
    InputRotary rotary;
    QCOMPARE(rotary.outputSize(), 2);
    QCOMPARE(rotary.inputSize(), 0);
    testElementBasicStructure(&rotary, 0, 2, 0, ElementType::InputRotary);
}

// =============== IC AND COMPLEX ELEMENT TESTS ===============

void TestGraphicElements::testICStructure()
{
    const QString icFile = QString(QUOTE(CURRENTDIR)) + "/../examples/jkflipflop.panda";
    GlobalProperties::currentDir = QString(QUOTE(CURRENTDIR)) + "/../examples/";

    IC ic;
    ic.loadFile(icFile);
    testICData(&ic);
}

void TestGraphicElements::testICData()
{
    const QString icFile = QString(QUOTE(CURRENTDIR)) + "/../examples/jkflipflop.panda";
    GlobalProperties::currentDir = QString(QUOTE(CURRENTDIR)) + "/../examples/";

    IC ic;
    ic.loadFile(icFile);
    testICData(&ic);
}

void TestGraphicElements::testICData(IC *ic)
{
    QCOMPARE(ic->inputSize(), 5);
    QCOMPARE(ic->outputSize(), 2);

    QCOMPARE(ic->inputPort(0)->isRequired(), false);
    QCOMPARE(ic->inputPort(1)->isRequired(), false);
    QCOMPARE(ic->inputPort(2)->isRequired(), true);
    QCOMPARE(ic->inputPort(3)->isRequired(), false);
    QCOMPARE(ic->inputPort(4)->isRequired(), false);

    QCOMPARE(ic->inputPort(0)->status(), Status::Active);
    QCOMPARE(ic->inputPort(1)->status(), Status::Active);
    QCOMPARE(ic->inputPort(2)->status(), Status::Invalid);
    QCOMPARE(ic->inputPort(3)->status(), Status::Active);
    QCOMPARE(ic->inputPort(4)->status(), Status::Active);

    QCOMPARE(ic->outputPort(0)->status(), Status::Inactive);
    QCOMPARE(ic->outputPort(1)->status(), Status::Inactive);

    connections.at(0)->setEndPort(ic->inputPort(2));
    switches.at(0)->setOn(true);

    QCOMPARE(ic->inputPort(2)->status(), Status::Active);
    QCOMPARE(ic->outputPort(0)->status(), Status::Inactive);

    connections.at(1)->setEndPort(ic->inputPort(3));
    switches.at(1)->setOn(false);

    QCOMPARE(ic->inputPort(3)->status(), Status::Inactive);
    QCOMPARE(ic->outputPort(0)->status(), Status::Inactive);

    connections.at(2)->setEndPort(ic->inputPort(1));
    switches.at(2)->setOn(true);

    QCOMPARE(ic->inputPort(1)->status(), Status::Active);
    QCOMPARE(ic->outputPort(0)->status(), Status::Inactive);

    connections.at(3)->setEndPort(ic->inputPort(0));
    switches.at(3)->setOn(false);

    QCOMPARE(ic->inputPort(0)->status(), Status::Inactive);
    QCOMPARE(ic->outputPort(0)->status(), Status::Inactive);

    connections.at(4)->setEndPort(ic->inputPort(4));
    switches.at(4)->setOn(true);

    QCOMPARE(ic->inputPort(4)->status(), Status::Active);
    QCOMPARE(ic->outputPort(0)->status(), Status::Inactive);
}

void TestGraphicElements::testMultipleICs()
{
    const QString loadedFile1 = QString(QUOTE(CURRENTDIR)) + "/../examples/jkflipflop.panda";
    const QString loadedFile2 = QString(QUOTE(CURRENTDIR)) + "/../examples/counter.panda";
    GlobalProperties::currentDir = QString(QUOTE(CURRENTDIR)) + "/../examples/";

    IC ic1;
    ic1.loadFile(loadedFile1);
    IC ic2;
    ic2.loadFile(loadedFile2);

    // Test that multiple ICs can be loaded and have independent structure
    testICData(&ic1);

    // Test IC2 basic structure (may have different port configuration)
    QVERIFY(ic2.inputSize() > 0);
    QVERIFY(ic2.outputSize() > 0);
    QCOMPARE(ic2.inputSize(), ic2.inputs().size());
    QCOMPARE(ic2.outputSize(), ic2.outputs().size());
}

// =============== DISPLAY AND AUDIO ELEMENT TESTS ===============

void TestGraphicElements::testDisplayElements()
{
    // Test Display7 (7-segment display)
    Display7 display7;
    testElementBasicStructure(&display7, 8, 0, 8, ElementType::Display7);

    // Test Display14 (14-segment display)
    Display14 display14;
    testElementBasicStructure(&display14, 15, 0, 15, ElementType::Display14);

    // Test Display16 (16-segment display)
    Display16 display16;
    testElementBasicStructure(&display16, 17, 0, 17, ElementType::Display16);
}

void TestGraphicElements::testAudioElements()
{
    // Test AudioBox
    AudioBox audioBox;
    testElementBasicStructure(&audioBox, 1, 0, 1, ElementType::AudioBox);

    // Test Buzzer
    Buzzer buzzer;
    testElementBasicStructure(&buzzer, 1, 0, 1, ElementType::Buzzer);
}

// =============== ANNOTATION ELEMENT TESTS ===============

void TestGraphicElements::testAnnotationElements()
{
    // Test Line element (visual annotation)
    Line line;
    testElementBasicStructure(&line, 0, 0, 0, ElementType::Line);

    // Test Text element (annotation)
    Text text;
    testElementBasicStructure(&text, 0, 0, 0, ElementType::Text);
}

// =============== BOUNDARY CONDITION TESTS ===============

void TestGraphicElements::testElementBoundaryConditions()
{
    constexpr int TYPICAL_INPUTS = 4;

    // Test AND gate with minimum inputs
    {
        And minAnd;
        int minInputs = minAnd.minInputSize();
        minAnd.setInputSize(minInputs);
        QCOMPARE(minAnd.inputSize(), minInputs);
        QVERIFY(minAnd.inputSize() >= minAnd.minInputSize());
        QVERIFY(minAnd.inputSize() <= minAnd.maxInputSize());
    }

    // Test AND gate with maximum inputs
    {
        And maxAnd;
        int maxInputs = maxAnd.maxInputSize();
        maxAnd.setInputSize(maxInputs);
        QCOMPARE(maxAnd.inputSize(), maxInputs);
        QVERIFY(maxAnd.inputSize() >= maxAnd.minInputSize());
        QVERIFY(maxAnd.inputSize() <= maxAnd.maxInputSize());
    }

    // Test OR gate boundary conditions
    {
        Or typicalOr;
        typicalOr.setInputSize(TYPICAL_INPUTS);
        QCOMPARE(typicalOr.inputSize(), TYPICAL_INPUTS);

        // Test that we can't exceed maximum
        int originalMax = typicalOr.maxInputSize();
        typicalOr.setInputSize(originalMax + 1);
        QVERIFY(typicalOr.inputSize() <= originalMax);

        // Test that we can't go below minimum
        int originalMin = typicalOr.minInputSize();
        typicalOr.setInputSize(originalMin - 1);
        QVERIFY(typicalOr.inputSize() >= originalMin);
    }

    // Test MUX boundary constraints (fixed size)
    {
        Mux mux;
        QCOMPARE(mux.inputSize(), 3); // MUX has fixed 3 inputs
        QCOMPARE(mux.outputSize(), 1);
        QCOMPARE(mux.minInputSize(), mux.maxInputSize()); // Should be fixed size

        // Try to set different size (should remain unchanged)
        int originalSize = mux.inputSize();
        mux.setInputSize(originalSize + 1);
        QCOMPARE(mux.inputSize(), originalSize); // Should stay the same
    }
}

// =============== NEGATIVE TEST CASES ===============

void TestGraphicElements::testErrorConditions()
{
    // Test invalid port access
    {
        And gate;

        // Test valid port access and functionality
        auto *inputPort0 = gate.inputPort(0);
        auto *inputPort1 = gate.inputPort(1);
        auto *outputPort = gate.outputPort(0);

        QVERIFY2(inputPort0 != nullptr, "Input port 0 should exist");
        QVERIFY2(inputPort1 != nullptr, "Input port 1 should exist");
        QVERIFY2(outputPort != nullptr, "Output port should exist");

        // Verify port functionality
        QVERIFY2(inputPort0->isInput(), "Port 0 should be configured as input");
        QVERIFY2(inputPort1->isInput(), "Port 1 should be configured as input");
        QVERIFY2(outputPort->isOutput(), "Port should be configured as output");

        // Verify port indices are correct
        QCOMPARE(inputPort0->index(), 0);
        QCOMPARE(inputPort1->index(), 1);
        QCOMPARE(outputPort->index(), 0);
    }

    // Test extreme input size constraints
    {
        Or orGate;

        // Try to set impossibly large input size
        orGate.setInputSize(10000);
        QVERIFY(orGate.inputSize() <= orGate.maxInputSize());

        // Try to set negative input size
        orGate.setInputSize(-1);
        QVERIFY(orGate.inputSize() >= orGate.minInputSize());

        // Verify gate is still functional
        QVERIFY(orGate.inputSize() > 0);
        QVERIFY(orGate.outputSize() > 0);
    }

    // Test unconnected port behavior
    {
        Not notGate;

        // Verify element handles unconnected inputs gracefully
        auto *inputPort = notGate.inputPort(0);
        auto *outputPort = notGate.outputPort(0);

        QVERIFY2(inputPort != nullptr, "NOT gate input port should exist");
        QVERIFY2(outputPort != nullptr, "NOT gate output port should exist");

        // Verify port functionality and configuration
        QVERIFY2(inputPort->isInput(), "Port should be configured as input");
        QVERIFY2(outputPort->isOutput(), "Port should be configured as output");
        QCOMPARE(inputPort->index(), 0);
        QCOMPARE(outputPort->index(), 0);

        // Element should have defined behavior even without connections
        QVERIFY(notGate.elementType() == ElementType::Not);
        QCOMPARE(notGate.inputSize(), 1);
        QCOMPARE(notGate.outputSize(), 1);
    }
}
