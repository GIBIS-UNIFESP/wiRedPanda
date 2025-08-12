// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class ElementLabel;

class TestElementLabel : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Constructor and basic functionality
    void testElementLabelConstruction();
    void testElementLabelWithParent();
    void testLabelText();
    void testSetLabelText();
    
    // Positioning and geometry
    void testLabelPosition();
    void testSetLabelPosition();
    void testLabelBounds();
    void testLabelAlignment();
    
    // Font and text properties
    void testLabelFont();
    void testSetLabelFont();
    void testFontMetrics();
    void testTextColor();
    void testSetTextColor();
    
    // Visibility and display
    void testLabelVisibility();
    void testShowHideLabel();
    void testLabelOpacity();
    
    // Widget behavior and interaction
    void testFocusHandling();
    void testElementTypeDisplay();
    void testTextInputValidation();
    void testElementNameConsistency();
    
    // Layout and positioning
    void testAutoPositioning();
    void testManualPositioning();
    void testPositionConstraints();
    void testLabelOffsets();
    
    // Graphics and rendering
    void testLabelRendering();
    void testLabelBackground();
    void testLabelBorder();
    void testRenderingFlags();
    
    // Parent element relationship
    void testParentElementAssociation();
    void testLabelFollowsParent();
    void testParentElementEvents();
    void testOrphanedLabels();
    
    // Event handling
    void testMouseEvents();
    void testKeyboardEvents();
    void testFocusEvents();
    void testContextMenu();
    
    // Serialization and persistence
    void testLabelSerialization();
    void testLabelDeserialization();
    void testSerializationRoundTrip();
    void testVersionCompatibility();
    
    // Performance and optimization
    void testLabelCaching();
    void testRenderingPerformance();
    void testMemoryManagement();
    
    // Edge cases and error handling
    void testEmptyLabel();
    void testVeryLongLabel();
    void testInvalidPositions();
    void testNullParent();
    
private:
    ElementLabel* createTestLabel();
    void validateLabelState(ElementLabel* label);
    void testLabelProperty(ElementLabel* label, const QString& property, const QVariant& value);
};