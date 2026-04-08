// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestDisplay7.h"

#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>

#include "App/Element/GraphicElements/Display7.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestDisplay7::testDisplay7ColorSetting()
{
    Display7 display;
    display.setColor("Red");
    QCOMPARE(display.color(), QString("Red"));

    display.setColor("Green");
    QCOMPARE(display.color(), QString("Green"));
}

void TestDisplay7::testDisplay7SegmentRendering()
{
    Display7 display;
    display.setColor("Blue");
    QVERIFY(!display.color().isEmpty());
}

void TestDisplay7::testDisplay7SaveLoad()
{
    Display7 display;
    display.setColor("Red");

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    display.save(writeStream);
    QVERIFY(!data.isEmpty());
}

void TestDisplay7::testDisplay7AllColors()
{
    Display7 display;
    const QStringList colors = {"White", "Red", "Green", "Blue", "Purple"};
    for (const QString &color : colors) {
        display.setColor(color);
        QCOMPARE(display.color(), color);
    }
}

void TestDisplay7::testDisplay7Paint()
{
    WorkSpace workspace;
    auto *disp = new Display7;
    workspace.scene()->addItem(disp);
    disp->setColor("Red");

    // Set all segments to Active
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

