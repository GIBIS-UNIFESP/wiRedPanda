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
 * \details Has no ports and no simulation logic (LogicNone).
 */
class Text : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs a Text element.
    explicit Text(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(Text)

