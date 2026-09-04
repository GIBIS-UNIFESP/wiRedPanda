// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Plain (non-Qt-Graphics-View) replacement for a GraphicElement's label child item.
 */

#pragma once

#include <QBrush>
#include <QFont>
#include <QFontMetricsF>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QTransform>

class GraphicElement;
class QPainter;

/**
 * \class GraphicElementLabel
 * \brief Owns a GraphicElement's name/trigger label: its text, font, brush, and the
 * position/transform pair that places it in the owner's local frame.
 *
 * \details Replaces the old QGraphicsSimpleTextItem child item now that GraphicElement no
 * longer derives from QGraphicsItem: nothing in Qt's Graphics View framework paints or
 * positions this any more, so it is a plain value collaborator (same pattern as
 * ElementAppearance/ElementOrientation/ElementPorts/ElementSimState — owner back-pointer,
 * used through GraphicElement's own public interface). \a pos()/\a transform() are the
 * label's own placement within the owner's local frame, exactly mirroring what a child
 * QGraphicsItem's pos()+transform() used to mean relative to its parent.
 */
class GraphicElementLabel
{
public:
    /// Constructs the label bound to its owning \a owner element.
    explicit GraphicElementLabel(GraphicElement *owner)
        : m_owner(owner)
    {
    }

    /// Returns the label's anchor position in the owner's local frame.
    QPointF pos() const { return m_pos; }
    /// Sets the label's anchor position in the owner's local frame.
    void setPos(const QPointF &pos) { m_pos = pos; }

    /// Returns the label's own persistent rotation about its own local origin, applied
    /// before transform() — e.g. IC ticks this once at construction (90°) to display its
    /// label vertically alongside the chip body, independent of and composed underneath
    /// whatever transform() separately does for per-frame orientation compensation. Mirrors
    /// the old QGraphicsItem::rotation() property, kept deliberately separate from the old
    /// QGraphicsItem::transform() (custom transform) it composed with — see
    /// GraphicElement::rotateFlipTransform()'s identical two-stage split for why the two
    /// must stay independent rather than collapsing into one transform.
    qreal rotation() const { return m_rotation; }
    /// Sets the label's own persistent rotation.
    void setRotation(qreal degrees) { m_rotation = degrees; }

    /// Returns the label's own extra transform (counter-rotation/flip), applied about its
    /// own local origin after rotation(), before pos() — see
    /// GraphicElement::updateLabelOrientation().
    QTransform transform() const { return m_transform; }
    /// Sets the label's own extra transform.
    void setTransform(const QTransform &transform) { m_transform = transform; }

    /// Returns the displayed text.
    QString text() const { return m_text; }
    /// Sets the displayed text.
    void setText(const QString &text) { m_text = text; }

    /// Returns the font used to draw the text.
    QFont font() const { return m_font; }
    /// Sets the font used to draw the text.
    void setFont(const QFont &font) { m_font = font; }

    /// Returns the brush (color) used to draw the text.
    QBrush brush() const { return m_brush; }
    /// Sets the brush (color) used to draw the text.
    void setBrush(const QBrush &brush) { m_brush = brush; }

    /// Returns \c true if the label should currently be painted.
    bool isVisible() const { return m_visible; }
    /// Sets whether the label should currently be painted.
    void setVisible(bool visible) { m_visible = visible; }

    /// Returns the label's bounding rectangle in its own local coordinates (text metrics,
    /// unaffected by pos()/transform()) — mirrors QGraphicsSimpleTextItem::boundingRect().
    QRectF boundingRect() const { return QFontMetricsF(m_font).boundingRect(m_text); }

    /// Maps a point in the label's own local coordinates into the owner's local frame,
    /// applying rotation() then transform() then pos() -- rotation() first, matching Qt's
    /// own composition order for a rotation *property* plus a separate custom transform
    /// (see GraphicElement::rotateFlipTransform()'s doc comment for how this was verified).
    QPointF mapToOwnerLocal(const QPointF &localPoint) const
    {
        QTransform rotate;
        rotate.rotate(m_rotation);
        return m_transform.map(rotate.map(localPoint)) + m_pos;
    }

    /// Returns the label's bounding rect in scene/world coordinates, going through the
    /// owner's own local-to-scene mapping — mirrors the old QGraphicsSimpleTextItem's
    /// sceneBoundingRect(), used to position the inline rename editor over the label.
    QRectF sceneBoundingRect() const;

    /// Returns the label's bounding rect mapped into the owner's own local coordinates
    /// (via mapToOwnerLocal()), without going all the way to scene coordinates. Used by
    /// element subclasses (e.g. Text) whose own boundingRect() needs to grow to cover a
    /// label that can extend well past the nominal pixmap box.
    QRectF boundingRectInOwnerLocal() const;

    /// Draws the text at local (0,0) with the current font/brush. The caller is responsible
    /// for translating/transforming the painter to this label's owner-local placement first
    /// (mapToOwnerLocal()'s pos()+transform()), mirroring GraphicElement::paint()'s own
    /// "caller translates to pos() first" contract.
    void paint(QPainter *painter) const;

private:
    GraphicElement *m_owner;

    QPointF m_pos;
    qreal m_rotation = 0;
    QTransform m_transform;
    QString m_text;
    QFont m_font;
    QBrush m_brush;
    bool m_visible = false;
};
