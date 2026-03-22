// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the real-time clock input.
 */

#pragma once

#include "App/Element/GraphicElementInput.h"

/**
 * \class Clock
 * \brief Event-driven real-time clock input element.
 *
 * \details Toggles its output at a configurable frequency (Hz) and optional
 * initial phase delay.  The clock is the only element driven by wall-clock time
 * rather than the synchronous simulation cycle.
 */
class Clock : public GraphicElementInput
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element with optional \a parent.
    explicit Clock(QGraphicsItem *parent = nullptr);

    // --- State Queries ---

    /// Returns \c true if the clock output is currently logic-high.
    bool isOn(const int port = 0) const override;
    /// Returns the clock frequency in Hz.
    float frequency() const override;
    /// Returns the clock phase delay as a fraction of the period [0, 1).
    float delay() const override;
    /// Returns a summary string of the clock's current frequency and delay settings.
    QString genericProperties() override;

    // --- State Setters ---

    /// Drives all output ports logic-high.
    void setOn() override;
    /// Drives all output ports logic-low.
    void setOff() override;
    /// Sets the clock output to \a value on port \a port.
    void setOn(const bool value, const int port = 0) override;
    /// Sets the clock output frequency to \a freq Hz.
    void setFrequency(const float freq) override;
    /// Sets the clock phase delay to \a delay (fraction of period).
    void setDelay(const float delay) override;

    // --- Simulation ---

    /// Resets the clock phase reference to \a globalTime.
    void resetClock(std::chrono::steady_clock::time_point globalTime);
    /// Advances the clock state based on elapsed time since \a globalTime.
    void updateClock(std::chrono::steady_clock::time_point globalTime);

    // --- Visual ---

    /// \reimp
    void setSkin(const bool defaultSkin, const QString &fileName) override;

    // --- Serialization ---

    /// \reimp
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    /// \reimp
    void save(QDataStream &stream) const override;

private:
    // --- Members ---

    bool m_isOn = false;
    double m_delay = 0;
    double m_frequency = 0;
    std::chrono::microseconds m_interval;
    std::chrono::steady_clock::time_point m_startTime;
};

Q_DECLARE_METATYPE(Clock)

