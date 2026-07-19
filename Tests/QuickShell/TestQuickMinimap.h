// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Phase 7f of the qtquick-rewrite plan: real coverage for CanvasItem::minimapContentRect() and
/// QuickMinimap (App/QuickShell/Chrome/QuickMinimap), the presenter backing Minimap.qml --
/// mirrors the real, portable slice of Tests/Unit/Ui/TestMinimapWidget.cpp's intent.
///
/// Twelve of that Widgets file's twenty tests have NO equivalent to port here, by design --
/// confirmed by reading project_minimap_qml_landed.md and QuickMinimap.h/Minimap.qml, not
/// assumed: MinimapWidget's own hand-rolled ResizeMode/moveHandleRect()/hover-state C++ machinery
/// was deliberately NOT ported when this class landed -- Minimap.qml's 8 resize handles and move
/// strip are QML-native MouseArea drag math instead, a fundamentally different implementation
/// with no C++ unit to test the same way (testResizeModeAtEdgesAndCorners/
/// testResizeModeAtInteriorIsNone/testApplyResize*/testMoveHandleRectCoversTopStrip/
/// testIsMoveHandleDetectsTopStripOnly/testMoveByClampsToParentBounds/testHoverStateOver* --
/// 9 tests -- plus 3 more of the same shape). testAccessibleNameSet has the same disposition
/// already established in TestQuickCanvasZoom.h's doc comment (zero Accessible.* usage anywhere
/// in App/QuickShell/ -- a real, separately-tracked accessibility gap, not this pass's job).
///
/// What DOES port, since it's real, portable geometry/navigation logic: MinimapWidget::
/// computeTransform() became CanvasItem::minimapContentRect() (note one deliberate behavioral
/// difference -- Quick's version unions the content bounds with the *live viewport* before
/// fitting, so a minimap's content rect can genuinely be larger than the elements alone; see
/// minimapContentRect()'s own doc comment); MinimapWidget::widgetToScene() became
/// QuickMinimap::navigateTo(). regenerate()'s imageUrl()/viewportRect() outputs are throttled
/// behind a real 200ms QTimer (a deliberate divergence from MinimapWidget's own content/view
/// throttle split, see QuickMinimap's class doc comment) -- exercised here with one real
/// QTest::qWait(), not mocked away.
class TestQuickMinimap : public QObject
{
    Q_OBJECT

private slots:
    void testMinimapContentRectEmptyWithNoContent();
    void testMinimapContentRectEmptyWithZeroTargetSize();
    void testMinimapContentRectMatchesTargetAspectForWideContent();
    void testMinimapContentRectMatchesTargetAspectForTallContent();
    void testMinimapContentRectNeverCropsElementBounds();

    void testContentAspectRatioDefaultsToOneWithNoCanvas();
    void testContentAspectRatioReflectsCircuitBounds();

    void testNavigateToPansCanvasToClickedContentCorner();
    void testNavigateToClampsOutOfBoundsToContentRect();
    void testNavigateToNoOpWithoutMinimapSize();

    void testGeometryPersistsViaSettings();
    void testRegenerateProducesImageAndViewportRectAfterThrottle();
};
