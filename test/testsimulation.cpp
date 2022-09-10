// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testsimulation.h"

#include "and.h"
#include "common.h"
#include "inputbutton.h"
#include "led.h"
#include "qneconnection.h"
#include "scene.h"
#include "workspace.h"

#include <QTest>

void TestSimulation::testCase1()
{
    WorkSpace workspace;

    InputButton button1;
    InputButton button2;
    And andItem;
    Led led;
    QNEConnection connection1;
    QNEConnection connection2;
    QNEConnection connection3;

    auto *scene = workspace.scene();
    scene->addItem(&led);
    scene->addItem(&andItem);
    scene->addItem(&button1);
    scene->addItem(&button2);
    scene->addItem(&connection1);
    scene->addItem(&connection2);
    scene->addItem(&connection3);

    connection1.setStart(button1.outputPort());
    connection1.setEnd(andItem.inputPort(0));
    connection2.setStart(button2.outputPort());
    connection2.setEnd(andItem.inputPort(1));
    connection3.setStart(andItem.outputPort());
    connection3.setEnd(led.inputPort());

    const auto elements(Common::sortGraphicElements(scene->elements()));

    QVERIFY(elements.at(0) == &button1 || elements.at(1) == &button1);
    QVERIFY(elements.at(0) == &button2 || elements.at(1) == &button2);
    QVERIFY(elements.at(2) == &andItem);
    QVERIFY(elements.at(3) == &led);
}
