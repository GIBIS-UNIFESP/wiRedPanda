// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/SceneItemRegistry.h"

#include <QtGlobal>

#include "App/Core/ItemWithId.h"

SceneItemRegistry::~SceneItemRegistry()
{
    // Detach every still-registered item's back-pointer so that items destroyed after this
    // registry (the QGraphicsScene base dtor runs after Scene's members) don't call forget()
    // on a dead registry.
    for (auto *item : m_elementRegistry) {
        if (item) {
            item->setRegistry(nullptr);
        }
    }
}

ItemWithId *SceneItemRegistry::itemById(const int id) const
{
    return m_elementRegistry.value(id, nullptr);
}

bool SceneItemRegistry::contains(const int id) const
{
    return m_elementRegistry.contains(id);
}

int SceneItemRegistry::lastId() const
{
    return m_lastId;
}

void SceneItemRegistry::setLastId(const int newLastId)
{
    m_lastId = qMax(m_lastId, newLastId);
}

int SceneItemRegistry::nextId()
{
    return ++m_lastId;
}

void SceneItemRegistry::updateItemId(ItemWithId *item, const int newId)
{
    // Called before addItem() to pre-assign a specific ID (undo/redo restore path).
    // The item is not yet in the registry; addItem() will preserve this positive ID.
    item->setId(newId);
    setLastId(newId);
}

void SceneItemRegistry::forgetItemId(const int id)
{
    // Clear the back-pointer of the (always-live, under the RAII invariant) mapped item so it
    // cannot later forget() into this registry after being detached from the map.
    if (auto *item = m_elementRegistry.value(id)) {
        item->setRegistry(nullptr);
    }
    m_elementRegistry.remove(id);
}

void SceneItemRegistry::forget(ItemWithId *item)
{
    // Called from ~ItemWithId. Identity-checked: only drop the entry if it still maps to THIS
    // item -- an id can be reused (morph/undo) before a stale item is destroyed. Do not
    // dereference the item beyond id() (it is mid-destruction).
    const int id = item->id();
    if (m_elementRegistry.value(id) == item) {
        m_elementRegistry.remove(id);
    }
}

void SceneItemRegistry::registerItem(ItemWithId *item)
{
    if (!item) {
        return;
    }
    // HC invariant: an id must map to exactly one live item — never to a stale pointer
    // left behind by a missed unregisterItem (the WIREDPANDA-HC family upstream condition)
    Q_ASSERT(!m_elementRegistry.contains(item->id())
          || m_elementRegistry.value(item->id()) == item);
    m_elementRegistry[item->id()] = item;
    // RAII ownership: the item drops this mapping itself on destruction (via ~ItemWithId).
    item->setRegistry(this);
}

void SceneItemRegistry::unregisterItem(ItemWithId *item)
{
    if (!item) {
        return;
    }
    m_elementRegistry.remove(item->id());
    // Clear the back-pointer so a properly-removed item (which may outlive this registry,
    // e.g. while held by an undo command) never calls forget() into a dead registry.
    item->setRegistry(nullptr);
    // HC postcondition: after unregister, the registry must not still resolve this id
    Q_ASSERT(!m_elementRegistry.contains(item->id()));
}
