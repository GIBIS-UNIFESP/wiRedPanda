// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief TextureAtlas: offscreen-render-to-texture cache backing the batched gate renderer.
 */

#pragma once

#include <functional>
#include <utility>

#include <QHash>
#include <QImage>
#include <QRectF>
#include <QSize>
#include <QString>

class QPainter;
class QQuickWindow;
class QSGTexture;

/**
 * \class TextureAtlas
 * \brief Packs offscreen-rendered element appearances into one shared GPU texture page.
 *
 * \details Every custom-painted element family (ordinary SVG icons via `ElementAppearance`,
 * `IC` via `ICRenderer`, the `Mux`/`Demux`/`TruthTable` vector-body trio, the `Display7`/
 * `Display14`/`Display16` segment trio) reuses its *existing*, unmodified `QPainter`-based
 * paint logic here, redirected to an offscreen tile instead of a live paint device — the same
 * technique `Node::renderWirelessPixmap()` already uses in production
 * (`App/Element/GraphicElements/Node.cpp`). This class is the shared cache + GPU upload
 * mechanism every family plugs into, so the batched `QSGGeometryNode` gate renderer draws real
 * per-element appearance with one texture bind, not one draw call per element.
 *
 * Cache keys are opaque strings the caller builds — deliberately not a fixed (path, angle,
 * flip) tuple, since different families need different key dimensions (port count for
 * `Mux`/`Demux`/`TruthTable`; embedded-state for `IC`; color + active-segment bitmask for the
 * `Display*` trio). Callers are expected to build a key that fully captures everything the
 * rendered tile's pixels depend on.
 *
 * Single-page design: one CPU-side `QImage` page, packed with simple shelf (row) packing,
 * starting at 2048x2048 and doubling (up to 8192x8192) whenever a tile doesn't fit rather than
 * failing outright — see allocate()/grow(). `appearanceKeyFor()` bakes live port status into
 * the cache key, so a large continuously-clocked circuit keeps needing new distinct entries
 * for as long as it runs; a fixed-capacity page would eventually fill, and every element that
 * then needed a *new* tile would silently stop rendering at all (CanvasItem's own `if
 * (!tile.isValid()) continue;`), permanently once that failure got cached too. Growing instead
 * of failing trades a bit more texture memory for correctness; see takeGrew()'s own doc
 * comment for the one caveat this shifts onto the caller.
 */
class TextureAtlas
{
public:
    TextureAtlas() = default;
    ~TextureAtlas();

    /// A located tile's placement within the atlas page: its UV rect (normalized [0,1]) and
    /// actual pixel size (for aspect-correct quad sizing). Deliberately carries no texture
    /// pointer — see texture()'s doc comment for why GPU upload is a separate, once-per-frame
    /// step rather than something lookup() does immediately on every cache miss.
    struct TileLocation {
        QRectF uv;
        QSize pixelSize;

        [[nodiscard]] bool isValid() const { return !pixelSize.isEmpty(); }
    };

    /// Returns the placement for \a key, rendering it via \a renderTile (called with a QPainter
    /// targeting a transparent, \a tileSize-sized surface at local origin (0,0)) on a cache
    /// miss. Purely CPU-side: a miss updates the page image and marks it dirty, but does not
    /// touch the GPU — call texture() once per frame, after every lookup() that frame, to
    /// actually upload. Grows the page first (see grow()) rather than failing if the current one
    /// has no room left — see takeGrew() for the one thing that shifts onto the caller when that
    /// happens. Returns an invalid (empty) TileLocation only if \a tileSize is empty, or doesn't
    /// fit even on a page already grown to kMaxPageSize.
    TileLocation lookup(const QString &key, const QSize &tileSize,
                        const std::function<void(QPainter &)> &renderTile);

    /// Returns the current GPU texture, re-uploading from the CPU-side page first if anything
    /// changed since the last call. Call once per frame, after all of that frame's lookup()
    /// calls — not once per lookup() — so N cache misses in one frame cost one upload, not N.
    /// \a window must be the consuming QQuickItem's own window(), valid from updatePaintNode().
    QSGTexture *texture(QQuickWindow *window);

    /// Drops every cached tile and the GPU texture (e.g. on a theme change that invalidates
    /// every cached appearance at once). The next lookup() for any key re-renders from scratch.
    void clear();

    /// Returns the number of distinct tiles currently cached. Exposed for tests/diagnostics.
    [[nodiscard]] qsizetype cachedTileCount() const { return m_entries.size(); }

    /// Returns whether the page grew since the last call (and resets the flag). Growing mid-
    /// frame changes the page's own size, so any UV a *different*, earlier lookup() already
    /// handed back this same frame (already baked into that element's own vertex data by the
    /// time a later element's lookup() triggers growth) is stale relative to the final, grown
    /// page -- self-corrects on the very next repaint, once every element's UV is recomputed
    /// against the now-stable size, so the caller just needs to schedule one (e.g.
    /// QQuickItem::update()) when this returns true, mirroring how a cache-miss mid-frame
    /// already costs a recompute, just one frame later than usual for a grow specifically.
    bool takeGrew() { return std::exchange(m_grew, false); }

private:
    Q_DISABLE_COPY_MOVE(TextureAtlas)

    struct CachedTile {
        QRect pixelRect; // this tile's rect within m_page, in pixels
        QSize pixelSize;
    };

    /// Finds space for a \a tileSize tile via shelf packing, advancing m_shelf* state -- growing
    /// the page first (see grow()) if the current one has no room left. Returns an empty QRect
    /// only if \a tileSize doesn't fit even on a page already grown to kMaxPageSize.
    [[nodiscard]] QRect allocate(const QSize &tileSize);

    /// Doubles the page's side length (up to kMaxPageSize), copying the existing pixel content
    /// into the new, larger QImage at its original (0,0) origin -- every existing entry's
    /// pixelRect stays valid unchanged; only the denominator lookup()/allocate() normalize UV
    /// against grows. Returns false if already at kMaxPageSize (nothing to do).
    bool grow();

    static constexpr int kInitialPageSize = 2048;
    // A widely-supported GPU texture size ceiling (every desktop/mobile GPU from the last
    // decade-plus supports at least 8192x8192 2D textures) -- growing past this would risk
    // exceeding some driver's real hardware limit instead of trading a bit more memory for
    // capacity, so allocate() gives up past this size rather than growing further.
    static constexpr int kMaxPageSize = 8192;

    QImage m_page;
    int m_pageSize = kInitialPageSize;
    QSGTexture *m_texture = nullptr;
    bool m_dirty = false;

    // Set by grow(); see takeGrew()'s own doc comment above.
    bool m_grew = false;

    int m_shelfY = 0;
    int m_shelfX = 0;
    int m_shelfHeight = 0;

    QHash<QString, CachedTile> m_entries;
};
