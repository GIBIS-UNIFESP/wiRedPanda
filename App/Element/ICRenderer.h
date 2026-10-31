// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICRenderer: builds an IC's hover-preview pixmap.
 */

#pragma once

#include <QList>

class IC;
class QGraphicsItem;

/**
 * \class ICRenderer
 * \brief Builds an IC's hover-preview pixmap.
 *
 * \details A friend of IC, it reaches the element's preview state directly. Its one method is static and takes the IC
 * explicitly; IC keeps all state. The body is drawn by the IC's presenter (ICPresenter).
 */
class ICRenderer
{
public:
    /// Snapshots \a items (the sub-circuit as designed, before boundary substitution) into
    /// \a ic's cached hover-preview pixmap.
    static void generatePreviewPixmap(IC &ic, const QList<QGraphicsItem *> &items);
};
