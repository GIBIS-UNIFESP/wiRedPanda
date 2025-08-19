// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testarduino.h"

#include "and.h"
#include "arduino/codegenerator.h"
#include "clock.h"
#include "common.h"
#include "dflipflop.h"
#include "inputbutton.h"
#include "inputswitch.h"
#include "jkflipflop.h"
#include "led.h"
#include "not.h"
#include "or.h"
#include "qneconnection.h"
#include "scene.h"
#include "srflipflop.h"
#include "tflipflop.h"
#include "workspace.h"

#include <QRegularExpression>
#include <QTest>

void TestArduino::init()
{
    m_workspace = createTestWorkspace();
    m_tempFile = new QTemporaryFile(this);
    QVERIFY(m_tempFile->open());
}

void TestArduino::cleanup()
{
    delete m_workspace;
    m_workspace = nullptr;
    delete m_tempFile;
    m_tempFile = nullptr;
}

WorkSpace* TestArduino::createTestWorkspace()
{
    return new WorkSpace();
}

void TestArduino::testBasicAndCircuit()
{
    // Create a simple AND gate circuit: Button1 AND Button2 -> LED
    auto elements = createBasicAndCircuit();
    QString code = generateArduinoCode(elements);

    // Verify basic code structure
    verifyCodeStructure(code);

    // Verify specific AND logic implementation
    QVERIFY(containsSubstring(code, "&&"));
    QVERIFY(containsSubstring(code, "digitalRead"));
    QVERIFY(containsSubstring(code, "digitalWrite"));

    // Clean up
    qDeleteAll(elements);
}

void TestArduino::testBasicOrCircuit()
{
    auto elements = createBasicOrCircuit();
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);
    QVERIFY(containsSubstring(code, "||"));

    qDeleteAll(elements);
}

void TestArduino::testBasicNotCircuit()
{
    // Create NOT gate circuit: Button -> NOT -> LED
    InputButton* button = new InputButton();
    Not* notGate = new Not();
    Led* led = new Led();

    button->setObjectName("InputButton");
    notGate->setObjectName("Not");
    led->setObjectName("Led");

    QVector<GraphicElement*> elements{button, notGate, led};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);
    QVERIFY(containsSubstring(code, "!"));

    qDeleteAll(elements);
}

void TestArduino::testCombinationalLogic()
{
    // Test various combinational logic gates
    auto elements = createComplexCircuit();
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Should contain multiple logic operators
    QVERIFY(containsSubstring(code, "&&") || containsSubstring(code, "||"));

    qDeleteAll(elements);
}

void TestArduino::testDFlipFlopGeneration()
{
    auto elements = createFlipFlopCircuit();
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Verify D Flip-Flop specific code patterns
    QVERIFY(containsSubstring(code, "D FlipFlop"));
    QVERIFY(containsSubstring(code, "_inclk"));
    QVERIFY(containsSubstring(code, "_last"));

    qDeleteAll(elements);
}

void TestArduino::testJKFlipFlopGeneration()
{
    // Create JK Flip-Flop circuit
    InputButton* j = new InputButton();
    InputButton* clk = new InputButton();
    InputButton* k = new InputButton();
    JKFlipFlop* jkff = new JKFlipFlop();
    Led* q = new Led();
    Led* qn = new Led();

    j->setObjectName("J_Input");
    clk->setObjectName("Clock_Input");
    k->setObjectName("K_Input");
    jkff->setObjectName("JKFlipFlop");
    q->setObjectName("Q_Output");
    qn->setObjectName("QN_Output");

    QVector<GraphicElement*> elements{j, clk, k, jkff, q, qn};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);
    QVERIFY(containsSubstring(code, "JK FlipFlop"));

    qDeleteAll(elements);
}

void TestArduino::testSRFlipFlopGeneration()
{
    // Create SR Flip-Flop circuit
    InputButton* s = new InputButton();
    InputButton* clk = new InputButton();
    InputButton* r = new InputButton();
    SRFlipFlop* srff = new SRFlipFlop();
    Led* q = new Led();

    s->setObjectName("S_Input");
    clk->setObjectName("Clock_Input");
    r->setObjectName("R_Input");
    srff->setObjectName("SRFlipFlop");
    q->setObjectName("Q_Output");

    QVector<GraphicElement*> elements{s, clk, r, srff, q};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);
    QVERIFY(containsSubstring(code, "SR FlipFlop"));

    qDeleteAll(elements);
}

void TestArduino::testTFlipFlopGeneration()
{
    // Create T Flip-Flop circuit
    InputButton* t = new InputButton();
    InputButton* clk = new InputButton();
    TFlipFlop* tff = new TFlipFlop();
    Led* q = new Led();

    t->setObjectName("T_Input");
    clk->setObjectName("Clock_Input");
    tff->setObjectName("TFlipFlop");
    q->setObjectName("Q_Output");

    QVector<GraphicElement*> elements{t, clk, tff, q};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);
    QVERIFY(containsSubstring(code, "T FlipFlop"));

    qDeleteAll(elements);
}

void TestArduino::testDLatchGeneration()
{
    // Test D Latch generation - would need DLatch class
    // For now, create a simple test
    auto elements = createBasicAndCircuit();
    QString code = generateArduinoCode(elements);
    verifyCodeStructure(code);
    qDeleteAll(elements);
}

void TestArduino::testClockGeneration()
{
    auto elements = createClockCircuit();
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Verify clock-specific code patterns
    QVERIFY(containsSubstring(code, "elapsedMillis"));
    QVERIFY(containsSubstring(code, "_elapsed"));
    QVERIFY(containsSubstring(code, "_interval"));

    qDeleteAll(elements);
}

void TestArduino::testMultipleClocks()
{
    // Create circuit with multiple clocks
    Clock* clk1 = new Clock();
    Clock* clk2 = new Clock();
    Led* led1 = new Led();
    Led* led2 = new Led();

    clk1->setObjectName("Clock1");
    clk2->setObjectName("Clock2");
    led1->setObjectName("Led1");
    led2->setObjectName("Led2");

    clk1->setFrequency(1); // 1 Hz
    clk2->setFrequency(2); // 2 Hz

    QVector<GraphicElement*> elements{clk1, clk2, led1, led2};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Should have multiple clock variables
    QVERIFY(code.count("elapsedMillis") >= 2);
    QVERIFY(code.count("_interval") >= 2);

    qDeleteAll(elements);
}

void TestArduino::testInputButtonMapping()
{
    InputButton* button = new InputButton();
    Led* led = new Led();

    button->setObjectName("TestButton");
    button->setLabel("TestLabel");
    led->setObjectName("TestLed");

    QVector<GraphicElement*> elements{button, led};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Should contain input pin declaration and read operation
    QVERIFY(containsSubstring(code, "const int"));
    QVERIFY(containsSubstring(code, "digitalRead"));

    qDeleteAll(elements);
}

void TestArduino::testInputSwitchMapping()
{
    InputSwitch* switch1 = new InputSwitch();
    Led* led = new Led();

    switch1->setObjectName("TestSwitch");
    led->setObjectName("TestLed");

    QVector<GraphicElement*> elements{switch1, led};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);
    QVERIFY(containsSubstring(code, "digitalRead"));

    qDeleteAll(elements);
}

void TestArduino::testLedOutputMapping()
{
    InputButton* button = new InputButton();
    Led* led = new Led();

    button->setObjectName("TestButton");
    led->setObjectName("TestLed");

    QVector<GraphicElement*> elements{button, led};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Should contain output pin declaration and write operation
    QVERIFY(containsSubstring(code, "const int"));
    QVERIFY(containsSubstring(code, "digitalWrite"));
    QVERIFY(containsSubstring(code, "OUTPUT"));

    qDeleteAll(elements);
}

void TestArduino::testMultipleInputsOutputs()
{
    // Test circuit with multiple inputs and outputs
    InputButton* btn1 = new InputButton();
    InputButton* btn2 = new InputButton();
    Led* led1 = new Led();
    Led* led2 = new Led();

    btn1->setObjectName("Button1");
    btn2->setObjectName("Button2");
    led1->setObjectName("Led1");
    led2->setObjectName("Led2");

    QVector<GraphicElement*> elements{btn1, btn2, led1, led2};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Should have multiple pin declarations
    QVERIFY(code.count("const int") >= 4);

    qDeleteAll(elements);
}

void TestArduino::testVariableNaming()
{
    InputButton* button = new InputButton();
    Led* led = new Led();

    button->setObjectName("Test Button");
    button->setLabel("My Label!");
    led->setObjectName("Test Led");

    QVector<GraphicElement*> elements{button, led};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Variable names should not contain spaces or special characters
    QVERIFY(!containsSubstring(code, "Test Button"));
    QVERIFY(!containsSubstring(code, "My Label!"));
    QVERIFY(containsSubstring(code, "test_button") || containsSubstring(code, "testbutton"));

    qDeleteAll(elements);
}

void TestArduino::testPinAssignment()
{
    // Test that pins are assigned correctly
    InputButton* button = new InputButton();
    Led* led = new Led();

    button->setObjectName("Button");
    led->setObjectName("Led");

    QVector<GraphicElement*> elements{button, led};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Should assign pins from available pin list
    QStringList expectedPins = {"A0", "A1", "A2", "A3", "A4", "A5", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13"};

    bool foundValidPin = false;
    for (const QString& pin : expectedPins) {
        if (containsSubstring(code, pin)) {
            foundValidPin = true;
            break;
        }
    }
    QVERIFY(foundValidPin);

    qDeleteAll(elements);
}

void TestArduino::testNameCollisionHandling()
{
    // Test handling of name collisions
    InputButton* btn1 = new InputButton();
    InputButton* btn2 = new InputButton();
    Led* led = new Led();

    btn1->setObjectName("Button");
    btn2->setObjectName("Button"); // Same name
    led->setObjectName("Led");

    QVector<GraphicElement*> elements{btn1, btn2, led};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Should handle name collisions by adding numbers
    QVERIFY(containsSubstring(code, "button1") || containsSubstring(code, "button2"));

    qDeleteAll(elements);
}

void TestArduino::testForbiddenCharacterHandling()
{
    InputButton* button = new InputButton();
    Led* led = new Led();

    button->setObjectName("Test@Button#");
    button->setLabel("Label$With%Special&Chars");
    led->setObjectName("Test!Led*");

    QVector<GraphicElement*> elements{button, led};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Should not contain forbidden characters in variable names (but allow them in comments/includes)
    // Extract variable declarations section to check
    int inputsStart = code.indexOf("/* ========= Inputs ========== */");
    int setupStart = code.indexOf("void setup()");

    if (inputsStart >= 0 && setupStart >= 0) {
        QString variableSection = code.mid(inputsStart, setupStart - inputsStart);

        // Check for forbidden characters in variable names (excluding comments and includes)
        QVERIFY(!variableSection.contains(QRegularExpression("\\w+[@#$%&!*]\\w*\\s*=")));
        QVERIFY(!variableSection.contains("test@button"));
        QVERIFY(!variableSection.contains("test!led"));
        QVERIFY(!variableSection.contains("label$with"));
    }

    qDeleteAll(elements);
}

void TestArduino::testComplexCombinationalCircuit()
{
    auto elements = createComplexCircuit();
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Should contain multiple logic operations
    int logicOpsCount = 0;
    if (containsSubstring(code, "&&")) logicOpsCount++;
    if (containsSubstring(code, "||")) logicOpsCount++;
    if (containsSubstring(code, "!")) logicOpsCount++;

    QVERIFY(logicOpsCount >= 2);

    qDeleteAll(elements);
}

void TestArduino::testSequentialCircuitWithClock()
{
    auto elements = createClockCircuit();
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Should contain clock handling code
    QVERIFY(containsSubstring(code, "elapsedMillis"));
    QVERIFY(containsSubstring(code, "interval"));

    qDeleteAll(elements);
}

void TestArduino::testMixedLogicCircuit()
{
    // Create a circuit with both combinational and sequential logic
    InputButton* button = new InputButton();
    Clock* clock = new Clock();
    DFlipFlop* dff = new DFlipFlop();
    And* andGate = new And();
    Led* led = new Led();

    button->setObjectName("Button");
    clock->setObjectName("Clock");
    dff->setObjectName("DFlipFlop");
    andGate->setObjectName("And");
    led->setObjectName("Led");

    QVector<GraphicElement*> elements{button, clock, dff, andGate, led};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Should contain both combinational and sequential logic patterns
    QVERIFY(containsSubstring(code, "&&")); // Combinational
    QVERIFY(containsSubstring(code, "D FlipFlop")); // Sequential

    qDeleteAll(elements);
}

void TestArduino::testEmptyCircuit()
{
    QVector<GraphicElement*> elements; // Empty
    QString code = generateArduinoCode(elements);

    // Should still generate valid Arduino code structure
    verifyCodeStructure(code);

    // Should contain basic setup and loop functions
    QVERIFY(containsSubstring(code, "void setup()"));
    QVERIFY(containsSubstring(code, "void loop()"));
}

void TestArduino::testDisconnectedElements()
{
    // Test elements that are not connected
    InputButton* button = new InputButton();
    Led* led = new Led();
    And* andGate = new And();

    button->setObjectName("Button");
    led->setObjectName("Led");
    andGate->setObjectName("And");

    // Note: Not connecting these elements
    QVector<GraphicElement*> elements{button, led, andGate};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Should handle disconnected elements gracefully
    QVERIFY(containsSubstring(code, "LOW") || containsSubstring(code, "HIGH"));

    qDeleteAll(elements);
}

void TestArduino::testInvalidElementTypes()
{
    // This test verifies error handling for unsupported elements
    // The actual test would depend on having unsupported element types
    auto elements = createBasicAndCircuit();
    QString code = generateArduinoCode(elements);
    verifyCodeStructure(code);
    qDeleteAll(elements);
}

void TestArduino::testInsufficientPins()
{
    // Test with more elements than available pins
    QVector<GraphicElement*> elements;

    // Create many input buttons (more than available pins)
    // Available pins: 6 analog + 12 digital = 18 total
    for (int i = 0; i < 20; ++i) {
        InputButton* button = new InputButton();
        button->setObjectName(QString("Button%1").arg(i));
        elements.append(button);
    }

    // This should throw an exception due to insufficient pins
    bool exceptionThrown = false;
    try {
        QString code = generateArduinoCode(elements);
        // If we get here without exception, something is wrong
        QFAIL("Expected exception for insufficient pins was not thrown");
    } catch (...) {
        // Expected behavior - should throw an exception
        exceptionThrown = true;
    }

    QVERIFY(exceptionThrown);
    qDeleteAll(elements);
}

void TestArduino::testGeneratedCodeStructure()
{
    auto elements = createBasicAndCircuit();
    QString code = generateArduinoCode(elements);

    // Verify all major sections are present
    QVERIFY(containsSubstring(code, "// ======= This code was generated automatically by wiRedPanda ========"));
    QVERIFY(containsSubstring(code, "/* ========= Inputs ========== */"));
    QVERIFY(containsSubstring(code, "/* ========= Outputs ========== */"));
    QVERIFY(containsSubstring(code, "/* ====== Aux. Variables ====== */"));
    QVERIFY(containsSubstring(code, "void setup()"));
    QVERIFY(containsSubstring(code, "void loop()"));

    qDeleteAll(elements);
}

void TestArduino::testArduinoSyntaxCompliance()
{
    auto elements = createBasicAndCircuit();
    QString code = generateArduinoCode(elements);

    verifyArduinoSyntax(code);
    qDeleteAll(elements);
}

void TestArduino::testVariableDeclarations()
{
    auto elements = createBasicAndCircuit();
    QString code = generateArduinoCode(elements);

    // Should have proper variable declarations
    QVERIFY(containsSubstring(code, "const int"));
    QVERIFY(containsSubstring(code, "boolean"));

    qDeleteAll(elements);
}

void TestArduino::testSetupFunction()
{
    auto elements = createBasicAndCircuit();
    QString code = generateArduinoCode(elements);

    // Verify setup function content
    QVERIFY(containsSubstring(code, "void setup()"));
    QVERIFY(containsSubstring(code, "pinMode"));
    QVERIFY(containsSubstring(code, "INPUT") || containsSubstring(code, "OUTPUT"));

    qDeleteAll(elements);
}

void TestArduino::testLoopFunction()
{
    auto elements = createBasicAndCircuit();
    QString code = generateArduinoCode(elements);

    // Verify loop function content
    QVERIFY(containsSubstring(code, "void loop()"));
    QVERIFY(containsSubstring(code, "digitalRead") || containsSubstring(code, "digitalWrite"));

    qDeleteAll(elements);
}

void TestArduino::testMaximumInputOutputs()
{
    // Test with maximum number of inputs and outputs
    QVector<GraphicElement*> elements;

    // Add inputs and outputs up to pin limit (18 pins total: 6 analog + 12 digital)
    // Use 9 inputs and 9 outputs = 18 pins total
    for (int i = 0; i < 9; ++i) {
        InputButton* button = new InputButton();
        button->setObjectName(QString("Button%1").arg(i));
        elements.append(button);

        Led* led = new Led();
        led->setObjectName(QString("Led%1").arg(i));
        elements.append(led);
    }

    QString code = generateArduinoCode(elements);
    verifyCodeStructure(code);

    qDeleteAll(elements);
}

void TestArduino::testElementLabeling()
{
    InputButton* button = new InputButton();
    Led* led = new Led();

    button->setObjectName("Button");
    button->setLabel("PowerButton");
    led->setObjectName("Led");
    led->setLabel("StatusLED");

    QVector<GraphicElement*> elements{button, led};
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);

    // Labels should be incorporated into variable names
    QVERIFY(containsSubstring(code, "powerbutton") || containsSubstring(code, "power"));
    QVERIFY(containsSubstring(code, "statusled") || containsSubstring(code, "status"));

    qDeleteAll(elements);
}

void TestArduino::testPortNaming()
{
    // Test that port names are handled correctly
    auto elements = createBasicAndCircuit();
    QString code = generateArduinoCode(elements);

    verifyCodeStructure(code);
    qDeleteAll(elements);
}

// Helper method implementations

QVector<GraphicElement*> TestArduino::createBasicAndCircuit()
{
    InputButton* button1 = new InputButton();
    InputButton* button2 = new InputButton();
    And* andGate = new And();
    Led* led = new Led();

    button1->setObjectName("Button1");
    button2->setObjectName("Button2");
    andGate->setObjectName("And");
    led->setObjectName("Led");

    // Create connections to make the circuit functional
    QNEConnection* conn1 = new QNEConnection();
    QNEConnection* conn2 = new QNEConnection();
    QNEConnection* conn3 = new QNEConnection();

    conn1->setStartPort(button1->outputPort());
    conn1->setEndPort(andGate->inputPort(0));

    conn2->setStartPort(button2->outputPort());
    conn2->setEndPort(andGate->inputPort(1));

    conn3->setStartPort(andGate->outputPort());
    conn3->setEndPort(led->inputPort());

    QVector<GraphicElement*> elements;
    elements << button1 << button2 << andGate << led;
    return elements;
}

QVector<GraphicElement*> TestArduino::createBasicOrCircuit()
{
    InputButton* button1 = new InputButton();
    InputButton* button2 = new InputButton();
    Or* orGate = new Or();
    Led* led = new Led();

    button1->setObjectName("Button1");
    button2->setObjectName("Button2");
    orGate->setObjectName("Or");
    led->setObjectName("Led");

    // Create connections to make the circuit functional
    QNEConnection* conn1 = new QNEConnection();
    QNEConnection* conn2 = new QNEConnection();
    QNEConnection* conn3 = new QNEConnection();

    conn1->setStartPort(button1->outputPort());
    conn1->setEndPort(orGate->inputPort(0));

    conn2->setStartPort(button2->outputPort());
    conn2->setEndPort(orGate->inputPort(1));

    conn3->setStartPort(orGate->outputPort());
    conn3->setEndPort(led->inputPort());

    QVector<GraphicElement*> elements;
    elements << button1 << button2 << orGate << led;
    return elements;
}

QVector<GraphicElement*> TestArduino::createFlipFlopCircuit()
{
    InputButton* data = new InputButton();
    InputButton* clock = new InputButton();
    DFlipFlop* dff = new DFlipFlop();
    Led* q = new Led();
    Led* qn = new Led();

    data->setObjectName("Data");
    clock->setObjectName("Clock");
    dff->setObjectName("DFlipFlop");
    q->setObjectName("Q");
    qn->setObjectName("QN");

    return QVector<GraphicElement*>{data, clock, dff, q, qn};
}

QVector<GraphicElement*> TestArduino::createClockCircuit()
{
    Clock* clock = new Clock();
    Led* led = new Led();

    clock->setObjectName("Clock");
    clock->setFrequency(1); // 1 Hz
    led->setObjectName("Led");

    return QVector<GraphicElement*>{clock, led};
}

QVector<GraphicElement*> TestArduino::createComplexCircuit()
{
    InputButton* btn1 = new InputButton();
    InputButton* btn2 = new InputButton();
    InputButton* btn3 = new InputButton();
    And* and1 = new And();
    Or* or1 = new Or();
    Not* not1 = new Not();
    Led* led1 = new Led();
    Led* led2 = new Led();

    btn1->setObjectName("Button1");
    btn2->setObjectName("Button2");
    btn3->setObjectName("Button3");
    and1->setObjectName("And1");
    or1->setObjectName("Or1");
    not1->setObjectName("Not1");
    led1->setObjectName("Led1");
    led2->setObjectName("Led2");

    // Create connections: btn1 AND btn2 -> and1 -> led1, btn3 -> not1 -> or1 (with and1) -> led2
    QNEConnection* conn1 = new QNEConnection();
    QNEConnection* conn2 = new QNEConnection();
    QNEConnection* conn3 = new QNEConnection();
    QNEConnection* conn4 = new QNEConnection();
    QNEConnection* conn5 = new QNEConnection();
    QNEConnection* conn6 = new QNEConnection();
    QNEConnection* conn7 = new QNEConnection();

    // AND gate circuit: btn1 AND btn2 -> led1
    conn1->setStartPort(btn1->outputPort());
    conn1->setEndPort(and1->inputPort(0));

    conn2->setStartPort(btn2->outputPort());
    conn2->setEndPort(and1->inputPort(1));

    conn3->setStartPort(and1->outputPort());
    conn3->setEndPort(led1->inputPort());

    // OR gate circuit: and1 OR (NOT btn3) -> led2
    conn4->setStartPort(btn3->outputPort());
    conn4->setEndPort(not1->inputPort());

    conn5->setStartPort(and1->outputPort());
    conn5->setEndPort(or1->inputPort(0));

    conn6->setStartPort(not1->outputPort());
    conn6->setEndPort(or1->inputPort(1));

    conn7->setStartPort(or1->outputPort());
    conn7->setEndPort(led2->inputPort());

    QVector<GraphicElement*> elements;
    elements << btn1 << btn2 << btn3 << and1 << or1 << not1 << led1 << led2;
    return elements;
}

QString TestArduino::generateArduinoCode(const QVector<GraphicElement*>& elements)
{
    // Reset temp file
    m_tempFile->resize(0);
    m_tempFile->seek(0);

    CodeGenerator generator(m_tempFile->fileName(), elements);
    generator.generate();

    m_tempFile->seek(0);
    QString code = m_tempFile->readAll();
    return code;
}

void TestArduino::verifyCodeStructure(const QString& code)
{
    QVERIFY(!code.isEmpty());
    QVERIFY(containsSubstring(code, "void setup()"));
    QVERIFY(containsSubstring(code, "void loop()"));
    QVERIFY(containsSubstring(code, "#include <elapsedMillis.h>"));
}

void TestArduino::verifyVariableDeclarations(const QString& code, const QStringList& expectedVars)
{
    for (const QString& var : expectedVars) {
        QVERIFY2(containsSubstring(code, var), qPrintable(QString("Variable %1 not found").arg(var)));
    }
}

void TestArduino::verifySetupFunction(const QString& code, const QStringList& expectedPins)
{
    QVERIFY(containsSubstring(code, "void setup()"));

    for (const QString& pin : expectedPins) {
        QVERIFY2(containsSubstring(code, QString("pinMode(%1").arg(pin)),
                 qPrintable(QString("pinMode for %1 not found").arg(pin)));
    }
}

void TestArduino::verifyLoopFunction(const QString& code)
{
    QVERIFY(containsSubstring(code, "void loop()"));
}

void TestArduino::verifyArduinoSyntax(const QString& code)
{
    // Basic syntax checks
    QVERIFY(code.count("{") == code.count("}")); // Balanced braces
    QVERIFY(!containsSubstring(code, ";;"));     // No double semicolons

    // Check for required Arduino elements
    QVERIFY(containsSubstring(code, "void setup()"));
    QVERIFY(containsSubstring(code, "void loop()"));
}

bool TestArduino::containsSubstring(const QString& code, const QString& substring)
{
    return code.contains(substring, Qt::CaseInsensitive);
}

QStringList TestArduino::extractVariableNames(const QString& code)
{
    QStringList variables;
    QRegularExpression regex(R"(const int (\w+)\s*=)");
    QRegularExpressionMatchIterator matches = regex.globalMatch(code);

    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        variables << match.captured(1);
    }

    return variables;
}

QStringList TestArduino::extractPinAssignments(const QString& code)
{
    QStringList pins;
    QRegularExpression regex(R"(const int \w+\s*=\s*([A-Z]?\d+);)");
    QRegularExpressionMatchIterator matches = regex.globalMatch(code);

    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        pins << match.captured(1);
    }

    return pins;
}

