// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestProperties : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Element property tests
    void testLabelProperty();
    void testInputSizeProperty();
    void testOutputSizeProperty();
    void testFrequencyProperty();
    void testAudioProperty();
    void testTriggerProperty();
    void testRotationProperty();
    void testColorProperty();
    
    // Element identification
    void testElementTypeProperty();
    void testElementGroupProperty();
    
    // State properties
    void testValueProperties();
    void testPositionProperty();
    void testSelectionProperty();
    void testEnabledProperty();
    
    // Visual properties
    void testBoundingRectProperty();
};