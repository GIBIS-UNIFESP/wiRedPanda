// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the NOT (inverter) gate.
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class Not
 * \brief Graphical representation of a NOT gate (1 input, 1 output).
 */
class Not : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs a Not gate element.
    explicit Not(QGraphicsItem *parent = nullptr);
};
