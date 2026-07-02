// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the D flip-flop.
 */

#pragma once

#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElement.h"

/**
 * \class DFlipFlop
 * \brief Graphical representation of an edge-triggered D flip-flop.
 *
 * \details Has 4 inputs (D, Clock, Preset̄, Clear̄) and 2 outputs (Q, Q̄).
 * The pixmap adapts to the current light/dark theme.
 */
class DFlipFlop : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit DFlipFlop(QGraphicsItem *parent = nullptr);

    // --- Visual ---

    static QString pixmapPath()
    {
        return ":/Components/Memory/" + ThemeManager::themePath() + "/D-flipflop.svg";
    }

    /// Updates port names for this flip-flop.
    void updatePortsProperties() override;

    /// Refreshes the pixmap when the application theme changes.
    void updateTheme() override;
    /// Updates output state on each rising clock edge.
    void updateLogic() override;

    /// Resets Q/~Q outputs and edge-detection state to power-on defaults.
    void resetSimState() override;

    /// \reimp Edge-detection state for step-debugger snapshots: {lastClk, lastValue}.
    QVector<Status> simInternalState() const override { return {m_simLastClk, m_simLastValue}; }
    /// \reimp
    void setSimInternalState(const QVector<Status> &state) override
    {
        if (state.size() == 2) {
            m_simLastClk = state.at(0);
            m_simLastValue = state.at(1);
        }
    }

private:
    Status m_simLastClk = Status::Inactive;
    Status m_simLastValue = Status::Active;
};
