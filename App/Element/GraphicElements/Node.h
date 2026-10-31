// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for a wire junction node.
 */

#pragma once

#include "App/Element/Element.h"

class QGraphicsPixmapItem;

/**
 * \class Node
 * \brief Visible wire junction (T-junction) element with 1 input and 1 output.
 *
 * \details Nodes allow a signal to branch by adding a visible dot on the wire.
 * They can be rotated to orient the output in any of the four cardinal directions.
 * When configured with a WirelessMode, the node's label acts as a channel name:
 * a Tx node broadcasts its input signal wirelessly; an Rx node receives it.
 */
class Node : public Element
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Node(QGraphicsItem *parent = nullptr);

    // --- Visual ---

    /// The input, at the icon's left midpoint.
    std::optional<PortAttributes> inputAttributes(int index) const override;

    /// The output, at the icon's right midpoint.
    std::optional<PortAttributes> outputAttributes(int index) const override;

    /// Copies the single input value to the single output (pass-through).
    void updateLogic() override;

protected:
    /**
     * \brief Follows a new wireless routing mode (ParamId::Wireless): the ports and the indicator.
     * \details In Rx mode the input port is made optional (setRequired(false)) because the signal arrives
     * wirelessly rather than via a physical wire, and hidden, as the output is in Tx mode. In None or Tx mode the
     * input port is required as usual.
     */
    void parameterChanged(ParamId id, const ParamValue &previous) override;

private:
    void updateWirelessColor(Status status);

    QGraphicsPixmapItem *m_wirelessIndicator = nullptr;
    QColor m_wirelessColor;
};
