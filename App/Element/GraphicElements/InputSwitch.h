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

    /// Drives all output ports logic-low (toggles; see implementation).
    void setOff() override;

    /// Sets the switch to the on (logic-1) state (toggles; see implementation).
    void setOn() override;

    // Bring the base setOn(bool, int) overload into scope (hidden by setOn() above).
    using GraphicElementInput::setOn;

    /// \reimp
    QList<QPair<int, QString>> appearanceStates() const override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, SerializationContext &context) override;
    /// \reimp
    void save(QDataStream &stream) const override;

protected:
    // --- Event handlers ---

    /// \reimp
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
};

