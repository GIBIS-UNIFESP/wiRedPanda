// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testelements.h"

#include "and.h"
#include "demux.h"
#include "dflipflop.h"
#include "dlatch.h"
#include "globalproperties.h"
#include "ic.h"
#include "icmanager.h"
#include "inputbutton.h"
#include "inputgnd.h"
#include "inputswitch.h"
#include "inputvcc.h"
#include "iostream"
#include "jkflipflop.h"
#include "led.h"
#include "mux.h"
#include "node.h"
#include "or.h"
#include "qneconnection.h"
#include "qneport.h"
#include "scene.h"
#include "simulationcontroller.h"
#include "srflipflop.h"
#include "tflipflop.h"

#include <QDebug>
#include <QTest>

void TestElements::init()
{
    for (int i = 0; i < connections.size(); ++i) {
        connections[i] = new QNEConnection();
        switches[i] = new InputSwitch();
        connections.at(i)->setStart(switches.at(i)->outputPort());
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
    QCOMPARE(static_cast<int>(vcc.outputPort()->value()), 1);
}

void TestElements::testGND()
{
    InputGnd gnd;
    QCOMPARE(gnd.outputSize(), 1);
    QCOMPARE(gnd.inputSize(), 0);
    QCOMPARE(static_cast<int>(gnd.outputPort()->value()), 0);
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

    QCOMPARE(static_cast<int>(ic->inputPort(0)->value()), 1);
    QCOMPARE(static_cast<int>(ic->inputPort(1)->value()), 1);
    QCOMPARE(static_cast<int>(ic->inputPort(2)->value()), -1);
    QCOMPARE(static_cast<int>(ic->inputPort(3)->value()), 1);
    QCOMPARE(static_cast<int>(ic->inputPort(4)->value()), 1);
}

void TestElements::testIC()
{
    const QString icFile = QString(CURRENTDIR) + "/../examples/jkflipflop.panda";
    GlobalProperties::currentFile = icFile;

    auto *ic = new IC();
    ICManager::loadIC(ic, icFile);

    testICData(ic);

    auto *clkButton = new InputButton();
    auto *prstButton = new InputButton();

    auto *led = new Led();
    auto *led2 = new Led();

    auto *connection = new QNEConnection();
    connection->setStart(clkButton->outputPort());
    connection->setEnd(ic->inputPort(2));

    auto *connection2 = new QNEConnection();
    connection2->setStart(ic->outputPort(0));
    connection2->setEnd(led->inputPort());

    auto *connection3 = new QNEConnection();
    connection3->setStart(prstButton->outputPort());
    connection3->setEnd(ic->inputPort(0));

    auto *connection4 = new QNEConnection();
    connection4->setStart(ic->outputPort(1));
    connection4->setEnd(led2->inputPort());

    Scene scene;
    scene.addItem(led);
    scene.addItem(led2);
    scene.addItem(clkButton);
    scene.addItem(prstButton);
    scene.addItem(ic);
    scene.addItem(connection);
    scene.addItem(connection2);
    scene.addItem(connection3);
    scene.addItem(connection4);

    SimulationController controller(&scene);
    controller.initialize();

    for (int i = 0; i < 10; ++i) {
        clkButton->setOn(false);
        prstButton->setOn(false);
        controller.update();
        controller.update();
        controller.update();
        controller.updateScene();

        QCOMPARE(static_cast<int>(ic->inputPort(2)->value()), 0);

        QCOMPARE(static_cast<int>(ic->outputPort(0)->value()), 1);
        QCOMPARE(static_cast<int>(ic->outputPort(1)->value()), 0);

        clkButton->setOn(false);
        prstButton->setOn(true);
        controller.update();
        controller.update();
        controller.update();
        controller.updateScene();

        QCOMPARE(static_cast<int>(ic->inputPort(2)->value()), 0);

        QCOMPARE(static_cast<int>(ic->outputPort(0)->value()), 1);
        QCOMPARE(static_cast<int>(ic->outputPort(1)->value()), 0);

        clkButton->setOn(false);
        controller.update();
        controller.update();
        controller.update();
        controller.updateScene();

        QCOMPARE(static_cast<int>(ic->inputPort(2)->value()), 0);

        QCOMPARE(static_cast<int>(ic->outputPort(0)->value()), 1);
        QCOMPARE(static_cast<int>(ic->outputPort(1)->value()), 0);

        clkButton->setOn(true);
        controller.update();
        controller.update();
        controller.update();
        controller.updateScene();

        QCOMPARE(static_cast<int>(ic->inputPort(2)->value()), 1);

        QCOMPARE(static_cast<int>(ic->outputPort(0)->value()), 0);
        QCOMPARE(static_cast<int>(ic->outputPort(1)->value()), 1);
    }
}

void TestElements::testICs()
{
    const QDir examplesDir(QString(CURRENTDIR) + "/../examples/");
    const auto files = examplesDir.entryInfoList(QStringList{"*.panda"});

    for (const auto &fileInfo : files) {
        GlobalProperties::currentFile = fileInfo.absoluteFilePath();
        IC ic;
        ICManager::loadIC(&ic, fileInfo.absoluteFilePath());
    }
}
