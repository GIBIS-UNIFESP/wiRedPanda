// Copyright 2015 - 2022, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testelements.h"

#include "and.h"
#include "demux.h"
#include "dflipflop.h"
#include "dlatch.h"
#include "globalproperties.h"
#include "ic.h"
#include "inputbutton.h"
#include "inputgnd.h"
#include "inputswitch.h"
#include "inputvcc.h"
#include "jkflipflop.h"
#include "led.h"
#include "mux.h"
#include "node.h"
#include "or.h"
#include "qneconnection.h"
#include "qneport.h"
#include "scene.h"
#include "simulation.h"
#include "srflipflop.h"
#include "tflipflop.h"

#include <QTest>

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
    const QString icFile = QString(CURRENTDIR) + "/../examples/jkflipflop.panda";
    GlobalProperties::currentDir = QString(CURRENTDIR) + "/../examples/";

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
        simulation.update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Inactive);

        QCOMPARE(ic->outputPort(0)->status(), Status::Active);
        QCOMPARE(ic->outputPort(1)->status(), Status::Inactive);

        // -------------------------------

        clkButton->setOff();
        prstButton->setOn();
        simulation.update();
        simulation.update();
        simulation.update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Inactive);

        QCOMPARE(ic->outputPort(0)->status(), Status::Active);
        QCOMPARE(ic->outputPort(1)->status(), Status::Inactive);

        // -------------------------------

        clkButton->setOff();
        simulation.update();
        simulation.update();
        simulation.update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Inactive);

        QCOMPARE(ic->outputPort(0)->status(), Status::Active);
        QCOMPARE(ic->outputPort(1)->status(), Status::Inactive);

        // -------------------------------

        clkButton->setOn();
        simulation.update();
        simulation.update();
        simulation.update();

        QCOMPARE(ic->inputPort(2)->status(), Status::Active);

        QCOMPARE(ic->outputPort(0)->status(), Status::Inactive);
        QCOMPARE(ic->outputPort(1)->status(), Status::Active);
    }
}

void TestElements::testICs()
{
    const QDir examplesDir(QString(CURRENTDIR) + "/../examples/");
    const auto files = examplesDir.entryInfoList(QStringList{"*.panda"});

    for (const auto &fileInfo : files) {
        GlobalProperties::currentDir = fileInfo.absolutePath();
        IC ic;
        ic.loadFile(fileInfo.absoluteFilePath());
    }
}
