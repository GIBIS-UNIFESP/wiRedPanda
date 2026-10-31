// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ElementChange: what a change to an element changed, and the interface that hears of it.
 */

#pragma once

#include <QFlags>

/// What a change to an element changed, as Element::changed() reports it.
enum class ElementChange {
    Placement = 0x01, ///< Its position, rotation or flips.
    Parameter = 0x02, ///< A saved parameter (a colour, a frequency, a switch's level, a truth table), its appearance, its trigger, an IC's definition.
    Body = 0x04,      ///< Its bounds in its own coordinates; changed() adds this itself whenever they move.
    Label = 0x08,     ///< The text its label shows, or the label's anchor.
    Ports = 0x10,     ///< Its ports: how many there are, and their places, names, defaults, requirements or visibility.
};
Q_DECLARE_FLAGS(ElementChanges, ElementChange)
Q_DECLARE_OPERATORS_FOR_FLAGS(ElementChanges)

/**
 * \class ElementChangeListener
 * \brief Hears of each change to an element in the scene that implements it.
 *
 * \details Lets an element report its changes without the Element layer naming the concrete Scene class: an element
 * finds the listener as it enters a scene.
 */
class ElementChangeListener
{
public:
    virtual ~ElementChangeListener() = default; // LCOV_EXCL_LINE -- inlined at every derived-class destruction site, not separately attributed

    /// Hears that \a what changed in the element whose id is \a id, once the element holds the change.
    virtual void reportElementChange(int id, ElementChanges what) = 0;
};
