// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for a bistable toggle switch input.
 */

#pragma once

#include "App/Element/Element.h"

/**
 * \class InputSwitch
 * \brief Toggle switch input element that maintains its on/off state between clicks.
 *
 * \details A single mouse click alternates the output between logic-0 and logic-1.
 * The state is persisted to the .panda file.
 */
class InputSwitch : public Element
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit InputSwitch(QGraphicsItem *parent = nullptr);

    /// \reimp
    QList<std::pair<int, QString>> appearanceStates() const override;
};
