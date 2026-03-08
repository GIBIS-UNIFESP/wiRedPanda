// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the Multiplexer (MUX).
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class Mux
 * \brief Graphical representation of a multiplexer (2^N-to-1).
 *
 * \details The number of inputs is configurable.  The pixmap is dynamically
 * generated to reflect the current input count.
 */
class Mux : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Mux(QGraphicsItem *parent = nullptr);

    // --- Port Configuration ---

    /// Recalculates port positions for the current port count.
    void updatePortsProperties() override;
};

Q_DECLARE_METATYPE(Mux)

