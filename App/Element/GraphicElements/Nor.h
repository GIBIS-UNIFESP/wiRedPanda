// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the NOR gate.
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class Nor
 * \brief Graphical representation of a NOR gate (2–8 inputs, 1 output).
 */
class Nor : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs a Nor gate element with default 2 inputs.
    explicit Nor(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(Nor)
