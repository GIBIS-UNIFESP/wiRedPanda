// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestGraphicsView : public QObject
{
    Q_OBJECT

private slots:

    // The zoom ladder, fit, centring, scrolling into view and the wheel zoom, the scroll position in every row, compared
    // with Tests/Integration/GraphicsView/behaviours.txt.
    void testViewBehavioursMatchGolden();

    void testZoomMinimumLimit();
    void testZoomMaximumLimit();
    void testResetZoom();
    void testZoomToFit();
    void testFastMode();
    void testDragModeToggle();
    void testAccessibleNameSet(); // #14 accessibility sweep

    // Middle-button drag panning: press starts the pan, release ends it.
    void testMiddleButtonPanPressAndRelease();

    // Auto-repeat key events must be forwarded without toggling space-pan mode.
    void testAutoRepeatKeyPressForwardsWithoutEnteringPanMode();
    void testAutoRepeatKeyReleaseDoesNotExitPanMode();

    // wheelEvent()'s zoom-out branch (zoom-in is covered by TestMainWindowGui).
    void testWheelEventZoomsOut();

    // A wheel zoom keeps the scene point under the cursor, and the view inside the scene rect near its edge.
    void testWheelKeepsCursorPoint();
    void testWheelNearSceneEdgeKeepsViewInsideSceneRect();
    void testZoomKeepsItsPivotPastTheSceneRect();

    // zoomToFit() fits the circuit, not a rubber band beside it.
    void testZoomToFitIgnoresARubberBandOutsideTheCircuit();

    // zoomToFit()'s remaining branches: no scene, selection-only, non-Scene fallback, empty target.
    void testZoomToFitWithNoSceneIsNoOp();
    void testZoomToFitFitsSelectionOnly();
    void testZoomToFitWithPlainSceneUsesItemsBoundingRect();
    void testZoomToFitWithEmptySceneIsNoOp();
};
