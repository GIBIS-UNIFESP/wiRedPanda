// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Regression coverage for App/QuickShell/Canvas/TextureAtlas.h's page-growth behavior: a
/// large, continuously-clocked circuit (thousands of elements, appearanceKeyFor() baking live
/// port status into the cache key) was reported live making flip-flops go permanently invisible
/// once the clock started toggling -- root-caused to the atlas's original fixed 2048x2048 page
/// silently failing new allocations once full, with CanvasItem::updatePaintNode()'s own
/// `if (!tile.isValid()) continue;` skipping the element's render entirely, forever (the failure
/// itself got cached too). Exercises lookup()/takeGrew()/clear() directly -- no QQuickWindow
/// needed, since texture() (the only method that touches the GPU) is deliberately not exercised
/// here.
class TestTextureAtlas : public QObject
{
    Q_OBJECT

private slots:
    void testLookupCacheHitReturnsIdenticalLocation();
    void testLookupGrowsPageWhenCapacityExceeded();
    void testTakeGrewResetsAfterBeingRead();
    void testLookupHandlesEmptyTileSizeGracefully();
    void testClearDropsAllCachedEntries();
};
