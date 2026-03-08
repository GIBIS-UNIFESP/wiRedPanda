// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the Demultiplexer (DEMUX).
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class Demux
 * \brief Graphical representation of a demultiplexer (1-to-2^N).
 *
 * \details Output size is configurable.  The number of inputs is automatically
 * derived (1 data + log2(outputs) select lines).
 */
class Demux : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Demux(QGraphicsItem *parent = nullptr);

    // --- Port Configuration ---

    /// Recalculates port positions for the current port count.
    void updatePortsProperties() override;
};

Q_DECLARE_METATYPE(Demux)
