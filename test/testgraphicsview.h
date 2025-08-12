// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/*!
 * @class TestGraphicsView
 * @brief Test suite for GraphicsView UI component
 * 
 * Tests the enhanced QGraphicsView functionality including zoom operations,
 * navigation capabilities, fast mode rendering, and keyboard interaction.
 * Ensures proper viewport management and user interaction handling.
 */
class TestGraphicsView : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Basic construction and properties
    void testConstruction();
    void testInitialState();
    void testViewportSettings();
    
    // Zoom functionality
    void testZoomIn();
    void testZoomOut();
    void testZoomBounds();
    void testResetZoom();
    void testZoomSignals();
    void testCanZoomQueries();
    
    // Navigation and interaction
    void testPanning();
    void testKeyboardNavigation();
    void testMouseInteraction();
    void testViewportTransformation();
    
    // Performance and rendering modes
    void testFastMode();
    void testRenderingQuality();
    void testViewportOptimization();
    
    // Redirect zoom functionality
    void testRedirectZoom();
    void testZoomRedirection();
    
    // Scene integration
    void testSceneAssignment();
    void testSceneCoordinates();
    void testItemInteraction();
    void testSelectionHandling();
    
    // Event handling
    void testWheelEvents();
    void testKeyEvents();
    void testMouseEvents();
    void testResizeEvents();
    
    // Edge cases and error handling
    void testNullSceneHandling();
    void testExtremeZoomLevels();
    void testInvalidOperations();
    void testMemoryManagement();
};