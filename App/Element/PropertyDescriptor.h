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
 * GraphicElement::editableProperties(). The ElementEditor uses this list both
 * to decide which UI sections to show (computeCapabilities / applyCapabilitiesToUi)
 * and to dispatch property writes back to the element (apply / applyProperty).
 * Adding a new property type requires only: a new Type enum value, one entry in
 * GraphicElement::editableProperties(), one case in ElementEditor::applyProperty(),
 * and one UI section in applyCapabilitiesToUi(). No other existing code changes.
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
        Skin,         ///< Custom pixmap skin selection.
        Volume,       ///< Audio playback volume (AudioBox, Buzzer).
        WirelessModeSelector, ///< Wireless routing mode (None / Tx / Rx) — Node elements only.
    };

    Type type; ///< Which property this descriptor represents.
};

