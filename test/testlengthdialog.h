// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLengthDialog : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Basic functionality tests
    void testConstruction();
    void testMinMaxValues();
    void testInitialValue();
    
    // Input validation tests
    void testInputValidation();
    void testAcceptReject();
    void testExecValue();
    
    // Edge cases
    void testEdgeValues();
    void testKeyboardInput();
    void testStepIncrement();
    
    // Signal/slot testing
    void testSignalsAndSlots();
};