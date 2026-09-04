// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestTextureAtlas.h"

#include <QPainter>
#include <QSize>

#include "App/QuickShell/Canvas/TextureAtlas.h"

namespace {
void noopRender(QPainter & /*painter*/) { }
} // namespace

void TestTextureAtlas::testLookupCacheHitReturnsIdenticalLocation()
{
    TextureAtlas atlas;
    const auto first = atlas.lookup(QStringLiteral("a"), QSize(64, 64), noopRender);
    QVERIFY(first.isValid());

    const auto second = atlas.lookup(QStringLiteral("a"), QSize(64, 64), noopRender);
    QVERIFY(second.isValid());
    QCOMPARE(second.uv, first.uv);
    QCOMPARE(second.pixelSize, first.pixelSize);
    QCOMPARE(atlas.cachedTileCount(), 1);
}

void TestTextureAtlas::testLookupGrowsPageWhenCapacityExceeded()
{
    TextureAtlas atlas;

    // A 2048x2048 initial page holds exactly 32x32 = 1024 non-overlapping 64x64 tiles via
    // shelf packing -- allocating clearly more than that with distinct keys forces at least
    // one grow(). Mirrors what a large continuously-clocked circuit does over time: each
    // element's own appearanceKeyFor() (rotation/flip/selection/label unchanged) still churns
    // through many distinct keys as its ports' live status combinations diverge.
    constexpr int kTileCount = 1100;
    for (int i = 0; i < kTileCount; ++i) {
        const auto tile = atlas.lookup(QString::number(i), QSize(64, 64), noopRender);
        QVERIFY2(tile.isValid(), qPrintable(QStringLiteral("tile %1 should never silently fail").arg(i)));
    }

    QCOMPARE(atlas.cachedTileCount(), kTileCount);
    QVERIFY2(atlas.takeGrew(), "page should have grown at least once past 1024 distinct 64x64 tiles");

    // Every already-cached key must still resolve correctly after growth -- a stale, pre-grow
    // UV baked into a still-cached entry would be exactly the invisible-element bug this class
    // exists to prevent, just for a *different* subset of elements than an outright allocation
    // failure would hit.
    for (int i = 0; i < kTileCount; ++i) {
        const auto tile = atlas.lookup(QString::number(i), QSize(64, 64), noopRender);
        QVERIFY(tile.isValid());
    }
}

void TestTextureAtlas::testTakeGrewResetsAfterBeingRead()
{
    TextureAtlas atlas;
    QVERIFY(!atlas.takeGrew()); // nothing grown yet on a fresh atlas

    for (int i = 0; i < 1100; ++i) {
        atlas.lookup(QString::number(i), QSize(64, 64), noopRender);
    }

    QVERIFY(atlas.takeGrew());  // consumes the flag
    QVERIFY(!atlas.takeGrew()); // already consumed -- stays false until the next real grow
}

void TestTextureAtlas::testLookupHandlesEmptyTileSizeGracefully()
{
    TextureAtlas atlas;
    const auto tile = atlas.lookup(QStringLiteral("degenerate"), QSize(0, 0), noopRender);
    QVERIFY(!tile.isValid());
    QCOMPARE(atlas.cachedTileCount(), 0);
}

void TestTextureAtlas::testClearDropsAllCachedEntries()
{
    TextureAtlas atlas;
    atlas.lookup(QStringLiteral("a"), QSize(64, 64), noopRender);
    atlas.lookup(QStringLiteral("b"), QSize(64, 64), noopRender);
    QCOMPARE(atlas.cachedTileCount(), 2);

    atlas.clear();
    QCOMPARE(atlas.cachedTileCount(), 0);

    // A key that was cached before clear() must be treated as a fresh miss, not silently
    // resolve to whatever pixelRect it used to have.
    const auto tile = atlas.lookup(QStringLiteral("a"), QSize(64, 64), noopRender);
    QVERIFY(tile.isValid());
    QCOMPARE(atlas.cachedTileCount(), 1);
}
