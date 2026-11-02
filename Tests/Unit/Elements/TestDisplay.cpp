// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestDisplay.h"

#include "App/Element/Element.h"
#include "App/Element/ElementColour.h"
#include "App/IO/SerializationContext.h"
#include "Tests/Common/TestUtils.h"

void TestDisplay::testDisplay14AllColors()
{
    Element display{ElementType::Display14};
    const QStringList colors = {"White", "Red", "Green", "Blue", "Purple"};
    for (const QString &color : colors) {
        display.setParam(ParamId::Colour, color);
        QCOMPARE(display.param<QString>(ParamId::Colour), color);
    }
}

void TestDisplay::testDisplay16AllColors()
{
    Element display{ElementType::Display16};
    const QStringList colors = {"White", "Red", "Green", "Blue", "Purple"};
    for (const QString &color : colors) {
        display.setParam(ParamId::Colour, color);
        QCOMPARE(display.param<QString>(ParamId::Colour), color);
    }
}

void TestDisplay::testDisplay14NextPrevColor()
{
    Element display{ElementType::Display14};
    display.setParam(ParamId::Colour, QStringLiteral("Red"));
    QString next = ElementColour::next(display.param<QString>(ParamId::Colour));
    QVERIFY(!next.isEmpty());
    QVERIFY(next != "Red");

    display.setParam(ParamId::Colour, next);
    QString prev = ElementColour::previous(display.param<QString>(ParamId::Colour));
    QCOMPARE(prev, QString("Red"));
}

void TestDisplay::testDisplay16NextPrevColor()
{
    Element display{ElementType::Display16};
    display.setParam(ParamId::Colour, QStringLiteral("Red"));
    QString next = ElementColour::next(display.param<QString>(ParamId::Colour));
    QVERIFY(!next.isEmpty());
    QVERIFY(next != "Red");

    display.setParam(ParamId::Colour, next);
    QString prev = ElementColour::previous(display.param<QString>(ParamId::Colour));
    QCOMPARE(prev, QString("Red"));
}

void TestDisplay::testDisplay14Save()
{
    Element display{ElementType::Display14};
    display.setParam(ParamId::Colour, QStringLiteral("Blue"));

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    display.save(stream, {.purpose = SerializationPurpose::PortableFile});
    QVERIFY(!data.isEmpty());
}

void TestDisplay::testDisplay16Save()
{
    Element display{ElementType::Display16};
    display.setParam(ParamId::Colour, QStringLiteral("Purple"));

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    display.save(stream, {.purpose = SerializationPurpose::PortableFile});
    QVERIFY(!data.isEmpty());
}
