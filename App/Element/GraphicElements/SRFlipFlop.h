// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the clocked SR flip-flop.
 */

#pragma once

#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElement.h"

/**
 * \class SRFlipFlop
 * \brief Graphical representation of an edge-triggered SR flip-flop.
 *
 * \details Has 5 inputs (S, R, Clock, Preset̄, Clear̄) and 2 outputs (Q, Q̄).
 */
class SRFlipFlop : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit SRFlipFlop(QGraphicsItem *parent = nullptr);

    // --- Visual ---

    static QString pixmapPath()
    {
        return ":/Components/Memory/" + ThemeManager::themePath() + "/SR-flipflop.svg";
    }

    /// Updates port names for this flip-flop.
    void updatePortsProperties() override;

    /// Refreshes the pixmap when the application theme changes.
    void updateTheme() override;

    /// Updates output state on each rising clock edge.
    void updateLogic() override;
    /// Resets Q/~Q outputs and edge-detection state to power-on defaults.
    void resetSimState() override;

    /// \reimp Edge-detection state for step-debugger snapshots: {lastClk, lastS, lastR}.
    QVector<Status> simInternalState() const override { return {m_simLastClk, m_simLastS, m_simLastR}; }
    /// \reimp
    void setSimInternalState(const QVector<Status> &state) override
    {
        if (state.size() == 3) {
            m_simLastClk = state.at(0);
            m_simLastS = state.at(1);
            m_simLastR = state.at(2);
        }
    }

private:
    Status m_simLastClk = Status::Inactive;
    Status m_simLastS = Status::Inactive;
    Status m_simLastR = Status::Inactive;
};
