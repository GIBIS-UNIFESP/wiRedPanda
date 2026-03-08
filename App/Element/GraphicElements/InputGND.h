// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the GND (logic-0 constant) input.
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class InputGnd
 * \brief Static GND element that always drives logic-0 on its output.
 */
class InputGnd : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs an InputGnd element.
    explicit InputGnd(QGraphicsItem *parent = nullptr);
};

Q_DECLARE_METATYPE(InputGnd)

