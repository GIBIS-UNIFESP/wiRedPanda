// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Phase 7e (part 4) of the qtquick-rewrite plan: real coverage for CanvasItem's own protected
/// mousePressEvent()/mouseMoveEvent()/mouseReleaseEvent()/mouseDoubleClickEvent()/wheelEvent()/
/// keyPressEvent()/keyReleaseEvent() -- the genuinely new gesture-dispatch logic (click-to-select,
/// drag-to-move, rubber-band, wire-creation-by-drag, keyboard shortcuts) that has no permanent
/// test anywhere, unlike the Edit/Transform-menu delegation TestQuickAppController (7e-1) already
/// covers by calling CanvasItem's public action methods directly.
///
/// Technique: synthesize a real QMouseEvent/QKeyEvent/QWheelEvent and deliver it via
/// QCoreApplication::sendEvent(&canvas, &event) -- QQuickItem::event() (its own QObject::event()
/// override) dispatches by QEvent::Type to the exact same protected virtual handler a real
/// window's input pipeline would call, so this exercises the identical code path a live user
/// gesture would, without needing a shown/exposed QQuickWindow (mirrors the technique
/// project_xwayland_synthetic_input_broken.md documents: bypass OS/window-level input delivery,
/// synthesize the Qt event directly in-process). No QQuickWindow, no friend declaration, and no
/// CanvasItem production-code change was needed to make this work.
///
/// Click/drag coordinates are computed directly from each element's/port's own pos()/scenePos(),
/// matching rebuildSpatialIndex()'s exact "boundingRect().translated(pos())" hit-test box (see
/// CanvasItem.cpp) -- not eyeballed pixel constants. worldToScreen()/screenToWorld() are the
/// identity transform at this class's default zoom/pan, so world and screen coordinates coincide
/// throughout unless a test explicitly changes zoom/pan.
///
/// Out of scope for this pass, left for later 7e sub-steps: context menu operations,
/// drag-from-palette, and every dialog/window-chrome-specific test (About, fullscreen, theme
/// switching, language change, toolbar/status-bar widget presence, Learn menu structure) -- none
/// of those go through CanvasItem's own event handlers at all.
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

    // --- Mouse interactions (CanvasItem::mousePressEvent/mouseMoveEvent/mouseReleaseEvent/
    // mouseDoubleClickEvent/wheelEvent dispatch) ---
    void testMouseClickSelectsElement();
    void testMouseClickEmptyDeselects();
    void testShiftClickTogglesMultiSelect();
    void testCtrlClickClonesSelectionInPlace();
    void testMouseDragMovesElement();
    void testRubberBandSelectionSelectsEnclosedElements();
    void testWireCreationByDraggingPortToPort();
    void testDoubleClickLabeledElementEmitsInlineEditRequested();
    void testWheelZoomsInAndOut();
};
