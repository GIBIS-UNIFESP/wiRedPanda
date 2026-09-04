// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Ported from the Widgets-side Tests/Unit/Elements/TestInputRotary.h/.cpp.
/// testMousePressAdvancesPort/testMousePressWhileLockedDoesNotAdvance exercise mouse input via
/// a real QMouseEvent sent through QCoreApplication::sendEvent(), since CanvasItem is the sole
/// recipient of mouse events and dispatches to activateOnPress()'s InputRotary branch.
class TestInputRotary : public QObject
{
    Q_OBJECT

private slots:
    // Port iteration tests
    void testInitialPortState();
    void testNextPortWrapping();
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
    void testPortConfigurationWithStandardSize_data();
    void testPortConfigurationWithStandardSize();

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

    // Negative positions (from file) clamp to port 0
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
    void testMousePressWhileLockedDoesNotAdvance();

    // setAppearance()
    void testSetAppearanceCustom();
    void testSetAppearanceResetToDefault();

    // load()'s pre-4.1 bare-field format and the "locked" map key
    void testLoadOldFormatBareFields();
    void testLoadLockedFlagFromMap();
};
