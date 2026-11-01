// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestDisplay.h"

#include "App/Element/Element.h"
#include "App/IO/SerializationContext.h"
#include "Tests/Common/TestUtils.h"

void TestDisplay::testDisplay14AllColors()
{
    Element display{ElementType::Display14};
    const QStringList colors = {"White", "Red", "Green", "Blue", "Purple"};
    for (const QString &color : colors) {
        display.setColor(color);
        QCOMPARE(display.color(), color);
    }
}

void TestDisplay::testDisplay16AllColors()
{
    Element display{ElementType::Display16};
    const QStringList colors = {"White", "Red", "Green", "Blue", "Purple"};
    for (const QString &color : colors) {
        display.setColor(color);
        QCOMPARE(display.color(), color);
    }
}

void TestDisplay::testDisplay14NextPrevColor()
{
    Element display{ElementType::Display14};
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
    Element display{ElementType::Display16};
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
    Element display{ElementType::Display14};
    display.setColor("Blue");

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    display.save(stream, {.purpose = SerializationPurpose::PortableFile});
    QVERIFY(!data.isEmpty());
}

void TestDisplay::testDisplay16Save()
{
    Element display{ElementType::Display16};
    display.setColor("Purple");

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    display.save(stream, {.purpose = SerializationPurpose::PortableFile});
    QVERIFY(!data.isEmpty());
}
