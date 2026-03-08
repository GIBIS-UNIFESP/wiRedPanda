// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the OR gate.
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class Or
 * \brief Graphical representation of an OR gate (2–8 inputs, 1 output).
 */
class Or : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs an Or gate element with default 2 inputs.
    explicit Or(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(Or)

