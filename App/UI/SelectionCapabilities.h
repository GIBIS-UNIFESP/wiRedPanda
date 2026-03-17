// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief SelectionCapabilities struct and computeCapabilities() free function.
 */

#pragma once

#include <QList>

#include "App/Core/Enums.h"

class GraphicElement;

/**
 * \struct SelectionCapabilities
 * \brief Computed capabilities and consensus state of the current element selection.
 *
 * \details Recomputed from scratch on every selection change by computeCapabilities().
 * Consumed by ElementEditor (UI updates), ElementContextMenu (menu building),
 * and any other observer of the selection state.
 */
struct SelectionCapabilities {
    // --- Feature flags (AND-reduced: true only if every selected element has the feature) ---

    bool hasAudioBox         = false;
    bool hasAudio            = false;
    bool hasColors           = false;
    bool hasDelay            = false;
    bool hasElements         = false;
    bool hasFrequency        = false;
    bool hasLabel            = false;
    bool hasOnlyInputs       = false;
    bool hasLatchedValue     = false;
    bool hasWirelessMode     = false;
    bool hasTrigger          = false;
    bool hasTruthTable       = false;

    // --- Mutability flags (derived from port count ranges across the selection) ---

    bool canChangeSkin       = false;
    bool canChangeInputSize  = false;
    bool canChangeOutputSize = false;
    bool canMorph            = false;

    // --- Consensus flags (true when all selected elements share the same value) ---

    bool hasSameAudio        = false;
    bool hasSameColors       = false;
    bool hasSameDelay        = false;
    bool hasSameFrequency    = false;
    bool hasSameInputSize    = false;
    bool hasSameLabel        = false;
    bool hasSameOutputSize   = false;
    bool hasSameOutputValue  = false;
    bool hasSameTrigger      = false;
    bool hasSameType         = false;

    // --- Derived state ---

    bool sameCheckState       = false;
    int  minimumInputs        = 0;
    int  maximumInputs        = 0;
    int  minimumOutputs       = 0;
    int  maximumOutputs       = 0;
    int  maxCurrentOutputSize = 0;
    ElementType elementType   = ElementType::Unknown;
};

/**
 * \brief Computes the SelectionCapabilities for a list of selected elements.
 * \param elements The currently selected GraphicElements (may be empty).
 * \return A fully populated SelectionCapabilities; all flags false if \a elements is empty.
 */
SelectionCapabilities computeCapabilities(const QList<GraphicElement *> &elements);
