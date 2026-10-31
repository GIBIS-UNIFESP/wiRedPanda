// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for a free-text annotation label (no simulation).
 */

#pragma once

#include "App/Element/Element.h"

/**
 * \class Text
 * \brief Free-text annotation element used for labelling circuit diagrams.
 *
 * \details Has no ports and is excluded from simulation.
 */
class Text : public Element
{
    Q_OBJECT
    friend class TestElement;

public:
    /// Constructs a Text element.
    explicit Text(QGraphicsItem *parent = nullptr);

    /// Refreshes the empty-state hint's color alongside the base label/port theming.
    void updateTheme() override;

    /// Sets the empty-state hint in the new language, its width part of the bounds.
    void languageChanged() override;

protected:
    /// Shows a faint "double-click to add text" hint whenever the label is empty, since an
    /// empty Text element is otherwise a fully transparent, near-invisible placeholder.
    void labelContentChanged() override;

private:
    /// Sets the empty-state hint's text in the current language.
    void retranslateHint();

    QGraphicsSimpleTextItem *m_emptyHint = new QGraphicsSimpleTextItem(this);
};
