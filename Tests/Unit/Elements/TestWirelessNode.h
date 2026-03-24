// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestWirelessNode : public QObject
{
    Q_OBJECT

private slots:
    // Wireless mode property tests (13 tests)
    void testDefaultModeIsNone();
    void testSetTxMode();
    void testSetRxMode();
    void testSetNoneModeRestoresRequired();
    void testSetNoneModeAfterTx();
    void testSetSameModeIdempotent();
    void testAllTransitionsIsRequired();
    void testSaveLoadRoundTrip();
    void testSaveLoadRxMode();
    void testSaveLoadNoneMode();
    void testLabelUsedAsChannelName();
    void testLabelPreservedAfterModeChange();
    void testHasWirelessModeFalseForNonNode();
    void testEditablePropertiesContainsWirelessMode();

    // Port visibility edge cases (3 tests)
    void testPortVisibilityTxMode();
    void testPortVisibilityRxMode();
    void testPortVisibilityAllTransitions();

    // Deserialization robustness (3 tests)
    void testLoadCorruptedWirelessModeClampsToNone();
    void testLoadNegativeWirelessModeClampsToNone();
    void testSaveLoadRxPortVisibility();
    void testSaveLoadTxPortVisibility();
};

