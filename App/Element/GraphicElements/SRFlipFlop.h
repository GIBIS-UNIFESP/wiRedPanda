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
};
