// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testelements.h"

#include "demux.h"
#include "dlatch.h"
#include "editor.h"
#include "ic.h"
#include "inputbutton.h"
#include "iostream"
#include "jkflipflop.h"
#include "jklatch.h"
#include "led.h"
#include "mux.h"
#include "node.h"
#include "srflipflop.h"
#include "tflipflop.h"
//#include "tlatch.h"

#include "and.h"
#include "dflipflop.h"
#include "icmanager.h"
#include "inputgnd.h"
#include "inputvcc.h"
#include "or.h"
#include "qneport.h"

#include <QDebug>
#include <iostream>

TestElements::TestElements(QObject *parent)
    : QObject(parent)
{
}

void TestElements::init()
{
    /* Creating connections */
    for (int i = 0; i < conn.size(); ++i) {
        conn[i] = new QNEConnection();
        sw[i] = new InputSwitch();
        conn.at(i)->setStart(sw.at(i)->output());
    }
}

void TestElements::cleanup()
{
    qDeleteAll(sw);
}

void TestElements::testNode()
{
    Node elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 1);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSz(), 1);
    QCOMPARE(elm.elementType(), ElementType::NODE);
}

void TestElements::testAnd()
{
    And elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSz(), 2);
    QCOMPARE(elm.elementType(), ElementType::AND);
}

void TestElements::testOr()
{
    Or elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 1);
    QCOMPARE(elm.minInputSz(), 2);
    QCOMPARE(elm.elementType(), ElementType::OR);
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
    QCOMPARE(elm.minInputSz(), 3);
    QCOMPARE(elm.elementType(), ElementType::MUX);
}

void TestElements::testDemux()
{
    Demux elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSz(), 2);
    QCOMPARE(elm.elementType(), ElementType::DEMUX);
}

void TestElements::testDFlipFlop()
{
    DFlipFlop elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 4);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSz(), 4);
    QCOMPARE(elm.maxInputSz(), 4);
    QCOMPARE(elm.minOutputSz(), 2);
    QCOMPARE(elm.maxOutputSz(), 2);
    QCOMPARE(elm.elementType(), ElementType::DFLIPFLOP);
}

void TestElements::testDLatch()
{
    DLatch elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 2);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSz(), 2);
    QCOMPARE(elm.maxInputSz(), 2);
    QCOMPARE(elm.minOutputSz(), 2);
    QCOMPARE(elm.maxOutputSz(), 2);
    QCOMPARE(elm.elementType(), ElementType::DLATCH);
}

void TestElements::testJKFlipFlop()
{
    JKFlipFlop elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 5);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSz(), 5);
    QCOMPARE(elm.maxInputSz(), 5);
    QCOMPARE(elm.minOutputSz(), 2);
    QCOMPARE(elm.maxOutputSz(), 2);
    QCOMPARE(elm.elementType(), ElementType::JKFLIPFLOP);
}

void TestElements::testSRFlipFlop()
{
    SRFlipFlop elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 5);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSz(), 5);
    QCOMPARE(elm.maxInputSz(), 5);
    QCOMPARE(elm.minOutputSz(), 2);
    QCOMPARE(elm.maxOutputSz(), 2);
    QCOMPARE(elm.elementType(), ElementType::SRFLIPFLOP);
}

void TestElements::testTFlipFlop()
{
    TFlipFlop elm;
    QCOMPARE(elm.inputSize(), elm.inputs().size());
    QCOMPARE(elm.inputSize(), 4);
    QCOMPARE(elm.outputSize(), elm.outputs().size());
    QCOMPARE(elm.outputSize(), 2);
    QCOMPARE(elm.minInputSz(), 4);
    QCOMPARE(elm.maxInputSz(), 4);
    QCOMPARE(elm.minOutputSz(), 2);
    QCOMPARE(elm.maxOutputSz(), 2);
    QCOMPARE(elm.elementType(), ElementType::TFLIPFLOP);
}

QString testFile(const QString& fname)
{
    return QString("%1/../examples/%2").arg(CURRENTDIR, fname);
}

void TestElements::testICData(const IC *ic)
{
    QCOMPARE(static_cast<int>(ic->inputSize()), 5);
    QCOMPARE(static_cast<int>(ic->outputSize()), 2);

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
    ICManager manager;
    QString icFile = testFile("jkflipflop.panda");
    Scene scene;

    IC *ic = new IC();
    manager.loadIC(ic, icFile);

    testICData(ic);

    InputButton *clkButton = new InputButton();

    InputButton *prstButton = new InputButton();

    Led *led = new Led();
    Led *led2 = new Led();

    QNEConnection *conn = new QNEConnection();
    conn->setStart(clkButton->output());
    conn->setEnd(ic->input(2));

    QNEConnection *conn2 = new QNEConnection();
    conn2->setStart(ic->output(0));
    conn2->setEnd(led->input());

    QNEConnection *conn3 = new QNEConnection();
    conn3->setStart(prstButton->output());
    conn3->setEnd(ic->input(0));

    QNEConnection *conn4 = new QNEConnection();
    conn4->setStart(ic->output(1));
    conn4->setEnd(led2->input());

    scene.addItem(led);
    scene.addItem(led2);
    scene.addItem(clkButton);
    scene.addItem(prstButton);
    scene.addItem(ic);
    scene.addItem(conn);
    scene.addItem(conn2);
    scene.addItem(conn3);
    scene.addItem(conn4);

    SimulationController sc(&scene);
    sc.reSortElms();
    for (int i = 0; i < 10; ++i) {
        clkButton->setOn(false);
        prstButton->setOn(false);
        sc.update();
        sc.update();
        sc.update();
        sc.updateScene(scene.itemsBoundingRect());

        QCOMPARE(static_cast<int>(ic->input(2)->value()), 0);

        QCOMPARE(static_cast<int>(ic->output(0)->value()), 1);
        QCOMPARE(static_cast<int>(ic->output(1)->value()), 0);

        clkButton->setOn(false);
        prstButton->setOn(true);
        sc.update();
        sc.update();
        sc.update();
        sc.updateScene(scene.itemsBoundingRect());
        QCOMPARE(static_cast<int>(ic->input(2)->value()), 0);

        QCOMPARE(static_cast<int>(ic->output(0)->value()), 1);
        QCOMPARE(static_cast<int>(ic->output(1)->value()), 0);

        clkButton->setOn(false);
        sc.update();
        sc.update();
        sc.update();
        sc.updateScene(scene.itemsBoundingRect());

        QCOMPARE(static_cast<int>(ic->input(2)->value()), 0);

        QCOMPARE(static_cast<int>(ic->output(0)->value()), 1);
        QCOMPARE(static_cast<int>(ic->output(1)->value()), 0);

        clkButton->setOn(true);
        sc.update();
        sc.update();
        sc.update();
        sc.updateScene(scene.itemsBoundingRect());

        QCOMPARE(static_cast<int>(ic->input(2)->value()), 1);

        std::cout << static_cast<int>(ic->output(0)->value()) << " " << static_cast<int>(ic->output(1)->value()) << std::endl;

        QCOMPARE(static_cast<int>(ic->output(0)->value()), 0);
        QCOMPARE(static_cast<int>(ic->output(1)->value()), 1);
    }
}

void TestElements::testICs()
{
    ICManager manager;
    QDir examplesDir(QString("%1/../examples/").arg(CURRENTDIR));
    /*  qDebug( ) << "Current dir: " << CURRENTDIR; */
    QStringList entries;
    entries << "*.panda";
    QFileInfoList files = examplesDir.entryInfoList(entries);
    for (const auto &f : qAsConst(files)) {
        qDebug() << "FILE: " << f.absoluteFilePath();
        IC ic;
        manager.loadIC(&ic, f.absoluteFilePath());
    }
}
