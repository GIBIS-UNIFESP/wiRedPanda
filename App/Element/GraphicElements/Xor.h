// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the XOR gate.
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class Xor
 * \brief Graphical representation of an XOR gate (2–8 inputs, 1 output).
 */
class Xor : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs an Xor gate element with default 2 inputs.
    explicit Xor(QGraphicsItem *parent = nullptr);

    /// Sets output to the logical XOR of all inputs.
    void updateLogic() override;
};

