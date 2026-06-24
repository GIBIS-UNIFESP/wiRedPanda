// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICRenderer: draws the IC body and builds its sizing/preview pixmaps.
 */

#pragma once

#include <QList>

class IC;
class QGraphicsItem;
class QPainter;

/**
 * \class ICRenderer
 * \brief Renders an IC: the vector DIP body drawn in paint(), the sizing pixmap that backs
 * boundingRect(), and the hover-preview snapshot of the designed sub-circuit.
 *
 * \details Extracted from IC (a friend of it) so rendering is separate from loading and
 * simulation. IC's paint()/generatePixmap()/generatePreviewPixmap() entry points delegate here.
 */
class ICRenderer
{
public:
    /// Draws the IC body (DIP rect, mascot logo, shadow, pin-1 notch) as vectors onto \a painter.
    static void drawBody(const IC &ic, QPainter *painter);

    /// Rebuilds the transparent sizing pixmap (used only for boundingRect()/pixmapCenter() size)
    /// from the current port layout, and requests a repaint.
    static void generatePixmap(IC &ic);

    /// Snapshots the designed circuit (\a items, before boundary substitution) into the IC's
    /// preview pixmap, scaled to the preview popup's max dimensions. Clears it for empty or
    /// oversized circuits.
    static void generatePreviewPixmap(IC &ic, const QList<QGraphicsItem *> &items);
};
