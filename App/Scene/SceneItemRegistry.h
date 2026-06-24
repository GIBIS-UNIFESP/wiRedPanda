// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief SceneItemRegistry: maps stable integer IDs to the scene's identifiable items.
 */

#pragma once

#include <QMap>

#include "App/Core/ItemRegistry.h"

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
class SceneItemRegistry : public ItemRegistry
{
public:
    SceneItemRegistry() = default;

    /// On destruction, detaches every still-registered item's back-pointer so items
    /// destroyed after this registry (the QGraphicsScene base dtor runs after Scene's
    /// members) never call forget() on a dead registry.
    ~SceneItemRegistry() override;

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

    /// Removes the mapping for \a id. Under the RAII invariant a map entry always points to
    /// a live item, so this clears that item's registry back-pointer before removing it
    /// (preventing a dangling back-pointer if the item is destroyed past this registry).
    void forgetItemId(int id);

    /// \reimp ItemRegistry — drops \a item's entry if it still owns its id (called from
    /// ~ItemWithId). Identity-checked so a stale/reused id cannot evict the new owner.
    void forget(ItemWithId *item) override;

    /// Registers \a item under its current id.
    void registerItem(ItemWithId *item);

    /// Removes \a item's mapping from the registry.
    void unregisterItem(ItemWithId *item);

private:
    QMap<int, ItemWithId *> m_elementRegistry;
    int m_lastId = 0;
};
