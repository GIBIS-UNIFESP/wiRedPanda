// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Canvas/TextureAtlas.h"

#include <algorithm>

#include <QPainter>
#include <QQuickWindow>
#include <QSGTexture>

TextureAtlas::~TextureAtlas()
{
    delete m_texture;
}

bool TextureAtlas::grow()
{
    if (m_pageSize >= kMaxPageSize) {
        return false;
    }

    const int newSize = std::min(m_pageSize * 2, kMaxPageSize);
    QImage grown(newSize, newSize, QImage::Format_ARGB32_Premultiplied);
    grown.fill(Qt::transparent);
    if (!m_page.isNull()) {
        QPainter painter(&grown);
        painter.drawImage(0, 0, m_page);
    }
    m_page = std::move(grown);
    m_pageSize = newSize;
    m_dirty = true;
    m_grew = true;
    return true;
}

QRect TextureAtlas::allocate(const QSize &tileSize)
{
    if (tileSize.isEmpty()) {
        return {};
    }

    if (m_page.isNull()) {
        m_page = QImage(m_pageSize, m_pageSize, QImage::Format_ARGB32_Premultiplied);
        m_page.fill(Qt::transparent);
    }

    // Shelf (row) packing: place tiles left-to-right on the current shelf; start a new shelf
    // below when the current one runs out of width; grow the whole page (see grow()) once
    // shelves run out of height, rather than failing -- no eviction/reflow within a page size,
    // see this class's own doc comment for why a fixed page that just gave up wasn't enough.
    if (m_shelfX + tileSize.width() > m_pageSize) {
        m_shelfY += m_shelfHeight;
        m_shelfX = 0;
        m_shelfHeight = 0;
    }

    while ((m_shelfX + tileSize.width() > m_pageSize) || (m_shelfY + tileSize.height() > m_pageSize)) {
        if (!grow()) {
            return {}; // tileSize itself doesn't fit even at kMaxPageSize
        }
    }

    const QRect rect(m_shelfX, m_shelfY, tileSize.width(), tileSize.height());
    m_shelfX += tileSize.width();
    m_shelfHeight = std::max(m_shelfHeight, tileSize.height());
    return rect;
}

TextureAtlas::TileLocation TextureAtlas::lookup(const QString &key, const QSize &tileSize,
                                                  const std::function<void(QPainter &)> &renderTile)
{
    if (tileSize.isEmpty()) {
        return {};
    }

    if (const auto it = m_entries.constFind(key); it != m_entries.constEnd()) {
        const auto &tile = it.value();
        const QRectF uv(qreal(tile.pixelRect.x()) / m_pageSize, qreal(tile.pixelRect.y()) / m_pageSize,
                         qreal(tile.pixelRect.width()) / m_pageSize, qreal(tile.pixelRect.height()) / m_pageSize);
        return TileLocation{uv, tile.pixelSize};
    }

    const QRect rect = allocate(tileSize);
    if (rect.isEmpty()) {
        return {}; // tileSize itself doesn't fit even at kMaxPageSize -- caller falls back to no appearance rather than crashing
    }

    QPainter painter(&m_page);
    painter.translate(rect.topLeft());
    painter.setClipRect(QRect(QPoint(0, 0), tileSize));
    renderTile(painter);
    painter.end();

    m_entries.insert(key, CachedTile{rect, tileSize});
    m_dirty = true;

    const QRectF uv(qreal(rect.x()) / m_pageSize, qreal(rect.y()) / m_pageSize,
                     qreal(rect.width()) / m_pageSize, qreal(rect.height()) / m_pageSize);
    return TileLocation{uv, tileSize};
}

QSGTexture *TextureAtlas::texture(QQuickWindow *window)
{
    if (m_dirty || !m_texture) {
        delete m_texture;
        m_texture = window->createTextureFromImage(m_page, QQuickWindow::TextureHasAlphaChannel);
        m_dirty = false;
    }
    return m_texture;
}

void TextureAtlas::clear()
{
    m_entries.clear();
    m_page = QImage();
    m_shelfX = 0;
    m_shelfY = 0;
    m_shelfHeight = 0;
    delete m_texture;
    m_texture = nullptr;
    m_dirty = false;
}
