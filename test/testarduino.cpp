// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testarduino.h"

#include "codegenerator.h"
#include "and.h"

#include <QTest>

void TestArduino::testMappedPin()
{
    MappedPin pin1;
    QCOMPARE(pin1.m_elm, nullptr);
    QCOMPARE(pin1.m_port, nullptr);
    QCOMPARE(pin1.m_pin, QString());
    QCOMPARE(pin1.m_varName, QString());
    QCOMPARE(pin1.m_portNumber, 0);

    And and_gate;
    MappedPin pin2(&and_gate, "D2", "var1", nullptr, 1);
    QCOMPARE(pin2.m_elm, &and_gate);
    QCOMPARE(pin2.m_pin, QString("D2"));
    QCOMPARE(pin2.m_varName, QString("var1"));
    QCOMPARE(pin2.m_portNumber, 1);
}

void TestArduino::testCodeGeneratorConstruction()
{
    QVector<GraphicElement *> elements;
    And and_gate;
    elements.append(&and_gate);

    CodeGenerator generator("test.ino", elements);
    
    // Just test that construction doesn't crash
    // We can't easily test generate() without a full setup
    QVERIFY(true);
}