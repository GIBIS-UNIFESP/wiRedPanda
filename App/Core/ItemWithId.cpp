// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/ItemWithId.h"

#include "App/Element/ElementFactory.h"
#include "App/GlobalProperties.h"

ItemWithId::ItemWithId()
{
    // Self-register with ElementFactory so every item gets a unique integer ID
    // and can be looked up globally by that ID (e.g. after undo/redo commands).
    // skipInit is set during deserialization to suppress registration until
    // the element has been fully loaded and assigned its persisted ID; this prevents
    // temporary auto-generated IDs from polluting the factory map during load.
    if (GlobalProperties::skipInit) {
        return;
    }

    ElementFactory::addItem(this);
}

ItemWithId::~ItemWithId()
{
    // Deregister on destruction to prevent dangling ID→pointer entries in the
    // factory map; safe to call even during exception unwinding.
    ElementFactory::removeItem(this);
}

int ItemWithId::id() const
{
    return m_id;
}

void ItemWithId::setId(const int id)
{
    m_id = id;
}
