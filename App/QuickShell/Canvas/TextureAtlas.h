// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief TextureAtlas: offscreen-render-to-texture cache backing Phase 2's batched gate rendering.
 */

#pragma once

#include <functional>

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
 * Single-page design: one CPU-side `QImage` page, packed with simple shelf (row) packing.
 * wiRedPanda's element icons are small (nominal 64x64) and the page is generous (2048x2048),
 * so a single page holds thousands of distinct cached tiles — multi-page support is not
 * implemented and not needed at this scale; see lookup()'s fallback behavior if the page ever
 * fills.
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
    /// actually upload. Returns an invalid (empty) TileLocation if \a tileSize is empty or the
    /// page has no room left for a new tile (see this class's doc comment on page sizing).
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

private:
    Q_DISABLE_COPY_MOVE(TextureAtlas)

    struct CachedTile {
        QRect pixelRect; // this tile's rect within m_page, in pixels
        QSize pixelSize;
    };

    /// Finds space for a \a tileSize tile via shelf packing, advancing m_shelf* state.
    /// Returns an empty QRect if the page has no room left.
    [[nodiscard]] QRect allocate(const QSize &tileSize);

    static constexpr int kPageSize = 2048;

    QImage m_page;
    QSGTexture *m_texture = nullptr;
    bool m_dirty = false;

    int m_shelfY = 0;
    int m_shelfX = 0;
    int m_shelfHeight = 0;

    QHash<QString, CachedTile> m_entries;
};
