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
#include "jklatch.h"
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
#include <iostream>

void TestElements::init()
{
    for (int i = 0; i < connections.size(); ++i) {
        connections[i] = new QNEConnection();
        switches[i] = new InputSwitch();
        connections.at(i)->setStart(switches.at(i)->output());
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
    QCOMPARE(static_cast<int>(vcc.output()->value()), 1);
}

void TestElements::testGND()
{
    InputGnd gnd;
    QCOMPARE(gnd.outputSize(), 1);
    QCOMPARE(gnd.inputSize(), 0);
    QCOMPARE(static_cast<int>(gnd.output()->value()), 0);
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

QString TestElements::testFile(const QString &fileName)
{
    return QString(CURRENTDIR) + "/../examples/" + fileName;
}

void TestElements::testICData(const IC *ic)
{
    QCOMPARE(ic->inputSize(), 5);
    QCOMPARE(ic->outputSize(), 2);

    QCOMPARE(ic->input(0)->isRequired(), false);
    QCOMPARE(ic->input(1)->isRequired(), false);
    QCOMPARE(ic->input(2)->isRequired(), true);
    QCOMPARE(ic->input(3)->isRequired(), false);
    QCOMPARE(ic->input(4)->isRequired(), false);

    QCOMPARE(static_cast<int>(ic->input(0)->value()), 1);
    QCOMPARE(static_cast<int>(ic->input(1)->value()), 1);
    QCOMPARE(static_cast<int>(ic->input(2)->value()), -1);
    QCOMPARE(static_cast<int>(ic->input(3)->value()), 1);
    QCOMPARE(static_cast<int>(ic->input(4)->value()), 1);
}

void TestElements::testIC()
{
    GlobalProperties::currentFile = QString(CURRENTDIR) + "/../examples/simple.panda";
    const QString icFile = testFile("jkflipflop.panda");

    auto *ic = new IC();
    ICManager::loadIC(ic, icFile);

    testICData(ic);

    auto *clkButton = new InputButton();
    auto *prstButton = new InputButton();

    auto *led = new Led();
    auto *led2 = new Led();

    auto *connection = new QNEConnection();
    connection->setStart(clkButton->output());
    connection->setEnd(ic->input(2));

    auto *connection2 = new QNEConnection();
    connection2->setStart(ic->output(0));
    connection2->setEnd(led->input());

    auto *connection3 = new QNEConnection();
    connection3->setStart(prstButton->output());
    connection3->setEnd(ic->input(0));

    auto *connection4 = new QNEConnection();
    connection4->setStart(ic->output(1));
    connection4->setEnd(led2->input());

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
    controller.reSortElements();

    for (int i = 0; i < 10; ++i) {
        clkButton->setOn(false);
        prstButton->setOn(false);
        controller.update();
        controller.update();
        controller.update();
        controller.updateScene(scene.itemsBoundingRect());

        QCOMPARE(static_cast<int>(ic->input(2)->value()), 0);

        QCOMPARE(static_cast<int>(ic->output(0)->value()), 1);
        QCOMPARE(static_cast<int>(ic->output(1)->value()), 0);

        clkButton->setOn(false);
        prstButton->setOn(true);
        controller.update();
        controller.update();
        controller.update();
        controller.updateScene(scene.itemsBoundingRect());

        QCOMPARE(static_cast<int>(ic->input(2)->value()), 0);

        QCOMPARE(static_cast<int>(ic->output(0)->value()), 1);
        QCOMPARE(static_cast<int>(ic->output(1)->value()), 0);

        clkButton->setOn(false);
        controller.update();
        controller.update();
        controller.update();
        controller.updateScene(scene.itemsBoundingRect());

        QCOMPARE(static_cast<int>(ic->input(2)->value()), 0);

        QCOMPARE(static_cast<int>(ic->output(0)->value()), 1);
        QCOMPARE(static_cast<int>(ic->output(1)->value()), 0);

        clkButton->setOn(true);
        controller.update();
        controller.update();
        controller.update();
        controller.updateScene(scene.itemsBoundingRect());

        QCOMPARE(static_cast<int>(ic->input(2)->value()), 1);

        QCOMPARE(static_cast<int>(ic->output(0)->value()), 0);
        QCOMPARE(static_cast<int>(ic->output(1)->value()), 1);
    }
}

void TestElements::testICs()
{
    GlobalProperties::currentFile = QString(CURRENTDIR) + "/../examples/simple.panda";
    const QDir examplesDir(QString(CURRENTDIR) + "/../examples/");
    // qCDebug(zero) << "Current dir:" << CURRENTDIR;
    const auto files = examplesDir.entryInfoList(QStringList{"*.panda"});
    // qCDebug(zero) << "files:" << files;
    for (const auto &fileInfo : qAsConst(files)) {
        // qCDebug(zero) << "FILE:" << f.absoluteFilePath();
        IC ic;
        ICManager::loadIC(&ic, fileInfo.absoluteFilePath());
    }
}
