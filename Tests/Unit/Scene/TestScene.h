// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestScene : public QObject
{
    Q_OBJECT

private slots:
    // Lifecycle
    void initTestCase();
    void cleanup();
    void cleanupTestCase();

    // Element management
    void testAddElement();
    void testRemoveElement();
    void testElementQuery();
    void testElementsInRect();
    void testConnectionsQuery();

    // Item queries
    void testItemsAtPoint();
    void testItemsAt();

    // Selection
    void testSelectAll();
    void testSelectedElements();
    void testClearSelection();

    // Bounds
    void testSceneBounds();

    // Special cases
    void testEmptyScene();

    void testCopySingleElement();
    void testCopyMultipleElements();
    void testCutElement();
    void testPasteElement();
    void testPasteMultipleTimes();
    void testCopyPasteEmptySelection();

    void testRotateRight();
    void testRotateLeft();
    void testMultipleRotations();
    void testFlipHorizontal();
    void testFlipVertically();
    void testRotateWithConnections();

    void testDeleteSingleElement();
    void testDeleteMultipleElements();
    void testDeleteWithConnections();

    void testUndoDelete();
    void testRedoDelete();
    void testUndoRotation();

    void testNextElementType();
    void testPrevElementType();
    void testElementMorphingWithConnections();
    void testElementMorphingPropertyPreservation();
    void testPropertyCycleFrequency();
    void testPropertyCycleColor();
    void testPropertyCycleAudio();
    void testMorphingMultipleElements();

    void testShowGatesToggle();
    void testShowWiresToggle();
    void testGateVisibilityStateTracking();
    void testWireVisibilityStateTracking();
    void testShowGatesWithMultipleElements();
    void testShowWiresWithMultipleConnections();

    // ============================================================
    // Topological Sort Tests (4 tests)
    // ============================================================
    void testSortSimpleChain();
    void testSortMultipleChains();
    void testSortCycleDetection();
    void testSortDisconnectedComponents();
};

