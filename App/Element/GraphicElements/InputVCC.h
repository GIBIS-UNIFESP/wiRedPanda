// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the VCC (logic-1 constant) input.
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class InputVcc
 * \brief Static VCC element that always drives logic-1 on its output.
 */
class InputVcc : public GraphicElement
{
    Q_OBJECT

public:
    /// Constructs an InputVcc element.
    explicit InputVcc(QGraphicsItem *parent = nullptr);
};

