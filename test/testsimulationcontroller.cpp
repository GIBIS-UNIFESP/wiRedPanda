// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testsimulationcontroller.h"

#include "and.h"
#include "elementmapping.h"
#include "inputbutton.h"
#include "led.h"
#include "qneconnection.h"
#include "scene.h"
#include "workspace.h"

#include <QTest>

void TestSimulationController::testCase1()
{
    auto *workspace = new WorkSpace();

    auto *btn1 = new InputButton();
    auto *btn2 = new InputButton();
    auto *andItem = new And();
    auto *led = new Led();
    auto *conn = new QNEConnection();
    auto *conn2 = new QNEConnection();
    auto *conn3 = new QNEConnection();

    auto *scene = workspace->scene();
    scene->addItem(led);
    scene->addItem(andItem);
    scene->addItem(btn1);
    scene->addItem(btn2);
    scene->addItem(conn);
    scene->addItem(conn2);
    scene->addItem(conn3);

    conn->setStart(btn1->output());
    conn->setEnd(andItem->input(0));
    conn2->setStart(btn2->output());
    conn2->setEnd(andItem->input(1));
    conn3->setStart(andItem->output());
    conn3->setEnd(led->input());

    const auto elms(ElementMapping::sortGraphicElements(scene->elements()));

    QVERIFY(elms.at(0) == btn1 || elms.at(1) == btn1);
    QVERIFY(elms.at(0) == btn2 || elms.at(1) == btn2);
    QVERIFY(elms.at(2) == andItem);
    QVERIFY(elms.at(3) == led);
}
