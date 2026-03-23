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

    // --- Visual / Port Configuration ---

    /// \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// \reimp Co-updates input count when output size changes.
    void setOutputSize(const int size) override;

    /// Recalculates port positions for the current port count.
    void updatePortsProperties() override;

    /// Routes the data input to the output selected by the select lines.
    void updateLogic() override;

private:
    void generatePixmap();

    // Prevent direct input size setting - inputs are derived from output size
    void setInputSize(const int size) override;
};

