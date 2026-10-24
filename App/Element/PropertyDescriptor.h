// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief PropertyDescriptor struct for describing editable element properties.
 */

#pragma once

/**
 * \struct PropertyDescriptor
 * \brief Describes one editable property that a GraphicElement exposes in the ElementEditor.
 *
 * \details Each element returns the list of descriptors it supports via
 * GraphicElement::editableProperties(), which lists the kinds an element offers.
 * The ElementEditor shows one field per kind (computeCapabilities / applyCapabilitiesToUi),
 * and each field sets its own property through SetPropertyCommand (ElementProperty).
 */
struct PropertyDescriptor {
    /// Identifies which property this descriptor represents.
    enum class Type {
        Label,       ///< User-visible text label.
        Color,       ///< Color selection (LEDs, displays).
        Frequency,   ///< Clock oscillation frequency in Hz.
        Delay,       ///< Phase delay as fraction of the clock period.
        Audio,       ///< Audio note selection (buzzer tone).
        AudioBox,    ///< AudioBox file selection dialog.
        Trigger,     ///< Keyboard trigger shortcut.
        TruthTable,  ///< Editable truth table dialog.
        Appearance,   ///< Custom pixmap appearance selection.
        Volume,       ///< Audio playback volume (AudioBox, Buzzer).
        WirelessModeSelector, ///< Wireless routing mode (None / Tx / Rx) — Node elements only.
    };

    Type type; ///< Which property this descriptor represents.
};
