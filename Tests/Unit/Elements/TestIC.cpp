// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestIC.h"

#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"

void TestICUnit::testICLoadFromFile()
{
    IC ic;
    QCOMPARE(ic.elementType(), ElementType::IC);
}

void TestICUnit::testICPortLabelResolution()
{
    IC ic;
    // Unloaded IC has no ports
    QVERIFY(ic.inputs().isEmpty());
    QVERIFY(ic.outputs().isEmpty());
}

void TestICUnit::testICNestedSaveLoad()
{
    IC ic;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    ic.save(stream);
    QVERIFY(!data.isEmpty());
}

void TestICUnit::testICInvalidFile()
{
    IC ic;
    QVERIFY(ic.file().isEmpty());
}

