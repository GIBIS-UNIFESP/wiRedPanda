// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLED : public QObject
{
    Q_OBJECT

private slots:
    // Constructor and configuration tests
    void testConstructorInitialization();
    void testPortConfiguration();
    void testInputPortNames();

    // Color management tests
    void testDefaultColor();
    void testSetColorWhite();
    void testSetColorRed();
    void testSetColorGreen();
    void testSetColorBlue();
    void testSetColorPurple();
    void testGetColor();

    // Color index tests
    void testColorIndexSingleInput();
    void testColorIndexTwoInputs();
    void testColorIndexThreeInputs();
    void testColorIndexFourInputs();

    // Port properties tests
    void testUpdatePortsProperties();
    void testPortNamesAfterUpdate();
    void testHasColorsProperty();

    // Serialization tests
    void testSaveColor();
    void testLoadColorOldVersion();
    void testLoadColorNewVersion();
    void testLoadColorDefault();

    // Generic properties tests
    void testGenericProperties();
};

