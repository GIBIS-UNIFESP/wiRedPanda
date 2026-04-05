// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the rotary switch input.
 */

#pragma once

#include "App/Element/GraphicElementInput.h"

/**
 * \class InputRotary
 * \brief Rotary switch input that activates one output port at a time.
 *
 * \details Each mouse click advances the active output by one.  Only the
 * currently selected port is driven logic-1; all others remain logic-0.
 */
class InputRotary : public GraphicElementInput
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the rotary switch with optional \a parent.
    explicit InputRotary(QGraphicsItem *parent = nullptr);

    // --- State Queries ---

    /// Returns \c true if output port \a port is the currently active selection.
    bool isOn(const int port = 0) const override;

    /// Returns the number of selectable output ports.
    int outputSize() const override;

    /// Returns the index of the currently active output port.
    int outputValue() const override;

    // --- State Setters ---

    /// Resets the selection to port 0 (logic-0 on all ports).
    void setOff() override;

    /// Activates port 0 (no-op if already at 0).
    void setOn() override;

    /**
     * \brief Sets the active port to \a port when \a value is true.
     * \param value When true, activates \a port.
     * \param port  Port index to activate.
     */
    void setOn(const bool value, const int port = 0) override;

    // --- Visual ---

    /// Paints the rotary wheel with the indicator arrow pointing to the active port.
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /// Refreshes the arrow direction and port positions.
    void refresh() override;

    /// Applies custom appearance images for the rotary body and arrow.
    void setAppearance(const bool defaultAppearance, const QString &fileName) override;

    /// Recalculates port positions based on the current output count.
    void updatePortsProperties() override;

    // --- Serialization ---

    /// Deserializes the current port selection.
    void load(QDataStream &stream, SerializationContext &context) override;

    /// Serializes the current port selection to \a stream.
    void save(QDataStream &stream) const override;

protected:
    // --- Event handlers ---

    /// \reimp
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    // --- Members ---

    QPixmap m_arrow;
    QPixmap m_rotary;
    int m_currentPort = 0;
};

