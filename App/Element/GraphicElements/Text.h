// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for a free-text annotation label (no simulation).
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class Text
 * \brief Free-text annotation element used for labelling circuit diagrams.
 *
 * \details Has no ports and is excluded from simulation.
 */
class Text : public GraphicElement
{
    Q_OBJECT
    friend class TestGraphicElement;

public:
    /// Constructs a Text element.
    explicit Text(QGraphicsItem *parent = nullptr);

    /// Returns a bounding rect that includes the label text, not just the pixmap.
    QRectF boundingRect() const override;

    /// Refreshes the empty-state hint's color alongside the base label/port theming.
    void updateTheme() override;

protected:
    /// Shows a faint "double-click to add text" hint whenever the label is empty, since an
    /// empty Text element is otherwise a fully transparent, near-invisible placeholder.
    void labelContentChanged() override;

private:
    QGraphicsSimpleTextItem *m_emptyHint = new QGraphicsSimpleTextItem(this);
};
