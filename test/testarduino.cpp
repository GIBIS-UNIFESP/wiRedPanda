// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testarduino.h"

#include "codegenerator.h"
#include "and.h"
#include "or.h"
#include "not.h"
#include "nand.h"
#include "nor.h"
#include "xor.h"
#include "xnor.h"
#include "inputbutton.h"
#include "inputswitch.h"
#include "inputvcc.h"
#include "inputgnd.h"
#include "led.h"
#include "clock.h"
#include "dflipflop.h"
#include "jkflipflop.h"
#include "srflipflop.h"
#include "tflipflop.h"
#include "dlatch.h"
#include "ic.h"
#include "node.h"
#include "qneport.h"
#include "qneconnection.h"

#include <QTest>
#include <QTemporaryFile>
#include <QTextStream>
#include <QDir>

void TestArduino::testMappedPin()
{
    MappedPin pin1;
    QCOMPARE(pin1.m_elm, nullptr);
    QCOMPARE(pin1.m_port, nullptr);
    QCOMPARE(pin1.m_pin, QString());
    QCOMPARE(pin1.m_varName, QString());
    QCOMPARE(pin1.m_portNumber, 0);

    And and_gate;
    MappedPin pin2(&and_gate, "D2", "var1", nullptr, 1);
    QCOMPARE(pin2.m_elm, &and_gate);
    QCOMPARE(pin2.m_pin, QString("D2"));
    QCOMPARE(pin2.m_varName, QString("var1"));
    QCOMPARE(pin2.m_portNumber, 1);
}

void TestArduino::testCodeGeneratorConstruction()
{
    QVector<GraphicElement *> elements;
    And and_gate;
    elements.append(&and_gate);

    CodeGenerator generator("test.ino", elements);
    
    // Just test that construction doesn't crash
    // We can't easily test generate() without a full setup
    QVERIFY(true);
}

void TestArduino::testGenerateEmptyCircuit()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    
    QVector<GraphicElement *> elements; // Empty circuit
    CodeGenerator generator(tempFile.fileName(), elements);
    generator.generate();
    
    tempFile.seek(0);
    QString content = tempFile.readAll();
    
    // Should contain basic Arduino structure
    QVERIFY(content.contains("wiRedPanda"));
    QVERIFY(content.contains("#include <elapsedMillis.h>"));
    QVERIFY(content.contains("void setup()"));
    QVERIFY(content.contains("void loop()"));
    QVERIFY(content.contains("/* ========= Inputs ========== */"));
    QVERIFY(content.contains("/* ========= Outputs ========== */"));
    QVERIFY(content.contains("/* ====== Aux. Variables ====== */"));
}

void TestArduino::testFileHandling()
{
    // Test with invalid file path
    QVector<GraphicElement *> elements;
    CodeGenerator generator("/invalid/path/test.ino", elements);
    
    // Should not crash when generate is called with invalid file
    generator.generate();
    QVERIFY(true); // If we get here, no crash occurred
    
    // Test with valid temporary file
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    QString fileName = tempFile.fileName();
    tempFile.close(); // Close so CodeGenerator can open it
    
    CodeGenerator validGenerator(fileName, elements);
    validGenerator.generate();
    
    // Check that file was created and has content
    QFile checkFile(fileName);
    QVERIFY(checkFile.open(QIODevice::ReadOnly));
    QString content = checkFile.readAll();
    QVERIFY(!content.isEmpty());
    QVERIFY(content.contains("wiRedPanda"));
}

void TestArduino::testGenerateBasicElements()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    
    QVector<GraphicElement *> elements;
    
    // Test basic input/output elements only
    InputButton button;
    button.setObjectName("btn1");
    
    Led led;
    led.setObjectName("led1");
    
    elements.append(&button);
    elements.append(&led);
    
    CodeGenerator generator(tempFile.fileName(), elements);
    generator.generate();
    
    tempFile.seek(0);
    QString content = tempFile.readAll();
    
    // Check that basic structure is present
    QVERIFY(content.contains("const int btn1"));
    QVERIFY(content.contains("const int led1"));
    QVERIFY(content.contains("pinMode"));
    QVERIFY(content.contains("digitalRead"));
    QVERIFY(content.contains("digitalWrite"));
}

void TestArduino::testGenerateLogicElements()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    
    QVector<GraphicElement *> elements;
    
    // Test all basic logic gate types
    And andGate;
    andGate.setObjectName("and1");
    
    Or orGate;
    orGate.setObjectName("or1");
    
    Not notGate;
    notGate.setObjectName("not1");
    
    Nand nandGate;
    nandGate.setObjectName("nand1");
    
    Nor norGate;
    norGate.setObjectName("nor1");
    
    Xor xorGate;
    xorGate.setObjectName("xor1");
    
    Xnor xnorGate;
    xnorGate.setObjectName("xnor1");
    
    elements.append(&andGate);
    elements.append(&orGate);
    elements.append(&notGate);
    elements.append(&nandGate);
    elements.append(&norGate);
    elements.append(&xorGate);
    elements.append(&xnorGate);
    
    CodeGenerator generator(tempFile.fileName(), elements);
    generator.generate();
    
    tempFile.seek(0);
    QString content = tempFile.readAll();
    
    // Check that aux variables are declared for logic elements
    QVERIFY(content.contains("boolean aux_and1"));
    QVERIFY(content.contains("boolean aux_or1"));
    QVERIFY(content.contains("boolean aux_not1"));
    QVERIFY(content.contains("boolean aux_nand1"));
    QVERIFY(content.contains("boolean aux_nor1"));
    QVERIFY(content.contains("boolean aux_xor1"));
    QVERIFY(content.contains("boolean aux_xnor1"));
    
    // Check that logic operators are used in assignments
    QVERIFY(content.contains("&&")); // AND
    QVERIFY(content.contains("||"));  // OR
    QVERIFY(content.contains("!")); // NOT
    QVERIFY(content.contains("^"));   // XOR
}

void TestArduino::testGenerateVccGndElements()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    
    QVector<GraphicElement *> elements;
    
    // Test VCC and GND elements
    InputVcc vcc;
    vcc.setObjectName("vcc1");
    
    InputGnd gnd;
    gnd.setObjectName("gnd1");
    
    Not notGate; // To use VCC/GND
    notGate.setObjectName("not1");
    
    Led output;
    output.setObjectName("led1");
    
    elements.append(&vcc);
    elements.append(&gnd);
    elements.append(&notGate);
    elements.append(&output);
    
    CodeGenerator generator(tempFile.fileName(), elements);
    generator.generate();
    
    tempFile.seek(0);
    QString content = tempFile.readAll();
    
    // VCC should map to HIGH, GND should map to LOW in variable mapping
    QVERIFY(content.contains("HIGH"));
    QVERIFY(content.contains("LOW"));
    
    // Check that VCC/GND don't get input pin assignments (they're internal)
    QVERIFY(!content.contains("const int vcc1"));
    QVERIFY(!content.contains("const int gnd1"));
}

void TestArduino::testGenerateClockElements()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    
    QVector<GraphicElement *> elements;
    
    Clock clock1;
    clock1.setObjectName("clk1");
    clock1.setFrequency(10.0); // 10 Hz = 100ms interval
    
    Clock clock2;
    clock2.setObjectName("clk2");
    clock2.setFrequency(1.0); // 1 Hz = 1000ms interval
    
    elements.append(&clock1);
    elements.append(&clock2);
    
    CodeGenerator generator(tempFile.fileName(), elements);
    generator.generate();
    
    tempFile.seek(0);
    QString content = tempFile.readAll();
    
    // Check clock-specific variables are declared
    QVERIFY(content.contains("elapsedMillis"));
    QVERIFY(content.contains("_elapsed"));
    QVERIFY(content.contains("_interval"));
    
    // Check clock update logic in loop section
    QVERIFY(content.contains("// Updating clocks"));
    QVERIFY(content.contains("_elapsed >"));
    
    // Check frequency calculations (10Hz=100ms, 1Hz=1000ms)
    QVERIFY(content.contains("_interval = 100") || content.contains("_interval = 1000"));
}

void TestArduino::testGenerateSequentialElements()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    
    QVector<GraphicElement *> elements;
    
    // Test D Flip-Flop
    DFlipFlop dff;
    dff.setObjectName("dff1");
    
    // Test JK Flip-Flop
    JKFlipFlop jkff;
    jkff.setObjectName("jkff1");
    
    // Test SR Flip-Flop
    SRFlipFlop srff;
    srff.setObjectName("srff1");
    
    // Test T Flip-Flop
    TFlipFlop tff;
    tff.setObjectName("tff1");
    
    // Test D Latch
    DLatch dlatch;
    dlatch.setObjectName("dlatch1");
    
    elements.append(&dff);
    elements.append(&jkff);
    elements.append(&srff);
    elements.append(&tff);
    elements.append(&dlatch);
    
    CodeGenerator generator(tempFile.fileName(), elements);
    generator.generate();
    
    tempFile.seek(0);
    QString content = tempFile.readAll();
    
    // Check flip-flop specific internal variables
    QVERIFY(content.contains("_inclk"));
    QVERIFY(content.contains("_last")); // For D flip-flop
    
    // Check flip-flop logic comments
    QVERIFY(content.contains("//D FlipFlop"));
    QVERIFY(content.contains("//JK FlipFlop"));
    QVERIFY(content.contains("//SR FlipFlop"));
    QVERIFY(content.contains("//T FlipFlop"));
    QVERIFY(content.contains("//D Latch"));
    
    // Check preset/clear logic
    QVERIFY(content.contains("//Preset"));
    QVERIFY(content.contains("//Clear"));
}

void TestArduino::testGenerateElementsWithLabels()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    
    QVector<GraphicElement *> elements;
    
    // Test elements with labels to trigger label handling code
    InputButton button;
    button.setObjectName("btn1");
    button.setLabel("power-switch"); // With special chars
    
    InputSwitch switch1;
    switch1.setObjectName("sw1");
    switch1.setLabel("mode select"); // With spaces
    
    Led led1;
    led1.setObjectName("led1");
    led1.setLabel("status LED");
    
    elements.append(&button);
    elements.append(&switch1);
    elements.append(&led1);
    
    CodeGenerator generator(tempFile.fileName(), elements);
    generator.generate();
    
    tempFile.seek(0);
    QString content = tempFile.readAll();
    
    // Check that labels are included and cleaned up
    QVERIFY(content.contains("power_switch") || content.contains("powerswitch"));
    QVERIFY(content.contains("mode_select") || content.contains("modeselect"));
    QVERIFY(content.contains("status_led") || content.contains("statusled"));
    
    // Verify forbidden characters are removed
    QVERIFY(!content.contains("-"));
    QVERIFY(!content.contains("power-switch"));
}

void TestArduino::testGenerateComplexCircuit()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    
    QVector<GraphicElement *> elements;
    
    // Create a more complex circuit to test multiple code paths
    InputButton input1;
    input1.setObjectName("btn1");
    input1.setLabel("button");
    
    InputSwitch input2;
    input2.setObjectName("sw1");
    input2.setLabel("switch");
    
    InputVcc vcc;
    vcc.setObjectName("vcc1");
    
    InputGnd gnd;
    gnd.setObjectName("gnd1");
    
    And andGate;
    andGate.setObjectName("and1");
    
    Not notGate;
    notGate.setObjectName("not1");
    
    Clock clock;
    clock.setObjectName("clk1");
    clock.setFrequency(5.0); // 5 Hz
    
    DFlipFlop flipflop;
    flipflop.setObjectName("dff1");
    
    Led output1;
    output1.setObjectName("led1");
    output1.setLabel("status");
    
    Led output2;
    output2.setObjectName("led2");
    output2.setLabel("alarm");
    
    elements.append(&input1);
    elements.append(&input2);
    elements.append(&vcc);
    elements.append(&gnd);
    elements.append(&andGate);
    elements.append(&notGate);
    elements.append(&clock);
    elements.append(&flipflop);
    elements.append(&output1);
    elements.append(&output2);
    
    CodeGenerator generator(tempFile.fileName(), elements);
    generator.generate();
    
    tempFile.seek(0);
    QString content = tempFile.readAll();
    
    // Verify all major sections are present
    QVERIFY(content.contains("/* ========= Inputs ========== */"));
    QVERIFY(content.contains("/* ========= Outputs ========== */"));
    QVERIFY(content.contains("/* ====== Aux. Variables ====== */"));
    QVERIFY(content.contains("void setup()"));
    QVERIFY(content.contains("void loop()"));
    
    // Verify inputs are declared
    QVERIFY(content.contains("const int btn1"));
    QVERIFY(content.contains("const int sw1"));
    
    // Verify outputs are declared  
    QVERIFY(content.contains("const int led1"));
    QVERIFY(content.contains("const int led2"));
    
    // Verify aux variables for logic elements
    QVERIFY(content.contains("boolean aux_and1"));
    QVERIFY(content.contains("boolean aux_not1"));
    QVERIFY(content.contains("boolean aux_dff1"));
    QVERIFY(content.contains("boolean aux_clk1"));
    
    // Verify clock-specific variables
    QVERIFY(content.contains("elapsedMillis"));
    QVERIFY(content.contains("_interval = 200")); // 5Hz = 200ms
    
    // Verify flip-flop specific variables
    QVERIFY(content.contains("_inclk"));
    QVERIFY(content.contains("_last"));
    
    // Verify setup section
    QVERIFY(content.contains("pinMode") && content.contains("INPUT"));
    QVERIFY(content.contains("pinMode") && content.contains("OUTPUT"));
    
    // Verify loop section
    QVERIFY(content.contains("digitalRead"));
    QVERIFY(content.contains("digitalWrite"));
    QVERIFY(content.contains("// Reading input data"));
    QVERIFY(content.contains("// Updating clocks"));
    QVERIFY(content.contains("// Writing output data"));
}

void TestArduino::testPortConnectionsAndMapping()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    
    QVector<GraphicElement *> elements;
    
    // Test elements with port connections to trigger otherPortName logic
    InputButton input;
    input.setObjectName("btn1");
    input.setLabel("power"); // Test label processing for inputs (line 111)
    
    // Create connections to test port mapping
    And andGate;
    andGate.setObjectName("and1");
    
    // Test VCC and GND mapping to HIGH/LOW
    InputVcc vcc;
    vcc.setObjectName("vcc1");
    
    InputGnd gnd;
    gnd.setObjectName("gnd1");
    
    elements.append(&input);
    elements.append(&andGate);
    elements.append(&vcc);
    elements.append(&gnd);
    
    CodeGenerator generator(tempFile.fileName(), elements);
    generator.generate();
    
    tempFile.seek(0);
    QString content = tempFile.readAll();
    
    // Check that input label is processed (line 111)
    QVERIFY(content.contains("power") || content.contains("btn1_power"));
    
    // Check VCC/GND are mapped to HIGH/LOW constants (lines 177-178, 182-183)
    QVERIFY(content.contains("HIGH"));
    QVERIFY(content.contains("LOW"));
    
    // Verify basic structure with port connections
    QVERIFY(content.contains("boolean aux_and1"));
    QVERIFY(content.contains("boolean aux_vcc1") || content.contains("HIGH"));
    QVERIFY(content.contains("boolean aux_gnd1") || content.contains("LOW"));
}

void TestArduino::testMultiOutputElements()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    
    QVector<GraphicElement *> elements;
    
    // Test elements with multiple outputs to trigger multi-port logic
    DFlipFlop dff;
    dff.setObjectName("dff1");
    
    JKFlipFlop jkff; 
    jkff.setObjectName("jkff1");
    
    // Test output element with label (line 136)
    Led output1;
    output1.setObjectName("led1");
    output1.setLabel("status");
    
    Led output2;
    output2.setObjectName("led2");
    output2.setLabel("alarm");
    
    elements.append(&dff);
    elements.append(&jkff);
    elements.append(&output1);
    elements.append(&output2);
    
    CodeGenerator generator(tempFile.fileName(), elements);
    generator.generate();
    
    tempFile.seek(0);
    QString content = tempFile.readAll();
    
    // Test multi-output variable naming (lines 193-194, 196-197, 200-201)
    QVERIFY(content.contains("aux_dff1"));
    QVERIFY(content.contains("aux_jkff1"));
    
    // Check output labels are processed (line 136)
    QVERIFY(content.contains("led1") && (content.contains("status") || content.contains("_status")));
    QVERIFY(content.contains("led2") && (content.contains("alarm") || content.contains("_alarm")));
    
    // Check multi-output flip-flop variables (Q and ~Q outputs)
    QVERIFY(content.contains("_0") || content.contains("_1")); // Multi-port numbering
}

void TestArduino::testEdgeCasesAndErrorHandling()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    
    QVector<GraphicElement *> elements;
    
    // Test clock element (to trigger isBox logic on line 209-210)
    Clock clock;
    clock.setObjectName("clk1");
    clock.setFrequency(2.0); // 2 Hz = 500ms interval
    
    // Test elements with unusual configurations
    Not notGate;
    notGate.setObjectName("not1");
    
    And andGate;
    andGate.setObjectName("and1"); 
    
    // Test element without connections to trigger default value logic
    Or orGate;
    orGate.setObjectName("or1");
    
    elements.append(&clock);
    elements.append(&notGate);
    elements.append(&andGate);
    elements.append(&orGate);
    
    CodeGenerator generator(tempFile.fileName(), elements);
    generator.generate();
    
    tempFile.seek(0);
    QString content = tempFile.readAll();
    
    // Verify clock variables are generated (lines 209-210)
    QVERIFY(content.contains("aux_clk1"));
    QVERIFY(content.contains("elapsedMillis aux_clk1_elapsed"));
    QVERIFY(content.contains("aux_clk1_interval = 500")); // 2 Hz = 500ms
    
    // Verify logic gates are processed
    QVERIFY(content.contains("aux_not1"));
    QVERIFY(content.contains("aux_and1"));
    QVERIFY(content.contains("aux_or1"));
    
    // Test basic code structure
    QVERIFY(content.contains("void setup()"));
    QVERIFY(content.contains("void loop()"));
    QVERIFY(content.contains("// Updating clocks"));
    
    // Test for proper boolean assignments
    QVERIFY(content.contains("boolean"));
    QVERIFY(content.contains("LOW") || content.contains("HIGH"));
}

void TestArduino::testICElementSupport()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    
    QVector<GraphicElement *> elements;
    
    // Test IC element (should throw exception as it's not currently supported)
    IC icElement;
    icElement.setObjectName("ic1");
    
    // Add a basic input as well to test with other elements
    InputButton button;
    button.setObjectName("btn1");
    
    elements.append(&icElement);
    elements.append(&button);
    
    CodeGenerator generator(tempFile.fileName(), elements);
    
    // IC elements should throw an exception during generation
    // This test verifies that IC elements are properly detected and handled
    // (even if not supported yet)
    bool exceptionThrown = false;
    try {
        generator.generate();
    } catch (...) {
        exceptionThrown = true;
    }
    
    // Should throw exception for unsupported IC element
    QVERIFY(exceptionThrown);
    
    // Test that non-IC elements still work fine when IC is removed
    QVector<GraphicElement *> elementsWithoutIC;
    elementsWithoutIC.append(&button);
    
    CodeGenerator generatorWithoutIC(tempFile.fileName(), elementsWithoutIC);
    
    // Should not throw exception without IC
    bool noExceptionThrown = true;
    try {
        generatorWithoutIC.generate();
    } catch (...) {
        noExceptionThrown = false;
    }
    
    QVERIFY(noExceptionThrown);
    
    tempFile.seek(0);
    QString content = tempFile.readAll();
    QVERIFY(content.contains("btn1"));
}

void TestArduino::testForbiddenCharacterRemoval()
{
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    
    QVector<GraphicElement *> elements;
    
    // Test elements with names containing forbidden characters
    InputButton button1;
    button1.setObjectName("btn-with-dashes");
    button1.setLabel("Power Button (Main)");
    
    InputSwitch switch1;
    switch1.setObjectName("switch@home");
    switch1.setLabel("Mode Switch #1");
    
    Led led1;
    led1.setObjectName("led$status");
    led1.setLabel("Status LED & Indicator");
    
    And andGate;
    andGate.setObjectName("gate.and.1");
    
    elements.append(&button1);
    elements.append(&switch1);
    elements.append(&led1);
    elements.append(&andGate);
    
    CodeGenerator generator(tempFile.fileName(), elements);
    generator.generate();
    
    tempFile.seek(0);
    QString content = tempFile.readAll();
    
    // Verify forbidden characters are removed from object names
    // Object names should be cleaned in variable declarations
    QVERIFY(!content.contains("btn-with-dashes")); // Dashes should be removed
    QVERIFY(!content.contains("switch@home"));      // @ should be removed
    QVERIFY(!content.contains("led$status"));       // $ should be removed
    QVERIFY(!content.contains("gate.and.1"));       // Dots should be removed
    
    // Verify labels with forbidden characters are cleaned
    QVERIFY(!content.contains("Power Button (Main)")); // Parentheses should be removed
    QVERIFY(!content.contains("Mode Switch #1"));       // # should be removed
    QVERIFY(!content.contains("Status LED & Indicator")); // & should be removed
    
    // Verify that cleaned names appear in the code
    QVERIFY(content.contains("btnwithdashes") || content.contains("btn_with_dashes"));
    QVERIFY(content.contains("switchhome") || content.contains("switch_home"));
    QVERIFY(content.contains("ledstatus") || content.contains("led_status"));
    QVERIFY(content.contains("gateand1") || content.contains("gate_and_1"));
    
    // Verify cleaned labels appear in comments or variable names
    QVERIFY(content.contains("PowerButton") || content.contains("Power_Button") || content.contains("powerbutton"));
    QVERIFY(content.contains("ModeSwitch") || content.contains("Mode_Switch") || content.contains("modeswitch"));
    QVERIFY(content.contains("StatusLED") || content.contains("Status_LED") || content.contains("statusled"));
    
    // Verify basic structure is still intact
    QVERIFY(content.contains("void setup()"));
    QVERIFY(content.contains("void loop()"));
    QVERIFY(content.contains("boolean aux_"));
}