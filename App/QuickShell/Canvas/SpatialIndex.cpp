// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Canvas/SpatialIndex.h"

#include <algorithm>
#include <cmath>

#include <QSet>

SpatialIndex::SpatialIndex(const qreal cellSize)
    : m_cellSize(cellSize)
{
}

quint64 SpatialIndex::cellKey(const int cellX, const int cellY)
{
    // Reinterpreting int as quint32 (not offsetting by a bias) is a bijection over the full
    // int32 range, so this stays unique for any coordinate the canvas can actually reach.
    return (quint64(quint32(cellX)) << 32) | quint64(quint32(cellY));
}

QRect SpatialIndex::cellRange(const QRectF &bounds) const
{
    const int minX = static_cast<int>(std::floor(bounds.left() / m_cellSize));
    const int minY = static_cast<int>(std::floor(bounds.top() / m_cellSize));
    // right()/bottom() are inclusive of the last occupied cell, not one-past-the-end: a rect
    // whose edge sits exactly on a cell boundary still needs that boundary cell counted on
    // both sides, and floor() of the edge coordinate already gives the correct inclusive index.
    const int maxX = static_cast<int>(std::floor(bounds.right() / m_cellSize));
    const int maxY = static_cast<int>(std::floor(bounds.bottom() / m_cellSize));
    return QRect(QPoint(minX, minY), QPoint(maxX, maxY));
}

void SpatialIndex::insertIntoCells(const quint64 id, const QRectF &bounds)
{
    const QRect range = cellRange(bounds);
    for (int cy = range.top(); cy <= range.bottom(); ++cy) {
        for (int cx = range.left(); cx <= range.right(); ++cx) {
            m_cells[cellKey(cx, cy)].append(id);
        }
    }
}

void SpatialIndex::removeFromCells(const quint64 id, const QRectF &bounds)
{
    const QRect range = cellRange(bounds);
    for (int cy = range.top(); cy <= range.bottom(); ++cy) {
        for (int cx = range.left(); cx <= range.right(); ++cx) {
            const quint64 key = cellKey(cx, cy);
            const auto it = m_cells.find(key);
            if (it == m_cells.end()) {
                continue;
            }
            it->removeAll(id);
            if (it->isEmpty()) {
                m_cells.erase(it);
            }
        }
    }
}

void SpatialIndex::insertBox(const quint64 id, const QRectF &bounds)
{
    remove(id);
    Entry entry;
    entry.bounds = bounds;
    entry.order = m_nextOrder++;
    m_entries.insert(id, entry);
    insertIntoCells(id, bounds);
}

void SpatialIndex::insertShape(const quint64 id, const QRectF &bounds, const QPainterPath &shape)
{
    remove(id);
    Entry entry;
    entry.bounds = bounds;
    entry.shape = shape;
    entry.order = m_nextOrder++;
    m_entries.insert(id, entry);
    insertIntoCells(id, bounds);
}

void SpatialIndex::remove(const quint64 id)
{
    const auto it = m_entries.find(id);
    if (it == m_entries.end()) {
        return;
    }
    removeFromCells(id, it->bounds);
    m_entries.erase(it);
}

bool SpatialIndex::contains(const quint64 id) const
{
    return m_entries.contains(id);
}

QRectF SpatialIndex::boundsOf(const quint64 id) const
{
    const auto it = m_entries.constFind(id);
    return it == m_entries.constEnd() ? QRectF() : it->bounds;
}

bool SpatialIndex::testPoint(const Entry &entry, const QPointF &pos)
{
    return entry.shape.isEmpty() ? entry.bounds.contains(pos) : entry.shape.contains(pos);
}

bool SpatialIndex::testRect(const Entry &entry, const QRectF &rect)
{
    return entry.shape.isEmpty() ? entry.bounds.intersects(rect) : entry.shape.intersects(rect);
}

QVector<quint64> SpatialIndex::queryPoint(const QPointF &pos) const
{
    const int cx = static_cast<int>(std::floor(pos.x() / m_cellSize));
    const int cy = static_cast<int>(std::floor(pos.y() / m_cellSize));
    const auto cellIt = m_cells.constFind(cellKey(cx, cy));
    if (cellIt == m_cells.constEnd()) {
        return {};
    }

    QVector<quint64> hits;
    for (const quint64 id : cellIt.value()) {
        const auto entryIt = m_entries.constFind(id);
        if (entryIt != m_entries.constEnd() && testPoint(entryIt.value(), pos)) {
            hits.append(id);
        }
    }
    std::sort(hits.begin(), hits.end(), [this](const quint64 a, const quint64 b) {
        return m_entries.value(a).order < m_entries.value(b).order;
    });
    return hits;
}

QVector<quint64> SpatialIndex::queryRect(const QRectF &rect) const
{
    const QRect range = cellRange(rect);
    QSet<quint64> seen;
    QVector<quint64> hits;
    for (int cy = range.top(); cy <= range.bottom(); ++cy) {
        for (int cx = range.left(); cx <= range.right(); ++cx) {
            const auto cellIt = m_cells.constFind(cellKey(cx, cy));
            if (cellIt == m_cells.constEnd()) {
                continue;
            }
            for (const quint64 id : cellIt.value()) {
                if (seen.contains(id)) {
                    continue;
                }
                const auto entryIt = m_entries.constFind(id);
                if (entryIt != m_entries.constEnd() && testRect(entryIt.value(), rect)) {
                    seen.insert(id);
                    hits.append(id);
                }
            }
        }
    }
    std::sort(hits.begin(), hits.end(), [this](const quint64 a, const quint64 b) {
        return m_entries.value(a).order < m_entries.value(b).order;
    });
    return hits;
}

void SpatialIndex::clear()
{
    m_entries.clear();
    m_cells.clear();
    m_nextOrder = 0;
}
