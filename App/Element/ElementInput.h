// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Abstract base class for user-controllable input elements.
 */

#pragma once

#include "App/Element/Element.h"

/**
 * \class ElementInput
 * \brief Abstract base for all interactive input elements (switches, buttons, clocks, etc.).
 *
 * \details Extends Element with the concept of an on/off state that can be
 * toggled by the user or the simulation engine.  Derived classes implement
 * isOn(), setOn(), and setOff() to expose their specific input behaviour.
 *
 * The concrete setOn(bool, int) and applyAppearance(bool, QString) implementations
 * shared by InputSwitch, InputButton, and Clock live here to avoid repetition.
 *
 * Every input has a lock (ParamId::Locked, lockSpec()); a switch's, a button's and a
 * clock's level is ParamId::On (levelSpec()), a rotary's selection its own parameter.
 */
class ElementInput : public Element
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit ElementInput(ElementType type, QGraphicsItem *parent = nullptr)
        : Element(type, parent) {}
    ~ElementInput() override = default; // LCOV_EXCL_LINE — gcov attributes no hit to an inline `= default` destructor's own line even though every subclass instance destroyed exercises it (see App/Core/ContextDirProvider.h for the same pattern).

    // --- Parameters ---

    /// The spec of an input's level (ParamId::On), kept in a file under \a key, or transient with none.
    static ParamSpec levelSpec(const char *key);

    /// The spec of an input's lock (ParamId::Locked), which every input has.
    static ParamSpec lockSpec();

    // --- State Queries ---

    /// Returns \c true if the element is locked against user interaction (ParamId::Locked).
    bool isLocked() const { return param<bool>(ParamId::Locked); }

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
     * Default implementation sets the level (ParamId::On) to \a value and shows it
     * (showLevel()), also when it is the level the element has, so a new element
     * drives its output with a definite level.
     * Override when the element has more than one output port or non-standard
     * pixmap indexing (e.g. InputRotary).
     *
     * \param value New logic value.
     * \param port  Port index (default 0, unused by this base implementation).
     */
    virtual void setOn(const bool value, const int port = 0);

    /**
     * \brief Applies a waveform-sweep cell \a value to output \a port.
     *
     * Used by the beWavedDolphin column sweep to drive each input port per time step.
     * The default simply forwards to setOn(value, port). Inputs whose ports are not
     * independent on/off bits override this — e.g. InputRotary selects exactly one active
     * port, so a low cell is implicit and must not write (which would re-select that port).
     *
     * \param value Cell logic value for this time step.
     * \param port  Port index (default 0).
     */
    virtual void setWaveformValue(const bool value, const int port = 0);

    /**
     * \brief Locks or unlocks user interaction on this element (ParamId::Locked).
     * \param locked \c true to prevent user toggling.
     */
    void setLocked(const bool locked) { setParam(ParamId::Locked, locked); }

    // --- Updates ---

    /// Propagates the current on/off state of each output to the backing logic element.
    void updateOutputs();

protected:
    /// Applies a custom appearance image in the slot of the current on/off state (index 0 = off, 1 = on). Override when
    /// the element uses a different slot scheme (e.g. InputRotary always uses slot 0).
    void applyAppearance(bool defaultAppearance, const QString &fileName) override;

    /// \reimp Shows a new level (ParamId::On) on the picture and the output.
    void parameterChanged(ParamId id, const ParamValue &previous) override;

    /// Shows the level (ParamId::On) on the picture (index 0 = off, 1 = on) and drives the output with it.
    void showLevel();
};

inline void ElementInput::updateOutputs()
{
    for (int portIndex = 0; portIndex < outputSize(); ++portIndex) {
        if (portIndex < simOutputSize()) {
            setOutputValue(portIndex, isOn(portIndex));
        }
    }
}
