// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Uniform-grid spatial index for the Quick canvas's hit-testing.
 */

#pragma once

#include <QHash>
#include <QPainterPath>
#include <QRectF>
#include <QVector>

/**
 * \class SpatialIndex
 * \brief Uniform-grid spatial index backing the Quick canvas's hit-testing.
 *
 * \details Replaces QGraphicsScene's BSP-tree-backed itemAt()/portAt()/items(rect) now that
 * gates and wires are batched into shared QSGGeometryNodes instead of being individual
 * QGraphicsItems the scene can index itself. Entries are identified by an opaque quint64
 * handle the caller controls (typically a GraphicElement/Port/Connection id) rather than owning
 * pointers, so the index has no lifetime coupling to whatever it indexes.
 *
 * Two entry kinds:
 *  - A box entry (insertBox()) is hit-tested against its axis-aligned bounds directly — used
 *    for gates and ports, which are always axis-aligned rectangles/squares in local space.
 *  - A shape entry (insertShape()) carries an exact QPainterPath (a wire's flattened Bézier
 *    stroke, mirroring Connection::shape()'s own m_cachedShape) — a point inside the shape's
 *    bounding box but off the stroke itself is correctly reported as a miss, exactly as
 *    Connection::shape()-based hit-testing already behaves for the Widgets canvas.
 *
 * Entries are stored in every grid cell their bounds overlap, so a point query only ever needs
 * to inspect the single cell containing that point — an entry can never be missed because it
 * always occupies the cell any point inside it would fall into.
 *
 * Z-order / stacking: queryPoint() returns exact hits in insertion order (last inserted last),
 * a reasonable proxy for "most recently added is on top" at this stage. This canvas has no
 * overlapping-element stacking policy yet (Phase 1's demo circuit never overlaps) — revisit
 * when Phase 3 (multi-select, IC embedding) introduces real stacking scenarios.
 */
class SpatialIndex
{
public:
    /// Constructs an empty index. \a cellSize should roughly match the typical entry size
    /// (the nominal 64x64 gate body) — too small wastes memory on cell bookkeeping, too large
    /// degrades toward "every query scans everything".
    explicit SpatialIndex(qreal cellSize = 64.0);

    /// Inserts or replaces the entry for \a id with axis-aligned \a bounds (a gate or a port).
    void insertBox(quint64 id, const QRectF &bounds);

    /// Inserts or replaces the entry for \a id with an exact \a shape (a wire's flattened
    /// stroke) — \a bounds must be shape.boundingRect(), passed separately so callers that
    /// already have it cached (as Connection does) don't pay to recompute it.
    void insertShape(quint64 id, const QRectF &bounds, const QPainterPath &shape);

    /// Removes the entry for \a id, if present. No-op if absent.
    void remove(quint64 id);

    /// Returns \c true if the index currently has an entry for \a id.
    [[nodiscard]] bool contains(quint64 id) const;

    /// Returns the number of entries currently indexed.
    [[nodiscard]] qsizetype size() const { return m_entries.size(); }

    /// Returns the stored bounds for \a id, or an empty QRectF if absent.
    [[nodiscard]] QRectF boundsOf(quint64 id) const;

    /// Returns every id whose bounds/shape actually contains \a pos (already exact-tested,
    /// not just bounding-box candidates), in insertion order.
    [[nodiscard]] QVector<quint64> queryPoint(const QPointF &pos) const;

    /// Returns every id whose bounds/shape actually intersects \a rect (already exact-tested),
    /// in insertion order. Mirrors QGraphicsScene::items(rect)'s IntersectsItemShape mode.
    [[nodiscard]] QVector<quint64> queryRect(const QRectF &rect) const;

    /// Removes every entry. The cell grid itself is retained (its bucket structure is cheap
    /// to keep and reused by the next insert wave — this canvas rebuilds the whole index once
    /// per structural change, not incrementally per element, so avoiding a full deallocate/
    /// reallocate cycle on every rebuild is worth the small retained footprint).
    void clear();

private:
    struct Entry {
        QRectF bounds;
        QPainterPath shape; // isEmpty() => box entry, hit-test against bounds directly
        int order = 0;      // insertion sequence, for the queryPoint()/queryRect() ordering guarantee
    };

    [[nodiscard]] static quint64 cellKey(int cellX, int cellY);
    [[nodiscard]] QRect cellRange(const QRectF &bounds) const;
    void insertIntoCells(quint64 id, const QRectF &bounds);
    void removeFromCells(quint64 id, const QRectF &bounds);
    [[nodiscard]] static bool testPoint(const Entry &entry, const QPointF &pos);
    [[nodiscard]] static bool testRect(const Entry &entry, const QRectF &rect);

    qreal m_cellSize;
    int m_nextOrder = 0;
    QHash<quint64, Entry> m_entries;
    QHash<quint64, QVector<quint64>> m_cells; // packed (cellX, cellY) -> entry ids
};
