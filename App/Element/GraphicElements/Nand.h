// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the NAND gate.
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class Nand
 * \brief Graphical representation of a NAND gate (2–8 inputs, 1 output).
 */
class Nand : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs a Nand gate element with default 2 inputs.
    explicit Nand(QGraphicsItem *parent = nullptr);

    /// Sets output to the logical NAND of all inputs.
    void updateLogic() override;
};

