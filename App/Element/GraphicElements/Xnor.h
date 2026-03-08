// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the XNOR gate.
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class Xnor
 * \brief Graphical representation of an XNOR gate (2–8 inputs, 1 output).
 */
class Xnor : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs an Xnor gate element with default 2 inputs.
    explicit Xnor(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(Xnor)

