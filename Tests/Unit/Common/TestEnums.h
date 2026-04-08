// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestEnums : public QObject
{
    Q_OBJECT

private slots:

    // ElementType Conversions (8 tests)
    void testElementTypeToString();
    void testStringToElementType();
    void testInvalidStringToElementType();
    void testUnknownElementType();
    void testRoundTripConversion();
    void testTypeToTitleText();
    void testElementTypeIncrement();
    void testElementTypeEdgeCases();

    // Element Type Cycling
    void testNextElementTypeCycling();
    void testPrevElementTypeCycling();
    void testNextPrevSymmetry();
    void testGateTypeCycling();
    void testFlipFlopCycling();
    void testOutputCycling();
    void testInputCycling();
    void testNotNodeCycling();
    void testUnknownTypeReturnsUnknown();

    // Stream Operations (3 tests)
    void testQDataStreamSerialization();
    void testQDataStreamDeserialization();
    void testStreamRoundTrip();

    // Status Enum Tests (2 tests)
    void testStatusEnumValues();
    void testStatusEnumToString();

    // ElementGroup Enum Tests (2 tests)
    void testElementGroupValues();
    void testElementGroupToString();
};

