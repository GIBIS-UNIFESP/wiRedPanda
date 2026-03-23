// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief PropertyShortcutHandler: dispatches keyboard shortcuts that cycle element properties.
 */

#pragma once

class Scene;

/**
 * \class PropertyShortcutHandler
 * \brief Dispatches keyboard shortcuts that cycle element properties and types.
 *
 * Extracted from Scene to decouple property-cycling dispatch logic (input count,
 * frequency, color, element morphing) from the main circuit scene.
 */
class PropertyShortcutHandler
{
public:
    explicit PropertyShortcutHandler(Scene *scene);

    /// Decrements the primary property of each selected element.
    void prevMainProperty();
    /// Increments the primary property of each selected element.
    void nextMainProperty();
    /// Decrements the secondary property of each selected element.
    void prevSecondaryProperty();
    /// Increments the secondary property of each selected element.
    void nextSecondaryProperty();
    /// Morphs each selected element to the next type in the cycle.
    void nextElement();
    /// Morphs each selected element to the previous type in the cycle.
    void prevElement();

private:
    Scene *m_scene = nullptr;
};

