// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for a decorative line shape (no simulation).
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class Line
 * \brief Decorative line element used for annotating circuit diagrams.
 *
 * \details Has no ports and no simulation logic (LogicNone).
 */
class Line : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs a Line element.
    explicit Line(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(Line)

