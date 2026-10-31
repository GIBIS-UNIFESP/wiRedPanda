// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the 16-segment LED display.
 */

#pragma once

#include "App/Element/Element.h"

/**
 * \class Display16
 * \brief 16-segment LED display output element (16 segments + decimal point = 17 inputs).
 *
 * \details Full alphanumeric coverage including diagonals.
 * Supports red, green, and blue color variants.
 */
class Display16 : public Element
{
    Q_OBJECT

public:
    /// Default segment color name. Single source of truth: used by the colour's
    /// spec and serialization's default-elision check.
    static constexpr const char *kDefaultColor = "Red";

    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Display16(QGraphicsItem *parent = nullptr);

    // --- Visual ---

    /// Refreshes the displayed segments based on current input states.
    void refresh() override;

    /// The 17 segment inputs, by their places and names.
    std::optional<PortAttributes> inputAttributes(int index) const override;
};
