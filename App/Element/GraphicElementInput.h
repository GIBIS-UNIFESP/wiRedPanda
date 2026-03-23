// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Abstract base class for user-controllable input elements.
 */

#pragma once

#include "App/Element/GraphicElement.h"

/**
 * \class GraphicElementInput
 * \brief Abstract base for all interactive input elements (switches, buttons, clocks, etc.).
 *
 * \details Extends GraphicElement with the concept of an on/off state that can be
 * toggled by the user or the simulation engine.  Derived classes implement
 * isOn(), setOn(), and setOff() to expose their specific input behaviour.
 */
class GraphicElementInput : public GraphicElement
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit GraphicElementInput(ElementType type, QGraphicsItem *parent = nullptr)
        : GraphicElement(type, parent) {}

    // --- State Queries ---

    /// Returns \c true if the element is locked against user interaction.
    bool isLocked() const { return m_locked; }

    /**
     * \brief Returns \c true if the output on \a port is currently active (logic-1).
     * \param port Output port index (default 0).
     */
    virtual bool isOn(const int port = 0) const = 0;

    /// Returns the number of output ports (default 1 for most input elements).
    virtual int outputSize() const { return 1; }

    /// Returns the integer encoding of the current output value.
    virtual int outputValue() const { return static_cast<int>(isOn()); }

    /// Sets all outputs to inactive (logic-0).
    virtual void setOff() = 0;

    /// Sets all outputs to active (logic-1).
    virtual void setOn() = 0;

    /**
     * \brief Sets output \a port to \a value.
     * \param value New logic value.
     * \param port  Port index (default 0).
     */
    virtual void setOn(const bool value, const int port = 0) = 0;

    /**
     * \brief Locks or unlocks user interaction on this element.
     * \param locked \c true to prevent user toggling.
     */
    void setLocked(const bool locked) { m_locked = locked; }

    // --- Updates ---

    /// Propagates the current on/off state of each output to the backing logic element.
    void updateOutputs();

protected:
    // --- Members ---

    bool m_locked = false;
};

inline void GraphicElementInput::updateOutputs()
{
    for (int portIndex = 0; portIndex < outputSize(); ++portIndex) {
        if (portIndex < simOutputSize()) {
            setOutputValue(portIndex, isOn(portIndex));
        }
    }
}

