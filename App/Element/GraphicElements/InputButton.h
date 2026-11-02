// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for a momentary push-button input.
 */

#pragma once

#include "App/Element/Element.h"

/**
 * \class InputButton
 * \brief Momentary push-button input element that is active only while pressed.
 *
 * \details Output is logic-1 while the mouse button is held down and logic-0
 * when released.
 */
class InputButton : public Element
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit InputButton(QGraphicsItem *parent = nullptr);

    /// \reimp
    QList<std::pair<int, QString>> appearanceStates() const override;
};
