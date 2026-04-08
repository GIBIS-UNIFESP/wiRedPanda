// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestDisplay.h"

#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display16.h"
#include "Tests/Common/TestUtils.h"

void TestDisplay::testDisplay14AllColors()
{
    Display14 display;
    const QStringList colors = {"White", "Red", "Green", "Blue", "Purple"};
    for (const QString &color : colors) {
        display.setColor(color);
        QCOMPARE(display.color(), color);
    }
}

void TestDisplay::testDisplay16AllColors()
{
    Display16 display;
    const QStringList colors = {"White", "Red", "Green", "Blue", "Purple"};
    for (const QString &color : colors) {
        display.setColor(color);
        QCOMPARE(display.color(), color);
    }
}

void TestDisplay::testDisplay14NextPrevColor()
{
    Display14 display;
    display.setColor("Red");
    QString next = display.nextColor();
    QVERIFY(!next.isEmpty());
    QVERIFY(next != "Red");

    display.setColor(next);
    QString prev = display.previousColor();
    QCOMPARE(prev, QString("Red"));
}

void TestDisplay::testDisplay16NextPrevColor()
{
    Display16 display;
    display.setColor("Red");
    QString next = display.nextColor();
    QVERIFY(!next.isEmpty());
    QVERIFY(next != "Red");

    display.setColor(next);
    QString prev = display.previousColor();
    QCOMPARE(prev, QString("Red"));
}

void TestDisplay::testDisplay14Save()
{
    Display14 display;
    display.setColor("Blue");

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    display.save(stream);
    QVERIFY(!data.isEmpty());
}

void TestDisplay::testDisplay16Save()
{
    Display16 display;
    display.setColor("Purple");

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    display.save(stream);
    QVERIFY(!data.isEmpty());
}

