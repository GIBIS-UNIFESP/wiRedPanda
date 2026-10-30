// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the Demultiplexer (DEMUX).
 */

#pragma once

#include "App/Element/Element.h"

/**
 * \class Demux
 * \brief Graphical representation of a demultiplexer (1-to-2^N).
 *
 * \details Output size is configurable.  The number of inputs is automatically
 * derived (1 data + log2(outputs) select lines).
 */
class Demux : public Element
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Demux(QGraphicsItem *parent = nullptr);

    // --- Visual / Port Configuration ---

    /// \reimp
    QRectF boundingRect() const override;

    /// \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// \reimp Co-updates input count when output size changes.
    void setOutputSize(const int size) override;

    /// Lays the ports out, then redraws the body for the new port count.
    void updatePortsProperties() override;

    /// The data input, In, on the left, then the select lines S0, S1, ... at the bottom.
    std::optional<PortAttributes> inputAttributes(int index) const override;

    /// The outputs Out0, Out1, ... on the right.
    std::optional<PortAttributes> outputAttributes(int index) const override;

    /// Routes the data input to the output selected by the select lines.
    void updateLogic() override;

    // --- Serialization ---

    /// \reimp Refuses inputs other than the data input and the select lines the outputs need.
    void load(QDataStream &stream, SerializationContext &context) override;

private:
    void generatePixmap();

    /// The body's height for the current outputs, a multiple of 16 of at least 64.
    int bodyHeight() const;

    /// Draws the trapezoid body straight onto \a painter as vectors, so it stays crisp at any
    /// zoom instead of blitting a fixed-resolution pixmap.
    void drawBody(QPainter *painter);

    // Prevent direct input size setting - inputs are derived from output size
    void setInputSize(const int size) override;

    /**
     * \brief Returns the minimum select-line count to address \a dataCount outputs.
     * \details Finds the smallest k such that 2^k >= dataCount.
     */
    static int calculateSelectLines(int dataCount);
};
