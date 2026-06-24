// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ItemRegistry: the interface an ItemWithId uses to drop its own id mapping.
 */

#pragma once

class ItemWithId;

/**
 * \class ItemRegistry
 * \brief Minimal interface that lets an ItemWithId remove its own id↔item mapping on
 * destruction, without depending on the concrete registry/scene type.
 *
 * \details Implemented by SceneItemRegistry. ItemWithId holds a back-pointer to its
 * registry (set on registration) and calls forget() from its destructor, so a stale
 * mapping can never outlive the item regardless of the destruction path (Scene::removeItem
 * or Qt's parent-child cascade). See ItemWithId and SceneItemRegistry.
 */
class ItemRegistry
{
public:
    virtual ~ItemRegistry() = default;

    /// Drops \a item's mapping, but only if the registry still maps that id to \a item
    /// (an id can be reused by a different live item). Must NOT dereference \a item beyond
    /// reading its id() — it may be mid-destruction when this is called.
    virtual void forget(ItemWithId *item) = 0;
};
