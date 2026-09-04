// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Proves the test_wiredpanda harness works end to end for the Quick shell (whole-archived
/// wiredpanda_lib, offscreen QPA, real SVG icon resources, CanvasItem/QuickAppController
/// compiled a second time into this binary via QUICK_SHELL_SOURCES) before the fuller
/// behavioral parity tests alongside it.
class TestCanvasItemSmoke : public QObject
{
    Q_OBJECT

private slots:
    void testConstructWithoutDemoStartsEmpty();
    void testAddElementViaCommandUpdatesElements();
    void testAddElementUndoRemovesIt();

    // renderMinimapImage() caching: friend-class access to m_minimapRebuildCount lets these
    // assert on the cache-skip/rebuild-forced behavior directly, without depending on paint
    // timing.
    void testRenderMinimapImageSkipsRebuildWhenViewportUnchanged();
    void testRenderMinimapImageRebuildsAfterStructuralEdit();
    void testRenderMinimapImageRebuildsWhenZoomedOutPastCachedBounds();

    // --- Scrollbar-driving properties ---
    void testScrollSizeIsFullWhenContentFitsInView();
    void testScrollSizeShrinksWhenContentExceedsView();
    void testSetHorizontalScrollPositionPansView();
    void testResizeRefreshesScrollSizeWithoutAPanOrZoom();
};
