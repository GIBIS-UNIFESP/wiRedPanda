// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for a bistable toggle switch input.
 */

#pragma once

#include "App/Element/GraphicElementInput.h"

/**
 * \class InputSwitch
 * \brief Toggle switch input element that maintains its on/off state between clicks.
 *
 * \details A single mouse click alternates the output between logic-0 and logic-1.
 * The state is persisted to the .panda file.
 */
class InputSwitch : public GraphicElementInput
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit InputSwitch(QGraphicsItem *parent = nullptr);

    // --- State Queries ---

    /// Returns \c true if output port \a port is driven logic-high.
    bool isOn(const int port = 0) const override;

    // --- State Setters ---

    /// Drives all output ports logic-low.
    void setOff() override;

    /// Sets the switch to the on (logic-1) state.
    void setOn() override;

    /**
     * \brief Sets the switch to \a value on port \a port.
     * \param value New state.
     * \param port  Port index (unused, always 0).
     */
    void setOn(const bool value, const int port = 0) override;

    /// Applies a custom skin image to reflect the on/off state.
    void setSkin(const bool defaultSkin, const QString &fileName) override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, SerializationContext &context) override;
    /// \reimp
    void save(QDataStream &stream) const override;

protected:
    // --- Event handlers ---

    /// \reimp
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    // --- Members ---

    bool m_isOn = false;
};

