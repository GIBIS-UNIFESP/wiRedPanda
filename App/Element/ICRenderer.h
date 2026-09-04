// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICRenderer: draws an IC's body and builds its cached pixmaps.
 */

#pragma once

#include <QList>

class IC;
class ItemWithId;
class QPainter;

/**
 * \class ICRenderer
 * \brief Renders an IC's DIP-package body and builds its sizing/preview pixmaps.
 *
 * \details A friend of IC that renders it as a collaborator rather than a cluster of
 * methods on the element itself, mirroring the GraphicElement / GraphicElementSerializer
 * split. It reaches the element's appearance/pixmap state directly. All methods are static
 * and take the IC explicitly; IC keeps all state, so this is a pure logic extraction, not a
 * new owner-back-pointer collaborator.
 */
class ICRenderer
{
public:
    /// Maximum preview dimensions in pixels. ICPreviewPopup (the Widgets-only popup that
    /// displays what this class renders) sizes itself off these same values instead of
    /// keeping its own, independent copies.
    static constexpr int MaxWidth = 500;
    static constexpr int MaxHeight = 350;

    /// Maximum number of internal elements before generatePreviewPixmap() skips generating a
    /// preview. Empirically chosen: circuits above this size render in >16 ms on a mid-range
    /// laptop, making the popup's 1-second hover delay feel unresponsive.
    static constexpr int MaxElementCount = 500;

    /// Draws the IC body (DIP rect, mascot logo, shadow, pin-1 notch) straight onto \a painter
    /// as vectors, so it stays crisp at any zoom instead of blitting a fixed-resolution pixmap.
    static void drawBody(IC &ic, QPainter *painter);

    /// Rebuilds \a ic's sizing pixmap (its image content is never displayed; only its size,
    /// which pixmapCenter()/boundingRect() depend on, matters — the body is drawn as vectors).
    static void generatePixmap(IC &ic);

    /// Snapshots \a items (the sub-circuit as designed, before boundary substitution) into
    /// \a ic's cached hover-preview pixmap. Draws each GraphicElement/Port/Connection directly
    /// via its own paint() -- there's no QGraphicsScene to borrow items into, since none of
    /// those classes are QGraphicsItem.
    static void generatePreviewPixmap(IC &ic, const QList<ItemWithId *> &items);
};
