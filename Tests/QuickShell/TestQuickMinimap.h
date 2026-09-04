// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Tests CanvasItem::minimapContentRect() and QuickMinimap (App/QuickShell/Chrome/QuickMinimap),
/// the presenter backing Minimap.qml.
///
/// Minimap.qml's resize handles and move strip are QML-native MouseArea drag math with no C++
/// unit to test directly, so this file covers only the portable geometry/navigation logic:
/// minimapContentRect() unions the content bounds with the *live viewport* before fitting, so a
/// minimap's content rect can be larger than the elements alone; QuickMinimap::navigateTo()
/// converts a minimap click back to a scene position. regenerate()'s imageUrl()/viewportRect()
/// outputs are throttled behind a real 200ms QTimer -- exercised here with one real
/// QTest::qWait()-backed QSignalSpy, not mocked away.
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
    void testThemeChangeInvalidatesCache();

    // --- clampGeometry() -- mirrors WorkSpace::applyMinimapGeometry()'s shared clamp block,
    // which Minimap.qml's clampToParent() delegates to instead of duplicating the arithmetic in
    // untestable QML.
    void testClampGeometryReturnsUnchangedWhenAlreadyWithinBounds();
    void testClampGeometryReclampsWhenParentShrinks();
    void testClampGeometryClampsOversizedCandidate();
};
