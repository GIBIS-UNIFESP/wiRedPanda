// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Tests CanvasItem's own protected mousePressEvent()/mouseMoveEvent()/mouseReleaseEvent()/
/// mouseDoubleClickEvent()/wheelEvent()/keyPressEvent()/keyReleaseEvent() gesture dispatch
/// (click-to-select, drag-to-move, rubber-band, wire-creation-by-drag, keyboard shortcuts).
///
/// Technique: synthesize a real QMouseEvent/QKeyEvent/QWheelEvent and deliver it via
/// QCoreApplication::sendEvent(&canvas, &event) -- QQuickItem::event() dispatches by
/// QEvent::Type to the same protected virtual handler a real window's input pipeline would
/// call, so this exercises the identical code path a live user gesture would, without needing
/// a shown/exposed QQuickWindow.
///
/// Click/drag coordinates are computed directly from each element's/port's own pos()/scenePos(),
/// matching rebuildSpatialIndex()'s "boundingRect().translated(pos())" hit-test box (see
/// CanvasItem.cpp) -- not eyeballed pixel constants. worldToScreen()/screenToWorld() are the
/// identity transform at this class's default zoom/pan, so world and screen coordinates coincide
/// unless a test explicitly changes zoom/pan.
class TestCanvasItemInteraction : public QObject
{
    Q_OBJECT

private slots:
    // --- Keyboard shortcuts (CanvasItem::keyPressEvent dispatch) ---
    void testDeleteKeyRemovesSelectedElement();
    void testCopyPasteViaKeyboard();
    void testCutPasteViaKeyboard();
    void testDuplicateViaKeyboard();
    void testSelectAllViaKeyboard();
    void testRotateRightAndLeftViaKeyboard();
    void testFlipHorizontallyViaKeyboard();
    void testZoomShortcuts();
    void testPropertyAndTypeCycleShortcuts();
    void testArrowKeyNudgesSelection();
    void testKeyboardTriggerFiresInputSwitch();
    void testAutoRepeatKeyPressDoesNotRetoggleTrigger();
    void testKeyReleaseTurnsOffMomentaryInputButton();
    void testCtrlAndAltArrowKeysDoNotNudgeSelection();

    // --- Mouse interactions (CanvasItem::mousePressEvent/mouseMoveEvent/mouseReleaseEvent/
    // mouseDoubleClickEvent/wheelEvent dispatch) ---
    void testMouseClickSelectsElement();
    void testMouseClickEmptyDeselects();
    void testShiftClickTogglesMultiSelect();
    void testCtrlClickClonesSelectionInPlace();
    void testCtrlDragClonesSelectionAtOffsetPosition();
    void testMouseDragMovesElement();
    void testRubberBandSelectionSelectsEnclosedElements();
    void testWireCreationByDraggingPortToPort();
    void testDoubleClickLabeledElementEmitsInlineEditRequested();
    void testDoubleClickEmptyLabelFallsBackToElementBoundingRect();
    void testDoubleClickICEmitsIcOpenRequested();
    void testWheelZoomsInAndOut();

    // --- Show Gates / Show Wires visibility (feature-gap decision #1) ---
    void testSetGatesVisibleHidesGatesFromHitTesting();
    void testSetWiresVisibleHidesNodesFromHitTesting();

    // --- Port-hover peer labels ---
    void testHoverOverPortEmitsPortHoverChangedWithPeers();
    void testHoverOverNonPortEmitsNoPortHoverChips();
    void testHoverLeaveClearsPortHover();

    // --- Wire-creation port-press edge cases ---
    void testPressOnPortWhileWireInProgressCompletesConnection();
    void testPressOnEmptyInputPortStartsWireFromInput();
    void testPressOnOccupiedInputPortDetachesWire();

    // --- Right-click context menu ---
    void testRightClickOnElementOpensContextMenu();
    void testRightClickOnEmptyCanvasEmitsEmptyContextMenuRequested();
    void testCtrlDoubleClickOnWireDoesNotInsertSplitNode();
};
