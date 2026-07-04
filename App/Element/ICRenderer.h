// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICRenderer: draws an IC's body and builds its cached pixmaps.
 */

#pragma once

#include <QList>

class IC;
class QGraphicsItem;
class QPainter;

/**
 * \class ICRenderer
 * \brief Renders an IC's DIP-package body and builds its sizing/preview pixmaps.
 *
 * \details Extracted from IC so rendering is a collaborator rather than a cluster of
 * methods on the element itself, mirroring the GraphicElement / GraphicElementSerializer
 * split. A friend of IC, it reaches the element's appearance/pixmap state directly. All
 * methods are static and take the IC explicitly; IC keeps all state, so this is a pure
 * logic extraction, not a new owner-back-pointer collaborator.
 */
class ICRenderer
{
public:
    /// Draws the IC body (DIP rect, mascot logo, shadow, pin-1 notch) straight onto \a painter
    /// as vectors, so it stays crisp at any zoom instead of blitting a fixed-resolution pixmap.
    static void drawBody(IC &ic, QPainter *painter);

    /// Rebuilds \a ic's sizing pixmap (its image content is never displayed; only its size,
    /// which pixmapCenter()/boundingRect() depend on, matters — the body is drawn as vectors).
    static void generatePixmap(IC &ic);

    /// Snapshots \a items (the sub-circuit as designed, before boundary substitution) into
    /// \a ic's cached hover-preview pixmap.
    static void generatePreviewPixmap(IC &ic, const QList<QGraphicsItem *> &items);
};
