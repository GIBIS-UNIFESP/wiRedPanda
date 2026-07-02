// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the JK flip-flop.
 */

#pragma once

#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElement.h"

/**
 * \class JKFlipFlop
 * \brief Graphical representation of an edge-triggered JK flip-flop.
 *
 * \details Has 5 inputs (J, K, Clock, Preset̄, Clear̄) and 2 outputs (Q, Q̄).
 */
class JKFlipFlop : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit JKFlipFlop(QGraphicsItem *parent = nullptr);

    // --- Visual ---

    static QString pixmapPath()
    {
        return ":/Components/Memory/" + ThemeManager::themePath() + "/JK-flipflop.svg";
    }

    /// Updates port names for this flip-flop.
    void updatePortsProperties() override;

    /// Refreshes the pixmap when the application theme changes.
    void updateTheme() override;

    /// Updates output state on each rising clock edge.
    void updateLogic() override;
    /// Resets Q/~Q outputs and edge-detection state to power-on defaults.
    void resetSimState() override;

    /// \reimp Edge-detection state for step-debugger snapshots: {lastClk, lastJ, lastK}.
    QVector<Status> simInternalState() const override { return {m_simLastClk, m_simLastJ, m_simLastK}; }
    /// \reimp
    void setSimInternalState(const QVector<Status> &state) override
    {
        if (state.size() == 3) {
            m_simLastClk = state.at(0);
            m_simLastJ = state.at(1);
            m_simLastK = state.at(2);
        }
    }

private:
    Status m_simLastClk = Status::Inactive;
    Status m_simLastJ = Status::Active;
    Status m_simLastK = Status::Active;
};
