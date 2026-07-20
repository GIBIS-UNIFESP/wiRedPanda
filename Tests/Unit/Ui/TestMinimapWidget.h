// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestMinimapWidget : public QObject
{
    Q_OBJECT

private slots:

    void testComputeTransformWideSourceFillsWidget();
    void testComputeTransformTallSourceFillsWidget();
    void testComputeTransformNullScene();
    void testWidgetToSceneTopLeft();
    void testWidgetToSceneCenter();
    void testWidgetToSceneClampsOutOfBoundsX();
    void testWidgetToSceneNullSceneDegradesToOrigin();
    void testAccessibleNameSet(); // #14 accessibility sweep

    // Drag-to-resize / drag-to-move
    void testResizeModeAtEdgesAndCorners();
    void testResizeModeAtInteriorIsNone();
    void testApplyResizePreservesAspectRatio();
    void testApplyResizeClampsToMinimumSize();
    void testApplyResizeFromLeftKeepsRightEdgeFixed();
    void testApplyResizeFromTopKeepsBottomEdgeFixed();
    void testMoveHandleRectCoversTopStrip();
    void testIsMoveHandleDetectsTopStripOnly();
    void testMoveByClampsToParentBounds();

    // Visual affordances: hover state driving the handle highlight/cursor.
    void testHoverStateOverCornerSetsResizeCursorAndHighlight();
    void testHoverStateOverMoveStripSetsOpenHandCursorAndHighlight();
    void testHoverStateOverInteriorClearsHighlightAndCursor();

    // Theme
    void testThemeChangeInvalidatesCache();

    void testSizeHint();
    void testPaintEventNoOpWithoutSceneOrView();
    void testCursorForResizeModeMapsEachModeToExpectedCursor();

    // Mouse press
    void testMousePressGuardsNullSceneOrView();
    void testMousePressOnResizeHandleStartsResizing();
    void testMousePressOnMoveHandleStartsMoving();
    void testMousePressElsewhereCentersViewAndStartsDragging();

    void testWheelEventSwallowsEvent();

    // applyResize()'s minimum-size clamp: regression coverage for the fix that makes it
    // actually re-derive the other dimension from the locked aspect ratio (previously dead
    // code -- the clamp ran unconditionally one line earlier, making the "did we just clamp"
    // checks always false).
    void testApplyResizeFallsBackToSquareAspectWithNoScene();
    void testApplyResizeDiagonalHorizontalDominant();
    void testApplyResizeDiagonalVerticalDominant();
    void testApplyResizeAtWidthFloorRederivesHeightFromAspect();
    void testApplyResizeAtHeightFloorRederivesWidthFromAspect();

    void testMoveByIsNoOpWithoutParent();

    // Mouse move
    void testMouseMoveWhileResizingAppliesResizeAndInvalidatesCache();
    void testMouseMoveWhileMovingRepositionsWidget();
    void testMouseMoveUpdatesHoverStateWhenNotDragging();
    void testMouseMoveFallsBackToBaseClassWithoutSceneOrView();
    void testMouseMoveWhileDraggingRecentersView();

    // Mouse release
    void testMouseReleaseWhileResizingStopsAndEmitsGeometryChanged();
    void testMouseReleaseWhileMovingStopsAndEmitsGeometryChanged();
    void testMouseReleaseWhileDraggingStopsDragging();
    void testMouseReleaseFallsBackToBaseClassWhenIdle();
};
