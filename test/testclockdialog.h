// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestClockDialog : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Basic functionality
    void testConstruction();
    void testFrequencySettings();
    void testFrequencyUnitConversion();
    
    // Dialog interaction
    void testAcceptRejectButtons();
    void testPersistenceOnAccept();
    
    // Validation
    void testFrequencyRanges();
    void testFrequencyValidation();
    
    // UI interaction
    void testKeyboardShortcuts();
    void testMultipleFrequencies();
    
    // Signals and slots
    void testSignalConnections();
};