// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Defines ItemWithId, the base class for all uniquely identified circuit items.
 */

#pragma once

#include <QtGlobal>

#include "App/Core/ItemRegistry.h"

/**
 * \class ItemWithId
 * \brief Base class providing a unique integer identifier for circuit items.
 *
 * \details Every graphic element and connection inherits from ItemWithId to
 * obtain a stable numeric ID managed by the owning Scene. IDs start as -1
 * (unassigned) and are set to a positive value when the item is added to a
 * Scene via Scene::addItem(). The ID is used for undo/redo serialization and
 * cross-reference during load/save.
 */
class ItemWithId
{
public:
    /// Constructs a new ItemWithId with an unassigned ID of -1.
    ItemWithId() = default;

    /// Destructor. Drops this item's registry mapping (if still registered) so a stale
    /// id->item entry can never outlive the item, regardless of the destruction path
    /// (Scene::removeItem or Qt's parent-child cascade).
    virtual ~ItemWithId()
    {
        if (m_registry) {
            m_registry->forget(this);
        }
    }

    /// Returns the unique integer identifier of this item, or -1 if unassigned.
    int id() const { return m_id; }

    /**
     * \brief Sets the identifier to \a id.
     * \param id New identifier value.
     */
    void setId(const int id) { m_id = id; }

    /// Sets the registry owning this item's id mapping. Called by the registry on register
    /// (with itself) and on unregister / its own destruction (with nullptr).
    void setRegistry(ItemRegistry *registry) { m_registry = registry; }

private:
    Q_DISABLE_COPY(ItemWithId)

    int m_id = -1; ///< The unique ID assigned by Scene; -1 means unassigned.
    ItemRegistry *m_registry = nullptr; ///< Owning registry back-pointer, or nullptr if unregistered.
};
