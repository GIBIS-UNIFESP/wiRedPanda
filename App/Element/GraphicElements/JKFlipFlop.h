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
    void updateLogic() override;
private:
    bool m_lastClk = false;
    bool m_lastJ = true;
    bool m_lastK = true;
};

