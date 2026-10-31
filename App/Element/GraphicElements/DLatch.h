// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the D latch.
 */

#pragma once

#include "App/Element/Element.h"

/**
 * \class DLatch
 * \brief Graphical representation of a level-sensitive D latch.
 *
 * \details Has 2 inputs (D, Enable) and 2 outputs (Q, Q̄).
 */
class DLatch : public Element
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit DLatch(QGraphicsItem *parent = nullptr);

    /// Data and Enable, by their places and names.
    std::optional<PortAttributes> inputAttributes(int index) const override;

    /// Q and ~Q, which start at 0 and 1.
    std::optional<PortAttributes> outputAttributes(int index) const override;

    /// Updates Q based on D and enable level.
    void updateLogic() override;
};
