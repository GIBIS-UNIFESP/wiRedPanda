// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestInputRotary : public QObject
{
    Q_OBJECT

private slots:
    // Port iteration tests
    void testInitialPortState();
    void testNextPortWrapping();
    void testPreviousPortWrapping();
    void testSetPortDirect();
    void testSetPortOutOfBounds();
    void testSetPortWrapsToZero();

    // Output value tests
    void testOutputValue();
    void testOutputSize();
    void testIsOnCurrentPort();
    void testIsOnOtherPort();
    void testIsOnAllPorts();

    // Port configuration tests
    void testPortConfigurationWithTwoPorts();
    void testPortConfigurationWithThreePorts();
    void testPortConfigurationWithFourPorts();
    void testPortConfigurationWithSixPorts();
    void testPortConfigurationWithEightPorts();
    void testPortConfigurationWithTenPorts();
    void testPortConfigurationWithTwelvePorts();
    void testPortConfigurationWithSixteenPorts();

    // Serialization tests
    void testSaveCurrentPort();
    void testLoadCurrentPort();
    void testSaveLoadPreservesState();

    // State management tests
    void testSetOffDoesNothing();
    void testMultipleSetOn();
    void testPortStatusAfterSetOn();

    // Basic InputRotary test (migrated from testelements)
    void testInputRotary();

    // Regression: F13 — negative positions (from file) clamp to port 0
    void testSetOnNegativePortClamps();

    // Non-standard output size (default branch of updatePortsProperties()'s switch)
    void testPortConfigurationWithNonStandardPortCount();

    // Painting
    void testPainting();

    // setWaveformValue()
    void testSetWaveformValueTrueSelectsPort();
    void testSetWaveformValueFalseIsNoOp();

    // mousePressEvent()
    void testMousePressAdvancesPort();

    // setAppearance()
    void testSetAppearanceCustom();
    void testSetAppearanceResetToDefault();

    // load()'s pre-4.1 bare-field format and the "locked" map key
    void testLoadOldFormatBareFields();
    void testLoadLockedFlagFromMap();
};
