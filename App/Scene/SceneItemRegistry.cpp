// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/SceneItemRegistry.h"

#include <algorithm>

#include <QtGlobal>

#include "App/Core/ItemWithId.h"

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
    m_lastId = (std::max)(m_lastId, newLastId);
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
    if (auto *item = m_elementRegistry.value(id, nullptr)) {
        item->setRegistry(nullptr);
    }
    m_elementRegistry.remove(id);
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
    item->setRegistry(this);
}

void SceneItemRegistry::unregisterItem(ItemWithId *item)
{
    if (!item) {
        return;
    }
    m_elementRegistry.remove(item->id());
    item->setRegistry(nullptr);
    // HC postcondition: after unregister, the registry must not still resolve this id
    Q_ASSERT(!m_elementRegistry.contains(item->id()));
}

void SceneItemRegistry::forget(ItemWithId *item)
{
    if (!item) {
        return;
    }
    // Identity-checked: a reused id may already have been claimed by a new item by the
    // time this fires (e.g. undo/redo restoring a different item under the same id).
    if (m_elementRegistry.value(item->id()) == item) {
        m_elementRegistry.remove(item->id());
    }
}

SceneItemRegistry::~SceneItemRegistry()
{
    for (auto *item : std::as_const(m_elementRegistry)) {
        item->setRegistry(nullptr);
    }
}
