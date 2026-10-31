// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the SR latch.
 */

#pragma once

#include "App/Element/Element.h"

/**
 * \class SRLatch
 * \brief Graphical representation of a level-sensitive SR latch.
 *
 * \details Has 2 inputs (S̄, R̄, active-low) and 2 outputs (Q, Q̄).
 */
class SRLatch : public Element
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit SRLatch(QGraphicsItem *parent = nullptr);

    /// Set and Reset, by their places and names.
    std::optional<PortAttributes> inputAttributes(int index) const override;

    /// Q and ~Q, which start at 0 and 1.
    std::optional<PortAttributes> outputAttributes(int index) const override;

    /// Updates Q based on S and R inputs.
    void updateLogic() override;
};
