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

    // Regression: C7 — forgetItemId drops registry entry without delete
    void testForgetItemIdC7();

    // Regression: WIREDPANDA-HC — drainConnections must clean the registry
    void testDrainConnectionsCleansRegistryHC();

    // Regression: F4 — paste gate must accept every format paste() reads
    void testClipboardCanPasteMatchesPasteFormats();
    void testBuildDragImageClampsExtremeSelectionExtent();

    // Regression: WIREDPANDA-KS — port/orientation geometry changes must reindex the scene BSP tree
    void testGeometryChangeKeepsSceneIndexConsistentKS();

    // Regression (#44): a held trigger key must fire once — auto-repeat presses must not
    // oscillate a latched InputSwitch.
    void testKeyTriggerIgnoresAutoRepeat();

    // Arrow keys nudge the selection by a grid step (Shift = larger), undoable.
    void testArrowKeyNudgesSelection();

    // #12: a .panda file dropped from the file manager is recognised for opening.
    void testDroppedPandaFileDetection();

    // Scene::mute() must silence every AudioOutputElement subclass (Buzzer and AudioBox),
    // not just Buzzer -- otherwise an AudioBox keeps playing audibly through a simulation pause.
    void testMuteSilencesAllAudioOutputElements();

    // Adaptive wire antialiasing: measured over-budget passes degrade (debounced), and
    // quality returns only via idle or sustained deep headroom -- never because the
    // degraded renderer merely measures "fast enough" (binary-knob oscillation).
    void testWireAntialiasingDegradesOnSlowPassesAndRestores();
};
