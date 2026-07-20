// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief GraphicElementLabel: a plain, framework-agnostic replacement for the
 * QGraphicsSimpleTextItem GraphicElement/Text used to display their label/empty-state hint.
 */

#pragma once

#include <QBrush>
#include <QFont>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QTransform>

class QPainter;

/**
 * \class GraphicElementLabel
 * \brief Plain text-glyph item: same pos()/transform()/font()/brush()/text()/isVisible()/paint()
 * surface `QGraphicsSimpleTextItem` offered, with zero QtWidgets dependency.
 *
 * \details `GraphicElement::m_label`/`Text::m_emptyHint` were both real `QGraphicsSimpleTextItem`
 * child items — that's a `QGraphicsItem` subclass declared in `<QGraphicsItem>` (part of
 * `Qt6::Widgets`), so simply not having `GraphicElement` itself inherit `QGraphicsItem` wasn't
 * enough on its own to drop the Widgets dependency; this member needed replacing too.
 * `pos()`/`transform()` are composed by the owner (`GraphicElement::updateLabelOrientation()`)
 * exactly as before; `paint()` assumes the caller has already applied both to the painter (the
 * same convention `CanvasItem::paintElementDecorations()` already used for the real
 * `QGraphicsSimpleTextItem::paint()` call it replaces) and draws at local (0,0), matching
 * `QGraphicsSimpleTextItem`'s own top-left-at-origin convention.
 */
class GraphicElementLabel
{
public:
    [[nodiscard]] QPointF pos() const { return m_pos; }
    void setPos(const QPointF &pos) { m_pos = pos; }

    [[nodiscard]] QTransform transform() const { return m_transform; }
    void setTransform(const QTransform &transform) { m_transform = transform; }

    [[nodiscard]] QFont font() const { return m_font; }
    void setFont(const QFont &font) { m_font = font; }

    [[nodiscard]] QBrush brush() const { return m_brush; }
    void setBrush(const QBrush &brush) { m_brush = brush; }

    [[nodiscard]] QString text() const { return m_text; }
    void setText(const QString &text) { m_text = text; }

    [[nodiscard]] bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    /// Local-frame bounding rect, top-left at (0,0) -- mirrors QGraphicsSimpleTextItem's own
    /// contract, which GraphicElement::labelSceneBoundingRect()/Text::boundingRect() depend on.
    [[nodiscard]] QRectF boundingRect() const;

    /// Draws the text at local (0,0) using font()/brush() -- caller applies pos()/transform() to
    /// the painter first (see class doc comment). No-op when !isVisible() or text().isEmpty().
    void paint(QPainter *painter) const;

private:
    QPointF m_pos;
    QTransform m_transform;
    QFont m_font;
    QBrush m_brush{Qt::black};
    QString m_text;
    bool m_visible = true;
};
