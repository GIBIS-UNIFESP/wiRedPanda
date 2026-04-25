// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestDolphinSerializer : public QObject
{
    Q_OBJECT

private slots:

    // Binary serialization (4 tests)
    void testBinarySaveSingleItem();
    void testBinaryLoadSingleItem();
    void testBinaryRoundTrip();
    void testBinaryWithMultipleColumns();

    // CSV serialization (4 tests)
    void testCSVSaveSingleItem();
    void testCSVLoadSingleItem();
    void testCSVRoundTrip();
    void testCSVWithMultipleRows();

    // Edge cases (3 tests)
    void testEmptyModelBinary();
    void testEmptyModelCSV();
    void testCorruptedDataHandling();

    // Regression: D20 — negative row count must throw, not UB-resize
    void testLoadBinaryRejectsNegativeRows();
    void testLoadCSVRejectsNegativeRows();
};

