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
};
