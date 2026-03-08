// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel2Decoder2To4 : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // Address Decoder (2→4) Tests
    void testAddressDecoder2to4_data();
    void testAddressDecoder2to4();
    void testSequentialScan();
    void testMutualExclusivity();
};

