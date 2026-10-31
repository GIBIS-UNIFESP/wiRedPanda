// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the 14-segment LED display.
 */

#pragma once

#include "App/Element/Element.h"

/**
 * \class Display14
 * \brief 14-segment LED display output element (14 segments + decimal point = 15 inputs).
 *
 * \details Supports alphanumeric characters.  Each input controls one segment.
 * Supports red, green, and blue color variants.
 */
class Display14 : public Element
{
    Q_OBJECT

public:
    /// Default segment color name. Single source of truth: used by the colour's
    /// spec and serialization's default-elision check.
    static constexpr const char *kDefaultColor = "Red";

    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Display14(QGraphicsItem *parent = nullptr);

    // --- Visual ---

    /// Refreshes the displayed segments based on current input states.
    void refresh() override;

    /// The 15 segment inputs, by their places and names.
    std::optional<PortAttributes> inputAttributes(int index) const override;
};
