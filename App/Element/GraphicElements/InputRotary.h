// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the rotary switch input.
 */

#pragma once

#include "App/Element/ElementInput.h"

/**
 * \class InputRotary
 * \brief Rotary switch input that activates one output port at a time.
 *
 * \details Each mouse click advances the active output by one.  Only the
 * currently selected port is driven logic-1; all others remain logic-0.
 */
class InputRotary : public ElementInput
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

    /// \reimp Also returns the selection to port 0. The selection (ParamId::Position) is the rotary's
    /// simulation state too -- isOn() reads it and updateOutputs() republishes it every tick -- so
    /// without this a BeWavedDolphin sweep inherits whatever position the user last left on the
    /// canvas, and its "reset for reproducibility" pass has no effect on this element.
    void resetSimState() override;

    /// \reimp Recovers the selection from the restored one-hot outputs. The base class already
    /// saves and restores those; what it cannot know is that they are republished from
    /// the selection on the next tick, which would overwrite them.
    void restoreSimState(const QVector<Status> &in, int &cursor) override;

    /// \reimp A rotary selects exactly one active port, so a low cell is implicit: only a
    /// high \a value writes (selecting \a port); a low one is a no-op.
    void setWaveformValue(const bool value, const int port = 0) override;

    // --- Visual ---

    /// Refreshes the arrow direction and port positions.
    void refresh() override;

    /// The outputs around the dial, by their places and names for the current output count.
    std::optional<PortAttributes> outputAttributes(int index) const override;

protected:
    /// Applies a custom appearance image for the rotary body, always in slot 0.
    void applyAppearance(bool defaultAppearance, const QString &fileName) override;

    /// Lays the ports out, then redraws the dial for the new output count.
    void layoutPorts() override;

    /// \reimp Shows a new selection (ParamId::Position) on the dial and the outputs.
    void parameterChanged(ParamId id, const ParamValue &previous) override;

    // --- Event handlers ---

    /// \reimp Selects the next output on a left press, unless it is locked; the scene records the selection as one
    /// undoable change (Scene::recordOperation()).
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
    /// Shows the selection on the dial and drives the outputs with it, the selected one Active.
    void showSelection();
};
