// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// The test readers in Tests/Common/TestUtils tell Unknown and Error from 0.
class TestPortReaders : public QObject
{
    Q_OBJECT

private slots:
    void testInputStatusReturnsWhatThePortHolds();
    void testOutputStatusReturnsWhatThePortHolds();
    void testBitsEqualANumberOnlyWhenEveryBitIsALevel();
    void testBitsPrintMostSignificantFirst();
    void testReadMultiBitOutputKeepsEachBitsStatus();
    void testValueFailsOnANonLevel();
};
