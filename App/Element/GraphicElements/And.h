// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the AND gate.
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class And
 * \brief Graphical representation of an AND gate (2–8 inputs, 1 output).
 */
class And : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs an And gate element with default 2 inputs.
    explicit And(QGraphicsItem *parent = nullptr);

    /// Sets output to the logical AND of all inputs.
    void updateLogic() override;
};

