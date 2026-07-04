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
    /// Nulls the registry back-pointer on every item still mapped, so a later cascade-delete
    /// (e.g. Qt's ~QGraphicsScene teardown of remaining items, which runs after this
    /// registry -- a Scene member -- has already been destroyed) can't call back into a
    /// dangling SceneItemRegistry via ItemWithId's destructor.
    ~SceneItemRegistry();

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

    /// Removes the mapping for \a id, clearing the mapped item's registry back-pointer
    /// if one is still found. Safe unconditionally: the RAII self-unregistration in
    /// ItemWithId's destructor guarantees this registry never holds an entry pointing
    /// at an already-destroyed item, so whatever is found here (if anything) is live.
    void forgetItemId(int id);

    /// Registers \a item under its current id.
    void registerItem(ItemWithId *item);

    /// Removes \a item's mapping from the registry.
    void unregisterItem(ItemWithId *item);

    /// Removes \a item's mapping if it is still the one on file, identity-checked so a
    /// reused id that a new item has since claimed is never evicted. Called from
    /// ItemWithId's own destructor -- this is the structural fix for the WIREDPANDA-HC
    /// stale-registry-entry bug family, working on ANY destruction path, not just the
    /// ones that happen to route through unregisterItem()/forgetItemId() explicitly.
    void forget(ItemWithId *item);

private:
    QHash<int, ItemWithId *> m_elementRegistry;
    int m_lastId = 0;
};
