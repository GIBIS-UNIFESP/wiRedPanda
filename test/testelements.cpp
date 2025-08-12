// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testelements.h"

#include "and.h"
#include "audiobox.h"
#include "buzzer.h"
#include "clock.h"
#include "demux.h"
#include "dflipflop.h"
#include "display_7.h"
#include "display_14.h"
#include "display_16.h"
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
#include "nand.h"
#include "node.h"
#include "nor.h"
#include "not.h"
#include "or.h"
#include "qneconnection.h"
#include "qneport.h"
#include "scene.h"
#include "simulation.h"
#include "srflipflop.h"
#include "srlatch.h"
#include "text.h"
#include "tflipflop.h"
#include "truth_table.h"
#include "xnor.h"
#include "xor.h"

#include <QTest>

#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string

void TestElements::init()
{
    for (int i = 0; i < connections.size(); ++i) {
        connections[i] = new QNEConnection();
        switches[i] = new InputSwitch();
        connections.at(i)->setStartPort(switches.at(i)->outputPort());
    }
}

void TestElements::cleanup()
{
    qDeleteAll(switches);
}

void TestElements::testNode()
{
    Node elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 1);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 1);
    QCOMPARE(elm.elementType(), ElementType::Node);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        Node skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::Node);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testAnd()
{
    And elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::And);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        And skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::And);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testOr()
{
    Or elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Or);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    Or skipInitElm;
    QCOMPARE(skipInitElm.elementType(), ElementType::Or);
    GlobalProperties::skipInit = false;
}

void TestElements::testVCC()
{
    InputVcc vcc;
    QCOMPARE(vcc.outputSize(), 1);
    QCOMPARE(vcc.inputSize(), 0);
    QCOMPARE(vcc.outputPort()->status(), Status::Active);
}

void TestElements::testGND()
{
    InputGnd gnd;
    QCOMPARE(gnd.outputSize(), 1);
    QCOMPARE(gnd.inputSize(), 0);
    QCOMPARE(gnd.outputPort()->status(), Status::Inactive);
}

void TestElements::testMux()
{
    Mux elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 3);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 3);
    QCOMPARE(elm.elementType(), ElementType::Mux);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        Mux skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::Mux);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testDemux()
{
    Demux elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Demux);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        Demux skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::Demux);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testDFlipFlop()
{
    DFlipFlop elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 4);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 4);
    QCOMPARE(elm.maxInputSize(), 4);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::DFlipFlop);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        DFlipFlop skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::DFlipFlop);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testDLatch()
{
    DLatch elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.maxInputSize(), 2);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::DLatch);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        DLatch skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::DLatch);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testJKFlipFlop()
{
    JKFlipFlop elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 5);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 5);
    QCOMPARE(elm.maxInputSize(), 5);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::JKFlipFlop);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        JKFlipFlop skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::JKFlipFlop);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testSRFlipFlop()
{
    SRFlipFlop elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 5);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 5);
    QCOMPARE(elm.maxInputSize(), 5);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::SRFlipFlop);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        SRFlipFlop skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::SRFlipFlop);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testTFlipFlop()
{
    TFlipFlop elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 4);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 4);
    QCOMPARE(elm.maxInputSize(), 4);
    QCOMPARE(elm.minOutputSize(), 2);
    QCOMPARE(elm.maxOutputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::TFlipFlop);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        TFlipFlop skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::TFlipFlop);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testICData(IC *ic)
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
}

void TestElements::testIC()
{
    const QString icFile = QString(QUOTE(CURRENTDIR)) + "/../examples/jkflipflop.panda";
    GlobalProperties::currentDir = QString(QUOTE(CURRENTDIR)) + "/../examples/";

    auto *ic = new IC();
    ic->loadFile(icFile);

    testICData(ic);

    auto *clkButton = new InputButton();
    auto *prstButton = new InputButton();

    auto *led1 = new Led();
    auto *led2 = new Led();

    auto *connection1 = new QNEConnection();
    connection1->setStartPort(clkButton->outputPort());
    connection1->setEndPort(ic->inputPort(2));

    auto *connection2 = new QNEConnection();
    connection2->setStartPort(prstButton->outputPort());
    connection2->setEndPort(ic->inputPort(0));

    auto *connection3 = new QNEConnection();
    connection3->setStartPort(ic->outputPort(0));
    connection3->setEndPort(led1->inputPort());

    auto *connection4 = new QNEConnection();
    connection4->setStartPort(ic->outputPort(1));
    connection4->setEndPort(led2->inputPort());

    Scene scene;
    scene.addItem(led1);
    scene.addItem(led2);
    scene.addItem(clkButton);
    scene.addItem(prstButton);
    scene.addItem(ic);
    scene.addItem(connection1);
    scene.addItem(connection2);
    scene.addItem(connection3);
    scene.addItem(connection4);

    Simulation simulation(&scene);
    simulation.initialize();

    for (int i = 0; i < 10; ++i) {
        clkButton->setOff();
        prstButton->setOff();
        simulation.update();
        simulation.update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Inactive);

        QCOMPARE(ic->outputPort(0)->status(), Status::Active);
        QCOMPARE(ic->outputPort(1)->status(), Status::Inactive);

        // -------------------------------

        clkButton->setOff();
        prstButton->setOn();
        simulation.update();
        simulation.update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Inactive);

        QCOMPARE(ic->outputPort(0)->status(), Status::Active);
        QCOMPARE(ic->outputPort(1)->status(), Status::Inactive);

        // -------------------------------

        clkButton->setOff();
        simulation.update();
        simulation.update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Inactive);

        QCOMPARE(ic->outputPort(0)->status(), Status::Active);
        QCOMPARE(ic->outputPort(1)->status(), Status::Inactive);

        // -------------------------------

        clkButton->setOn();
        simulation.update();
        simulation.update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Active);

        QCOMPARE(ic->outputPort(0)->status(), Status::Inactive);
        QCOMPARE(ic->outputPort(1)->status(), Status::Active);
    }
}

void TestElements::testICs()
{
    const QDir examplesDir(QString(QUOTE(CURRENTDIR)) + "/../examples/");
    const auto files = examplesDir.entryInfoList(QStringList{"*.panda"});

    for (const auto &fileInfo : files) {
        GlobalProperties::currentDir = fileInfo.absolutePath();
        IC ic;
        ic.loadFile(fileInfo.absoluteFilePath());
    }
}

void TestElements::testNAND()
{
    Nand elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Nand);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        Nand skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::Nand);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testNOR()
{
    Nor elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Nor);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    Nor skipInitElm;
    QCOMPARE(skipInitElm.elementType(), ElementType::Nor);
    GlobalProperties::skipInit = false;
}

void TestElements::testNOT()
{
    Not elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 1);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 1);
    QCOMPARE(elm.elementType(), ElementType::Not);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    Not skipInitElm;
    QCOMPARE(skipInitElm.elementType(), ElementType::Not);
    GlobalProperties::skipInit = false;
}

void TestElements::testXOR()
{
    Xor elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Xor);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    Xor skipInitElm;
    QCOMPARE(skipInitElm.elementType(), ElementType::Xor);
    GlobalProperties::skipInit = false;
}

void TestElements::testXNOR()
{
    Xnor elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::Xnor);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    Xnor skipInitElm;
    QCOMPARE(skipInitElm.elementType(), ElementType::Xnor);
    GlobalProperties::skipInit = false;
}

void TestElements::testLED()
{
    Led elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 1);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 0);
    QCOMPARE(elm.minInputSize(), 1);
    QCOMPARE(elm.elementType(), ElementType::Led);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        Led skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::Led);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testInputButton()
{
    InputButton elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 0);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 0);
    QCOMPARE(elm.elementType(), ElementType::InputButton);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        InputButton skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::InputButton);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testSRLatch()
{
    SRLatch elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::SRLatch);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        SRLatch skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::SRLatch);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testAudioBox()
{
    AudioBox elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 1);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 0);
    QCOMPARE(elm.minInputSize(), 1);
    QCOMPARE(elm.elementType(), ElementType::AudioBox);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        AudioBox skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::AudioBox);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testBuzzer()
{
    Buzzer elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 1);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 0);
    QCOMPARE(elm.minInputSize(), 1);
    QCOMPARE(elm.elementType(), ElementType::Buzzer);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        Buzzer skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::Buzzer);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testClock()
{
    Clock elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 0);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 0);
    QCOMPARE(elm.elementType(), ElementType::Clock);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        Clock skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::Clock);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testDisplay7()
{
    Display7 elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 8);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 0);
    QCOMPARE(elm.minInputSize(), 8);
    QCOMPARE(elm.elementType(), ElementType::Display7);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        Display7 skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::Display7);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testDisplay14()
{
    Display14 elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 15);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 0);
    QCOMPARE(elm.minInputSize(), 15);
    QCOMPARE(elm.elementType(), ElementType::Display14);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        Display14 skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::Display14);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testDisplay16()
{
    Display16 elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 17);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 0);
    QCOMPARE(elm.minInputSize(), 17);
    QCOMPARE(elm.elementType(), ElementType::Display16);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        Display16 skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::Display16);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testInputRotary()
{
    InputRotary elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 0);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSize(), 0);
    QCOMPARE(elm.elementType(), ElementType::InputRotary);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        InputRotary skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::InputRotary);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testInputSwitch()
{
    InputSwitch elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 0);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 0);
    QCOMPARE(elm.elementType(), ElementType::InputSwitch);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        InputSwitch skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::InputSwitch);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testLine()
{
    Line elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 0);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 0);
    QCOMPARE(elm.minInputSize(), 0);
    QCOMPARE(elm.elementType(), ElementType::Line);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        Line skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::Line);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testText()
{
    Text elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 0);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 0);
    QCOMPARE(elm.minInputSize(), 0);
    QCOMPARE(elm.elementType(), ElementType::Text);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        Text skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::Text);
    }
    GlobalProperties::skipInit = false;
}

void TestElements::testTruthTable()
{
    TruthTable elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSize(), 2);
    QCOMPARE(elm.elementType(), ElementType::TruthTable);

    // Test skipInit path for constructor coverage
    GlobalProperties::skipInit = true;
    {
        TruthTable skipInitElm;
        QCOMPARE(skipInitElm.elementType(), ElementType::TruthTable);
    }
    GlobalProperties::skipInit = false;
}
