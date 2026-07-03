// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief SceneItemRegistry: maps stable integer IDs to the scene's identifiable items.
 */

#pragma once

#include <QHash>

class ItemWithId;

/**
 * \class SceneItemRegistry
 * \brief Owns the id↔item bookkeeping for a Scene: the id-to-item map and the
 * monotonically increasing last-assigned id.
 *
 * \details Extracted from Scene. Pure data structure with no back-reference to the
 * scene; Scene forwards its itemById/nextId/register/… API here. Keeping the
 * invariant (one id ↔ one live item) local makes the registry independently testable.
 */
class SceneItemRegistry
{
public:
    /// Returns the item registered under \a id, or nullptr.
    [[nodiscard]] ItemWithId *itemById(int id) const;

    /// Returns \c true if an item is registered under \a id.
    [[nodiscard]] bool contains(int id) const;

    /// Returns the highest id assigned so far.
    int lastId() const;

    /// Raises the last-assigned id to \a newLastId (never lowers it).
    void setLastId(int newLastId);

    /// Returns a fresh, previously unused id.
    int nextId();

    /// Pre-assigns \a newId to \a item before it is added (undo/redo restore path).
    void updateItemId(ItemWithId *item, int newId);

    /// Removes the mapping for \a id without touching the item.
    void forgetItemId(int id);

    /// Registers \a item under its current id.
    void registerItem(ItemWithId *item);

    /// Removes \a item's mapping from the registry.
    void unregisterItem(ItemWithId *item);

private:
    QHash<int, ItemWithId *> m_elementRegistry;
    int m_lastId = 0;
};
