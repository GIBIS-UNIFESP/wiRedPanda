// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestTruthTable : public QObject
{
    Q_OBJECT

private slots:
    // Constructor and configuration tests
    void testConstructorInitialization();
    void testInputPortNames();
    void testOutputPortNames();
    void testPortConfiguration();

    // Key management tests
    void testKeyInitialization();
    void testSetKey();
    void testKeyAccess();
    void testKeyBitArray();

    // Port properties tests
    void testUpdatePortsProperties();
    void testInputPortPositioning();
    void testOutputPortPositioning();

    // Serialization tests
    void testSaveKey();
    void testLoadKeyVersion42();
    void testLoadKeyOldVersion();
};

