// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Graphic element for the rotary switch input.
 */

#pragma once

#include "App/Element/Element.h"

/**
 * \class InputRotary
 * \brief Rotary switch input that activates one output port at a time.
 *
 * \details Each mouse click advances the active output by one.  Only the
 * currently selected port is driven logic-1; all others remain logic-0.
 */
class InputRotary : public Element
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the rotary switch with optional \a parent.
    explicit InputRotary(QGraphicsItem *parent = nullptr);

    /// \reimp Also returns the selection to port 0. The selection (ParamId::Position) is the rotary's
    /// simulation state too -- isOn() reads it and updateSourceOutputs() republishes it every tick -- so
    /// without this a BeWavedDolphin sweep inherits whatever position the user last left on the
    /// canvas, and its "reset for reproducibility" pass has no effect on this element.
    void resetSimState() override;

    /// \reimp Recovers the selection from the restored one-hot outputs. The base class already
    /// saves and restores those; what it cannot know is that they are republished from
    /// the selection on the next tick, which would overwrite them.
    void restoreSimState(const QVector<Status> &in, int &cursor) override;

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
};
