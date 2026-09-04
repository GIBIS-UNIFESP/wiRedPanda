// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for a wire junction node.
 */

#pragma once

#include <QPixmap>
#include <QPointF>

#include "App/Element/GraphicElement.h"

/**
 * \class Node
 * \brief Visible wire junction (T-junction) element with 1 input and 1 output.
 *
 * \details Nodes allow a signal to branch by adding a visible dot on the wire.
 * They can be rotated to orient the output in any of the four cardinal directions.
 * When configured with a WirelessMode, the node's label acts as a channel name:
 * a Tx node broadcasts its input signal wirelessly; an Rx node receives it.
 */
class Node : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Node(QObject *parent = nullptr);

    // --- Visual ---

    /// Recalculates port positions for the current port count.
    void updatePortsProperties() override;

    /// \reimp Draws the base pixmap, then the wireless-channel indicator glyph if active.
    void paint(QPainter *painter) override;

    /// Copies the single input value to the single output (pass-through).
    void updateLogic() override;

    // --- Wireless mode ---

    /// Returns the current wireless routing mode (None / Tx / Rx).
    WirelessMode wirelessMode() const override;

    /// Returns true — Node supports wireless mode configuration.
    bool hasWirelessMode() const override;

    /**
     * \brief Sets the wireless routing mode and adjusts port requirements accordingly.
     * \details In Rx mode the input port is made optional (setRequired(false)) because
     * the signal arrives wirelessly rather than via a physical wire.  In None or Tx mode
     * the input port is required as usual.
     * \param mode The new WirelessMode to apply.
     */
    void setWirelessMode(WirelessMode mode);

    // --- Serialization ---

    void save(QDataStream &stream, SerializationOptions options) const override;
    void load(QDataStream &stream, SerializationContext &context) override;

private:
    void updateWirelessColor(Status status);

    WirelessMode m_wirelessMode = WirelessMode::None;

    /// The small channel-indicator glyph (fan of arcs), drawn by paint() in this element's own
    /// local frame (GraphicElement does not parent Qt Graphics View children). Its own local
    /// origin lands at the pixmap's dot (see renderWirelessPixmap()'s SVG), so
    /// m_wirelessIndicatorPos is exactly where that dot should land in Node's local frame -- Tx
    /// draws it unflipped, Rx horizontally flipped (arcs open the opposite way), never rotated,
    /// so a plain flip flag is enough; no need for the general per-glyph QTransform machinery
    /// Port/GraphicElementLabel use.
    QPixmap m_wirelessIndicatorPixmap;
    QPointF m_wirelessIndicatorPos;
    bool m_wirelessIndicatorFlipped = false;
    bool m_wirelessIndicatorVisible = false;
    QColor m_wirelessColor;
};
