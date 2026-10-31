// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the Multiplexer (MUX).
 */

#pragma once

#include "App/Element/Element.h"

/**
 * \class Mux
 * \brief Graphical representation of a multiplexer (2^N-to-1).
 *
 * \details The number of inputs is configurable.  The pixmap is dynamically
 * generated to reflect the current input count.
 */
class Mux : public Element
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Mux(QGraphicsItem *parent = nullptr);

    // --- Visual / Port Configuration ---

    /// Draws the trapezoid body straight onto \a painter as vectors, so it stays crisp at any
    /// zoom instead of blitting a fixed-resolution pixmap.
    void drawBody(QPainter &painter) const override;

    /// The data inputs In0, In1, ... on the left, then the select lines S0, S1, ... at the bottom.
    std::optional<PortAttributes> inputAttributes(int index) const override;

    /// The output, Out, on the right.
    std::optional<PortAttributes> outputAttributes(int index) const override;

    /// Routes the data input selected by the select lines to the output.
    void updateLogic() override;

protected:
    /// Lays the ports out, then redraws the body for the new port count.
    void layoutPorts() override;

private:
    void generatePixmap();

    /// The body's height for the current data inputs, a multiple of 16 of at least 64.
    int bodyHeight() const;

    /**
     * \brief Returns the minimum select-line count for a Mux with \a totalInputs ports.
     * \details Finds the smallest k such that 2^k >= totalInputs - k, so that
     * every data input (totalInputs - k of them) can be addressed by the k select lines.
     */
    static int calculateSelectLines(int totalInputs);
};
