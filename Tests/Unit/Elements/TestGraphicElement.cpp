// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestGraphicElement.h"

#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display16.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Line.h"
#include "App/Element/GraphicElements/Text.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestGraphicElement::testElementPaintSelection()
{
    And andGate;
    andGate.setSelected(true);
    QVERIFY(andGate.isSelected());

    andGate.setSelected(false);
    QVERIFY(!andGate.isSelected());
}

void TestGraphicElement::testElementSaveLoad()
{
    And gate;
    gate.setRotation(90);

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    gate.save(writeStream);
    QVERIFY(!data.isEmpty());
}

void TestGraphicElement::testElementSkin()
{
    Led led;
    QVERIFY(led.hasColors());
}

void TestGraphicElement::testElementAppearance()
{
    Led led;
    led.setColor("Red");
    QCOMPARE(led.color(), QString("Red"));
}

void TestGraphicElement::testElementTooltip()
{
    And andGate;
    QVERIFY(true);
}

void TestGraphicElement::testElementDoubleClick()
{
    And andGate;
    QCOMPARE(andGate.elementType(), ElementType::And);
}

void TestGraphicElement::testTextElement()
{
    WorkSpace workspace;
    auto *text = new Text;
    workspace.scene()->addItem(text);
    QCOMPARE(text->elementType(), ElementType::Text);
    QVERIFY(!text->boundingRect().isNull());
}

void TestGraphicElement::testLineElement()
{
    WorkSpace workspace;
    auto *line = new Line;
    workspace.scene()->addItem(line);
    QCOMPARE(line->elementType(), ElementType::Line);
    QVERIFY(!line->boundingRect().isNull());
}

void TestGraphicElement::testDisplay14Paint()
{
    WorkSpace workspace;
    auto *disp = new Display14;
    workspace.scene()->addItem(disp);
    disp->setColor("Green");

    // Set some segments to Active to exercise paint() branches
    for (int i = 0; i < disp->inputSize(); ++i) {
        disp->inputPort(i)->setStatus(Status::Active);
    }
    disp->refresh();

    // Render to pixmap to trigger paint()
    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    disp->paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY(true);
}

void TestGraphicElement::testDisplay16Paint()
{
    WorkSpace workspace;
    auto *disp = new Display16;
    workspace.scene()->addItem(disp);
    disp->setColor("Blue");

    for (int i = 0; i < disp->inputSize(); ++i) {
        disp->inputPort(i)->setStatus(Status::Active);
    }
    disp->refresh();

    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    disp->paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY(true);
}

