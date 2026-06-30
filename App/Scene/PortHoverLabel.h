// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief PortHoverLabel: a transient tooltip-style chip showing a port's label on the canvas.
 */

#pragma once

#include <QFont>
#include <QGraphicsObject>
#include <QPainterPath>

/**
 * \class PortHoverLabel
 * \brief A small floating label drawn next to a port while a connected port is hovered.
 *
 * \details When the user hovers a port, ConnectionManager spawns one of these next to
 * every port on the other end of that port's wires, so the labels of all connected
 * endpoints are visible at once (e.g. to check that an IC pin and a 7-segment input
 * carry matching names) without tracing each wire by hand. The chip mirrors the
 * application tooltip styling and is removed when the hover ends.
 *
 * It derives from QGraphicsObject (rather than QGraphicsItem) so ConnectionManager can
 * track the live instances with QPointer and stay safe if the scene is cleared while a
 * hover is active. The chip is styled to match the native QToolTip and ignores the view
 * transform, so it reads identically to the hovered port's own tooltip at any zoom.
 */
class PortHoverLabel : public QGraphicsObject
{
public:
    /// Which side of the port the chip is biased towards, away from the element body.
    enum class Side { Left, Right, Top, Bottom };

    enum { Type = QGraphicsItem::UserType + 4 };
    int type() const override { return Type; }

    /**
     * \brief Constructs a label chip showing \a text, anchored at the item's origin (the port).
     * \param text The connected port's label.
     * \param side Side of the origin to draw the chip on, biased away from the element body.
     * \param parent Optional parent item.
     */
    explicit PortHoverLabel(const QString &text, Side side, QGraphicsItem *parent = nullptr);

    /// Returns the label text (the connected port's name).
    QString text() const { return m_text; }

    /// \reimp
    QRectF boundingRect() const override;
    /// \reimp
    QPainterPath shape() const override;
    /// \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QString m_text;
    QRectF m_bounds;
    QFont m_font;
};
