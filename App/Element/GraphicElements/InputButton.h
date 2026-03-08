// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for a momentary push-button input.
 */

#pragma once

#include "App/Element/GraphicElementInput.h"

/**
 * \class InputButton
 * \brief Momentary push-button input element that is active only while pressed.
 *
 * \details Output is logic-1 while the mouse button is held down and logic-0
 * when released.
 */
class InputButton : public GraphicElementInput
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit InputButton(QGraphicsItem *parent = nullptr);

    // --- State Queries ---

    /// Returns \c true if output port \a port is driven logic-high.
    bool isOn(const int port = 0) const override;

    // --- State Setters ---

    /// Drives all output ports logic-low.
    void setOff() override;

    /// Presses the button (logic-1).
    void setOn() override;

    /**
     * \brief Sets the button to \a value.
     * \param value New state.
     * \param port  Port index (unused, always 0).
     */
    void setOn(const bool value, const int port = 0) override;

    /// Applies a custom skin image to reflect the pressed/released state.
    void setSkin(const bool defaultSkin, const QString &fileName) override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    /// \reimp
    void save(QDataStream &stream) const override;

protected:
    // --- Event handlers ---

    /// \reimp
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    /// \reimp
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    // --- Members ---

    bool m_isOn = false;
};

Q_DECLARE_METATYPE(InputButton)

