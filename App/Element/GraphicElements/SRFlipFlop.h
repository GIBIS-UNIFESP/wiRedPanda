// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the clocked SR flip-flop.
 */

#pragma once

#include "App/Element/Element.h"

/**
 * \class SRFlipFlop
 * \brief Graphical representation of an edge-triggered SR flip-flop.
 *
 * \details Has 5 inputs (S, R, Clock, Preset̄, Clear̄) and 2 outputs (Q, Q̄).
 */
class SRFlipFlop : public Element
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit SRFlipFlop(QGraphicsItem *parent = nullptr);

    /// The data inputs, the clock and the asynchronous ~Preset and ~Clear, by their places and names.
    std::optional<PortAttributes> inputAttributes(int index) const override;

    /// Q and ~Q, which start at 0 and 1.
    std::optional<PortAttributes> outputAttributes(int index) const override;

    /// Updates output state on each rising clock edge.
    void updateLogic() override;
    /// Resets Q/~Q outputs and edge-detection state to power-on defaults.
    void resetSimState() override;
    /// \reimp Adds the edge-detection history to the base class's output values.
    void saveSimState(QVector<Status> &out) const override;
    /// \reimp
    void restoreSimState(const QVector<Status> &in, int &cursor) override;
private:
    Status m_simLastClk = Status::Inactive;
    Status m_simLastS = Status::Inactive;
    Status m_simLastR = Status::Inactive;
};
