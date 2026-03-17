// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Abstract base class for user-controllable input elements.
 */

#pragma once

#include "App/Element/GraphicElement.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Simulation/SimEvent.h"

/**
 * \class GraphicElementInput
 * \brief Abstract base for all interactive input elements (switches, buttons, clocks, etc.).
 *
 * \details Extends GraphicElement with the concept of an on/off state that can be
 * toggled by the user or the simulation engine.  Derived classes implement
 * isOn(), setOn(), and setOff() to expose their specific input behaviour.
 *
 * The concrete setOn(bool, int) and setAppearance(bool, QString) implementations
 * shared by InputSwitch, InputButton, and Clock live here to avoid repetition.
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
     *
     * Default implementation stores \a value in m_isOn, updates the pixmap
     * (index 0 = off, 1 = on), and drives the output port status.
     * Override when the element has more than one output port or non-standard
     * pixmap indexing (e.g. InputRotary).
     *
     * \param value New logic value.
     * \param port  Port index (default 0, unused by this base implementation).
     */
    virtual void setOn(const bool value, const int port = 0);

    /// Applies a custom appearance image indexed by the current on/off state.
    void setAppearance(const bool defaultAppearance, const QString &fileName) override;

    /**
     * \brief Locks or unlocks user interaction on this element.
     * \param locked \c true to prevent user toggling.
     */
    void setLocked(const bool locked) { m_locked = locked; }

    // --- Updates (functional mode) ---

    /// Propagates the current on/off state of each output to the simulation layer.
    void updateOutputs();

    // --- Updates (temporal mode) ---

    /// Schedules events for any output ports whose value changed since the last call.
    void scheduleIfChanged(EventQueue &queue, SimTime now);

    /// Resets the last-scheduled tracking so the next scheduleIfChanged() detects all ports as changed.
    void resetScheduledState() { m_lastScheduled.clear(); }

protected:
    // --- Members ---

    bool m_isOn = false;
    bool m_locked = false;
    QVector<Status> m_lastScheduled; ///< Tracks last-scheduled value per output port (temporal mode).
};

inline void GraphicElementInput::updateOutputs()
{
    for (int portIndex = 0; portIndex < outputSize(); ++portIndex) {
        if (portIndex < simOutputSize()) {
            setOutputValue(portIndex, isOn(portIndex));
        }
    }
}

inline void GraphicElementInput::scheduleIfChanged(EventQueue &queue, SimTime now)
{
    // Lazy initialization: size the tracking vector on first call.
    if (m_lastScheduled.size() != outputSize()) {
        m_lastScheduled.fill(Status::Unknown, outputSize());
    }

    for (int portIndex = 0; portIndex < outputSize(); ++portIndex) {
        const Status newVal = isOn(portIndex) ? Status::Active : Status::Inactive;
        if (newVal != m_lastScheduled[portIndex]) {
            // Write the new output value directly so successors see it when evaluated.
            setOutputValue(portIndex, newVal);
            m_lastScheduled[portIndex] = newVal;

            // Schedule re-evaluation of all elements connected to this output port.
            auto *outPort = outputPort(portIndex);
            if (!outPort) {
                continue;
            }
            for (auto *conn : outPort->connections()) {
                if (auto *inPort = conn ? conn->endPort() : nullptr) {
                    if (auto *succ = inPort->graphicElement()) {
                        SimEvent ev;
                        ev.time = now + succ->propagationDelay();
                        ev.target = succ;
                        queue.schedule(ev);
                    }
                }
            }
        }
    }
}
