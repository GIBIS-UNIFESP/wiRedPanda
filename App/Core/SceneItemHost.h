// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief SceneItemHost: narrow interface for registry-aware removal of a scene item.
 */

#pragma once

class QGraphicsItem;

/**
 * \class SceneItemHost
 * \brief Minimal interface that lets a lower layer (Nodes) remove an item from its scene
 * through the scene's id-registry-aware removal path, without depending on the concrete
 * Scene type.
 *
 * \details Implemented by Scene. Qt's non-virtual QGraphicsScene::removeItem is dispatched
 * during the parent-child destruction cascade and bypasses Scene's override, which would
 * leave a stale per-scene id-registry entry. A QNEPort that drains its connections routes
 * the deletion through this interface so the registry stays consistent, while keeping
 * Nodes free of a concrete dependency on Scene.
 */
class SceneItemHost
{
public:
    virtual ~SceneItemHost() = default;

    /// Removes \a item from the scene and its id registry.
    virtual void removeItem(QGraphicsItem *item) = 0;
};
